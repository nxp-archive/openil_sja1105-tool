// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for Automotive switch SJA1105.
 *
 * Copyright (C) 2018 ...
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/spi/spi.h>
#include <linux/errno.h>
#include <linux/firmware.h>
#include <linux/netdev_features.h>
#include <linux/etherdevice.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/phy.h>
#include <linux/gpio/consumer.h>
#include "sja1105.h"
#include <lib/include/static-config.h>
#include <lib/include/staging-area.h>
#include <lib/include/dynamic-config.h>
#include <lib/include/spi.h>


MODULE_AUTHOR("Vladimir Oltean <vladimir.oltean@nxp.com>");
MODULE_AUTHOR("Georg Waibel <georg.waibel@sensor-technik.de>");
MODULE_DESCRIPTION("SJA1105 Driver");
MODULE_LICENSE("GPL");


/* Common function used for issuing a reset pulse
 * to either the switch (priv->reset_gpio) or to an individual
 * external PHY (port->reset_gpio).
 */
static int sja1105_hw_reset_chip(struct gpio_desc *gpio,
                                 unsigned int pulse_len,
                                 unsigned int startup_delay)
{
	if (IS_ERR(gpio))
		return PTR_ERR(gpio);

	gpiod_set_value_cansleep(gpio, 1);
	/* Wait for minimum reset pulse length */
	msleep(pulse_len);
	gpiod_set_value_cansleep(gpio, 0);
	/* Wait until chip is ready after reset */
	msleep(startup_delay);

	return 0;
}

/*
 * Load the SJA1105 static configuration from rootfs and unpack it
 * into privat data.
 */
int sja1105_load_firmware(struct sja1105_spi_private *priv)
{
	int rc;
	void* buf;
	struct device *dev = &priv->spi_dev->dev;
	const struct firmware *fw_entry;
	const char fw_name[] = FIRMWARE_FILENAME;

	/* Load firmware from user space (rootfs) and unpack it */
	rc = request_firmware(&fw_entry, fw_name, dev);
	if (rc) {
		dev_err(dev, "Error requesting firmware %s\n", fw_name);
		goto err_out;
	}

	/* sja1105_static_config_unpack() wants a non-const data pointer but
	 * fw_entry is const -> copy data to a local buffer */
	buf = kmalloc(fw_entry->size, GFP_KERNEL);
	if (!buf) {
		rc = -ENOMEM;
		goto err_fw;
	}
	memcpy(buf, fw_entry->data, fw_entry->size);

	rc = sja1105_static_config_unpack(buf, fw_entry->size,
	                                   &priv->static_config);
	if (rc < 0) {
		dev_err(dev, "Invalid static configuration data\n");
		goto err_mem;
	}

err_mem:
	kfree(buf);
err_fw:
	release_firmware(fw_entry);
err_out:
	return rc;
}


static void sja1105_cleanup(struct sja1105_spi_private *priv)
{
	struct list_head *pos, *q;
	struct sja1105_port *port;

	sja1105_sysfs_remove(priv);

	list_for_each_safe(pos, q, &(priv->port_list_head.list)) {
		port = list_entry(pos, struct sja1105_port, list);
		if (port) {
			dev_dbg(&priv->spi_dev->dev,
			        "Cleaning up port %s\n",
			        port->net_dev->name);
			if (!IS_ERR(port->reset_gpio))
				gpiod_put(port->reset_gpio);
			if (of_phy_is_fixed_link(port->node))
				of_phy_deregister_fixed_link(port->node);
			else if (port->phy_dev && port->phy_dev->attached_dev)
				phy_disconnect(port->phy_dev);
			sja1105_netdev_remove_port(port);
			list_del(pos);
		}
	}

	if (!IS_ERR(priv->reset_gpio))
		gpiod_put(priv->reset_gpio);

	kfree(priv);
}


