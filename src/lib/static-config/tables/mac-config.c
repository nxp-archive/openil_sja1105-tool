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

static void
sja1105et_mac_config_entry_access(void *buf,
                                  struct sja1105_mac_config_entry *entry,
                                  int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = SIZE_MAC_CONFIG_ENTRY_ET;
	int    offset;
	int    i;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(entry, 0, sizeof(*entry));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	offset = 72;
	for (i = 0; i < 8; i++) {
		pack_or_unpack(buf, &entry->enabled[i], offset +  0, offset +  0, size);
		pack_or_unpack(buf, &entry->base[i],    offset +  9, offset +  1, size);
		pack_or_unpack(buf, &entry->top[i],     offset + 18, offset + 10, size);
		offset += 19;
	}
	pack_or_unpack(buf, &entry->ifg,         71, 67, size);
	pack_or_unpack(buf, &entry->speed,       66, 65, size);
	pack_or_unpack(buf, &entry->tp_delin,    64, 49, size);
	pack_or_unpack(buf, &entry->tp_delout,   48, 33, size);
	pack_or_unpack(buf, &entry->maxage,      32, 25, size);
	pack_or_unpack(buf, &entry->vlanprio,    24, 22, size);
	pack_or_unpack(buf, &entry->vlanid,      21, 10, size);
	pack_or_unpack(buf, &entry->ing_mirr,     9,  9, size);
	pack_or_unpack(buf, &entry->egr_mirr,     8,  8, size);
	pack_or_unpack(buf, &entry->drpnona664,   7,  7, size);
	pack_or_unpack(buf, &entry->drpdtag,      6,  6, size);
	pack_or_unpack(buf, &entry->drpuntag,     5,  5, size);
	pack_or_unpack(buf, &entry->retag,        4,  4, size);
	pack_or_unpack(buf, &entry->dyn_learn,    3,  3, size);
	pack_or_unpack(buf, &entry->egress,       2,  2, size);
	pack_or_unpack(buf, &entry->ingress,      1,  1, size);
}

static void
sja1105pqrs_mac_config_entry_access(void *buf,
                                    struct sja1105_mac_config_entry *entry,
                                    int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = SIZE_MAC_CONFIG_ENTRY_PQRS;
	int    offset;
	int    i;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(entry, 0, sizeof(*entry));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	offset = 104;
	for (i = 0; i < 8; i++) {
		pack_or_unpack(buf, &entry->enabled[i], offset +  0, offset +  0, size);
		pack_or_unpack(buf, &entry->base[i],    offset +  9, offset +  1, size);
		pack_or_unpack(buf, &entry->top[i],     offset + 18, offset + 10, size);
		offset += 19;
	}
	pack_or_unpack(buf, &entry->ifg,        103, 99, size);
	pack_or_unpack(buf, &entry->speed,       98, 97, size);
	pack_or_unpack(buf, &entry->tp_delin,    96, 81, size);
	pack_or_unpack(buf, &entry->tp_delout,   80, 65, size);
	pack_or_unpack(buf, &entry->maxage,      64, 57, size);
	pack_or_unpack(buf, &entry->vlanprio,    56, 54, size);
	pack_or_unpack(buf, &entry->vlanid,      53, 42, size);
	pack_or_unpack(buf, &entry->ing_mirr,    41, 41, size);
	pack_or_unpack(buf, &entry->egr_mirr,    40, 40, size);
	pack_or_unpack(buf, &entry->drpnona664,  39, 39, size);
	pack_or_unpack(buf, &entry->drpdtag,     38, 38, size);
	pack_or_unpack(buf, &entry->drpsotag,    37, 37, size);
	pack_or_unpack(buf, &entry->drpsitag,    36, 36, size);
	pack_or_unpack(buf, &entry->drpuntag,    35, 35, size);
	pack_or_unpack(buf, &entry->retag,       34, 34, size);
	pack_or_unpack(buf, &entry->dyn_learn,   33, 33, size);
	pack_or_unpack(buf, &entry->egress,      32, 32, size);
	pack_or_unpack(buf, &entry->ingress,     31, 31, size);
	pack_or_unpack(buf, &entry->mirrcie,     30, 30, size);
	pack_or_unpack(buf, &entry->mirrcetag,   29, 29, size);
	pack_or_unpack(buf, &entry->ingmirrvid,  28, 17, size);
	pack_or_unpack(buf, &entry->ingmirrpcp,  16, 14, size);
	pack_or_unpack(buf, &entry->ingmirrdei,  13, 13, size);
}
/*
 * sja1105et_mac_config_entry_pack
 * sja1105et_mac_config_entry_unpack
 * sja1105pqrs_mac_config_entry_pack
 * sja1105pqrs_mac_config_entry_unpack
 */
