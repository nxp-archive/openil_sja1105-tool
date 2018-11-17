/*
 * SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
 *
 * Copyright (c) 2016-2018, NXP Semiconductors
 */
#include <lib/include/gtable.h>
#include <lib/include/static-config.h>
#include <lib/include/gtable.h>
#include <common.h>
#include "sja1105.h"

#define SJA1105_MGMT_ROUTE_COUNT 4

struct sja1105_mgmt_entry {
	uint64_t ts_regid;
	uint64_t egr_ts;
	uint64_t macaddr;
	uint64_t destports;
	uint64_t enfport;
	uint64_t index;
};

union sja1105_dyn_l2_lookup_entry {
	struct sja1105_l2_lookup_entry l2;
	struct sja1105_mgmt_entry mgmt;
};

struct sja1105_dyn_l2_lookup_cmd {
	uint64_t valid;
	uint64_t rdwrset;
	uint64_t errors;
	uint64_t lockeds;
	uint64_t valident;
	uint64_t mgmtroute;
	union sja1105_dyn_l2_lookup_entry entry;
};

int sja1105_inhibit_tx(struct sja1105_spi_private *priv,
                       struct sja1105_egress_port_mask *port_mask)
{
	const int ETH_PORT_CONTROL_ADDR = 0x11;
	const int BUF_LEN = 4;
	uint8_t packed_buf[BUF_LEN];
	int i;

	for (i = 0; i < SJA1105_NUM_PORTS; i++)
		gtable_pack(packed_buf, &port_mask->inhibit_tx[i], i, i, 4);

	return sja1105_spi_send_packed_buf(priv, SPI_WRITE,
	                                   CORE_ADDR + ETH_PORT_CONTROL_ADDR,
	                                   packed_buf, 4);
}

/* This function just packs/unpacks the structure for writing to
 * UM10944.pdf Table 62 (address 30h), which commits a change of
 * CBS parameters.
 * Care must be taken that a prior write be performed to Table 63
 * (address 2Fh to 2Ch) in order to set the CBS parameters to
 * the desired values. */
static void
sja1105_cbs_cmd_access(void *buf,
                       struct sja1105_cbs *cbs,
                       int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = 4;
	/* No point to keeping the "valid" bit as part of the
	 * structure, since we will never use it for anything
	 * else than triggering a write */
	uint64_t valid = 1;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(cbs, 0, sizeof(*cbs));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &valid,      31, 31, 4);
	pack_or_unpack(buf, &cbs->index, 19, 16, 4);
	pack_or_unpack(buf, &cbs->port,   5,  3, 4);
	pack_or_unpack(buf, &cbs->prio,   2,  0, 4);
}
#define sja1105_cbs_cmd_pack(buf, cbs)   sja1105_cbs_cmd_access(buf, cbs, 1)
#define sja1105_cbs_cmd_unpack(buf, cbs) sja1105_cbs_cmd_access(buf, cbs, 0)

/* Wrapper around sja1105_spi_send_packed_buf() */
int sja1105_cbs_commit(struct sja1105_spi_private *priv,
                       struct sja1105_cbs *cbs)
{
	const int CBS_CONTROL_ADDR = 0x30;
	const int CBS_DATA_ADDR    = 0x2C;
	/* packed_buf will accomodate 2 transfers, one of
	 * 32 bytes and one of 4 bytes. */
	const int BUF_LEN = 32;
	uint8_t packed_buf[BUF_LEN];
	/* Make pointer arithmetic work on 4 bytes */
	uint32_t *p = (uint32_t*) packed_buf;
	int rc;

