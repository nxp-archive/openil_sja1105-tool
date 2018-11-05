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
#include <lib/helpers.h>
#include <common.h>

static void sja1105_l2_policing_entry_access(
		void *buf,
		struct sja1105_l2_policing_entry *table,
		int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = SIZE_L2_POLICING_ENTRY;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(table, 0, sizeof(*table));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &table->sharindx,  63, 58, size);
	pack_or_unpack(buf, &table->smax,      57, 42, size);
	pack_or_unpack(buf, &table->rate,      41, 26, size);
	pack_or_unpack(buf, &table->maxlen,    25, 15, size);
	pack_or_unpack(buf, &table->partition, 14, 12, size);
}
/*
 * sja1105_l2_policing_entry_pack
 * sja1105_l2_policing_entry_unpack
 */
DEFINE_COMMON_PACK_UNPACK_ACCESSORS(l2_policing);

void sja1105_l2_policing_entry_fmt_show(
		char *print_buf,
		size_t len,
		char *fmt,
		struct sja1105_l2_policing_entry *table)
{
	formatted_append(print_buf, len, fmt, "SHARINDX  0x%" PRIX64, table->sharindx);
	formatted_append(print_buf, len, fmt, "SMAX      0x%" PRIX64, table->smax);
	formatted_append(print_buf, len, fmt, "RATE      0x%" PRIX64, table->rate);
	formatted_append(print_buf, len, fmt, "MAXLEN    0x%" PRIX64, table->maxlen);
	formatted_append(print_buf, len, fmt, "PARTITION 0x%" PRIX64, table->partition);
}

