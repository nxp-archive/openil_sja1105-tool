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
	struct mutex lock;

	struct sja1105_static_config static_config;
	struct gpio_desc *reset_gpio;

	u64 reg_addr; /* register address to read from */
	u64 vlanid; /* vlan lookup entry to read */

	u64 device_id;
	u64 part_nr; /* Needed for P/R distinction (same switch core) */
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

int sja1105_spi_send_packed_buf(struct sja1105_spi_private*,
                                enum sja1105_spi_access_mode,
                                uint64_t, void*, uint64_t);
int sja1105_spi_send_int(struct sja1105_spi_private*,
                         enum sja1105_spi_access_mode,
                         uint64_t, uint64_t*, uint64_t);
int sja1105_spi_send_long_packed_buf(struct sja1105_spi_private*,
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
int sja1105_mac_config_get(struct sja1105_spi_private *priv,
                           struct sja1105_mac_config_entry *entry,
                           int port);
int sja1105_mac_config_set(struct sja1105_spi_private *priv,
                           struct sja1105_mac_config_entry *entry,
                           int port);
int sja1105_vlan_lookup_get(struct sja1105_spi_private *priv,
                            struct sja1105_vlan_lookup_entry *entry);
int sja1105_vlan_lookup_set(struct sja1105_spi_private *priv,
                            struct sja1105_vlan_lookup_entry *entry,
                            int valident);
int sja1105_inhibit_tx(struct sja1105_spi_private *priv,
                       struct sja1105_egress_port_mask *port_mask);

/* sja1105-clocking.c */
int sja1105_clocking_setup_port(struct sja1105_spi_private *priv, int port,
                                struct sja1105_xmii_params_entry *params,
                                struct sja1105_mac_config_entry  *mac_config);
int sja1105_clocking_setup(struct sja1105_spi_private *priv,
                           struct sja1105_xmii_params_entry *params,
                           struct sja1105_mac_config_entry  *mac_configs);

/* sja1105-status.c */
int sja1105_device_id_get(struct sja1105_spi_private *priv);

/* sja1105-reset.c */
int sja1105_cold_reset(struct sja1105_spi_private *priv);

/* sja1105-spi.c */
int sja1105_static_config_flush(struct sja1105_spi_private *priv);

/* sja1105-status.c */

#define CORE_ADDR   0x000000
#define ACU_ADDR    0x100800

struct sja1105_general_status {
	uint64_t configs;
	uint64_t crcchkl;
	uint64_t ids;
	uint64_t crcchkg;
	uint64_t nslot;
	uint64_t vlind;
	uint64_t vlparind;
	uint64_t vlroutes;
	uint64_t vlparts;
	uint64_t macaddl;
	uint64_t portenf;
	uint64_t fwds_03h;
	uint64_t macfds;
	uint64_t enffds;
	uint64_t l2busyfds;
	uint64_t l2busys;
	uint64_t macaddu;
	uint64_t macaddhcl;
	uint64_t vlanidhc;
	uint64_t hashconfs;
	uint64_t macaddhcu;
	uint64_t wpvlanid;
	uint64_t port_07h;
	uint64_t vlanbusys;
	uint64_t wrongports;
	uint64_t vnotfounds;
	uint64_t vlid;
	uint64_t portvl;
	uint64_t vlnotfound;
	uint64_t emptys;
	uint64_t buffers;
	uint64_t buflwmark; /* Only on P/Q/R/S */
	uint64_t port_0ah;
	uint64_t fwds_0ah;
	uint64_t parts;
	uint64_t ramparerrl;
	uint64_t ramparerru;
};

struct sja1105_port_status_mac {
	uint64_t n_runt;
	uint64_t n_soferr;
	uint64_t n_alignerr;
	uint64_t n_miierr;
	uint64_t typeerr;
	uint64_t sizeerr;
	uint64_t tctimeout;
	uint64_t priorerr;
	uint64_t nomaster;
	uint64_t memov;
	uint64_t memerr;
	uint64_t invtyp;
	uint64_t intcyov;
	uint64_t domerr;
	uint64_t pcfbagdrop;
	uint64_t spcprior;
	uint64_t ageprior;
	uint64_t portdrop;
	uint64_t lendrop;
	uint64_t bagdrop;
	uint64_t policeerr;
	uint64_t drpnona664err;
	uint64_t spcerr;
	uint64_t agedrp;
};

struct sja1105_port_status_hl1 {
	uint64_t n_n664err;
	uint64_t n_vlanerr;
	uint64_t n_unreleased;
	uint64_t n_sizerr;
	uint64_t n_crcerr;
	uint64_t n_vlnotfound;
	uint64_t n_ctpolerr;
	uint64_t n_polerr;
	uint64_t n_rxfrmsh;
	uint64_t n_rxfrm;
	uint64_t n_rxbytesh;
	uint64_t n_rxbyte;
	uint64_t n_txfrmsh;
	uint64_t n_txfrm;
	uint64_t n_txbytesh;
	uint64_t n_txbyte;
};

struct sja1105_port_status_hl2 {
	uint64_t n_qfull;
	uint64_t n_part_drop;
	uint64_t n_egr_disabled;
	uint64_t n_not_reach;
	uint64_t qlevel_hwm[8]; /* Only for P/Q/R/S */
	uint64_t qlevel[8];     /* Only for P/Q/R/S */
};

struct sja1105_port_status {
	struct sja1105_port_status_mac mac;
	struct sja1105_port_status_hl1 hl1;
	struct sja1105_port_status_hl2 hl2;
};

struct sja1105_ptp_status {
	uint64_t syncstate;
	uint64_t integcy;
	uint64_t actcorr;
	uint64_t maxcorr;
	uint64_t mincorr;
	uint64_t syncloss;
	uint64_t locmem;
	uint64_t minmemvar;
	uint64_t maxmemvar;
	uint64_t localsynmem0;
	uint64_t localsynmem1;
	uint64_t minasynmem;
	uint64_t maxasynmem;
	uint64_t camem;
	uint64_t synmem;
	uint64_t moffset[8];
	uint64_t timer;
	uint64_t clock;
};
int sja1105_port_status_clear(struct sja1105_spi_private*, int);
int sja1105_port_status_get_hl1(struct sja1105_spi_private *priv,
                                struct sja1105_port_status_hl1 *status,
                                int port);
int sja1105_port_status_get(struct sja1105_spi_private*,
                            struct sja1105_port_status*,
                            int port);
int  sja1105_general_status_get(struct sja1105_spi_private*,
                                struct sja1105_general_status*);
void sja1105_general_status_show(struct sja1105_general_status*,
                                 char*, size_t,
                                 uint64_t device_id);

#endif
