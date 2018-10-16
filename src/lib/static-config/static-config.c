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

static void sja1105_table_write_crc(char *table_start, char *crc_ptr)
{
	uint64_t computed_crc;
	int len_bytes;

	len_bytes = (int) (crc_ptr - table_start);
	computed_crc = ether_crc32_le(table_start, len_bytes);
	gtable_pack(crc_ptr, &computed_crc, 31, 0, 4);
}

#define CHECK_COUNT(entry_count, max_entry_count, table_name)                 \
{                                                                             \
	if ((entry_count) > (max_entry_count)) {                              \
		printf("There can be no more than %d %s entries "             \
		       "(%d present)\n",  (max_entry_count),                  \
		       (table_name), (entry_count));                          \
		return -1;                                                    \
	}                                                                     \
}

#define POPULATE_CONFIG_TABLE(device, table, buf,                             \
                              max_entry_count, table_name)                    \
{                                                                             \
	int count = config->table##_count;                                    \
	struct sja1105_##table##_entry entry;                                 \
	CHECK_COUNT(count, (max_entry_count), (table_name));                  \
	sja1105##device##_##table##_entry_unpack(buf, &entry);                \
	config->table[count++] = entry;                                       \
	config->table##_count = count;                                        \
}

/* Input: struct sja1105_table_header *hdr
 *        void *buf
 *        config->device_id
 * Output: Depending on the provided table header,
 *         the packed buffer is parsed accordingly and
 *         unpacked into the corresponding
 *         struct sja1105_static_config *config
 *         field.
 */
int sja1105_static_config_add_entry(struct sja1105_table_header *hdr, void *buf,
                                    struct sja1105_static_config *config)
{

