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

static void schedule_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_SCHEDULE_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_SCHEDULE_COUNT];
	char *fmt = "%-30s\n";
	int   i;

	printf("Schedule Table: %d entries\n", config->schedule_count);
	for (i = 0; i < config->schedule_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_schedule_entry_fmt_show(tmp_buf[i], fmt, &config->schedule[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->schedule_count);
}

static void schedule_entry_points_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_SCHEDULE_ENTRY_POINTS_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_SCHEDULE_ENTRY_POINTS_COUNT];
	char *fmt = "%-30s\n";
	int   i;

	printf("Schedule Entry Points Table: %d entries\n", config->schedule_entry_points_count);
	for (i = 0; i < config->schedule_entry_points_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_schedule_entry_points_entry_fmt_show(
				tmp_buf[i], fmt, &config->schedule_entry_points[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->schedule_entry_points_count);
}

static void l2_lookup_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_L2_LOOKUP_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_L2_LOOKUP_COUNT];
	char *fmt = "%-30s\n";
	int   i;

	printf("L2 Address Lookup Table: %d entries\n", config->l2_lookup_count);
	for (i = 0; i < config->l2_lookup_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_l2_lookup_entry_fmt_show(tmp_buf[i], fmt, &config->l2_lookup[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->l2_lookup_count);
}

static void l2_policing_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_L2_POLICING_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_L2_POLICING_COUNT];
	char *fmt = "%-20s\n";
	int   i;

	printf("L2 Policing Table: %d entries\n", config->l2_policing_count);
	for (i = 0; i < config->l2_policing_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_l2_policing_entry_fmt_show(tmp_buf[i], fmt, &config->l2_policing[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->l2_policing_count);
}

static void vlan_lookup_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_VLAN_LOOKUP_COUNT][MAX_LINE_SIZE / 4];
	char *print_bufs[MAX_VLAN_LOOKUP_COUNT];
	char *fmt = "%-30s\n";
	int   i;

	printf("VLAN Lookup Table: %d entries\n", config->vlan_lookup_count);
	for (i = 0; i < config->vlan_lookup_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE / 4);
		formatted_append(tmp_buf[i], fmt, "Entry %d", i);
		sja1105_vlan_lookup_entry_fmt_show(tmp_buf[i], fmt, &config->vlan_lookup[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->vlan_lookup_count);
}