static int sja1105_connect_phy(struct sja1105_port *port,
                               const char* port_name)
{
	struct device *dev = &port->spi_dev->dev;
	struct net_device *net_dev = port->net_dev;


	/* Configure reset pin and bring up PHY */
	port->reset_gpio =
#if KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE
		devm_fwnode_get_gpiod_from_child(dev, "phy-reset",
		                                &port->node->fwnode,
		                                GPIOD_OUT_HIGH,
		                                port->node->name);
#elif KERNEL_VERSION(4, 9, 0) <= LINUX_VERSION_CODE
		devm_get_gpiod_from_child(dev, "phy-reset",
		                          &port->node->fwnode);
#else
		/* TODO there are kernel versions where we are still
		 * incompatible with GPIOD API */
		devm_get_gpiod_from_child(dev, "phy-reset", NULL,
		                          &port->node->fwnode);
#endif
	if (IS_ERR(port->reset_gpio)) {
		dev_info(dev, "%s: reset-gpios (PHY reset pin) not defined, ignoring...\n",
		         port_name);
	}
	else {
		port->reset_duration = 1;  /* preset with default value */
		port->reset_delay = 10;    /* preset with default value */
		of_property_read_u32(port->node, "phy-reset-duration",
		                     &port->reset_duration);
		of_property_read_u32(port->node, "phy-reset-delay",
		                     &port->reset_delay);
		sja1105_hw_reset_chip(port->reset_gpio, port->reset_duration,
		                      port->reset_delay);
	}

	port->phy_node = of_parse_phandle(port->node, "phy-handle", 0);
	if (IS_ERR_OR_NULL(port->phy_node)) {
		if (!of_phy_is_fixed_link(port->node)) {
			dev_err(dev, "%s: phy-handle or fixed-link properties missing!\n",
			        port_name);
			goto err_out;
		}
		/* phy-handle is missing, but fixed-link isn't.
		 * So it's a fixed link.
		 */
		if (of_phy_register_fixed_link(port->node) != 0) {
			dev_err(dev, "Failed to register %s as fixed link!\n",
			        port_name);
			goto err_out;
		}
	}
	else {
		port->phy_dev = of_phy_connect(net_dev, port->phy_node,
		                               sja1105_netdev_adjust_link,
		                               0 /* flags */, port->phy_mode);
		/* Regardless of error status, decrement refcount now
		 * (of_parse_phandle)
		 */
		of_node_put(port->phy_node);
		if (IS_ERR_OR_NULL(port->phy_dev)) {
			dev_err(dev, "%s: Could not connect to PHY\n",
			        port_name);
			goto err_out;
		}
	}

	return 0;

err_out:
	return -ENODEV;
}

/*
 * It is assumed that the static configuration is already loaded
 * into the driver private data struct.
 */
static int sja1105_parse_dt(struct sja1105_spi_private *priv)
{
	struct device       *dev = &priv->spi_dev->dev;
	struct device_node  *switch_node = dev->of_node;
	struct device_node  *child;
	const char          *port_name;
	struct sja1105_port *port;
	phy_interface_t     phy_mode;
	u32 port_index;
	u64 speed;
	int rc;

	for_each_child_of_node(switch_node, child) {
		rc = of_property_read_string(child, "sja1105,port-label",
		                            &port_name);
		if (rc) {
			dev_err(dev, "Invalid port node in device tree\n");
			goto err_out;
		}

		/* Get switch port number from DT */
		rc = of_property_read_u32(child, "reg", &port_index);
		if (rc) {
			dev_err(dev, "Port number not defined in device tree (property \"reg\")\n");
			goto err_out;
		}

		/* Get PHY mode from DT */
		phy_mode = of_get_phy_mode(child);
		if (phy_mode < 0) {
			dev_err(dev, "Failed to read %s phy-mode or phy-interface-type property\n",
			        port_name);
			goto err_out;
		}

		/* Create a net_device for the port */
		port = sja1105_netdev_create_port(priv, port_name);
		if (IS_ERR(port))
			goto err_out;

		port->index = port_index;
		speed = priv->static_config.mac_config[port->index].speed;
		port->auto_speed = (speed == 0) ? 1 : 0;
		port->node = child;
		port->phy_mode = phy_mode;

		/* Connect PHY */
		rc = sja1105_connect_phy(port, port_name);
		if (rc) {
			sja1105_netdev_remove_port(port);
			goto err_out;
		}
		if (IS_ERR_OR_NULL(port->phy_node))
			dev_info(dev, "Probed %s as fixed link\n", port_name);
		else
			dev_info(dev, "Probed %s: %s\n",
			         port_name, port->phy_dev->drv->name);
		list_add_tail(&(port->list), &(priv->port_list_head.list));
	}

	return 0;

err_out:
	return -ENODEV;
}

