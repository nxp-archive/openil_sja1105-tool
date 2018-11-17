/*
 * SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2018, NXP Semiconductors
 */
#include <linux/ethtool.h>
#include <linux/etherdevice.h>
#include <linux/net_tstamp.h>
#include "sja1105.h"

static void sja1105_get_drvinfo(struct net_device *net_dev,
                                struct ethtool_drvinfo *drvinfo)
{
	strlcpy(drvinfo->driver, KBUILD_MODNAME, sizeof(drvinfo->driver));
	snprintf(drvinfo->version, 32, "%s", VERSION);
}

static char sja1105_port_stats[][ETH_GSTRING_LEN] = {
	/* MAC-Level Diagnostic Counters */
	"N_RUNT",
	"N_SOFERR",
	"N_ALIGNERR",
	"N_MIIERR",
	/* MAC-Level Diagnostic Flags */
	"TYPEERR",
	"SIZEERR",
	"TCTIMEOUT",
	"PRIORERR",
	"NOMASTER",
	"MEMOV",
	"MEMERR",
	"INVTYP",
	"INTCYOV",
	"DOMERR",
	"PCFBAGDROP",
	"SPCPRIOR",
	"AGEPRIOR",
	"PORTDROP",
	"LENDROP",
	"BAGDROP",
	"POLICEERR",
	"DRPNONA664ERR",
	"SPCERR",
	"AGEDRP",
	/* High-Level Diagnostic Counters */
	"N_N664ERR",
	"N_VLANERR",
	"N_UNRELEASED",
	"N_SIZERR",
	"N_CRCERR",
	"N_VLNOTFOUND",
	"N_CTPOLERR",
	"N_POLERR",
	"N_RXFRM",
	"N_RXBYTE",
	"N_TXFRM",
	"N_TXBYTE",
	"N_QFULL",
	"N_PART_DROP",
	"N_EGR_DISABLED",
	"N_NOT_REACH",
};

static char sja1105pqrs_extra_port_stats[][ETH_GSTRING_LEN] = {
	/* Queue Levels */
	"QLEVEL_HWM_0",
	"QLEVEL_HWM_1",
	"QLEVEL_HWM_2",
	"QLEVEL_HWM_3",
	"QLEVEL_HWM_4",
	"QLEVEL_HWM_5",
	"QLEVEL_HWM_6",
	"QLEVEL_HWM_7",
	"QLEVEL_0",
	"QLEVEL_1",
	"QLEVEL_2",
	"QLEVEL_3",
	"QLEVEL_4",
	"QLEVEL_5",
	"QLEVEL_6",
	"QLEVEL_7",
};

