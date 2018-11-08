// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for Automotive switch SJA1105.
 *
 * Copyright (C) 2018 ...
 */

#include <linux/device.h>
#include "sja1105.h"
#include "common.h"


/*
 * Parse the string to find a specific port.
 * The port can be specified by ist port number (the chip's port number) or
 * by the name of the net device that is assigned to this port/PHY.
 * Return NULL if port is not found ind priv->port_list_head.
 */
static struct sja1105_port *sja1105_parse_port(struct sja1105_spi_private *priv,
                                               const char *buf)
{
	struct sja1105_port *port = NULL;
	struct list_head *pos, *q;
	int port_index;
	int found_port = 0;

	list_for_each_safe(pos, q, &(priv->port_list_head.list)) {
		port = list_entry(pos, struct sja1105_port, list);
		if (sysfs_streq(buf, port->net_dev->name)) {
			/* net device name match */
			found_port = 1;
		}
		else if (!kstrtoint(buf, 0, &port_index) &&
			 (port_index == port->index)) {
			/* port number match */
			found_port = 1;
		}

		if (found_port)
			break;
	}

	return port;
}


/*
 * Parse the string written to sysfs attribute reg_access.
 * Format: "address" => read operation, value read on sysfs attribure read
 *         "address value" => write operation
 * Return: 1: got an address
 *         2: got address and value
 *        -1: error
 */
static  int sja1105_reg_access_parse_input(const char* buf,
                                           u64 *addr, u64 *value)
{
	int rc;
	int count = -1;
	char *str;
	char *par1, *par2, *end;

	str = kstrdup(buf, GFP_KERNEL);
	if (!str)
		return -1;

	par1 = strim(str);
	end  = str + strlen(str);
	par2 = strchr(par1, ' ');
	if (par2)
		*par2++ = '\0'; /* make sure par1 is \0 terminated
		                 * else kstrtou64 fails */
	rc = kstrtou64(par1, 0, addr);

	if (rc)
		goto out_free;
	count = 1;

	if (par2 && (par2 < end)) {
		par2 = strim(par2);
		rc = kstrtou64(par2, 0, value);
		if (rc)
			goto out_free;
		count = 2;
	}

out_free:
	kfree(str);
	return count;
}


static ssize_t sja1105_sysfs_wr(struct device *dev,
                                struct device_attribute *attr,
                                const char *buf, size_t count);
static ssize_t sja1105_sysfs_rd(struct device *dev,
                                struct device_attribute *attr, char *buf);

static DEVICE_ATTR(device_id,      S_IRUGO, sja1105_sysfs_rd, NULL);
static DEVICE_ATTR(general_status, S_IRUGO, sja1105_sysfs_rd, NULL);
static DEVICE_ATTR(port_status,    S_IRUGO | S_IWUSR, sja1105_sysfs_rd,
                                                      sja1105_sysfs_wr);
static DEVICE_ATTR(port_status_clear, S_IWUSR, NULL, sja1105_sysfs_wr);
static DEVICE_ATTR(port_mapping,   S_IRUGO, sja1105_sysfs_rd, NULL);
static DEVICE_ATTR(reg_access,     S_IRUGO | S_IWUSR, sja1105_sysfs_rd, NULL);
static DEVICE_ATTR(config_upload, S_IWUSR, NULL, sja1105_sysfs_wr);


static ssize_t sja1105_sysfs_wr(struct device *dev,
                                struct device_attribute *attr,
                                const char *buf, size_t count)
{
	int rc = -EOPNOTSUPP;
	struct spi_device *spi = to_spi_device(dev);
	struct sja1105_spi_private *priv = spi_get_drvdata(spi);
	struct sja1105_port *port = NULL;
	struct list_head *pos, *q;
	u64 speed, addr, value;
	int port_no;

	if (attr == &dev_attr_config_upload) {
		if (buf[0] != '1')
			goto out_error;

		/* Load firmware from user space (rootfs) and unpack it */
		rc = sja1105_load_firmware(priv);
		if (rc)
			goto out_error;

		/* Upload static configuration */
		mutex_lock(&priv->lock);
		rc = sja1105_static_config_flush(&priv->spi_setup,
		                                  &priv->static_config);
		mutex_unlock(&priv->lock);
		if (rc < 0)
			goto out_error;
		dev_info(dev, "Uploaded static configuration to device\n");

		/* Update auto-speed property according to static config */
		list_for_each_safe(pos, q, &(priv->port_list_head.list)) {
			port = list_entry(pos, struct sja1105_port, list);
			speed = priv->static_config.mac_config[port->index].speed;
			port->auto_speed = (speed == 0) ? 1 : 0;
		}
		/* TODO: Resetting / Reinit of PHYs required ? */
		rc = count;
	}
	else if (attr == &dev_attr_port_status) {
		rc = -ENOENT;
		port = sja1105_parse_port(priv, buf);
		if (port) {
			priv->selected_port = port;
			rc = count;
		}
	}
	else if (attr == &dev_attr_port_status_clear) {
		if (sysfs_streq(buf, "all")) {
			port_no = -1;
			rc = 0;
		}
		else {
			rc = -ENOENT;
			port = sja1105_parse_port(priv, buf);
			if (port) {
				port_no = port->index;
				rc = 0;
			}
		}

		if (rc == 0) {
			rc = sja1105_port_status_clear(&priv->spi_setup,
							port_no);
			rc = (rc) ? -EIO : count;
		}
	}
	else if (attr == &dev_attr_reg_access) {
		rc = sja1105_reg_access_parse_input(buf, &addr, &value);
		if (rc == 1) {
			/* read operation => remember address and read
			 * on sysfs read operation */
			dev_info(dev, "Prepare reading of register 0x%08X\n",
			         (u32)addr);
			priv->reg_addr = addr;
			rc = count;
		}
		else if (rc == 2) {
			/* write operation */
			rc = sja1105_spi_send_int(&priv->spi_setup, SPI_WRITE,
			                           addr, &value, 4);
			dev_info(dev, "Writing value 0x%08X to register 0x%08X, rc=%i\n",
			         (u32)value, (u32)addr, rc);
			rc = (rc) ? rc : count;
		}
		else {
			dev_err(dev, "Invalid parameter written to \"reg_access\"\n");
			rc = -EINVAL;
		}
	}

out_error:
	return rc;
}