static int sja1105_probe(struct spi_device *spi)
{
	int rc;
	struct device *dev = &spi->dev;
	struct sja1105_spi_private *priv;
	const char *chip_name;

	if (!dev->of_node) {
		dev_err(dev, "No DTS bindings for SJA1105 driver\n");
		return -EINVAL;
	}

	priv = kzalloc(sizeof(struct sja1105_spi_private), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	INIT_LIST_HEAD(&(priv->port_list_head.list));
	mutex_init(&priv->lock);

	/* Populate our driver private structure (priv) based on
	 * the device tree node that was probed (spi) */
	priv->spi_dev = spi;
	spi_set_drvdata(spi, priv);

	/* Configure the SPI bus */
	spi->mode = SPI_CPHA;
	spi->bits_per_word = 8;
	rc = spi_setup(spi);
	if (rc) {
		dev_err(dev, "Could not init SPI\n");
		goto out_error;
	}

	/* Configure reset pin and bring up switch */
	priv->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(priv->reset_gpio))
		dev_info(dev, "reset-gpios (switch reset pin) not defined, ignoring...\n");
	else
		sja1105_hw_reset_chip(priv->reset_gpio, 1, 1);

	/* Probe device */
	rc = sja1105_device_id_get(&priv->spi_setup,
	                            &priv->spi_setup.device_id,
	                            &priv->spi_setup.part_nr);
	if (rc < 0)
		goto out_error;
	chip_name = sja1105_device_id_string_get(priv->spi_setup.device_id,
	                                         priv->spi_setup.part_nr);
	dev_info(dev, "Probed switch chip: %s\n", chip_name);

	/* Load firmware from user space (rootfs) and unpack it */
	rc = sja1105_load_firmware(priv);
	if (rc)
		goto out_error;

	/* Upload static configuration */
	rc = sja1105_static_config_flush(&priv->spi_setup,
	                                  &priv->static_config);
	if (rc < 0)
		goto out_error;
	dev_info(dev, "Uploaded static configuration to device\n");

	rc = sja1105_sysfs_init(priv);
	if (rc) {
		dev_err(dev, "Failed to create sysfs entries\n");
		goto out_error;
	}

	/* Parse device tree and bring up PHYs */
	mutex_lock(&priv->lock);
	rc = sja1105_parse_dt(priv);
	mutex_unlock(&priv->lock);
	if (rc < 0)
		goto out_error;

	return 0;

out_error:
	sja1105_cleanup(priv);
	return rc;
}

static int sja1105_remove(struct spi_device *spi)
{
	struct sja1105_spi_private *priv = spi_get_drvdata(spi);

	sja1105_cleanup(priv);
	return 0;
}

static const struct of_device_id sja1105_dt_ids[] = {
	{ .compatible = "nxp,sja1105e" },
	{ .compatible = "nxp,sja1105t" },
	{ .compatible = "nxp,sja1105p" },
	{ .compatible = "nxp,sja1105q" },
	{ .compatible = "nxp,sja1105r" },
	{ .compatible = "nxp,sja1105s" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, sja1105_dt_ids);

static struct spi_driver sja1105_driver = {
	.driver = {
		.name  = "sja1105",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(sja1105_dt_ids),
	},
	.probe    = sja1105_probe,
	.remove   = sja1105_remove,
};

static int __init sja1105_init(void)
{
	int rc;

	rc = spi_register_driver(&sja1105_driver);
	if (rc < 0)
		printk(KERN_ERR "NXP SJA1105 driver failed to register\n");

	return rc;
}
module_init(sja1105_init);

static void __exit sja1105_exit(void)
{
	spi_unregister_driver(&sja1105_driver);
}
module_exit(sja1105_exit);

