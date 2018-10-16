/******************************************************************************
 * Copyright (c) 2018, NXP Semiconductors
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
#include <lib/include/dynamic-config.h>
#include <lib/include/static-config.h>
#include <lib/include/gtable.h>
#include <lib/include/spi.h>
#include <lib/helpers.h>
#include <common.h>

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

static int sja1105et_mac_config_commit(struct sja1105_spi_setup *spi_setup,
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
	rc = sja1105_spi_send_packed_buf(spi_setup,
	                                 SPI_WRITE,
	                                 ENTRY_ADDR,
	                                 packed_buf,
	                                 BUF_LEN);
	if (rc < 0) {
		loge("failed to read from spi");
		goto out;
	}
out:
	return rc;
}

static int sja1105pqrs_mac_config_commit(struct sja1105_spi_setup *spi_setup,
                                         struct sja1105_mac_config_entry *entry,
                                         int read_or_write,
                                         int portidx)
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
	if (read_or_write == SPI_WRITE) {
		/* Put the argument into the SPI payload */
		memcpy(&cmd.entry, entry, sizeof(*entry));
	}

	sja1105pqrs_dyn_mac_reconfig_entry_pack(packed_buf, &cmd);

	/* Send SPI write operation: "read/write mac config table entry" */
	rc = sja1105_spi_send_packed_buf(spi_setup,
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
		rc = sja1105_spi_send_packed_buf(spi_setup,
		                                 SPI_READ,
		                                 ENTRY_ADDR,
		                                 packed_buf,
		                                 BUF_LEN);
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

int sja1105_mac_config_get(struct sja1105_spi_setup *spi_setup,
                           struct sja1105_mac_config_entry *entry,
                           int port)
{
	if (IS_ET(spi_setup->device_id)) {
		loge("Reading MAC reconfiguration table not supported on E/T!\n");
		return -EINVAL;
	}
	return sja1105pqrs_mac_config_commit(spi_setup, entry,
	                                     SPI_READ, port);
}

int sja1105_mac_config_set(struct sja1105_spi_setup *spi_setup,
                           struct sja1105_mac_config_entry *entry,
                           int port)
{
	int rc;

	if (IS_ET(spi_setup->device_id))
		rc = sja1105et_mac_config_commit(spi_setup, entry, port);
	else
		rc = sja1105pqrs_mac_config_commit(spi_setup, entry,
		                                   SPI_WRITE, port);
	return rc;
}
