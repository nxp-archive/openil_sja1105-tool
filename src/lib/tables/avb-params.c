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
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* These are our own include files */
#include <lib/include/static-config.h>
#include <lib/include/gtable.h>
#include <lib/helpers.h>
#include <common.h>

static void sja1105_avb_params_table_access(
		void *buf,
		struct sja1105_avb_params_table *table,
		int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = SIZE_AVB_PARAMS_TABLE;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(table, 0, sizeof(*table));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &table->destmeta, 95, 48, size);
	pack_or_unpack(buf, &table->srcmeta,  47,  0, size);
}

void sja1105_avb_params_table_pack(void *buf,
                                  struct sja1105_avb_params_table *table)
{
	sja1105_avb_params_table_access(buf, table, 1);
}

void sja1105_avb_params_table_unpack(void *buf,
                                  struct sja1105_avb_params_table *table)
{
	sja1105_avb_params_table_access(buf, table, 0);
}

void sja1105_avb_params_table_fmt_show(
		char *print_buf,
		char *fmt,
		struct sja1105_avb_params_table *table)
{
	char mac_buf[MAC_ADDR_SIZE];

	memset(mac_buf, 0, sizeof(mac_buf));
	mac_addr_sprintf(mac_buf, table->destmeta);
	formatted_append(print_buf, fmt, "DESTMETA %s", mac_buf);
	memset(mac_buf, 0, sizeof(mac_buf));
	mac_addr_sprintf(mac_buf, table->srcmeta);
	formatted_append(print_buf, fmt, "SRCMETA  %s", mac_buf);
}

void sja1105_avb_params_table_show(struct sja1105_avb_params_table *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_avb_params_table_fmt_show(print_buf, fmt, entry);
	puts(print_buf);
}
