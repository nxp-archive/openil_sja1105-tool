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

static void sja1105_xmii_params_table_access(
		void *buf,
		struct sja1105_xmii_params_table *table,
		int write)
{
	int  (*get_or_set)(void*, uint64_t*, int, int, int);
	int    size = SIZE_XMII_MODE_PARAMS_TABLE;
	int    offset;
	int    i;

	if (write == 0) {
		get_or_set = generic_table_field_get;
		memset(table, 0, sizeof(*table));
	} else {
		get_or_set = generic_table_field_set;
		memset(buf, 0, size);
	}
	offset = 17;
	for (i = 0; i < 5; i++) {
		get_or_set(buf, &table->xmii_mode[i], offset + 1, offset + 0, size);
		get_or_set(buf, &table->phy_mac[i],   offset + 2, offset + 2, size);
		offset += 3;
	}
}

void sja1105_xmii_params_table_set(
		void *buf,
		struct sja1105_xmii_params_table *table)
{
	sja1105_xmii_params_table_access(buf, table, 1);
}

void sja1105_xmii_params_table_get(
		void *buf,
		struct sja1105_xmii_params_table *table)
{
	sja1105_xmii_params_table_access(buf, table, 0);
}

void sja1105_xmii_params_table_fmt_show(
		char *print_buf,
		char *fmt,
		struct sja1105_xmii_params_table *table)
{
	char phy_mac_buf[MAX_LINE_SIZE];
	char xmii_mode_buf[MAX_LINE_SIZE];
	print_array(phy_mac_buf, table->phy_mac, 5);
	print_array(xmii_mode_buf, table->xmii_mode, 5);
	formatted_append(print_buf, fmt, "PHY_MAC   %s", phy_mac_buf);
	formatted_append(print_buf, fmt, "xMII_MODE %s", xmii_mode_buf);
}

void sja1105_xmii_params_table_show(struct sja1105_xmii_params_table *table)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_xmii_params_table_fmt_show(print_buf, fmt, table);
	fprintf(stdout, print_buf);
}

