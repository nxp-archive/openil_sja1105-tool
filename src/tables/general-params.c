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
#include "internal.h"

static void sja1105_general_params_table_access(
		void *buf,
		struct sja1105_general_params_table *table,
		int write)
{
	int  (*get_or_set)(void*, uint64_t*, int, int, int);
	int    size = SIZE_GENERAL_PARAMS_TABLE;

	if (write == 0) {
		get_or_set = generic_table_field_get;
		memset(table, 0, sizeof(*table));
	} else {
		get_or_set = generic_table_field_set;
		memset(buf, 0, size);
	}
	get_or_set(buf, &table->vllupformat, 319, 319, size);
	get_or_set(buf, &table->mirr_ptacu,  318, 318, size);
	get_or_set(buf, &table->switchid,    317, 315, size);
	get_or_set(buf, &table->hostprio,    314, 312, size);
	get_or_set(buf, &table->mac_fltres1, 311, 264, size);
	get_or_set(buf, &table->mac_fltres0, 263, 216, size);
	get_or_set(buf, &table->mac_flt1,    215, 168, size);
	get_or_set(buf, &table->mac_flt0,    167, 120, size);
	get_or_set(buf, &table->incl_srcpt1, 119, 119, size);
	get_or_set(buf, &table->incl_srcpt0, 118, 118, size);
	get_or_set(buf, &table->send_meta1,  117, 117, size);
	get_or_set(buf, &table->send_meta0,  116, 116, size);
	get_or_set(buf, &table->casc_port,   115, 113, size);
	get_or_set(buf, &table->host_port,   112, 110, size);
	get_or_set(buf, &table->mirr_port,   109, 107, size);
	get_or_set(buf, &table->vimarker,    106, 75,  size);
	get_or_set(buf, &table->vimask,      74, 43,   size);
	get_or_set(buf, &table->tpid,        42, 27,   size);
	get_or_set(buf, &table->ignore2stf,  26, 26,   size);
	get_or_set(buf, &table->tpid2,       25, 10,   size);
}

void sja1105_general_params_table_set(void *buf,
                                      struct sja1105_general_params_table *table)
{
	sja1105_general_params_table_access(buf, table, 1);
}

void sja1105_general_params_table_get(void *buf,
                                      struct sja1105_general_params_table *table)
{
	sja1105_general_params_table_access(buf, table, 0);
}

void sja1105_general_params_table_fmt_show(
		char *print_buf,
		char *fmt,
		struct sja1105_general_params_table *table)
{
	char mac_buf[MAC_ADDR_SIZE];

	formatted_append(print_buf, fmt, "VLLUPFORMAT 0x%" PRIX64, table->vllupformat);
	formatted_append(print_buf, fmt, "MIRR_PTACU  0x%" PRIX64, table->mirr_ptacu);
	formatted_append(print_buf, fmt, "SWITCHID    0x%" PRIX64, table->switchid);
	formatted_append(print_buf, fmt, "HOSTPRIO    0x%" PRIX64, table->hostprio);
	memset(mac_buf, 0, sizeof(mac_buf));
	mac_addr_sprintf(mac_buf, table->mac_fltres1);
	formatted_append(print_buf, fmt, "MAC_FLTRES1 %s", mac_buf);
	memset(mac_buf, 0, sizeof(mac_buf));
	mac_addr_sprintf(mac_buf, table->mac_fltres0);
	formatted_append(print_buf, fmt, "MAC_FLTRES0 %s", mac_buf);
	memset(mac_buf, 0, sizeof(mac_buf));
	mac_addr_sprintf(mac_buf, table->mac_flt1);
	formatted_append(print_buf, fmt, "MAC_FLT1    %s", mac_buf);
	memset(mac_buf, 0, sizeof(mac_buf));
	mac_addr_sprintf(mac_buf, table->mac_flt0);
	formatted_append(print_buf, fmt, "MAC_FLT0    %s", mac_buf);
	formatted_append(print_buf, fmt, "INCL_SRCPT1 0x%" PRIX64, table->incl_srcpt1);
	formatted_append(print_buf, fmt, "INCL_SRCPT0 0x%" PRIX64, table->incl_srcpt0);
	formatted_append(print_buf, fmt, "SEND_META1  0x%" PRIX64, table->send_meta1);
	formatted_append(print_buf, fmt, "SEND_META0  0x%" PRIX64, table->send_meta0);
	formatted_append(print_buf, fmt, "CASC_PORT   0x%" PRIX64, table->casc_port);
	formatted_append(print_buf, fmt, "HOST_PORT   0x%" PRIX64, table->host_port);
	formatted_append(print_buf, fmt, "MIRR_PORT   0x%" PRIX64, table->mirr_port);
	formatted_append(print_buf, fmt, "VIMARKER    0x%" PRIX64, table->vimarker);
	formatted_append(print_buf, fmt, "VIMASK      0x%" PRIX64, table->vimask);
	formatted_append(print_buf, fmt, "TPID        0x%" PRIX64, table->tpid);
	formatted_append(print_buf, fmt, "IGNORE2STF  0x%" PRIX64, table->ignore2stf);
	formatted_append(print_buf, fmt, "TPID2       0x%" PRIX64, table->tpid2);
}

void sja1105_general_params_table_show(struct sja1105_general_params_table *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_general_params_table_fmt_show(print_buf, fmt, entry);
	fprintf(stdout, print_buf);
}