	switch (hdr->block_id) {
	case BLKID_SCHEDULE_TABLE:
	{
		POPULATE_CONFIG_TABLE(, schedule, buf, MAX_SCHEDULE_COUNT, "Schedule Table");
		return SIZE_SCHEDULE_ENTRY;
	}
	case BLKID_SCHEDULE_ENTRY_POINTS_TABLE:
	{
		POPULATE_CONFIG_TABLE(, schedule_entry_points, buf, MAX_SCHEDULE_ENTRY_POINTS_COUNT, "Schedule Entry Points");
		return SIZE_SCHEDULE_ENTRY_POINTS_ENTRY;
	}
	case BLKID_VL_LOOKUP_TABLE:
	{
		POPULATE_CONFIG_TABLE(, vl_lookup, buf, MAX_VL_LOOKUP_COUNT, "VL Lookup");
		return SIZE_VL_LOOKUP_ENTRY;
	}
	case BLKID_VL_POLICING_TABLE:
	{
		POPULATE_CONFIG_TABLE(, vl_policing, buf, MAX_VL_POLICING_COUNT, "VL Policing");
		return SIZE_VL_POLICING_ENTRY;
	}
	case BLKID_VL_FORWARDING_TABLE:
	{
		POPULATE_CONFIG_TABLE(, vl_forwarding, buf, MAX_VL_FORWARDING_COUNT, "VL Forwarding");
		return SIZE_VL_FORWARDING_ENTRY;
	}
	case BLKID_L2_LOOKUP_TABLE:
	{
		if (IS_ET(config->device_id)) {
			POPULATE_CONFIG_TABLE(et, l2_lookup, buf, MAX_L2_LOOKUP_COUNT, "L2 Lookup");
			return SIZE_L2_LOOKUP_ENTRY_ET;
		} else {
			POPULATE_CONFIG_TABLE(pqrs, l2_lookup, buf, MAX_L2_LOOKUP_COUNT, "L2 Lookup");
			return SIZE_L2_LOOKUP_ENTRY_PQRS;
		}
	}
	case BLKID_L2_POLICING_TABLE:
	{
		POPULATE_CONFIG_TABLE(, l2_policing, buf, MAX_L2_POLICING_COUNT, "L2 Policing");
		return SIZE_L2_POLICING_ENTRY;
	}
	case BLKID_VLAN_LOOKUP_TABLE:
	{
		POPULATE_CONFIG_TABLE(, vlan_lookup, buf, MAX_VLAN_LOOKUP_COUNT, "VLAN Lookup");
		return SIZE_VLAN_LOOKUP_ENTRY;
	}
	case BLKID_L2_FORWARDING_TABLE:
	{
		POPULATE_CONFIG_TABLE(, l2_forwarding, buf, MAX_L2_FORWARDING_COUNT, "L2 Forwarding");
		return SIZE_L2_FORWARDING_ENTRY;
	}
	case BLKID_MAC_CONFIG_TABLE:
	{
		if (IS_ET(config->device_id)) {
			POPULATE_CONFIG_TABLE(et, mac_config, buf, MAX_MAC_CONFIG_COUNT, "Mac Configuration");
			return SIZE_MAC_CONFIG_ENTRY_ET;
		} else {
			POPULATE_CONFIG_TABLE(pqrs, mac_config, buf, MAX_MAC_CONFIG_COUNT, "Mac Configuration");
			return SIZE_MAC_CONFIG_ENTRY_PQRS;
		}
	}
	case BLKID_SCHEDULE_PARAMS_TABLE:
	{
		POPULATE_CONFIG_TABLE(, schedule_params, buf, MAX_SCHEDULE_PARAMS_COUNT, "Schedule Parameters");
		return SIZE_SCHEDULE_PARAMS_ENTRY;
	}
	case BLKID_SCHEDULE_ENTRY_POINTS_PARAMS_TABLE:
	{
		POPULATE_CONFIG_TABLE(, schedule_entry_points_params, buf, MAX_SCHEDULE_ENTRY_POINTS_PARAMS_COUNT, "Schedule Entry Points Parameters");
		return SIZE_SCHEDULE_ENTRY_POINTS_PARAMS_ENTRY;
	}
	case BLKID_VL_FORWARDING_PARAMS_TABLE:
	{
		POPULATE_CONFIG_TABLE(, vl_forwarding_params, buf, MAX_VL_FORWARDING_PARAMS_COUNT, "VL Forwarding Parameters");
		return SIZE_VL_FORWARDING_PARAMS_ENTRY;
	}
	case BLKID_L2_LOOKUP_PARAMS_TABLE:
	{
		if (IS_ET(config->device_id)) {
			POPULATE_CONFIG_TABLE(et, l2_lookup_params, buf, MAX_L2_LOOKUP_PARAMS_COUNT, "L2 Lookup Parameters");
			return SIZE_L2_LOOKUP_PARAMS_ENTRY_ET;
		} else {
			POPULATE_CONFIG_TABLE(pqrs, l2_lookup_params, buf, MAX_L2_LOOKUP_PARAMS_COUNT, "L2 Lookup Parameters");
			return SIZE_L2_LOOKUP_PARAMS_ENTRY_PQRS;
		}
	}
	case BLKID_L2_FORWARDING_PARAMS_TABLE:
	{
		POPULATE_CONFIG_TABLE(, l2_forwarding_params, buf, MAX_L2_FORWARDING_PARAMS_COUNT, "L2 Forwarding Parameters");
		return SIZE_L2_FORWARDING_PARAMS_ENTRY;
	}
	case BLKID_CLK_SYNC_PARAMS_TABLE:
	{
		logv("Clock Synchronization Parameters Table Unimplemented\n");
		return SIZE_CLK_SYNC_PARAMS_ENTRY;
	}
	case BLKID_AVB_PARAMS_TABLE:
	{
		struct sja1105_avb_params_entry entry;
		CHECK_COUNT(config->avb_params_count, MAX_AVB_PARAMS_COUNT, "AVB Parameters");
		if (IS_ET(config->device_id)) {
			sja1105et_avb_params_entry_unpack(buf, &entry);
			config->avb_params[config->avb_params_count++] = entry;
			return SIZE_AVB_PARAMS_ENTRY_ET;
		} else {
			sja1105pqrs_avb_params_entry_unpack(buf, &entry);
			config->avb_params[config->avb_params_count++] = entry;
			return SIZE_AVB_PARAMS_ENTRY_PQRS;
		}
	}
	case BLKID_GENERAL_PARAMS_TABLE:
	{
		struct sja1105_general_params_entry entry;
		CHECK_COUNT(config->general_params_count, MAX_GENERAL_PARAMS_COUNT, "General Parameters");
		if (IS_ET(config->device_id)) {
			sja1105et_general_params_entry_unpack(buf, &entry);
			config->general_params[config->general_params_count++] = entry;
			return SIZE_GENERAL_PARAMS_ENTRY_ET;
		} else {
			sja1105pqrs_general_params_entry_unpack(buf, &entry);
			config->general_params[config->general_params_count++] = entry;
			return SIZE_GENERAL_PARAMS_ENTRY_PQRS;
		}
	}
	case BLKID_RETAGGING_TABLE:
	{
		logv("Retagging Table Unimplemented\n");
		return SIZE_RETAGGING_ENTRY;
	}
	case BLKID_XMII_MODE_PARAMS_TABLE:
	{
		POPULATE_CONFIG_TABLE(, xmii_params, buf, MAX_XMII_PARAMS_COUNT, "xMII Parameters");
		return SIZE_XMII_MODE_PARAMS_ENTRY;
	}
	case BLKID_SGMII_TABLE:
	{
		POPULATE_CONFIG_TABLE(, sgmii, buf, MAX_SGMII_COUNT, "SGMII Table");
		return SIZE_SGMII_ENTRY;
	}
	default:
		printf("Unknown Table %" PRIX64 "\n", hdr->block_id);
		return -1;
	}
	return 0;
}