	/* Data portion of transaction */
	gtable_pack(p + 0, &cbs->idle_slope, 31, 0, 4);
	gtable_pack(p + 1, &cbs->send_slope, 31, 0, 4);
	gtable_pack(p + 2, &cbs->credit_hi,  31, 0, 4);
	gtable_pack(p + 3, &cbs->credit_lo,  31, 0, 4);
	rc = sja1105_spi_send_packed_buf(priv, SPI_WRITE,
	                                 CORE_ADDR + CBS_DATA_ADDR,
	                                 p, BUF_LEN);
	if (rc < 0) {
		loge("spi_send_packed_buf failed for cbs data transaction");
		goto out;
	}
	/* Command portion of transaction */
	sja1105_cbs_cmd_pack(packed_buf, cbs);
	rc = sja1105_spi_send_packed_buf(priv, SPI_WRITE,
	                                 CORE_ADDR + CBS_CONTROL_ADDR,
	                                 packed_buf, 4);
	if (rc < 0) {
		loge("spi_send_packed_buf failed for cbs command transaction");
		goto out;
	}
out:
	return rc;
}

struct sja1105_dyn_mac_reconfig_entry {
	uint64_t valid;
	uint64_t errors;  /* Only on P/Q/R/S */
	uint64_t rdwrset; /* Only on P/Q/R/S */
	uint64_t portidx;
	struct sja1105_mac_config_entry entry;
};

static void
sja1105et_dyn_mac_reconfig_entry_access(void *buf, struct
                                        sja1105_dyn_mac_reconfig_entry *cmd,
                                        int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	void *reg2 = (void*)buf; /* yup */
	void *reg1 = (void*)((char*)buf + 4);
	int   size = 8;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(cmd, 0, sizeof(*cmd));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(reg1, &cmd->valid,           31, 31, 4);
	pack_or_unpack(reg1, &cmd->entry.speed,     30, 29, 4);
	pack_or_unpack(reg1, &cmd->portidx,         26, 24, 4);
	pack_or_unpack(reg1, &cmd->entry.drpdtag,   23, 23, 4);
	pack_or_unpack(reg1, &cmd->entry.drpuntag,  22, 22, 4);
	pack_or_unpack(reg1, &cmd->entry.retag,     21, 21, 4);
	pack_or_unpack(reg1, &cmd->entry.dyn_learn, 20, 20, 4);
	pack_or_unpack(reg1, &cmd->entry.egress,    19, 19, 4);
	pack_or_unpack(reg1, &cmd->entry.ingress,   18, 18, 4);
	pack_or_unpack(reg1, &cmd->entry.ing_mirr,  17, 17, 4);
	pack_or_unpack(reg1, &cmd->entry.egr_mirr,  16, 16, 4);
	pack_or_unpack(reg1, &cmd->entry.vlanprio,  14, 12, 4);
	pack_or_unpack(reg1, &cmd->entry.vlanid,    11,  0, 4);
	pack_or_unpack(reg2, &cmd->entry.tp_delin,  31, 16, 4);
	pack_or_unpack(reg2, &cmd->entry.tp_delout, 15,  0, 4);
	/* MAC configuration table entries which can't be reconfigured:
	 * top, base, enabled, ifg, maxage, drpnona664 */
}

static void
sja1105pqrs_dyn_mac_reconfig_entry_access(void *buf, struct
                                          sja1105_dyn_mac_reconfig_entry *cmd,
                                          int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	void (*mac_entry_pack_or_unpack)(void*, struct
	                                 sja1105_mac_config_entry*);
	uint8_t *entry_ptr = (uint8_t*) buf;
	uint8_t *cmd_ptr   = (uint8_t*) buf + SIZE_MAC_CONFIG_ENTRY_PQRS;

	if (write == 0) {
		mac_entry_pack_or_unpack = sja1105pqrs_mac_config_entry_unpack;
		pack_or_unpack = gtable_unpack;
		memset(cmd, 0, sizeof(*cmd));
	} else {
		mac_entry_pack_or_unpack = sja1105pqrs_mac_config_entry_pack;
		pack_or_unpack = gtable_pack;
		memset(buf, 0, SIZE_MAC_CONFIG_ENTRY_PQRS);
	}
	pack_or_unpack(cmd_ptr, &cmd->valid,     31, 31, 4);
	pack_or_unpack(cmd_ptr, &cmd->errors,    30, 30, 4);
	pack_or_unpack(cmd_ptr, &cmd->rdwrset,   29, 29, 4);
	pack_or_unpack(cmd_ptr, &cmd->portidx,    2,  0, 4);
	mac_entry_pack_or_unpack(entry_ptr, &cmd->entry);
}
/*
 * sja1105et_dyn_mac_reconfig_entry_pack
 * sja1105et_dyn_mac_reconfig_entry_unpack
 * sja1105pqrs_dyn_mac_reconfig_entry_pack
 * sja1105pqrs_dyn_mac_reconfig_entry_unpack
 */
