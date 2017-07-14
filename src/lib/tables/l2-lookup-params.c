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
#include <lib/include/static-config.h>
#include <lib/include/gtable.h>
#include <common.h>

static void sja1105_l2_lookup_params_table_access(
		void *buf,
		struct sja1105_l2_lookup_params_table *table,
		int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = SIZE_L2_LOOKUP_PARAMS_TABLE;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(table, 0, sizeof(*table));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &table->maxage,         31, 17, size);
	pack_or_unpack(buf, &table->dyn_tbsz,       16, 14, size);
	pack_or_unpack(buf, &table->poly,           13,  6, size);
	pack_or_unpack(buf, &table->shared_learn,    5,  5, size);
	pack_or_unpack(buf, &table->no_enf_hostprt,  4,  4, size);
	pack_or_unpack(buf, &table->no_mgmt_learn,   3,  3, size);
}

void sja1105_l2_lookup_params_table_pack(
		void *buf,
		struct sja1105_l2_lookup_params_table *table)
{
	sja1105_l2_lookup_params_table_access(buf, table, 1);
}

void sja1105_l2_lookup_params_table_unpack(
		void *buf,
		struct sja1105_l2_lookup_params_table *table)
{
	sja1105_l2_lookup_params_table_access(buf, table, 0);
}

void sja1105_l2_lookup_params_table_fmt_show(
		char *print_buf,
		char *fmt,
		struct sja1105_l2_lookup_params_table *table)
{
	formatted_append(print_buf, fmt, "MAXAGE         0x%" PRIX64, table->maxage);
	formatted_append(print_buf, fmt, "DYN_TBSZ       0x%" PRIX64, table->dyn_tbsz);
	formatted_append(print_buf, fmt, "POLY           0x%" PRIX64, table->poly);
	formatted_append(print_buf, fmt, "SHARED_LEARN   0x%" PRIX64, table->shared_learn);
	formatted_append(print_buf, fmt, "NO_ENF_HOSTPRT 0x%" PRIX64, table->no_enf_hostprt);
	formatted_append(print_buf, fmt, "NO_MGMT_LEARN  0x%" PRIX64, table->no_mgmt_learn);
}

void sja1105_l2_lookup_params_table_show(struct sja1105_l2_lookup_params_table *table)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_l2_lookup_params_table_fmt_show(print_buf, fmt, table);
	puts(print_buf);
}