/* Returns number of bytes that were dumped
 * (length of static config) */
int sja1105_static_config_hexdump(void *buf)
{
	struct sja1105_table_header hdr;
	struct sja1105_static_config config;
	char *p = buf;
	char *table_end;
	int bytes;

	memset(&config, 0, sizeof(config));
	/* Retrieve device_id from first 4 bytes of packed buffer */
	gtable_unpack(p, &config.device_id, 31, 0, 4);
	printf("Device ID is 0x%08" PRIx64 " (%s)\n",
	       config.device_id, sja1105_device_id_string_get(
	       config.device_id, SJA1105_PART_NR_DONT_CARE));
	p += SIZE_SJA1105_DEVICE_ID;

	while (1) {
		sja1105_table_header_unpack(p, &hdr);
		/* This should match on last table header */
		if (hdr.len == 0) {
			break;
		}
		sja1105_table_header_show(&hdr);
		printf("Header:\n");
		gtable_hexdump(p, SIZE_TABLE_HEADER);
		p += SIZE_TABLE_HEADER;

		table_end = p + hdr.len * 4;
		while (p < table_end) {
			bytes = sja1105_static_config_add_entry(&hdr, p,
			                                        &config);
			if (bytes < 0) {
				goto error;
			}
			printf("Entry (%d bytes):\n", bytes);
			gtable_hexdump(p, bytes);
			p += bytes;
		};
		if (p != table_end) {
			loge("WARNING: Incorrect table length specified in header!");
			printf("Extra:\n");
			gtable_hexdump(p, (ptrdiff_t) (table_end - p));
			p = table_end;
		}
		printf("Table Data CRC:\n");
		gtable_hexdump(p, 4);
		p += 4;
		printf("\n");
	}
	return ((ptrdiff_t) (p - (char*) buf)) * sizeof(*buf);
error:
	return -1;
}

static void
sja1105_static_config_patch_vllupformat(struct sja1105_static_config *config)
{
	int i;

	for (i = 0; i < config->vl_lookup_count; i++) {
		config->vl_lookup[i].format = config->general_params->vllupformat;
	}
}

