/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2018, Sensor-Technik Wiedemann GmbH
 * Copyright (c) 2018, NXP Semiconductors
 */
#include <linux/device.h>
#include <linux/version.h>
#include <linux/etherdevice.h>
#include <linux/of_mdio.h>
#include "sja1105.h"

/* Convert mac speed from mbps to sja1105 mac config table value */
int sja1105_get_speed_cfg(unsigned int speed_mbps)
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

/* This function must be called after sja1105_parse_dt.
 * If called before sja1105_connect_phy, it will use the port->phy_node
 * reference. When called at this time, this function will yield a correct
 * answer only for fixed-link ports.
 * If called after sja1105_connect_phy, the phy_node pointer is NULL'ed
 * and the information is retrieved from the started phylib state machine
 * (valid for both physical PHYs and fixed-links).
 */
int sja1105_port_get_speed(struct sja1105_port *port)
{
	struct phy_device *phy_dev;
	int phy_speed;

	if (port->phy_node)
		/* Get a reference to the phy_device created with
		 * of_phy_register_fixed_link, but which is normally
		 * given to us only later, in of_phy_connect.  Since we
		 * must perform this before static_config_flush, and
		 * therefore also before of_phy_connect, we must make
		 * use of the reference to phy_node that we still have
		 * from of_parse_phandle in sja1105_parse_dt.
		 */
		phy_dev = of_phy_find_device(port->phy_node);
	else
		phy_dev = port->phy_dev;

	phy_speed = phy_dev->speed;

	if (port->phy_node)
		/* Drop reference count from of_phy_find_device */
		put_device(&phy_dev->mdio.dev);

	return phy_speed;
}

/*
 * Set link speed and enable/disable rx/tx in the sja1105's mac configuration
 * for a specific port.
 * speed_mbps=0 leaves the speed config unchanged
 * enable=0 disables rx and tx for this port (mac_config.ingress/egress = 0)
 * enable=1 sets the values from the static configuration for mac config table
 * entries ingress and egress (if port is disable in static configuration, it
 * remains disabled).
 */
static int sja1105_adjust_port_config(struct sja1105_port *port,
                                      int speed_mbps, int enable)
{
	struct device *dev = &port->spi_dev->dev;
	struct sja1105_spi_private *priv = spi_get_drvdata(port->spi_dev);
	struct sja1105_mac_config_entry mac_entry;
	int speed = SJA1105_SPEED_AUTO;
	int xmii_mode;
	int rc;
	char string[128] = "";
	struct sja1105_mac_config_entry *mac_sconfig =
	                        &priv->static_config.mac_config[port->index];

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
		mac_entry = *mac_sconfig;
	}

	if (speed_mbps > 0) {
		speed = sja1105_get_speed_cfg(speed_mbps);
		if (speed < 0) {
			rc = -EINVAL;
			dev_err(dev, "%s: Invalid speed (%iMbps)\n",
			        port->net_dev->name, speed_mbps);
			goto err_out;
		}
		mac_entry.speed = speed;
	}
	mac_entry.ingress = (enable) ? mac_sconfig->ingress : 0;
	mac_entry.egress = (enable) ? mac_sconfig->egress : 0;
	rc = sja1105_mac_config_set(priv, &mac_entry, port->index);
	if (rc < 0) {
		dev_err(dev, "%s: MAC configuration write to device failed\n",
		        port->net_dev->name);
		goto err_out;
	}

	/*
	 * Reconfigure the CGU only for RGMII and SGMII interfaces.
	 * xmii_mode and mac_phy setting cannot change at this point, only
	 * speed does. For MII and RMII no change of the clock setup is
	 * required. Actually, changing the clock setup does interrupt the
	 * clock signal for a certain time which causes trouble for all PHYs
	 * relying on this signal.
	 */
	xmii_mode = priv->static_config.xmii_params[0].xmii_mode[port->index];
	if ((xmii_mode == XMII_MODE_RGMII) || (xmii_mode == XMII_MODE_SGMII)) {
		rc = sja1105_clocking_setup_port(port);
		if (rc < 0) {
			dev_err(dev, "%s: Clocking setup failed\n",
			        port->net_dev->name);
			goto err_out;
		}
	}

	if (enable)
		snprintf(string, sizeof(string),
		         "Adjusted MAC speed to %iMbps, ",
		         port->phy_dev->speed);
	dev_info(dev, "%s: %sRX %s, TX %s\n",
	         port->net_dev->name, string,
	         (mac_entry.ingress) ? "enabled" : "disabled",
	         (mac_entry.egress) ? "enabled" : "disabled");
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
		mutex_lock(&priv->lock);
		sja1105_adjust_port_config(port, 0, 0);
		mutex_unlock(&priv->lock);
		netif_carrier_off(net_dev);
		return;
	}

	netif_carrier_on(net_dev);
	mutex_lock(&priv->lock);
	sja1105_adjust_port_config(port, phy_dev->speed, 1);
	mutex_unlock(&priv->lock);
}

