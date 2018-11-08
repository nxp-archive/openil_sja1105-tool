/******************************************************************************
 * Copyright (c) 2018, Sensor-Technik Wiedemann GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
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


#define FIRMWARE_FILENAME          "sja1105.bin"


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