int
sja1105_static_config_check_memory_size(struct sja1105_static_config *config)
{
	int max_mem;
	int mem = 0;
	int i;

	for (i = 0; i < 8; i++) {
		mem += config->l2_forwarding_params[0].part_spc[i];
	}
	if (config->vl_forwarding_params_count) {
		for (i = 0; i < 8; i++) {
			mem += config->vl_forwarding_params[0].partspc[i];
		}
	}
	if (config->retagging_count > 0) {
		max_mem = MAX_FRAME_MEMORY_RETAGGING;
	} else {
		max_mem = MAX_FRAME_MEMORY;
	}
	if (mem > max_mem) {
		loge("Not allowed to overcommit frame memory. "
		     "This is asking for trouble.");
		loge("L2 memory partitions and VL memory partitions "
		     "share the same space.");
		loge("The sum of all 16 memory partitions is not allowed "
		     "to be larger than %d 128-byte blocks.", max_mem);
		loge("Please adjust l2-forwarding-parameters-table.part_spc "
		     "and/or vl-forwarding-parameters-table.partspc.");
		return -1;
	}
	return 0;
}

int sja1105_static_config_check_valid(struct sja1105_static_config *config)
{
	if (config->schedule_count > 0) {
		if (config->schedule_entry_points_count == 0) {
			loge("schedule-table not empty, but schedule-entry-points-table empty");
			return -1;
		}
		if (config->schedule_params_count != MAX_SCHEDULE_PARAMS_COUNT) {
			loge("schedule-table not empty, but schedule-parameters-table empty");
			return -1;
		}
		if (config->schedule_entry_points_params_count != MAX_SCHEDULE_ENTRY_POINTS_PARAMS_COUNT) {
			loge("schedule-table not empty, but schedule-entry-points-parameters-table empty");
			return -1;
		}
	}
	if (config->vl_lookup_count > 0) {
		if (config->vl_policing_count == 0) {
			loge("vl-lookup-table not empty, but vl-policing-table empty");
			return -1;
		}
		if (config->vl_forwarding_count == 0)  {
			loge("vl-lookup-table not empty, but vl-forwarding-table empty");
			return -1;
		}
		if (config->vl_forwarding_params_count != MAX_VL_FORWARDING_PARAMS_COUNT) {
			loge("vl-forwarding-table not empty, but vl-forwarding-parameters-table empty");
			return -1;
		}
	}
	if (config->l2_policing_count == 0) {
		loge("l2-policing-table empty");
		return -1;
	}
	if (config->vlan_lookup_count == 0) {
		loge("vlan-lookup-table empty");
		return -1;
	}
	if (config->l2_forwarding_count != MAX_L2_FORWARDING_COUNT) {
		loge("l2-forwarding-table does not have %d entries",
		     MAX_L2_FORWARDING_PARAMS_COUNT);
		return -1;
	}
	if (config->mac_config_count != MAX_MAC_CONFIG_COUNT) {
		loge("mac-config-table does not have %d entries",
		     MAX_MAC_CONFIG_COUNT);
		return -1;
	}
	if (config->l2_forwarding_params_count != MAX_L2_FORWARDING_PARAMS_COUNT) {
		loge("l2-forwarding-parameters-table does not have %d entries",
		     MAX_L2_FORWARDING_PARAMS_COUNT);
		return -1;
	}
	if (config->general_params_count != MAX_GENERAL_PARAMS_COUNT) {
		loge("general-parameters-table is empty");
		return -1;
	}
	if (config->xmii_params_count != MAX_XMII_PARAMS_COUNT) {
		loge("xmii-mode-parameters-table is empty");
		return -1;
	}
	return sja1105_static_config_check_memory_size(config);
}

