/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2018, Sensor-Technik Wiedemann GmbH
 * Copyright (c) 2018, NXP Semiconductors
 */
#ifndef _SJA1105_H
#define _SJA1105_H

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/netdevice.h>
#include <linux/phy.h>
#include <linux/spi/spi.h>
#include <linux/gpio/consumer.h>
#include <linux/ratelimit.h>

#include <lib/include/static-config.h>
#include <lib/include/spi.h>

struct sja1105_port {
	struct device_node *node;
	struct device_node *phy_node;
	struct phy_device  *phy_dev;
	struct net_device  *net_dev;
	struct spi_device  *spi_dev;
	struct list_head    list;
	phy_interface_t     phy_mode;
	uint32_t msg_enable; /* For netif_msg_link() */
	struct gpio_desc *reset_gpio;
	u32 reset_duration;
	u32 reset_delay;
	int index; /* switch port index */
	int auto_speed;
	int running; /* 1 if port is openend */
	struct ratelimit_state get_stats_ratelimit;
	struct rtnl_link_stats64 stats;
};

struct sja1105_spi_private {
	struct spi_device  *spi_dev;
	struct sja1105_port port_list_head;
	struct mutex lock; /* SPI device lock */

	struct sja1105_static_config static_config;
	struct sja1105_spi_setup spi_setup;
	struct gpio_desc *reset_gpio;

	struct sja1105_port *selected_port; /* port to read status from */
	u64 reg_addr; /* register address to read from */

	const char *staging_area;
};

/* sja1105-kmod.c */
int sja1105_load_firmware(struct sja1105_spi_private *priv);

/* sja1105-sysfs.c */
int sja1105_sysfs_init(struct sja1105_spi_private *priv);
void sja1105_sysfs_remove(struct sja1105_spi_private *priv);

/* sja1105-netdev.c */
struct sja1105_port* sja1105_netdev_create_port(
                struct sja1105_spi_private *priv, const char* port_name);
void sja1105_netdev_remove_port(struct sja1105_port *port);

void sja1105_netdev_adjust_link(struct net_device *net_dev);

#endif
