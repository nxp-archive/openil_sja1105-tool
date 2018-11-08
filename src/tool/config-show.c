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
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "internal.h"
/* From libsja1105 */
#include <lib/include/static-config.h>
#include <common.h>

#define DECLARE_TABLE_SHOW_FN(TABLE_NAME, MAX_TABLE_SIZE,                     \
                              STRING_NAME, FMT)                               \
	static int                                                            \
	TABLE_NAME##_table_show(struct sja1105_static_config *config,         \
	                        int index)                                    \
	{                                                                     \
		const int entry_count = config->TABLE_NAME##_count;           \
		char *print_bufs[MAX_TABLE_SIZE];                             \
		int   start = (index == -1) ? 0 : index;                      \
		int   end   = (index == -1) ? entry_count : index + 1;        \
		int   i;                                                      \
		                                                              \
		if (entry_count == 0) {                                       \
			loge(STRING_NAME " is empty");                        \
			return -1;                                            \
		}                                                             \
		if (index < -1 || index >= entry_count) {                     \
			loge("Index out of bounds!");                         \
			loge("Please adjust the entry count of the table:");  \
			loge("* config show <table>[%d to %d]", 0,            \
			     entry_count - 1);                                \
			return -1;                                            \
		}                                                             \
		printf(STRING_NAME ": %d entries\n", entry_count);            \
		for (i = start; i < end; i++) {                               \
			print_bufs[i] = calloc(sizeof(char), MAX_LINE_SIZE);  \
			if (print_bufs[i] == NULL) {                          \
				loge("malloc failed");                        \
				while (i-- > start) {                         \
					free(print_bufs[i]);                  \
				}                                             \
				return -1;                                    \
			}                                                     \
		}                                                             \
		for (i = start; i < end; i++) {                               \
			formatted_append(print_bufs[i], MAX_LINE_SIZE, FMT, "Entry %d:", i); \
			sja1105_##TABLE_NAME##_entry_fmt_show(                \
					print_bufs[i],              \
					(MAX_LINE_SIZE - i),                         \
					FMT,                                  \
					&config-> TABLE_NAME [i]);            \
			formatted_append(print_bufs[i], MAX_LINE_SIZE, FMT, "");             \
		}                                                             \
		show_print_bufs(print_bufs + start, end - start);             \
		for (i = start; i < end; i++) {                               \
			free(print_bufs[i]);                                  \
		}                                                             \
		return 0;                                                     \
	}

DECLARE_TABLE_SHOW_FN(schedule, MAX_SCHEDULE_COUNT, "Schedule Table", "%-30s\n")
DECLARE_TABLE_SHOW_FN(schedule_entry_points, MAX_SCHEDULE_ENTRY_POINTS_COUNT, "Schedule Entry Points Table", "%-30s\n")
DECLARE_TABLE_SHOW_FN(l2_lookup, MAX_L2_LOOKUP_COUNT, "L2 Address Lookup Table", "%-30s\n")
DECLARE_TABLE_SHOW_FN(l2_policing, MAX_L2_POLICING_COUNT, "L2 Policing Table", "%-20s\n")
DECLARE_TABLE_SHOW_FN(vlan_lookup, MAX_VLAN_LOOKUP_COUNT, "VLAN Lookup Table", "%-20s\n")
DECLARE_TABLE_SHOW_FN(l2_forwarding, MAX_L2_FORWARDING_COUNT, "L2 Forwarding Table", "%-45s\n")
DECLARE_TABLE_SHOW_FN(mac_config, MAX_MAC_CONFIG_COUNT, "MAC Configuration Table", "%-60s\n")
DECLARE_TABLE_SHOW_FN(schedule_params, MAX_SCHEDULE_PARAMS_COUNT, "Schedule Parameters Table", "%-50s\n")
DECLARE_TABLE_SHOW_FN(schedule_entry_points_params, MAX_SCHEDULE_ENTRY_POINTS_PARAMS_COUNT, "Schedule Entry Points Parameters Table", "%-30s\n")
DECLARE_TABLE_SHOW_FN(l2_lookup_params, MAX_L2_LOOKUP_PARAMS_COUNT, "L2 Address Lookup Parameters Table", "%-30s\n")
DECLARE_TABLE_SHOW_FN(l2_forwarding_params, MAX_L2_FORWARDING_PARAMS_COUNT, "L2 Forwarding Parameters Table", "%-50s\n")
DECLARE_TABLE_SHOW_FN(general_params, MAX_GENERAL_PARAMS_COUNT, "General Parameters Table", "%-30s\n")
DECLARE_TABLE_SHOW_FN(xmii_params, MAX_XMII_PARAMS_COUNT, "xMII Mode Parameters Table", "%-35s\n")
DECLARE_TABLE_SHOW_FN(sgmii, MAX_SGMII_COUNT, "SGMII Table", "%-35s\n")
DECLARE_TABLE_SHOW_FN(vl_lookup, MAX_VL_LOOKUP_COUNT, "Virtual Link Address Lookup Table:", "%-35s\n")
DECLARE_TABLE_SHOW_FN(vl_policing, MAX_VL_POLICING_COUNT, "Virtual Link Policing Table:", "%-35s\n")
DECLARE_TABLE_SHOW_FN(vl_forwarding, MAX_VL_FORWARDING_COUNT, "Virtual Link Forwarding Table", "%-35s\n")
DECLARE_TABLE_SHOW_FN(avb_params, MAX_AVB_PARAMS_COUNT, "Audio/Video Bridging Parameters Table", "%-35s\n")
DECLARE_TABLE_SHOW_FN(vl_forwarding_params, MAX_VL_FORWARDING_PARAMS_COUNT, "Virtual Link Forwarding Parameters Table", "%-50s\n")