DEFINE_SEPARATE_PACK_UNPACK_ACCESSORS(mac_config);

void
sja1105_mac_config_entry_fmt_show(
		char *print_buf,
		size_t len,
		char *fmt,
		struct sja1105_mac_config_entry *entry)
{
	char    base_buf[MAX_LINE_SIZE];
	char     top_buf[MAX_LINE_SIZE];
	char enabled_buf[MAX_LINE_SIZE];

	print_array(base_buf,    entry->base, 8);
	print_array(top_buf,     entry->top, 8);
	print_array(enabled_buf, entry->enabled, 8);
	/* We have to compromise by keeping the device_id out of the prototype
	 * definition of this function. It is therefore preferable to see a few
	 * extra zero-valued fields on the E/T rather than not see the values at
	 * all on the P/Q/R/S.
	 */
	formatted_append(print_buf, len, fmt, "BASE      %s", base_buf);
	formatted_append(print_buf, len, fmt, "TOP       %s", top_buf);
	formatted_append(print_buf, len, fmt, "ENABLED   %s", enabled_buf);
	formatted_append(print_buf, len, fmt, "IFG        0x%" PRIX64, entry->ifg);
	formatted_append(print_buf, len, fmt, "SPEED      0x%" PRIX64, entry->speed);
	formatted_append(print_buf, len, fmt, "TP_DELIN   0x%" PRIX64, entry->tp_delin);
	formatted_append(print_buf, len, fmt, "TP_DELOUT  0x%" PRIX64, entry->tp_delout);
	formatted_append(print_buf, len, fmt, "MAXAGE     0x%" PRIX64, entry->maxage);
	formatted_append(print_buf, len, fmt, "VLANPRIO   0x%" PRIX64, entry->vlanprio);
	formatted_append(print_buf, len, fmt, "VLANID     0x%" PRIX64, entry->vlanid);
	formatted_append(print_buf, len, fmt, "ING_MIRR   0x%" PRIX64, entry->ing_mirr);
	formatted_append(print_buf, len, fmt, "EGR_MIRR   0x%" PRIX64, entry->egr_mirr);
	formatted_append(print_buf, len, fmt, "DRPNONA664 0x%" PRIX64, entry->drpnona664);
	formatted_append(print_buf, len, fmt, "DRPDTAG    0x%" PRIX64, entry->drpdtag);
	formatted_append(print_buf, len, fmt, "DRPUNTAG   0x%" PRIX64, entry->drpuntag);
	formatted_append(print_buf, len, fmt, "DRPSOTAG   0x%" PRIX64, entry->drpsotag);
	formatted_append(print_buf, len, fmt, "DRPSITAG   0x%" PRIX64, entry->drpsitag);
	formatted_append(print_buf, len, fmt, "RETAG      0x%" PRIX64, entry->retag);
	formatted_append(print_buf, len, fmt, "DYN_LEARN  0x%" PRIX64, entry->dyn_learn);
	formatted_append(print_buf, len, fmt, "EGRESS     0x%" PRIX64, entry->egress);
	formatted_append(print_buf, len, fmt, "INGRESS    0x%" PRIX64, entry->ingress);
	formatted_append(print_buf, len, fmt, "MIRRCIE    0x%" PRIX64, entry->mirrcie);
	formatted_append(print_buf, len, fmt, "MIRRCETAG  0x%" PRIX64, entry->mirrcetag);
	formatted_append(print_buf, len, fmt, "INGMIRRVID 0x%" PRIX64, entry->ingmirrvid);
	formatted_append(print_buf, len, fmt, "INGMIRRPCP 0x%" PRIX64, entry->ingmirrpcp);
	formatted_append(print_buf, len, fmt, "INGMIRRDEI 0x%" PRIX64, entry->ingmirrdei);
}

void sja1105_mac_config_entry_show(struct sja1105_mac_config_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_mac_config_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