DEFINE_SEPARATE_PACK_UNPACK_ACCESSORS(dyn_mac_reconfig);

static int sja1105et_mac_config_commit(struct sja1105_spi_private *priv,
                                       struct sja1105_mac_config_entry *entry,
                                       int port)
{
	/* UM10944 Table 71. MAC configuration table reconfiguration register 2
	 * (address 36h) */
	const int ENTRY_ADDR = 0x36;
	const int BUF_LEN    = 8;
	/* SPI payload buffer */
	uint8_t packed_buf[BUF_LEN];
	/* Structure to hold command we are constructing */
	struct sja1105_dyn_mac_reconfig_entry cmd;
	int rc;

	memset(&cmd, 0, sizeof(cmd));
	cmd.valid     = 1;
	cmd.portidx   = port;
	memcpy(&cmd.entry, entry, sizeof(*entry));

	sja1105et_dyn_mac_reconfig_entry_pack(packed_buf, &cmd);

	/* Send SPI write operation: "write mac reconfig table entry" */
	rc = sja1105_spi_send_packed_buf(priv, SPI_WRITE, ENTRY_ADDR,
	                                 packed_buf, BUF_LEN);
	if (rc < 0) {
		loge("failed to read from spi");
		goto out;
	}
out:
	return rc;
}

static int sja1105pqrs_mac_config_commit(struct sja1105_spi_private *priv,
                                         struct sja1105_mac_config_entry *entry,
                                         int read_or_write, int portidx)
{
	/* MAC configuration table reconfiguration register */
	const int ENTRY_ADDR = 0x4B;
	const int BUF_LEN = 4 + SIZE_MAC_CONFIG_ENTRY_PQRS;
	/* SPI payload buffer */
	uint8_t packed_buf[BUF_LEN];
	/* Structure to hold command we are constructing */
	struct sja1105_dyn_mac_reconfig_entry cmd;
	int rc;

	memset(&cmd, 0, sizeof(cmd));
	cmd.valid     = 1;
	cmd.rdwrset   = (read_or_write == SPI_WRITE);
	cmd.portidx   = portidx;
	if (read_or_write == SPI_WRITE)
		/* Put the argument into the SPI payload */
		memcpy(&cmd.entry, entry, sizeof(*entry));

	sja1105pqrs_dyn_mac_reconfig_entry_pack(packed_buf, &cmd);

	/* Send SPI write operation: "read/write mac config table entry" */
	rc = sja1105_spi_send_packed_buf(priv, SPI_WRITE, ENTRY_ADDR,
	                                 packed_buf, BUF_LEN);
	if (rc < 0) {
		loge("failed to read from spi");
		goto out;
	}

	if (read_or_write == SPI_READ) {
		/* If previous operation was a read, retrieve its result:
		 * the mac config table entry requested for */
		memset(packed_buf, 0, BUF_LEN);
		rc = sja1105_spi_send_packed_buf(priv, SPI_READ, ENTRY_ADDR,
		                                 packed_buf, BUF_LEN);
		if (rc < 0) {
			loge("failed to read from spi");
			goto out;
		}
		sja1105pqrs_dyn_mac_reconfig_entry_unpack(packed_buf, &cmd);
		memcpy(entry, &cmd.entry, sizeof(*entry));
	}
out:
	return rc;
}