static int
retagging_table_show(__attribute__((unused)) struct sja1105_static_config *config,
                     __attribute__((unused)) int index)
{
	logv("Retagging Table unimplemented");
	return 0;
}

static int
clock_sync_params_table_show(__attribute__((unused)) struct sja1105_static_config *config,
                             __attribute__((unused)) int index)
{
	logv("Clock Synchronization Table unimplemented");
	return 0;
}

int
sja1105_staging_area_show(struct sja1105_staging_area *staging_area,
                          char *table_name)
{
	const char *options[] = {
		"schedule-table",
		"schedule-entry-points-table",
		"vl-lookup-table",
		"vl-policing-table",
		"vl-forwarding-table",
		"l2-address-lookup-table",
		"l2-policing-table",
		"vlan-lookup-table",
		"l2-forwarding-table",
		"mac-configuration-table",
		"schedule-parameters-table",
		"schedule-entry-points-parameters-table",
		"vl-forwarding-parameters-table",
		"l2-address-lookup-parameters-table",
		"l2-forwarding-parameters-table",
		"clock-synchronization-parameters-table",
		"avb-parameters-table",
		"general-parameters-table",
		"retagging-table",
		"xmii-mode-parameters-table",
		"sgmii-table",
	};
	int (*next_config_table_show[])(struct sja1105_static_config *, int) = {
		schedule_table_show,
		schedule_entry_points_table_show,
		vl_lookup_table_show,
		vl_policing_table_show,
		vl_forwarding_table_show,
		l2_lookup_table_show,
		l2_policing_table_show,
		vlan_lookup_table_show,
		l2_forwarding_table_show,
		mac_config_table_show,
		schedule_params_table_show,
		schedule_entry_points_params_table_show,
		vl_forwarding_params_table_show,
		l2_lookup_params_table_show,
		l2_forwarding_params_table_show,
		clock_sync_params_table_show,
		avb_params_table_show,
		general_params_table_show,
		retagging_table_show,
		xmii_params_table_show,
		sgmii_table_show,
	};
	struct sja1105_static_config *static_config;
	char *index_ptr;
	uint64_t entry_index_u64;
	int entry_index;
	unsigned int i;
	int rc = 0;

	static_config = &staging_area->static_config;

	if (table_name == NULL || strlen(table_name) == 0) {
		logv("Showing all config tables");
		printf("Device ID is 0x%08" PRIx64 " (%s)\n",
		       static_config->device_id, sja1105_device_id_string_get(
		       static_config->device_id, SJA1105_PART_NR_DONT_CARE));
		for (i = 0; i < ARRAY_SIZE(next_config_table_show); i++) {
			rc = next_config_table_show[i](static_config, -1);
		}
	} else {
		index_ptr = strchr(table_name, '[');
		if (index_ptr == NULL) {
			/* No index specified => show all */
			entry_index = -1;
		} else {
			/* Little trick to reuse the code, since the index
			 * is surrounded by [ ], same as an array would be */
			rc = read_array(index_ptr, &entry_index_u64, 1);
			if (rc < 0) {
				goto out;
			}
			entry_index = (int) entry_index_u64;
			/* Execute string comparison only on the table_name,
			 * but not on the entry index */
			*index_ptr = '\0';
		}
		rc = get_match(table_name, options,
		               ARRAY_SIZE(options));
		if (rc < 0) {
			goto out;
		}
		rc = next_config_table_show[rc](static_config, entry_index);
	}
out:
	return rc;
}