static
#if KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE
void
#else
struct rtnl_link_stats64*
#endif
sja1105_get_stats(struct net_device *net_dev, struct rtnl_link_stats64 *storage)
{
	struct sja1105_port *port = netdev_priv(net_dev);

	*storage = port->stats;

#if KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE
	return;
#else
	return storage;
#endif
}

static int sja1105_skb_ring_put(struct sja1105_skb_ring *ring,
                                struct sk_buff *skb)
{
	int index;

	if (ring->count == SJA1105_SKB_RING_SIZE)
		return -1;

	index = ring->pi;
	ring->skb[index] = skb;
	ring->pi = (index + 1) % SJA1105_SKB_RING_SIZE;
	ring->count++;
	return index;
}

static int sja1105_skb_ring_get(struct sja1105_skb_ring *ring,
                                struct sk_buff **skb)
{
	int index;

	if (ring->count == 0)
		return -1;

	index = ring->ci;
	*skb = ring->skb[index];
	ring->ci = (index + 1) % SJA1105_SKB_RING_SIZE;
	ring->count--;
	return index;
}

#define macaddr_to_u64(mac) \
	(((u64) (mac[0]) << 40) | \
	 ((u64) (mac[1]) << 32) | \
	 ((u64) (mac[2]) << 24) | \
	 ((u64) (mac[3]) << 16) | \
	 ((u64) (mac[4]) <<  8) | \
	 ((u64) (mac[5]) <<  0))

/* Deferred work because setting up the management route cannot be done
 * from atomit context (SPI transfer takes a sleepable lock on the bus)
 */
static void sja1105_xmit_work_handler(struct work_struct *work)
{
	struct sja1105_port *port = container_of(work, struct sja1105_port,
	                                         xmit_work);
	struct sja1105_spi_private *priv = spi_get_drvdata(port->spi_dev);
	struct net_device *host = priv->host_net_dev;
	struct sja1105_general_params_entry *gp;
	struct sk_buff *skb;
	int i, rc;

	gp = &priv->static_config.general_params[0];

	while ((i = sja1105_skb_ring_get(&port->xmit_ring, &skb)) >= 0) {

		struct sja1105_mgmt_entry mgmt_route;
		int slot = port->mgmt_slot;
		struct ethhdr *hdr;
		int timeout = 500;
		int skb_len;
		u64 dmac;

		skb_len = skb->len;
		hdr = eth_hdr(skb);
		dmac = macaddr_to_u64(hdr->h_dest);

		if (((dmac & gp->mac_flt0) != gp->mac_fltres0) &&
		    ((dmac & gp->mac_flt1) != gp->mac_fltres1)) {
			/* We must free the skb because we haven't sent
			 * it to the host port yet
			 */
			dev_kfree_skb_any(skb);
			port->net_dev->stats.tx_dropped++;
			continue;
		}
		dev_dbg(&port->net_dev->dev, "%s i=%d\n", __func__, i);
		dev_dbg(&port->net_dev->dev, "mac src %pM dst %pM\n",
		        hdr->h_source, hdr->h_dest);

		memset(&mgmt_route, 0, sizeof(struct sja1105_mgmt_entry));
		mgmt_route.macaddr = macaddr_to_u64(hdr->h_dest);
		mgmt_route.destports = (1 << port->index);
		mgmt_route.ts_regid = 0;
		mgmt_route.egr_ts = 1;

		rc = sja1105_mgmt_route_set(priv, &mgmt_route, slot);
		if (rc < 0) {
			/* We must free the skb because we haven't sent
			 * it to the host port yet
			 */
			dev_kfree_skb_any(skb);
			port->net_dev->stats.tx_dropped++;
			continue;
		}

		/* Transfer skb to the host port.
		 * Only overwrite the source MAC address if the network stack
		 * populated that with the switch netdev MAC address
		 * (00:00:00:00:00:00) since the switch doesn't have a macaddr.
		 * If the skb was generated using a raw socket and the source
		 * MAC addr was populated by the application, that is kept in
		 * place instead of replacing it.
		 */
		if (macaddr_to_u64(hdr->h_source) == 0)
			memcpy(hdr->h_source, host->dev_addr, host->addr_len);

		skb->dev = host;
		dev_queue_xmit(skb);

		/* Wait until the switch has processed the frame */
		do {
			rc = sja1105_mgmt_route_get(priv, &mgmt_route, slot);
			if (rc < 0) {
				port->stats.tx_errors++;
				continue;
			}

			/* UM10944: The ENFPORT flag of the respective entry is
			 * cleared when a match is found. The host can use this
			 * flag as an acknowledgement.
			 */
			usleep_range(1000, 2000);
		} while (mgmt_route.enfport && --timeout);

		if (!timeout) {
			dev_err(&port->net_dev->dev, "xmit timed out\n");
			port->stats.tx_errors++;
			continue;
		}

		port->stats.tx_packets++;
		port->stats.tx_bytes += skb_len;
	}
	if (netif_queue_stopped(port->net_dev))
		netif_wake_queue(port->net_dev);
}