int sja1105_mac_config_get(struct sja1105_spi_private *priv,
                           struct sja1105_mac_config_entry *entry,
                           int port)
{
	if (IS_ET(priv->device_id)) {
		loge("Reading MAC reconfiguration table not supported on E/T!\n");
		return -EINVAL;
	}
	return sja1105pqrs_mac_config_commit(priv, entry,
	                                     SPI_READ, port);
}

int sja1105_mac_config_set(struct sja1105_spi_private *priv,
                           struct sja1105_mac_config_entry *entry, int port)
{
	int rc;

	if (IS_ET(priv->device_id))
		rc = sja1105et_mac_config_commit(priv, entry, port);
	else
		rc = sja1105pqrs_mac_config_commit(priv, entry,
		                                   SPI_WRITE, port);
	return rc;
}

/*
 * TODO: Update management routes for SJA1105 P/Q/R/S compatibility
 */

/* Buffer is segregated into 2 parts:
 *   * ENTRY: a portion of SIZE_L2_LOOKUP_ENTRY (12) bytes,
 *            corresponding to addresses 0x20, 0x21 and 0x22
 *   * CMD: a portion of 4 bytes, corresponding to address 0x23
 */
static void
sja1105_dyn_l2_lookup_cmd_access(void *buf,
                                 struct sja1105_dyn_l2_lookup_cmd *cmd,
                                 int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = 4 + SIZE_L2_LOOKUP_ENTRY_ET;
	uint8_t *entry_ptr = (uint8_t*) buf;
	uint8_t *cmd_ptr   = (uint8_t*) buf + SIZE_L2_LOOKUP_ENTRY_ET;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(cmd, 0, sizeof(*cmd));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(cmd_ptr, &cmd->valid,     31, 31, 4);
	pack_or_unpack(cmd_ptr, &cmd->rdwrset,   30, 30, 4);
	pack_or_unpack(cmd_ptr, &cmd->errors,    29, 29, 4);
	pack_or_unpack(cmd_ptr, &cmd->lockeds,   28, 28, 4);
	pack_or_unpack(cmd_ptr, &cmd->valident,  27, 27, 4);
	pack_or_unpack(cmd_ptr, &cmd->mgmtroute, 26, 26, 4);
	if (cmd->mgmtroute) {
		/* Management route */
		pack_or_unpack(entry_ptr, &cmd->entry.mgmt.ts_regid,  85, 85, SIZE_L2_LOOKUP_ENTRY_ET);
		pack_or_unpack(entry_ptr, &cmd->entry.mgmt.egr_ts,    84, 84, SIZE_L2_LOOKUP_ENTRY_ET);
		pack_or_unpack(entry_ptr, &cmd->entry.mgmt.macaddr,   83, 36, SIZE_L2_LOOKUP_ENTRY_ET);
		pack_or_unpack(entry_ptr, &cmd->entry.mgmt.destports, 35, 31, SIZE_L2_LOOKUP_ENTRY_ET);
		pack_or_unpack(entry_ptr, &cmd->entry.mgmt.enfport,   30, 30, SIZE_L2_LOOKUP_ENTRY_ET);
		pack_or_unpack(entry_ptr, &cmd->entry.mgmt.index,     29, 20, SIZE_L2_LOOKUP_ENTRY_ET);
	} else {
		/* Regular L2 lookup entry.
		 * Code duplicated from src/lib/static-config/tables/l2-lookup.c */
		pack_or_unpack(entry_ptr, &cmd->entry.l2.vlanid,    95, 84, SIZE_L2_LOOKUP_ENTRY_ET);
		pack_or_unpack(entry_ptr, &cmd->entry.l2.macaddr,   83, 36, SIZE_L2_LOOKUP_ENTRY_ET);
		pack_or_unpack(entry_ptr, &cmd->entry.l2.destports, 35, 31, SIZE_L2_LOOKUP_ENTRY_ET);
		pack_or_unpack(entry_ptr, &cmd->entry.l2.enfport,   30, 30, SIZE_L2_LOOKUP_ENTRY_ET);
		pack_or_unpack(entry_ptr, &cmd->entry.l2.index,     29, 20, SIZE_L2_LOOKUP_ENTRY_ET);
	}
}
#define sja1105_dyn_l2_lookup_cmd_pack(buf, cmd) \
	sja1105_dyn_l2_lookup_cmd_access(buf, cmd, 1)
