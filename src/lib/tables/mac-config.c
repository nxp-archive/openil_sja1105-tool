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
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* These are our own include files */
#include <lib/include/config.h>
#include <lib/include/gtable.h>
#include <common.h>

static void sja1105_mac_config_entry_access(
		void *buf,
		struct sja1105_mac_config_entry *entry,
		int write)
{
	int  (*get_or_set)(void*, uint64_t*, int, int, int);
	int    size = SIZE_MAC_CONFIG_ENTRY;
	int    offset;
	int    i;

	if (write == 0) {
		get_or_set = generic_table_field_get;
		memset(entry, 0, sizeof(*entry));
	} else {
		get_or_set = generic_table_field_set;
		memset(buf, 0, size);
	}
	offset = 72;
	for (i = 0; i < 8; i++) {
		get_or_set(buf, &entry->enabled[i], offset +  0, offset +  0, size);
		get_or_set(buf, &entry->base[i],    offset +  9, offset +  1, size);
		get_or_set(buf, &entry->top[i],     offset + 18, offset + 10, size);
		offset += 19;
	}
	get_or_set(buf, &entry->ifg,         71, 67, size);
	get_or_set(buf, &entry->speed,       66, 65, size);
	get_or_set(buf, &entry->tp_delin,    64, 49, size);
	get_or_set(buf, &entry->tp_delout,   48, 33, size);
	get_or_set(buf, &entry->maxage,      32, 25, size);
	get_or_set(buf, &entry->vlanprio,    24, 22, size);
	get_or_set(buf, &entry->vlanid,      21, 10, size);
	get_or_set(buf, &entry->ing_mirr,     9,  9, size);
	get_or_set(buf, &entry->egr_mirr,     8,  8, size);
	get_or_set(buf, &entry->drpnona664,   7,  7, size);
	get_or_set(buf, &entry->drpdtag,      6,  6, size);
	get_or_set(buf, &entry->drpuntag,     5,  5, size);
	get_or_set(buf, &entry->retag,        4,  4, size);
	get_or_set(buf, &entry->dyn_learn,    3,  3, size);
	get_or_set(buf, &entry->egress,       2,  2, size);
	get_or_set(buf, &entry->ingress,      1,  1, size);
}

void sja1105_mac_config_entry_set(
		void *buf,
		struct sja1105_mac_config_entry *entry)
{
	sja1105_mac_config_entry_access(buf, entry, 1);
}

void sja1105_mac_config_entry_get(
		void *buf,
		struct sja1105_mac_config_entry *entry)
{
	sja1105_mac_config_entry_access(buf, entry, 0);
}

void sja1105_mac_config_entry_fmt_show(
		char *print_buf,
		char *fmt,
		struct sja1105_mac_config_entry *entry)
{
	char    base_buf[MAX_LINE_SIZE];
	char     top_buf[MAX_LINE_SIZE];
	char enabled_buf[MAX_LINE_SIZE];

	print_array(base_buf,    entry->base, 8);
	print_array(top_buf,     entry->top, 8);
	print_array(enabled_buf, entry->enabled, 8);
	formatted_append(print_buf, fmt, "BASE      %s", base_buf);
	formatted_append(print_buf, fmt, "TOP       %s", top_buf);
	formatted_append(print_buf, fmt, "ENABLED   %s", enabled_buf);
	formatted_append(print_buf, fmt, "IFG        0x%" PRIX64, entry->ifg);
	formatted_append(print_buf, fmt, "SPEED      0x%" PRIX64, entry->speed);
	formatted_append(print_buf, fmt, "TP_DELIN   0x%" PRIX64, entry->tp_delin);
	formatted_append(print_buf, fmt, "TP_DELOUT  0x%" PRIX64, entry->tp_delout);
	formatted_append(print_buf, fmt, "MAXAGE     0x%" PRIX64, entry->maxage);
	formatted_append(print_buf, fmt, "VLANPRIO   0x%" PRIX64, entry->vlanprio);
	formatted_append(print_buf, fmt, "VLANID     0x%" PRIX64, entry->vlanid);
	formatted_append(print_buf, fmt, "ING_MIRR   0x%" PRIX64, entry->ing_mirr);
	formatted_append(print_buf, fmt, "EGR_MIRR   0x%" PRIX64, entry->egr_mirr);
	formatted_append(print_buf, fmt, "DRPNONA664 0x%" PRIX64, entry->drpnona664);
	formatted_append(print_buf, fmt, "DRPDTAG    0x%" PRIX64, entry->drpdtag);
	formatted_append(print_buf, fmt, "DRPUNTAG   0x%" PRIX64, entry->drpuntag);
	formatted_append(print_buf, fmt, "RETAG      0x%" PRIX64, entry->retag);
	formatted_append(print_buf, fmt, "DYN_LEARN  0x%" PRIX64, entry->dyn_learn);
	formatted_append(print_buf, fmt, "EGRESS     0x%" PRIX64, entry->egress);
	formatted_append(print_buf, fmt, "INGRESS    0x%" PRIX64, entry->ingress);
}

void sja1105_mac_config_entry_show(struct sja1105_mac_config_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_mac_config_entry_fmt_show(print_buf, fmt, entry);
	fprintf(stdout, print_buf);
}