static netdev_tx_t sja1105_xmit(struct sk_buff *skb, struct net_device *net_dev)
{
	struct sja1105_port *port = netdev_priv(net_dev);
	struct sja1105_spi_private *priv = spi_get_drvdata(port->spi_dev);
	struct net_device *host = priv->host_net_dev;

	if (!host || !netif_running(host))
		goto err_out;

	if (sja1105_skb_ring_put(&port->xmit_ring, skb) < 0)
		goto err_out;

	if (port->xmit_ring.count == SJA1105_SKB_RING_SIZE)
		netif_stop_queue(net_dev);

	schedule_work(&port->xmit_work);
	goto out;

err_out:
	port->stats.tx_fifo_errors++;
	dev_kfree_skb_any(skb);
out:
	return NETDEV_TX_OK;
}

static int sja1105_open(struct net_device *net_dev)
{
	struct sja1105_port *port = netdev_priv(net_dev);
	struct sja1105_spi_private *priv = spi_get_drvdata(port->spi_dev);

	/* Don't allow setting of management routes on the
	 * host-facing switch port */
	if (port == priv->switch_host_port)
		netif_tx_stop_all_queues(net_dev);
	else
		INIT_WORK(&port->xmit_work, sja1105_xmit_work_handler);

	if (port->phy_dev)
		phy_start(port->phy_dev);

	return 0;
}

static int sja1105_close(struct net_device *net_dev)
{
	struct sja1105_port *port = netdev_priv(net_dev);
	struct sk_buff *skb;

	if (port->phy_dev)
		phy_stop(port->phy_dev);

	cancel_work_sync(&port->xmit_work);
	while (sja1105_skb_ring_get(&port->xmit_ring, &skb) >= 0) {
		dev_kfree_skb_any(skb);
	}
	return 0;
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
	memset(&port->stats, 0, sizeof(port->stats));

	/*
	 * Link net_dev->dev.parent to spi->dev.
	 * The net_dev device parent is checked
	 * in of_phy_connect.
	 */
	SET_NETDEV_DEV(net_dev, dev);

	net_dev->netdev_ops  = &sja1105_netdev_ops;
	net_dev->ethtool_ops = &sja1105_ethtool_ops;
	rc = register_netdev(net_dev);
	if (rc < 0) {
		dev_err(dev, "%s: Cannot register net device\n", port_name);
		goto err_free;
	}

	/* Let the phylib set the RUNNING flag */
	netif_carrier_off(net_dev);

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