#define sja1105_dyn_l2_lookup_cmd_unpack(buf, cmd) \
	sja1105_dyn_l2_lookup_cmd_access(buf, cmd, 0)

static inline int
sja1105_mgmt_route_commit(struct sja1105_spi_private *priv,
                          struct sja1105_mgmt_entry *entry,
                          int read_or_write, int index)
{
	/* Access to CMD_ADDR = 0x23 is implicit and done
	 * through the same SPI transaction */
	const int ENTRY_ADDR = 0x20;
	const int BUF_LEN = 4 + SIZE_L2_LOOKUP_ENTRY_ET;
	/* SPI payload buffer */
	uint8_t packed_buf[BUF_LEN];
	/* Structure to hold command we are constructing,
	 * and mgmt entry we are reading/writing */
	struct sja1105_dyn_l2_lookup_cmd cmd;
	int rc;

	memset(&cmd, 0, sizeof(cmd));
	cmd.valid     = 1;
	cmd.rdwrset   = (read_or_write == SPI_WRITE);
	cmd.valident  = 1;
	cmd.mgmtroute = 1;
	if (read_or_write == SPI_WRITE) {
		/* Put the argument into the SPI payload,
		 * as new mgmt entry */
		memcpy(&cmd.entry, entry, sizeof(*entry));
	}
	cmd.entry.mgmt.index = index;
	cmd.entry.mgmt.enfport = 1;
	sja1105_dyn_l2_lookup_cmd_pack(packed_buf, &cmd);

	/* Send SPI write operation: "read/write mgmt table entry" */
	rc = sja1105_spi_send_packed_buf(priv, SPI_WRITE, ENTRY_ADDR,
	                                 packed_buf, BUF_LEN);
	if (rc < 0) {
		loge("failed to read from spi");
		goto out;
	}

	if (read_or_write == SPI_READ) {
		/* If previous operation was a read, retrieve its result:
		 * the mgmt table entry requested for */
		memset(packed_buf, 0, BUF_LEN);
		rc = sja1105_spi_send_packed_buf(priv, SPI_READ, ENTRY_ADDR,
		                                 packed_buf, BUF_LEN);
		if (rc < 0) {
			loge("failed to read from spi");
			goto out;
		}
		sja1105_dyn_l2_lookup_cmd_unpack(packed_buf, &cmd);
		memcpy(entry, &cmd.entry, sizeof(*entry));
	}
out:
	return rc;
}
#define sja1105_mgmt_route_get(priv, entry, index) \
	sja1105_mgmt_route_commit(priv, entry, SPI_READ, index)
#define sja1105_mgmt_route_set(priv, entry, index) \
	sja1105_mgmt_route_commit(priv, entry, SPI_WRITE, index)


struct sja1105_dyn_vlan_lookup_entry {
	uint64_t valid;
	uint64_t rdwrset;
	uint64_t valident;
	struct sja1105_vlan_lookup_entry entry;
};

/*
 * There is a 1 register gap between VLAN Lookup table reconfiguration
 * register 1,2 (addr 0x2d..2e) and 0 (addr 0x30). Add this offset to
 * the entry size.
 */
#define SIZE_VLAN_LOOKUP_RECONFIG_ENTRY   (SIZE_VLAN_LOOKUP_ENTRY + 4)

