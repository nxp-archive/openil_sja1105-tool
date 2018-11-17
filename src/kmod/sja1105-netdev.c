/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2018, Sensor-Technik Wiedemann GmbH
 * Copyright (c) 2018, NXP Semiconductors
 */
#include <linux/device.h>
#include <linux/version.h>
#include <linux/etherdevice.h>
#include "sja1105.h"

/* Convert mac speed from sja1105 mac config table value to mbps */
static int sja1105_get_speed_mbps(unsigned int speed_cfg)
{
	int speed_mbps;

	switch (speed_cfg) {
	case SJA1105_SPEED_10MBPS:
		speed_mbps = 10;
		break;
	case SJA1105_SPEED_100MBPS:
		speed_mbps = 100;
		break;
	case SJA1105_SPEED_1000MBPS:
		speed_mbps = 1000;
		break;
	case SJA1105_SPEED_AUTO:
		/* Dynamic speed configuration */
		speed_mbps = 0;
		break;
	default:
		speed_mbps = -1;
	}
	return speed_mbps;
}

/* Convert mac speed from mbps to sja1105 mac config table value */
static int sja1105_get_speed_cfg(unsigned int speed_mbps)
{
	int speed_cfg;

	switch (speed_mbps) {
	case 10:
		speed_cfg = SJA1105_SPEED_10MBPS;
		break;
	case 100:
		speed_cfg = SJA1105_SPEED_100MBPS;
		break;
	case 1000:
		speed_cfg = SJA1105_SPEED_1000MBPS;
		break;
	default:
		speed_cfg = -1;
	}
	return speed_cfg;
}

/* Set link speed in the sja1105's mac configuration for a specific port */
static int sja1105_adjust_port_speed(struct sja1105_port *port, int speed_mbps)
{
	int rc;
	int speed;
	struct device *dev = &port->spi_dev->dev;
	struct sja1105_spi_private *priv = spi_get_drvdata(port->spi_dev);
	struct sja1105_mac_config_entry mac_entry;

	speed = sja1105_get_speed_cfg(speed_mbps);
	if (speed < 0) {
		rc = -EINVAL;
		dev_err(dev, "%s: Invalid speed (%iMbps)\n",
		        port->net_dev->name, speed_mbps);
		goto err_out;
	}

	/* If current speed == new speed -> do nothing */
	if (priv->static_config.mac_config[port->index].speed == speed)
		goto err_ok;

	/* Check if speed in static_config is 0 */
	if (!port->auto_speed) {
		int old_speed_cfg = priv->static_config.mac_config[port->index].speed;
		int old_speed_mbps = sja1105_get_speed_mbps(old_speed_cfg);
		rc = -EINVAL;
		dev_err(dev, "%s: Speed is fixed at %iMbps, cannot set to %iMbps\n",
		        port->net_dev->name, old_speed_mbps, speed_mbps);
		goto err_out;
	}

	/* Read, modify and write MAC config table */
	if (IS_PQRS(priv->device_id)) {
		/*
		 * We can read from the device via the MAC
		 * reconfiguration tables. In fact we do just that.
		 */
		rc = sja1105_mac_config_get(priv, &mac_entry, port->index);
		if (rc < 0) {
			dev_err(dev, "%s: MAC configuration read from device failed\n",
			        port->net_dev->name);
			goto err_out;
		}
	} else {
		/*
		 * On E/T, MAC reconfig tables are not readable.
		 * We have to *know* what the MAC looks like.
		 * We'll use the static configuration tables as a
		 * reasonable approximation.
		 */
		mac_entry = priv->static_config.mac_config[port->index];
	}

	mac_entry.speed = speed;
	rc = sja1105_mac_config_set(priv, &mac_entry, port->index);
	if (rc < 0) {
		dev_err(dev, "%s: MAC configuration write to device failed\n",
		        port->net_dev->name);
		goto err_out;
	}

	/* Write back speed configuration to static_config */
	priv->static_config.mac_config[port->index].speed = speed;

	/* Configure the CGU (PHY link modes and speeds) */
	rc = sja1105_clocking_setup_port(priv, port->index,
	                                 &priv->static_config.xmii_params[0],
	                                 &mac_entry);
	if (rc < 0) {
		dev_err(dev, "%s: Clocking setup failed\n", port->net_dev->name);
		goto err_out;
	}

err_ok:
	dev_info(dev, "%s: Adjusted MAC speed to %iMbps\n",
	         port->net_dev->name, port->phy_dev->speed);
	rc = 0;
err_out:
	return rc;
}

void sja1105_netdev_adjust_link(struct net_device *net_dev)
{
	struct sja1105_port *port = netdev_priv(net_dev);
	struct sja1105_spi_private *priv = spi_get_drvdata(port->spi_dev);
	struct phy_device *phy_dev = port->phy_dev;

	if (netif_msg_link(port))
		phy_print_status(phy_dev);

	if (!phy_dev->link) {
		netif_carrier_off(net_dev);
		return;
	}

	netif_carrier_on(net_dev);
	mutex_lock(&priv->lock);
	sja1105_adjust_port_speed(port, phy_dev->speed);
	mutex_unlock(&priv->lock);
}

static
#if KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE
void
#else
struct rtnl_link_stats64*
#endif
sja1105_get_stats(struct net_device *net_dev,
                  struct rtnl_link_stats64 *storage)
{
	struct sja1105_port *port = netdev_priv(net_dev);
	struct spi_device *spi = port->spi_dev;
	struct device *dev = &spi->dev;
	struct sja1105_spi_private *priv = spi_get_drvdata(spi);
	struct sja1105_port_status_hl1 status;
	u64 errcnt;
	int rc;

