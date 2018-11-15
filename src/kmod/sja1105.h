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
#include <linux/ptp_clock_kernel.h>

#include <lib/include/static-config.h>
#include <lib/include/spi.h>

enum sja1105_ptp_clk_add_mode {
	PTP_SET_MODE = 0,
	PTP_ADD_MODE,
};

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

	u64 reg_addr; /* register address to read from */

	const char *staging_area;

	struct ptp_clock *clock;
	struct ptp_clock_info ptp_caps;
	enum sja1105_ptp_clk_add_mode ptp_add_mode;
};

struct sja1105_spi_message {
	uint64_t access;
	uint64_t read_count;
	uint64_t address;
};

enum sja1105_spi_access_mode {
	SPI_READ = 0,
	SPI_WRITE = 1,
};

int sja1105_spi_send_packed_buf(struct sja1105_spi_setup*,
                                enum sja1105_spi_access_mode,
                                uint64_t, void*, uint64_t);
int sja1105_spi_send_int(struct sja1105_spi_setup*,
                         enum sja1105_spi_access_mode,
                         uint64_t, uint64_t*, uint64_t);
int sja1105_spi_send_long_packed_buf(struct sja1105_spi_setup*,
                                     enum sja1105_spi_access_mode,
                                     uint64_t, char *, uint64_t);

#define SIZE_SPI_MSG_HEADER    4
#define SIZE_SPI_MSG_MAXLEN    64 * 4

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

/* sja1105-ptp.c */
int  sja1105_ptp_clock_register(struct sja1105_spi_private *priv);
void sja1105_ptp_clock_unregister(struct sja1105_spi_private *priv);

/* sja1105-dynamic-config.c */

#define SJA1105T_NUM_PORTS 5

struct sja1105_cbs {
	/* UM10944.pdf Table 62. Credit-based shaping block
	 * register 1 (address 30h).
	 * Used as identification */
	uint64_t index;
	uint64_t port;
	uint64_t prio;
	/* UM10944.pdf Table 63. Credit-based shaping block
	 * registers 2 to 5 (address 2Fh to 2Ch).
	 * Used for actual configuration. */
	uint64_t credit_lo;
	uint64_t credit_hi;
	uint64_t send_slope;
	uint64_t idle_slope;
};

struct sja1105_egress_port_mask {
	uint64_t inhibit_tx[SJA1105T_NUM_PORTS];
};
int sja1105_mac_config_get(struct sja1105_spi_setup *spi_setup,
                           struct sja1105_mac_config_entry *entry,
                           int port);
int sja1105_mac_config_set(struct sja1105_spi_setup *spi_setup,
                           struct sja1105_mac_config_entry *entry,
                           int port);
int sja1105_vlan_lookup_get(struct sja1105_spi_setup *spi_setup,
                            struct sja1105_vlan_lookup_entry *entry);
int sja1105_vlan_lookup_set(struct sja1105_spi_setup *spi_setup,
                            struct sja1105_vlan_lookup_entry *entry,
                            int valident);
int sja1105_inhibit_tx(struct sja1105_spi_setup *spi_setup,
                       struct sja1105_egress_port_mask *port_mask);

/* sja1105-clocking.c */
int sja1105_clocking_setup_port(struct sja1105_spi_setup *spi_setup, int port,
                                struct sja1105_xmii_params_entry *params,
                                struct sja1105_mac_config_entry  *mac_config);
int sja1105_clocking_setup(struct sja1105_spi_setup *spi_setup,
                           struct sja1105_xmii_params_entry *params,
                           struct sja1105_mac_config_entry  *mac_configs);

/* sja1105-status.c */
int sja1105_device_id_get(struct sja1105_spi_setup *spi_setup,
                          uint64_t *device_id, uint64_t *part_nr);

/* sja1105-reset.c */
int sja1105_cold_reset(struct sja1105_spi_setup *spi_setup);

#endif