int
sja1105_static_config_unpack(void *buf, struct sja1105_static_config *config)
{
	struct sja1105_table_header hdr;
	char *p = buf;
	char *table_end;
	int bytes;
	uint64_t read_crc;
	uint64_t computed_crc;

	memset(config, 0, sizeof(*config));
	/* Retrieve device_id from first 4 bytes of packed buffer */
	gtable_unpack(p, &config->device_id, 31, 0, 4);
	logv("Device ID is 0x%08" PRIx64 " (%s)",
	     config->device_id, sja1105_device_id_string_get(
	     config->device_id, SJA1105_PART_NR_DONT_CARE));
	if (DEVICE_ID_VALID(config->device_id) == 0) {
		loge("Invalid device id in staging area: 0x%08" PRIx64,
		     config->device_id);
		goto error;
	}
	p += SIZE_SJA1105_DEVICE_ID;

	while (1) {
		sja1105_table_header_unpack(p, &hdr);
		/* This should match on last table header */
		if (hdr.len == 0) {
			break;
		}
		/* Print table header with same verbosity level as "logv" */
		if (SJA1105_VERBOSE_CONDITION) {
			sja1105_table_header_show(&hdr);
		}
		computed_crc = ether_crc32_le(p, SIZE_TABLE_HEADER - 4);
		computed_crc &= 0xFFFFFFFF;
		read_crc = hdr.crc & 0xFFFFFFFF;
		if (read_crc != computed_crc) {
			loge("Table header CRC is invalid, exiting.");
			loge("Read %" PRIX64 ", computed %" PRIX64,
			     read_crc, computed_crc);
			goto error;
		}
		p += SIZE_TABLE_HEADER;

		table_end = p + hdr.len * 4;
		computed_crc = ether_crc32_le(p, hdr.len * 4);
		while (p < table_end) {
			bytes = sja1105_static_config_add_entry(&hdr, p,
			                                        config);
			if (bytes < 0) {
				goto error;
			}
			p += bytes;
		};
		if (p != table_end) {
			loge("WARNING: Incorrect table length for:");
			sja1105_table_header_show(&hdr);
			loge("Table data has %td extra bytes compared to header!",
			     (ptrdiff_t) (table_end - p));
			p = table_end;
		}
		gtable_unpack(p, &read_crc, 31, 0, 4);
		p += 4;
		if (computed_crc != read_crc) {
			loge("Data CRC is invalid, exiting.");
			loge("Read %" PRIX64 ", computed %" PRIX64,
			     read_crc, computed_crc);
			goto error;
		}
	}
	sja1105_static_config_patch_vllupformat(config);
	return 0;
error:
	return -1;
}