	/* If net device is not up, do nothing. *storage is
	 * zeroed by upper layer */
	if (!port->running)
		goto err_out;

	/* Rate limit status read to once every 250ms per port
	 * to limit SPI traffic load */
	rc = __ratelimit(&port->get_stats_ratelimit);
	if (!rc)
		goto out;

	/* Time to read stats from switch */
	mutex_lock(&priv->lock);
	rc = sja1105_port_status_get_hl1(priv, &status, port->index);
	if (rc) {
		mutex_unlock(&priv->lock);
		dev_err(dev, "%s: Could not read status\n", net_dev->name);
		goto out;
	}

	errcnt = status.n_n664err + status.n_vlanerr + status.n_unreleased +
	         status.n_sizerr  + status.n_crcerr  + status.n_vlnotfound +
	         status.n_polerr  + status.n_ctpolerr;

	port->stats.rx_packets       = status.n_rxfrm;
	port->stats.tx_packets       = status.n_txfrm;
	port->stats.rx_bytes         = status.n_rxbyte;
	port->stats.tx_bytes         = status.n_txbyte;
	port->stats.rx_errors        = errcnt;
	port->stats.rx_length_errors = status.n_sizerr;
	port->stats.rx_crc_errors    = status.n_crcerr;
	mutex_unlock(&priv->lock);
out:
	*storage = port->stats;
err_out:
#if KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE
	return;
#else
	return storage;
#endif
}

static int sja1105_open(struct net_device *net_dev)
{
	struct sja1105_port *port = netdev_priv(net_dev);
	struct device *dev = &port->spi_dev->dev;

	dev_dbg(dev, "%s called on port %s\n", __func__, net_dev->name);
	if (port->phy_dev)
		phy_start(port->phy_dev);

	port->running = 1;

	return 0;
}

static int sja1105_close(struct net_device *net_dev)
{
	struct sja1105_port *port = netdev_priv(net_dev);
	struct device *dev = &port->spi_dev->dev;

	dev_dbg(dev, "%s called on port %s\n", __func__, net_dev->name);
	if (port->phy_dev)
		phy_stop(port->phy_dev);

	port->running = 0;

	return 0;
}

static netdev_tx_t sja1105_xmit(struct sk_buff *skb,
                                struct net_device *net_dev)
{
	/* packet I/O not supported, drop the frame */
	dev_kfree_skb_any(skb);

	return NETDEV_TX_OK;
}

static int sja1105_ioctl(struct net_device *net_dev, struct ifreq *rq, int cmd)
{
	if (!net_dev->phydev)
		return -ENODEV;

	return phy_mii_ioctl(net_dev->phydev, rq, cmd);
}

const struct net_device_ops sja1105_netdev_ops = {
	.ndo_get_stats64      = sja1105_get_stats,
	.ndo_open             = sja1105_open,
	.ndo_stop             = sja1105_close,
	.ndo_start_xmit       = sja1105_xmit,
	.ndo_do_ioctl         = sja1105_ioctl,
};

/* sja1105-ethtool.c */
extern const struct ethtool_ops sja1105_ethtool_ops;

struct sja1105_port*
sja1105_netdev_create_port(struct sja1105_spi_private *priv,
                           const char* port_name)
{
	int rc;
	struct device *dev = &priv->spi_dev->dev;
	struct net_device *net_dev;
	struct sja1105_port *port;

	/*
	 * Allocate 1x struct net_device + 1x struct sja1105_port
	 * as its private data.
	 */
	net_dev = alloc_etherdev(sizeof(struct sja1105_port));
	if (IS_ERR_OR_NULL(net_dev)) {
		dev_err(dev, "%s: Failed to allocate net device\n",
		        port_name);
		goto err_out;
	}

	if (dev_alloc_name(net_dev, port_name) < 0) {
		dev_err(dev, "%s: Failed to set net device name\n",
		        port_name);
		goto err_free;
	}

	/*
	 * port is the struct sja1105_port appended to net_dev,
	 * reserved by the alloc_etherdev call
	 */
	port = netdev_priv(net_dev);
	port->net_dev = net_dev; /* Back pointer from priv to net_dev */
	port->spi_dev = priv->spi_dev;
	port->running = 0;
	/* Enable most messages by default */
	port->msg_enable = (NETIF_MSG_IFUP << 1) - 1;

	/* Limit get_stats to once every 250ms to limit SPI traffic */
	ratelimit_state_init(&port->get_stats_ratelimit, (HZ/4), 1);
	ratelimit_set_flags(&port->get_stats_ratelimit,
	                    RATELIMIT_MSG_ON_RELEASE);
	memset(&port->stats, 0, sizeof(port->stats));

	/*
	 * Link net_dev->dev.parent to spi->dev.
	 * The net_dev device parent is checked
	 * in of_phy_connect.
	 */
	SET_NETDEV_DEV(net_dev, dev);

	net_dev->netdev_ops  = &sja1105_netdev_ops;
	net_dev->ethtool_ops = &sja1105_ethtool_ops;
	/* Don't allow explicit Tx on switch ports from Linux */
	netif_tx_stop_all_queues(net_dev);
	rc = register_netdev(net_dev);
	if (rc < 0) {
		dev_err(dev, "%s: Cannot register net device\n", port_name);
		goto err_free;
	}

	return port;

err_free:
	free_netdev(net_dev);
err_out:
	return NULL;
}

void sja1105_netdev_remove_port(struct sja1105_port *port)
{
	struct net_device *net_dev = port->net_dev;

	unregister_netdev(net_dev);
	free_netdev(net_dev);
}
