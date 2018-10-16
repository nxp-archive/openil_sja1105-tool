/******************************************************************************
 * Copyright (c) 2016, NXP Semiconductors
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
#include <lib/include/static-config.h>
#include <lib/include/gtable.h>
#include <common.h>

void sja1105_table_header_access(
		void *buf,
		struct sja1105_table_header *hdr,
		int write)
{
	int (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int size = SIZE_TABLE_HEADER;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(hdr, 0, sizeof(*hdr));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &hdr->block_id, 31, 24, size);
	pack_or_unpack(buf, &hdr->len,      55, 32, size);
	pack_or_unpack(buf, &hdr->crc,      95, 64, size);
}

void sja1105_table_header_unpack(
		void *buf,
		struct sja1105_table_header *hdr)
{
	sja1105_table_header_access(buf, hdr, 0);
}

void sja1105_table_header_pack(
		void *buf,
		struct sja1105_table_header *hdr)
{
	sja1105_table_header_access(buf, hdr, 1);
}

void sja1105_table_header_pack_with_crc(
		void *buf,
		struct sja1105_table_header *hdr)
{
	/* First copy the table as-is, then get the CRC,
	 * and finally re-copy the table with the proper
	 * CRC in place */
	sja1105_table_header_pack(buf, hdr);
	hdr->crc = ether_crc32_le(buf, SIZE_TABLE_HEADER - 4);
	gtable_pack(buf + SIZE_TABLE_HEADER - 4, &hdr->crc, 31, 0, 4);
}

void sja1105_table_header_show(struct sja1105_table_header *hdr)
{
	switch (hdr->block_id) {
		case BLKID_SCHEDULE_TABLE:
			printf("Schedule Table");
			break;
		case BLKID_SCHEDULE_ENTRY_POINTS_TABLE:
			printf("Schedule Entry Points Table");
			break;
		case BLKID_VL_LOOKUP_TABLE:
			printf("VL Lookup Table");
			break;
		case BLKID_VL_POLICING_TABLE:
			printf("VL Policing Table");
			break;
		case BLKID_VL_FORWARDING_TABLE:
			printf("VL Forwarding Table");
			break;
		case BLKID_L2_LOOKUP_TABLE:
			printf("L2 Lookup Table");
			break;
		case BLKID_L2_POLICING_TABLE:
			printf("L2 Policing Table");
			break;
		case BLKID_VLAN_LOOKUP_TABLE:
			printf("VLAN Lookup Table");
			break;
		case BLKID_L2_FORWARDING_TABLE:
			printf("L2 Forwarding Table");
			break;
		case BLKID_MAC_CONFIG_TABLE:
			printf("MAC Configuration Table");
			break;
		case BLKID_SCHEDULE_PARAMS_TABLE:
			printf("Schedule Parameters Table");
			break;
		case BLKID_SCHEDULE_ENTRY_POINTS_PARAMS_TABLE:
			printf("Schedule Entry Points Parameters Table");
			break;
		case BLKID_VL_FORWARDING_PARAMS_TABLE:
			printf("VL Forwarding Parameters Table");
			break;
		case BLKID_L2_LOOKUP_PARAMS_TABLE:
			printf("L2 Lookup Parameters Table");
			break;
		case BLKID_L2_FORWARDING_PARAMS_TABLE:
			printf("L2 Forwarding Parameters Table");
			break;
		case BLKID_CLK_SYNC_PARAMS_TABLE:
			printf("Clock Synchronization Parameters Table");
			break;
		case BLKID_AVB_PARAMS_TABLE:
			printf("AVB Parameters Table");
			break;
		case BLKID_GENERAL_PARAMS_TABLE:
			printf("General Parameters Table");
			break;
		case BLKID_RETAGGING_TABLE:
			printf("Retagging Table");
			break;
		case BLKID_XMII_MODE_PARAMS_TABLE:
			printf("xMII Mode Parameters Table");
			break;
		case BLKID_SGMII_TABLE:
			printf("SGMII Table");
			break;
		default:
			printf("Unknown Table %" PRIX64 " ", hdr->block_id);
	}
	printf(", length %" PRIu64 " bytes (%" PRIu64 " x 32-bit words), CRC %" PRIX64 "\n",
	       hdr->len * 4, hdr->len, hdr->crc);
}