static ssize_t sja1105_sysfs_rd(struct device *dev,
                                struct device_attribute *attr,
                                char *buf)
{
	int  rc = -EOPNOTSUPP;
	struct spi_device *spi = to_spi_device(dev);
	struct sja1105_spi_private *priv = spi_get_drvdata(spi);
	struct sja1105_general_status gen_status;
	struct sja1105_port_status port_status;
	struct list_head *pos, *q;
	struct sja1105_port *port = NULL;
	int port_index;
	u64 value;

	if (attr == &dev_attr_device_id) {
		const char *name;

		name = sja1105_device_id_string_get(priv->spi_setup.device_id,
		                                    priv->spi_setup.part_nr);
		rc = snprintf(buf, PAGE_SIZE, "0x%08X 0x%08X \"%s\"\n",
		               (u32)priv->spi_setup.device_id,
		               (u32)(u32)priv->spi_setup.part_nr,
		               name);
	}
	else if (attr == &dev_attr_general_status) {
		mutex_lock(&priv->lock);
		rc = sja1105_general_status_get(&priv->spi_setup, &gen_status);
		mutex_unlock(&priv->lock);
		if (rc) {
			rc = -EIO;
			goto err_out;
		}
		sja1105_general_status_show(&gen_status, buf, PAGE_SIZE,
		                            priv->spi_setup.device_id);
		rc = strlen(buf);
	}
	else if (attr == &dev_attr_port_status) {
		if (!priv->selected_port) {
			rc = -ENOENT;
			goto err_out;
		}
		port_index = priv->selected_port->index;
		mutex_lock(&priv->lock);
		rc = sja1105_port_status_get(&priv->spi_setup, &port_status,
		                              port_index);
		mutex_unlock(&priv->lock);
		if (rc) {
			rc = -EIO;
			goto err_out;
		}
		sja1105_port_status_show(&port_status, port_index, buf,
		                         PAGE_SIZE, priv->spi_setup.device_id);
		rc = strlen(buf);
	}
	else if (attr == &dev_attr_port_mapping) {
		list_for_each_safe(pos, q, &(priv->port_list_head.list)) {
			port = list_entry(pos, struct sja1105_port, list);
			formatted_append(buf, PAGE_SIZE, "%s\n", "%s %i",
			                 port->net_dev->name, port->index);
		}
		rc = strlen(buf);
	}
	else if (attr == &dev_attr_reg_access) {
		/* read a register value and return in format "address value" */
		rc = sja1105_spi_send_int(&priv->spi_setup, SPI_READ,
						priv->reg_addr, &value, 4);
		dev_info(dev, "Reading register 0x%08X 0x%08X, rc=%i\n",
			 (u32)priv->reg_addr, (u32)value, rc);
		if (rc)
			goto err_out;

		rc = snprintf(buf, PAGE_SIZE, "0x%08X 0x%08X\n",
		               (u32)priv->reg_addr, (u32)value);
	}

err_out:
	return rc;
}

int sja1105_sysfs_init(struct sja1105_spi_private *priv)
{
	int rc;
	struct device *dev = &priv->spi_dev->dev;

	rc = device_create_file(dev, &dev_attr_device_id);
	rc |= device_create_file(dev, &dev_attr_general_status);
	rc |= device_create_file(dev, &dev_attr_port_status);
	rc |= device_create_file(dev, &dev_attr_port_status_clear);
	rc |= device_create_file(dev, &dev_attr_port_mapping);
	rc |= device_create_file(dev, &dev_attr_reg_access);
	rc |= device_create_file(dev, &dev_attr_config_upload);

	return (rc) ? -1 : 0;
}

void sja1105_sysfs_remove(struct sja1105_spi_private *priv)
{
	struct device *dev = &priv->spi_dev->dev;

	device_remove_file(dev, &dev_attr_device_id);
	device_remove_file(dev, &dev_attr_general_status);
	device_remove_file(dev, &dev_attr_port_status);
	device_remove_file(dev, &dev_attr_port_status_clear);
	device_remove_file(dev, &dev_attr_port_mapping);
	device_remove_file(dev, &dev_attr_reg_access);
	device_remove_file(dev, &dev_attr_config_upload);
}