int
sja1105_static_config_pack(void *buf, struct sja1105_static_config *config)
{
#define PACK_TABLE_IN_BUF_FN(entry_count, entry_size, blk_id, set_fn, array) \
	if (entry_count) {                                                   \
		header.block_id = (blk_id);                                  \
		header.len = (entry_count) * (entry_size) / 4;               \
		sja1105_table_header_pack_with_crc(p, &header);              \
		p += SIZE_TABLE_HEADER;                                      \
		table_start = p;                                             \
		for (i = 0; i < (entry_count); i++) {                        \
			set_fn(p, &(array)[i]);                              \
			p += (entry_size);                                   \
		}                                                            \
		sja1105_table_write_crc(table_start, p);                     \
		p += 4;                                                      \
	}

	struct sja1105_table_header header = {0};
	char  *p = buf;
	char  *table_start;
	int    i;

	if (!DEVICE_ID_VALID(config->device_id)) {
		loge("Cannot pack invalid Device ID 0x08%"
		     PRIx64 "!", config->device_id);
		return -EINVAL;
	}

	gtable_pack(p, &config->device_id, 31, 0, 4);
	p += SIZE_SJA1105_DEVICE_ID;

	PACK_TABLE_IN_BUF_FN(config->schedule_count,
	                     SIZE_SCHEDULE_ENTRY,
	                     BLKID_SCHEDULE_TABLE,
	                     sja1105_schedule_entry_pack,
	                     config->schedule);
	PACK_TABLE_IN_BUF_FN(config->schedule_entry_points_count,
	                     SIZE_SCHEDULE_ENTRY_POINTS_ENTRY,
	                     BLKID_SCHEDULE_ENTRY_POINTS_TABLE,
	                     sja1105_schedule_entry_points_entry_pack,
	                     config->schedule_entry_points);
	PACK_TABLE_IN_BUF_FN(config->vl_lookup_count,
	                     SIZE_VL_LOOKUP_ENTRY,
	                     BLKID_VL_LOOKUP_TABLE,
	                     sja1105_vl_lookup_entry_pack,
	                     config->vl_lookup);
	PACK_TABLE_IN_BUF_FN(config->vl_policing_count,
	                     SIZE_VL_POLICING_ENTRY,
	                     BLKID_VL_POLICING_TABLE,
	                     sja1105_vl_policing_entry_pack,
	                     config->vl_policing);
	PACK_TABLE_IN_BUF_FN(config->vl_forwarding_count,
	                     SIZE_VL_FORWARDING_ENTRY,
	                     BLKID_VL_FORWARDING_TABLE,
	                     sja1105_vl_forwarding_entry_pack,
	                     config->vl_forwarding);
	if (IS_ET(config->device_id)) {
		PACK_TABLE_IN_BUF_FN(config->l2_lookup_count,
		                     SIZE_L2_LOOKUP_ENTRY_ET,
		                     BLKID_L2_LOOKUP_TABLE,
		                     sja1105et_l2_lookup_entry_pack,
		                     config->l2_lookup);
	} else {
		PACK_TABLE_IN_BUF_FN(config->l2_lookup_count,
		                     SIZE_L2_LOOKUP_ENTRY_PQRS,
		                     BLKID_L2_LOOKUP_TABLE,
		                     sja1105pqrs_l2_lookup_entry_pack,
		                     config->l2_lookup);
	}
	PACK_TABLE_IN_BUF_FN(config->l2_policing_count,
	                     SIZE_L2_POLICING_ENTRY,
	                     BLKID_L2_POLICING_TABLE,
	                     sja1105_l2_policing_entry_pack,
	                     config->l2_policing);
	PACK_TABLE_IN_BUF_FN(config->vlan_lookup_count,
	                     SIZE_VLAN_LOOKUP_ENTRY,
	                     BLKID_VLAN_LOOKUP_TABLE,
	                     sja1105_vlan_lookup_entry_pack,
	                     config->vlan_lookup);
	PACK_TABLE_IN_BUF_FN(config->l2_forwarding_count,
	                     SIZE_L2_FORWARDING_ENTRY,
	                     BLKID_L2_FORWARDING_TABLE,
	                     sja1105_l2_forwarding_entry_pack,
	                     config->l2_forwarding);
	if (IS_ET(config->device_id)) {
		PACK_TABLE_IN_BUF_FN(config->mac_config_count,
		                     SIZE_MAC_CONFIG_ENTRY_ET,
		                     BLKID_MAC_CONFIG_TABLE,
		                     sja1105et_mac_config_entry_pack,
		                     config->mac_config);
	} else {
		PACK_TABLE_IN_BUF_FN(config->mac_config_count,
		                     SIZE_MAC_CONFIG_ENTRY_PQRS,
		                     BLKID_MAC_CONFIG_TABLE,
		                     sja1105pqrs_mac_config_entry_pack,
		                     config->mac_config);
	}
	PACK_TABLE_IN_BUF_FN(config->schedule_params_count,
	                     SIZE_SCHEDULE_PARAMS_ENTRY,
	                     BLKID_SCHEDULE_PARAMS_TABLE,
	                     sja1105_schedule_params_entry_pack,
	                     config->schedule_params);
	PACK_TABLE_IN_BUF_FN(config->schedule_entry_points_params_count,
	                     SIZE_SCHEDULE_ENTRY_POINTS_PARAMS_ENTRY,
	                     BLKID_SCHEDULE_ENTRY_POINTS_PARAMS_TABLE,
	                     sja1105_schedule_entry_points_params_entry_pack,
	                     config->schedule_entry_points_params);
	PACK_TABLE_IN_BUF_FN(config->vl_forwarding_params_count,
	                     SIZE_VL_FORWARDING_PARAMS_ENTRY,
	                     BLKID_VL_FORWARDING_PARAMS_TABLE,
	                     sja1105_vl_forwarding_params_entry_pack,
	                     config->vl_forwarding_params);
	if (IS_ET(config->device_id)) {
		PACK_TABLE_IN_BUF_FN(config->l2_lookup_params_count,
		                     SIZE_L2_LOOKUP_PARAMS_ENTRY_ET,
		                     BLKID_L2_LOOKUP_PARAMS_TABLE,
		                     sja1105et_l2_lookup_params_entry_pack,
		                     config->l2_lookup_params);
	} else {
		PACK_TABLE_IN_BUF_FN(config->l2_lookup_params_count,
		                     SIZE_L2_LOOKUP_PARAMS_ENTRY_PQRS,
		                     BLKID_L2_LOOKUP_PARAMS_TABLE,
		                     sja1105pqrs_l2_lookup_params_entry_pack,
		                     config->l2_lookup_params);
	}
	PACK_TABLE_IN_BUF_FN(config->l2_forwarding_params_count,
	                     SIZE_L2_FORWARDING_PARAMS_ENTRY,
	                     BLKID_L2_FORWARDING_PARAMS_TABLE,
	                     sja1105_l2_forwarding_params_entry_pack,
	                     config->l2_forwarding_params);
	if (IS_ET(config->device_id)) {
		PACK_TABLE_IN_BUF_FN(config->avb_params_count,
		                     SIZE_AVB_PARAMS_ENTRY_ET,
		                     BLKID_AVB_PARAMS_TABLE,
		                     sja1105et_avb_params_entry_pack,
		                     config->avb_params);
	} else {
		PACK_TABLE_IN_BUF_FN(config->avb_params_count,
		                     SIZE_AVB_PARAMS_ENTRY_PQRS,
		                     BLKID_AVB_PARAMS_TABLE,
		                     sja1105pqrs_avb_params_entry_pack,
		                     config->avb_params);
	}
	if (IS_ET(config->device_id)) {
		PACK_TABLE_IN_BUF_FN(config->general_params_count,
		                     SIZE_GENERAL_PARAMS_ENTRY_ET,
		                     BLKID_GENERAL_PARAMS_TABLE,
		                     sja1105et_general_params_entry_pack,
		                     config->general_params);
	} else {
		PACK_TABLE_IN_BUF_FN(config->general_params_count,
		                     SIZE_GENERAL_PARAMS_ENTRY_PQRS,
		                     BLKID_GENERAL_PARAMS_TABLE,
		                     sja1105pqrs_general_params_entry_pack,
		                     config->general_params);
	}
	PACK_TABLE_IN_BUF_FN(config->xmii_params_count,
	                     SIZE_XMII_MODE_PARAMS_ENTRY,
	                     BLKID_XMII_MODE_PARAMS_TABLE,
	                     sja1105_xmii_params_entry_pack,
	                     config->xmii_params);
	PACK_TABLE_IN_BUF_FN(config->sgmii_count,
	                     SIZE_SGMII_ENTRY,
	                     BLKID_SGMII_TABLE,
	                     sja1105_sgmii_entry_pack,
	                     config->sgmii);
	/* Final header */
	header.block_id = 0;      /* Does not matter */
	header.len = 0;           /* Marks that header is final */
	header.crc = 0xDEADBEEF;  /* Will be replaced on-the-fly on "config upload" */
	sja1105_table_header_pack(p, &header);
	return 0;
}

