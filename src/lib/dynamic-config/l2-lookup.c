/******************************************************************************
 * Copyright (c) 2017, NXP Semiconductors
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

void sja1105_dyn_l2_lookup_cmd_pack(void *buf, struct
                                    sja1105_dyn_l2_lookup_cmd *cmd)
{
	sja1105_dyn_l2_lookup_cmd_access(buf, cmd, 1);
}

void sja1105_dyn_l2_lookup_cmd_unpack(void *buf, struct
                                      sja1105_dyn_l2_lookup_cmd *cmd)
{
	sja1105_dyn_l2_lookup_cmd_access(buf, cmd, 0);
}

void sja1105_mgmt_entry_show(struct sja1105_mgmt_entry *entry)
{
	printf("TS_REGID  %" PRIX64 "\n", entry->ts_regid);
	printf("EGR_TS    %" PRIX64 "\n", entry->egr_ts);
	printf("MACADDR   %" PRIX64 "\n", entry->macaddr);
	printf("DESTPORTS %" PRIX64 "\n", entry->destports);
	printf("ENFPORT   %" PRIX64 "\n", entry->enfport);
	printf("INDEX     %" PRIX64 "\n", entry->index);
}

void sja1105_dyn_l2_lookup_cmd_show(struct sja1105_dyn_l2_lookup_cmd *cmd)
{
	printf("VALID     %" PRIX64 "\n", cmd->valid);
	printf("RDWRSET   %" PRIX64 "\n", cmd->rdwrset);
	printf("ERRORS    %" PRIX64 "\n", cmd->errors);
	printf("LOCKEDS   %" PRIX64 "\n", cmd->lockeds);
	printf("VALIDENT  %" PRIX64 "\n", cmd->valident);
	printf("MGMTROUTE %" PRIX64 "\n", cmd->mgmtroute);
	if (cmd->mgmtroute) {
		sja1105_mgmt_entry_show(&cmd->entry.mgmt);
	} else {
		sja1105_l2_lookup_entry_show(&cmd->entry.l2);
	}
}

static inline int
sja1105_mgmt_route_commit(struct sja1105_spi_setup *spi_setup,
                          struct sja1105_mgmt_entry *entry,
                          int read_or_write,
                          int index)
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
		 * the mgmt table entry requested for */
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
		sja1105_dyn_l2_lookup_cmd_unpack(packed_buf, &cmd);
		memcpy(entry, &cmd.entry, sizeof(*entry));
	}
out:
	return rc;
}

int sja1105_mgmt_route_get(struct sja1105_spi_setup *spi_setup,
                           struct sja1105_mgmt_entry *entry,
                           int index)
{
	return sja1105_mgmt_route_commit(spi_setup, entry, SPI_READ, index);
}

int sja1105_mgmt_route_set(struct sja1105_spi_setup *spi_setup,
                           struct sja1105_mgmt_entry *entry,
                           int index)
{
	return sja1105_mgmt_route_commit(spi_setup, entry, SPI_WRITE, index);
}