static void
sja1105_get_ethtool_stats(struct net_device *net_dev,
                          struct ethtool_stats *stats,
                          u64 *data)
{
	struct sja1105_port *port = netdev_priv(net_dev);
	struct spi_device *spi = port->spi_dev;
	struct sja1105_spi_private *priv = spi_get_drvdata(spi);
	struct sja1105_port_status status;
	int i, k = 0;
	int rc;

	mutex_lock(&priv->lock);
	rc = sja1105_port_status_get(priv, &status, port->index);
	mutex_unlock(&priv->lock);
	if (rc) {
		rc = -EIO;
		goto out;
	}
	memset(data, 0, ARRAY_SIZE(sja1105_port_stats) * sizeof(u64));
	data[k++] = status.mac.n_runt;
	data[k++] = status.mac.n_soferr;
	data[k++] = status.mac.n_alignerr;
	data[k++] = status.mac.n_miierr;
	data[k++] = status.mac.typeerr;
	data[k++] = status.mac.sizeerr;
	data[k++] = status.mac.tctimeout;
	data[k++] = status.mac.priorerr;
	data[k++] = status.mac.nomaster;
	data[k++] = status.mac.memov;
	data[k++] = status.mac.memerr;
	data[k++] = status.mac.invtyp;
	data[k++] = status.mac.intcyov;
	data[k++] = status.mac.domerr;
	data[k++] = status.mac.pcfbagdrop;
	data[k++] = status.mac.spcprior;
	data[k++] = status.mac.ageprior;
	data[k++] = status.mac.portdrop;
	data[k++] = status.mac.lendrop;
	data[k++] = status.mac.bagdrop;
	data[k++] = status.mac.policeerr;
	data[k++] = status.mac.drpnona664err;
	data[k++] = status.mac.spcerr;
	data[k++] = status.mac.agedrp;
	data[k++] = status.hl1.n_n664err;
	data[k++] = status.hl1.n_vlanerr;
	data[k++] = status.hl1.n_unreleased;
	data[k++] = status.hl1.n_sizerr;
	data[k++] = status.hl1.n_crcerr;
	data[k++] = status.hl1.n_vlnotfound;
	data[k++] = status.hl1.n_ctpolerr;
	data[k++] = status.hl1.n_polerr;
	data[k++] = status.hl1.n_rxfrm;
	data[k++] = status.hl1.n_rxbyte;
	data[k++] = status.hl1.n_txfrm;
	data[k++] = status.hl1.n_txbyte;
	data[k++] = status.hl2.n_qfull;
	data[k++] = status.hl2.n_part_drop;
	data[k++] = status.hl2.n_egr_disabled;
	data[k++] = status.hl2.n_not_reach;

	if (!IS_PQRS(priv->device_id))
		goto out;

	memset(data + k, 0, ARRAY_SIZE(sja1105pqrs_extra_port_stats) * sizeof(u64));
	for (i = 0; i < 8; i++) {
		data[k++] = status.hl2.qlevel_hwm[i];
		data[k++] = status.hl2.qlevel[i];
	}
out:
	return;
}

static void sja1105_get_strings(struct net_device *net_dev, u32 stringset,
                                u8 *data)
{
	struct sja1105_port *port = netdev_priv(net_dev);
	struct spi_device *spi = port->spi_dev;
	struct sja1105_spi_private *priv = spi_get_drvdata(spi);
	u8 *p = data;
	int i;

	switch (stringset) {
	case ETH_SS_STATS:
		for (i = 0; i < ARRAY_SIZE(sja1105_port_stats); i++) {
			strlcpy(p, sja1105_port_stats[i], ETH_GSTRING_LEN);
			p += ETH_GSTRING_LEN;
		}
		if (!IS_PQRS(priv->device_id))
			return;
		for (i = 0; i < ARRAY_SIZE(sja1105pqrs_extra_port_stats); i++) {
			strlcpy(p, sja1105pqrs_extra_port_stats[i], ETH_GSTRING_LEN);
			p += ETH_GSTRING_LEN;
		}
		break;
	}
}

static int sja1105_get_sset_count(struct net_device *net_dev, int sset)
{
	struct sja1105_port *port = netdev_priv(net_dev);
	struct spi_device *spi = port->spi_dev;
	struct sja1105_spi_private *priv = spi_get_drvdata(spi);

	if (IS_PQRS(priv->device_id))
		return ARRAY_SIZE(sja1105_port_stats) +
		       ARRAY_SIZE(sja1105pqrs_extra_port_stats);
	else
		return ARRAY_SIZE(sja1105_port_stats);
}

static int sja1105_get_regs_len(struct net_device *net_dev)
{
	/* General status registers are held in memory as u64, but we are
	 * performing the register dump as u32. So the number of bytes
	 * to be dumped is half (64 / 32) the total bytes of the struct.
	 */
	return sizeof(struct sja1105_general_status) / 2;
}

/* Dump of the sja1105_general_status structure. Each field
 * is printed as a big-endian u32 value.
 */