static void l2_fw_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_L2_FORWARDING_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_L2_FORWARDING_COUNT];
	char *fmt = "%-45s\n";
	int   i;

	printf("L2 Forwarding Table: %d entries\n", config->l2_forwarding_count);
	for (i = 0; i < config->l2_forwarding_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_l2_forwarding_entry_fmt_show(tmp_buf[i], fmt, &config->l2_forwarding[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->l2_forwarding_count);
}

static void mac_config_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_MAC_CONFIG_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_MAC_CONFIG_COUNT];
	char *fmt = "%-60s\n";
	int   i;

	printf("MAC Configuration Table: %d entries\n", config->mac_config_count);
	for (i = 0; i < config->mac_config_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_mac_config_entry_fmt_show(tmp_buf[i], fmt, &config->mac_config[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->mac_config_count);
}

static void schedule_params_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_SCHEDULE_PARAMS_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_SCHEDULE_PARAMS_COUNT];
	char *fmt = "%-50s\n";
	int   i;

	printf("Schedule Parameters Table: %d entries\n", config->schedule_params_count);
	for (i = 0; i < config->schedule_params_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_schedule_params_entry_fmt_show(tmp_buf[i], fmt, &config->schedule_params[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->schedule_params_count);
}

static void schedule_entry_points_params_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_SCHEDULE_ENTRY_POINTS_PARAMS_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_SCHEDULE_ENTRY_POINTS_PARAMS_COUNT];
	char *fmt = "%-30s\n";
	int   i;

	printf("Schedule Entry Points Parameters Table: %d entries\n", config->schedule_entry_points_params_count);
	for (i = 0; i < config->schedule_entry_points_params_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_schedule_entry_points_params_fmt_show(
				tmp_buf[i], fmt, &config->schedule_entry_points_params[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->schedule_entry_points_params_count);
}

static void l2_lookup_params_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_L2_LOOKUP_PARAMS_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_L2_LOOKUP_PARAMS_COUNT];
	char *fmt = "%-30s\n";
	int   i;

	printf("L2 Address Lookup Parameters Table: %d entries\n", config->l2_lookup_params_count);
	for (i = 0; i < config->l2_lookup_params_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_l2_lookup_params_table_fmt_show(
				tmp_buf[i], fmt, &config->l2_lookup_params[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->l2_lookup_params_count);
}

static void l2_fw_params_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_L2_FORWARDING_PARAMS_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_L2_FORWARDING_PARAMS_COUNT];
	char *fmt = "%-50s\n";
	int   i;

	printf("L2 Forwarding Parameters Table: %d entries\n", config->l2_forwarding_params_count);
	for (i = 0; i < config->l2_forwarding_params_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_l2_forwarding_params_table_fmt_show(
				tmp_buf[i], fmt, &config->l2_forwarding_params[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->l2_forwarding_params_count);
}

static void general_params_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_GENERAL_PARAMS_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_GENERAL_PARAMS_COUNT];
	char *fmt = "%-30s\n";
	int   i;

	printf("General Parameters Table: %d entries\n", config->general_params_count);
	for (i = 0; i < config->general_params_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_general_params_table_fmt_show(
				tmp_buf[i], fmt, &config->general_params[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->general_params_count);
}

static void xmii_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_XMII_PARAMS_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_XMII_PARAMS_COUNT];
	char *fmt = "%-35s\n";
	int   i;

	printf("xMII Mode Parameters Table: %d entries\n", config->xmii_params_count);
	for (i = 0; i < config->xmii_params_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_xmii_params_table_fmt_show(
				tmp_buf[i], fmt, &config->xmii_params[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->xmii_params_count);
}

static void vl_lookup_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_VL_LOOKUP_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_VL_LOOKUP_COUNT];
	char *fmt = "%-35s\n";
	int   i;

	printf("VL Lookup Table: %d entries\n", config->vl_lookup_count);
	for (i = 0; i < config->vl_lookup_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_vl_lookup_entry_fmt_show(
				tmp_buf[i], fmt, &config->vl_lookup[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->vl_lookup_count);
}

static void vl_policing_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_VL_POLICING_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_VL_POLICING_COUNT];
	char *fmt = "%-35s\n";
	int   i;

	printf("VL Policing Table: %d entries\n", config->vl_policing_count);
	for (i = 0; i < config->vl_policing_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_vl_policing_entry_fmt_show(
				tmp_buf[i], fmt, &config->vl_policing[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->vl_policing_count);
}

static void vl_fw_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_VL_FORWARDING_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_VL_FORWARDING_COUNT];
	char *fmt = "%-35s\n";
	int   i;

	printf("VL Forwarding Table: %d entries\n", config->vl_forwarding_count);
	for (i = 0; i < config->vl_forwarding_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_vl_forwarding_entry_fmt_show(
				tmp_buf[i], fmt, &config->vl_forwarding[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->vl_forwarding_count);
}

static void retagging_table_show(__attribute__((unused)) struct sja1105_static_config *config)
{
	loge("Retagging Table unimplemented");
}

static void avb_params_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_AVB_PARAMS_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_AVB_PARAMS_COUNT];
	char *fmt = "%-35s\n";
	int   i;

	printf("AVB Parameters Table: %d entries\n", config->avb_params_count);
	for (i = 0; i < config->avb_params_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_avb_params_table_fmt_show(
				tmp_buf[i], fmt, &config->avb_params[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->avb_params_count);
}

static void clock_sync_params_table_show(__attribute__((unused)) struct sja1105_static_config *config)
{
	loge("Clock Synchronization Table unimplemented");
}

static void vl_fw_params_table_show(struct sja1105_static_config *config)
{
	char  tmp_buf[MAX_VL_FORWARDING_PARAMS_COUNT][MAX_LINE_SIZE];
	char *print_bufs[MAX_VL_FORWARDING_PARAMS_COUNT];
	char *fmt = "%-35s\n";
	int   i;

	printf("VL Forwarding Parameters Table: %d entries\n", config->vl_forwarding_params_count);
	for (i = 0; i < config->vl_forwarding_params_count; i++) {
		memset(tmp_buf[i], 0, MAX_LINE_SIZE);
		formatted_append(tmp_buf[i], fmt, "Entry %d:", i);
		sja1105_vl_forwarding_params_table_fmt_show(
				tmp_buf[i], fmt,
				&config->vl_forwarding_params_table[i]);
		formatted_append(tmp_buf[i], fmt, "");
		print_bufs[i] = tmp_buf[i];
	}
	show_print_bufs(print_bufs, config->vl_forwarding_params_count);
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
	};
	void (*next_config_table_show[])(struct sja1105_static_config *) = {
		schedule_table_show,
		schedule_entry_points_table_show,
		vl_lookup_table_show,
		vl_policing_table_show,
		vl_fw_table_show,
		l2_lookup_table_show,
		l2_policing_table_show,
		vlan_lookup_table_show,
		l2_fw_table_show,
		mac_config_table_show,
		schedule_params_table_show,
		schedule_entry_points_params_table_show,
		vl_fw_params_table_show,
		l2_lookup_params_table_show,
		l2_fw_params_table_show,
		clock_sync_params_table_show,
		avb_params_table_show,
		general_params_table_show,
		retagging_table_show,
		xmii_table_show,
	};
	struct sja1105_static_config *static_config;
	unsigned int i;
	int rc = 0;

	static_config = &staging_area->static_config;

	if (table_name == NULL || strlen(table_name) == 0) {
		logv("Showing all config tables");
		for (i = 0; i < ARRAY_SIZE(next_config_table_show); i++) {
			next_config_table_show[i](static_config);
		}
	} else {
		rc = get_match(table_name, options,
		               ARRAY_SIZE(options));
		if (rc < 0) {
			goto out;
		}
		next_config_table_show[rc](static_config);
	}
out:
	return rc;
}