unsigned int
sja1105_static_config_get_length(struct sja1105_static_config *config)
{
	unsigned int sum = 0;
	unsigned int header_count = 0;

	/* Table headers */
	header_count += (config->schedule_count != 0);
	header_count += (config->schedule_entry_points_count != 0);
	header_count += (config->vl_lookup_count != 0);
	header_count += (config->vl_policing_count != 0);
	header_count += (config->vl_forwarding_count != 0);
	header_count += (config->l2_lookup_count != 0);
	header_count += (config->l2_policing_count != 0);
	header_count += (config->vlan_lookup_count != 0);
	header_count += (config->l2_forwarding_count != 0);
	header_count += (config->mac_config_count != 0);
	header_count += (config->schedule_params_count != 0);
	header_count += (config->schedule_entry_points_params_count != 0);
	header_count += (config->vl_forwarding_params_count != 0);
	header_count += (config->l2_lookup_params_count != 0);
	header_count += (config->l2_forwarding_params_count != 0);
	header_count += (config->avb_params_count != 0);
	header_count += (config->general_params_count != 0);
	header_count += (config->xmii_params_count != 0);
	header_count += (config->sgmii_count != 0);
	header_count += 1; /* Ending header */
	sum += SIZE_SJA1105_DEVICE_ID;
	sum += header_count * (SIZE_TABLE_HEADER + 4); /* plus CRC at the end */
	sum += config->schedule_count * SIZE_SCHEDULE_ENTRY;
	sum += config->schedule_entry_points_count * SIZE_SCHEDULE_ENTRY_POINTS_ENTRY;
	sum += config->vl_lookup_count * SIZE_VL_LOOKUP_ENTRY;
	sum += config->vl_policing_count * SIZE_VL_POLICING_ENTRY;
	sum += config->vl_forwarding_count * SIZE_VL_FORWARDING_ENTRY;
	sum += config->l2_lookup_count * (IS_PQRS(config->device_id) ? SIZE_L2_LOOKUP_ENTRY_PQRS : SIZE_L2_LOOKUP_ENTRY_ET);
	sum += config->l2_policing_count * SIZE_L2_POLICING_ENTRY;
	sum += config->vlan_lookup_count * SIZE_VLAN_LOOKUP_ENTRY;
	sum += config->l2_forwarding_count * SIZE_L2_FORWARDING_ENTRY;
	sum += config->mac_config_count * (IS_PQRS(config->device_id) ? SIZE_MAC_CONFIG_ENTRY_PQRS : SIZE_MAC_CONFIG_ENTRY_ET);
	sum += config->schedule_params_count * SIZE_SCHEDULE_PARAMS_ENTRY;
	sum += config->schedule_entry_points_params_count * SIZE_SCHEDULE_ENTRY_POINTS_PARAMS_ENTRY;
	sum += config->vl_forwarding_params_count * SIZE_VL_FORWARDING_PARAMS_ENTRY;
	sum += config->l2_lookup_params_count * (IS_PQRS(config->device_id) ? SIZE_L2_LOOKUP_PARAMS_ENTRY_PQRS : SIZE_L2_LOOKUP_PARAMS_ENTRY_ET);
	sum += config->l2_forwarding_params_count * SIZE_L2_FORWARDING_PARAMS_ENTRY;
	sum += config->avb_params_count * (IS_PQRS(config->device_id) ? SIZE_AVB_PARAMS_ENTRY_PQRS : SIZE_AVB_PARAMS_ENTRY_ET);
	sum += config->general_params_count * (IS_PQRS(config->device_id) ? SIZE_GENERAL_PARAMS_ENTRY_PQRS : SIZE_GENERAL_PARAMS_ENTRY_ET);
	sum += config->xmii_params_count * SIZE_XMII_MODE_PARAMS_ENTRY;
	sum += config->sgmii_count * SIZE_SGMII_ENTRY;
	sum -= 4; /* Last header does not have an extra CRC because there is no data */
	logv("total: %d bytes", sum);
	return sum;
}