static void
sja1105_get_regs(struct net_device *net_dev, struct ethtool_regs *regs,
                 void *regbuf)
{
	struct sja1105_port *port = netdev_priv(net_dev);
	struct spi_device *spi = port->spi_dev;
	struct sja1105_spi_private *priv = spi_get_drvdata(spi);
	struct sja1105_general_status status;
	u32    *buf = (u32*) regbuf;
	int    rc, k = 0;

	mutex_lock(&priv->lock);
	rc = sja1105_general_status_get(priv, &status);
	mutex_unlock(&priv->lock);
	if (rc)
		return;

	buf[k++] = cpu_to_be32((u32) status.configs);
	buf[k++] = cpu_to_be32((u32) status.crcchkl);
	buf[k++] = cpu_to_be32((u32) status.ids);
	buf[k++] = cpu_to_be32((u32) status.nslot);
	buf[k++] = cpu_to_be32((u32) status.vlind);
	buf[k++] = cpu_to_be32((u32) status.vlparind);
	buf[k++] = cpu_to_be32((u32) status.vlroutes);
	buf[k++] = cpu_to_be32((u32) status.vlparts);
	buf[k++] = cpu_to_be32((u32) status.macaddl);
	buf[k++] = cpu_to_be32((u32) status.portenf);
	buf[k++] = cpu_to_be32((u32) status.fwds_03h);
	buf[k++] = cpu_to_be32((u32) status.macfds);
	buf[k++] = cpu_to_be32((u32) status.enffds);
	buf[k++] = cpu_to_be32((u32) status.l2busyfds);
	buf[k++] = cpu_to_be32((u32) status.l2busys);
	buf[k++] = cpu_to_be32((u32) status.macaddu);
	buf[k++] = cpu_to_be32((u32) status.macaddu);
	buf[k++] = cpu_to_be32((u32) status.macaddhcl);
	buf[k++] = cpu_to_be32((u32) status.vlanidhc);
	buf[k++] = cpu_to_be32((u32) status.hashconfs);
	buf[k++] = cpu_to_be32((u32) status.macaddhcu);
	buf[k++] = cpu_to_be32((u32) status.wpvlanid);
	buf[k++] = cpu_to_be32((u32) status.port_07h);
	buf[k++] = cpu_to_be32((u32) status.vlanbusys);
	buf[k++] = cpu_to_be32((u32) status.wrongports);
	buf[k++] = cpu_to_be32((u32) status.vnotfounds);
	buf[k++] = cpu_to_be32((u32) status.vlid);
	buf[k++] = cpu_to_be32((u32) status.portvl);
	buf[k++] = cpu_to_be32((u32) status.vlnotfound);
	buf[k++] = cpu_to_be32((u32) status.emptys);
	buf[k++] = cpu_to_be32((u32) status.buffers);
	buf[k++] = cpu_to_be32((u32) status.buflwmark);
	buf[k++] = cpu_to_be32((u32) status.port_0ah);
	buf[k++] = cpu_to_be32((u32) status.fwds_0ah);
	buf[k++] = cpu_to_be32((u32) status.parts);
	buf[k++] = cpu_to_be32((u32) status.ramparerrl);
	buf[k++] = cpu_to_be32((u32) status.ramparerru);
}

static int sja1105_get_ts_info(struct net_device *net_dev,
                               struct ethtool_ts_info *info)
{
	struct sja1105_port *port = netdev_priv(net_dev);
	struct spi_device *spi = port->spi_dev;
	struct sja1105_spi_private *priv = spi_get_drvdata(spi);

	info->so_timestamping = SOF_TIMESTAMPING_TX_HARDWARE |
	                        SOF_TIMESTAMPING_RX_HARDWARE |
	                        SOF_TIMESTAMPING_RAW_HARDWARE;
	info->tx_types = (1 << HWTSTAMP_TX_OFF) |
	                 (1 << HWTSTAMP_TX_ON);
	info->rx_filters = (1 << HWTSTAMP_FILTER_NONE) |
	                   (1 << HWTSTAMP_FILTER_ALL);
	info->phc_index = ptp_clock_index(priv->clock);
	return 0;
}

const struct ethtool_ops sja1105_ethtool_ops = {
	.get_drvinfo        = sja1105_get_drvinfo,
	.get_link           = ethtool_op_get_link,
	.get_link_ksettings = phy_ethtool_get_link_ksettings,
	.set_link_ksettings = phy_ethtool_set_link_ksettings,
	.get_sset_count     = sja1105_get_sset_count,
	.get_ethtool_stats  = sja1105_get_ethtool_stats,
	.get_strings        = sja1105_get_strings,
	.get_regs_len       = sja1105_get_regs_len,
	.get_regs           = sja1105_get_regs,
	.get_ts_info        = sja1105_get_ts_info,
};