static void
sja1105_dyn_vlan_lookup_cmd_access(void *buf,
                                   struct sja1105_dyn_vlan_lookup_entry *cmd,
                                   int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	void (*vlan_entry_pack_or_unpack)(void*, struct
	                                  sja1105_vlan_lookup_entry*);
	uint8_t *entry_ptr = (uint8_t*) buf;
	uint8_t *cmd_ptr   = (uint8_t*) buf + SIZE_VLAN_LOOKUP_RECONFIG_ENTRY;

	if (write == 0) {
		vlan_entry_pack_or_unpack = sja1105_vlan_lookup_entry_unpack;
		pack_or_unpack = gtable_unpack;
		memset(cmd, 0, sizeof(*cmd));
	} else {
		vlan_entry_pack_or_unpack = sja1105_vlan_lookup_entry_pack;
		pack_or_unpack = gtable_pack;
		memset(buf, 0, SIZE_VLAN_LOOKUP_RECONFIG_ENTRY);
	}
	pack_or_unpack(cmd_ptr, &cmd->valid,     31, 31, 4);
	pack_or_unpack(cmd_ptr, &cmd->rdwrset,   30, 30, 4);
	pack_or_unpack(cmd_ptr, &cmd->valident,  27, 27, 4);
	vlan_entry_pack_or_unpack(entry_ptr, &cmd->entry);
}
#define sja1105_dyn_vlan_lookup_cmd_pack(buf, cmd) \
	sja1105_dyn_vlan_lookup_cmd_access(buf, cmd, 1)
#define sja1105_dyn_vlan_lookup_cmd_unpack(buf, cmd) \
	sja1105_dyn_vlan_lookup_cmd_access(buf, cmd, 0)

static int sja1105_vlan_lookup_commit(struct sja1105_spi_private *priv,
                                      struct sja1105_vlan_lookup_entry *entry,
                                      int read_or_write,
                                      int valident)
{
	/* MAC configuration table reconfiguration register */
	const int ENTRY_ADDR = 0x2D;
	const int BUF_LEN = 4 + SIZE_VLAN_LOOKUP_RECONFIG_ENTRY;
	/* SPI payload buffer */
	uint8_t packed_buf[BUF_LEN];
	/* Structure to hold command we are constructing */
	struct sja1105_dyn_vlan_lookup_entry cmd;
	int rc;

	memset(&cmd, 0, sizeof(cmd));
	cmd.valid     = 1;
	cmd.rdwrset   = (read_or_write == SPI_WRITE);
	cmd.valident  = valident;
	/* Put the argument into the SPI payload (for read and write,
	 * for read only entry.vlanid is evaluated by chip) */
	memcpy(&cmd.entry, entry, sizeof(*entry));

	sja1105_dyn_vlan_lookup_cmd_pack(packed_buf, &cmd);

	/* Send SPI write operation: "read/write vlan lookup table entry" */
	rc = sja1105_spi_send_packed_buf(priv,
	                                 SPI_WRITE,
	                                 ENTRY_ADDR,
	                                 packed_buf,
	                                 BUF_LEN);
	if (rc < 0) {
		loge("failed to read from spi");
		goto out;
	}

	if (read_or_write == SPI_READ) {
		/* If previous operation was a read, retrieve its result:
		 * the mac config table entry requested for */
		memset(packed_buf, 0, BUF_LEN);
		rc = sja1105_spi_send_packed_buf(priv,
		                                 SPI_READ,
		                                 ENTRY_ADDR,
		                                 packed_buf,
		                                 BUF_LEN);
		if (rc < 0) {
			loge("failed to read from spi");
			goto out;
		}
		sja1105_dyn_vlan_lookup_cmd_unpack(packed_buf, &cmd);
		memcpy(entry, &cmd.entry, sizeof(*entry));
	}
out:
	return rc;
}

int sja1105_vlan_lookup_get(struct sja1105_spi_private *priv,
                            struct sja1105_vlan_lookup_entry *entry)
{
	return sja1105_vlan_lookup_commit(priv, entry, SPI_READ, 0);
}

int sja1105_vlan_lookup_set(struct sja1105_spi_private *priv,
                            struct sja1105_vlan_lookup_entry *entry,
                            int valident)
{
	if (IS_ET(priv->device_id))
		logi("Parameter VALIDENT is ignored on E/T!\n");

	return sja1105_vlan_lookup_commit(priv, entry,
	                                  SPI_WRITE, valident);
}
