#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/netdevice.h>
#include <linux/spi/spi.h>
#include <linux/errno.h>
#include <linux/firmware.h>
#include <linux/netdev_features.h>
#include <linux/etherdevice.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/phy.h>

MODULE_AUTHOR("Vladimir Oltean <vladimir.oltean@nxp.com>");
MODULE_DESCRIPTION("SJA1105 Driver");
MODULE_LICENSE("GPL");

struct sja1105_port {
	struct device_node *node;
	struct phy_device  *phy_dev;
	struct net_device  *net_dev;
	struct list_head    list;
	phy_interface_t     phy_mode;
	uint32_t msg_enable; /* For netif_msg_link() */
};

struct sja1105_spi_private {
	struct spi_device  *spi_dev;
	struct sja1105_port port_list_head;
};

static void sja1105_adjust_link(struct net_device *net_dev)
{
	struct sja1105_port *port = netdev_priv(net_dev);

	if (netif_msg_link(port))
		phy_print_status(port->phy_dev);

	if (port->phy_dev->link)
		netif_carrier_on(net_dev);
	else
		netif_carrier_off(net_dev);
}

static void sja1105_get_stats(struct net_device *net_dev,
                              struct rtnl_link_stats64 *storage)
{
	printk(KERN_ERR "%s: called\n", __func__);
}

static int sja1105_open(struct net_device *net_dev)
{
	struct sja1105_port *port = netdev_priv(net_dev);

	printk(KERN_ERR "%s called on port %s\n", __func__, net_dev->name);
	if (port->phy_dev)
		phy_start(port->phy_dev);
	return 0;
}

static int sja1105_close(struct net_device *net_dev)
{
	struct sja1105_port *port = netdev_priv(net_dev);

	printk(KERN_ERR "%s called on port %s\n", __func__, net_dev->name);
	if (port->phy_dev)
		phy_stop(port->phy_dev);
	return 0;
}

static netdev_tx_t sja1105_xmit(struct sk_buff *skb,
                                struct net_device *net_dev)
{
	/* packet I/O not supported, drop the frame */
	dev_kfree_skb_any(skb);

	return NETDEV_TX_OK;
}

const struct net_device_ops sja1105_netdev_ops = {
	.ndo_get_stats64      = sja1105_get_stats,
	.ndo_open             = sja1105_open,
	.ndo_stop             = sja1105_close,
	.ndo_start_xmit       = sja1105_xmit,
	.ndo_set_mac_address  = eth_mac_addr,
};

static void sja1105_cleanup(struct sja1105_spi_private *priv)
{
	struct device *dev = &priv->spi_dev->dev;
	struct list_head *pos, *q;
	struct sja1105_port *port;

	list_for_each_safe(pos, q, &(priv->port_list_head.list)) {
		port = list_entry(pos, struct sja1105_port, list);
		dev_err(dev, "Unregistering port %s\n", port->net_dev->name);
		if (of_phy_is_fixed_link(port->node))
			of_phy_deregister_fixed_link(port->node);
		else if (port->phy_dev)
			phy_disconnect(port->phy_dev);
		unregister_netdev(port->net_dev);
		free_netdev(port->net_dev);
		list_del(pos);
	}
	kfree(priv);
}

static int sja1105_parse_dt(struct sja1105_spi_private *priv)
{
	struct device       *dev = &priv->spi_dev->dev;
	struct device_node  *switch_node = dev->of_node;
	struct device_node  *phy_node;
	struct device_node  *child;
	const char          *port_name;
	struct sja1105_port *port;
	struct net_device   *net_dev;
	int rc;

	INIT_LIST_HEAD(&(priv->port_list_head.list));

	for_each_child_of_node(switch_node, child) {
		rc = of_property_read_string(child, "sja1105,port-label",
		                            &port_name);
		dev_dbg(dev, "Probing port %s\n", port_name);
		/* Allocate 1x struct net_device + 1x struct sja1105_port
		 * as its private data.
		 */
		net_dev = alloc_etherdev(sizeof(struct sja1105_port));
		if (IS_ERR_OR_NULL(net_dev)) {
			dev_err(dev, "Failed to allocate net device for port %s\n",
			        port_name);
			goto err;
		}
		/* port is the struct sja1105_port appended to net_dev,
		 * reserved by the alloc_etherdev call
		 */
		port = netdev_priv(net_dev);
		port->net_dev = net_dev; /* Back pointer from priv to net_dev */
		if (dev_alloc_name(net_dev, port_name) < 0) {
			dev_err(dev, "Failed to set net device name for port %s\n",
			        port_name);
			goto err;
		}

		net_dev->netdev_ops = &sja1105_netdev_ops;
		/* Link net_dev->dev.parent to spi->dev.
		 * The net_dev device parent is checked
		 * in of_phy_connect.
		 */
		SET_NETDEV_DEV(net_dev, dev);
		/* Don't allow explicit Tx on switch ports from Linux */
		netif_tx_stop_all_queues(net_dev);
		rc = register_netdev(net_dev);
		if (rc < 0) {
			dev_err(dev, "Cannot register net device for port %s\n", port_name);
			goto err;
		}

		/* Enable most messages by default */
		port->msg_enable = (NETIF_MSG_IFUP << 1) - 1;
		port->node = child;
		port->phy_mode = of_get_phy_mode(port->node);
		if (port->phy_mode < 0) {
			dev_err(dev, "Failed to read port %s phy-mode or phy-interface-type property\n",
			        port_name);
			goto err;
		}
		phy_node = of_parse_phandle(port->node, "phy-handle", 0);
		if (IS_ERR_OR_NULL(phy_node)) {
			if (!of_phy_is_fixed_link(port->node)) {
				dev_err(dev, "Port %s: phy-handle or fixed-link properties missing!\n", port_name);
				goto err;
			}
			/* phy-handle is missing, but fixed-link isn't.
			 * So it's a fixed link.
			 */
			if (of_phy_register_fixed_link(port->node) != 0) {
				dev_err(dev, "Failed to register port %s as fixed link!\n", port_name);
				goto err;
			}
		} else {
			port->phy_dev = of_phy_connect(net_dev, phy_node,
			                               sja1105_adjust_link,
			                               0 /* flags */, port->phy_mode);
			if (IS_ERR_OR_NULL(port->phy_dev)) {
				dev_err(dev, "Could not attach to PHY on port %s\n", port_name);
				goto err;
			}
		}
		/* Decrement refcount (of_parse_phandle) */
		of_node_put(phy_node);
		list_add_tail(&(port->list), &(priv->port_list_head.list));
	}
	return 0;
err:
	return -ENODEV;
}

static int sja1105_probe(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	struct sja1105_spi_private *priv;
	int rc;

	if (!dev->of_node) {
		dev_err(dev, "No DTS bindings for SJA1105 driver\n");
		return -EINVAL;
	}

	priv = kzalloc(sizeof(struct sja1105_spi_private), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	/* Populate our driver private structure (priv) based on
	 * the device tree node that was probed (spi) */
	priv->spi_dev = spi;
	spi_set_drvdata(spi, priv);

	rc = sja1105_parse_dt(priv);
	if (rc < 0) {
		sja1105_cleanup(priv);
		return rc;
	}
	return 0;
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

