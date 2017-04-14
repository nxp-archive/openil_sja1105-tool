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
#include "internal.h"

static void sja1105_vl_policing_entry_access(
		void *buf,
		struct sja1105_vl_policing_entry *entry,
		int write)
{
	int  (*get_or_set)(void*, uint64_t*, int, int, int);
	int    size = SIZE_VL_POLICING_ENTRY;

	if (write == 0) {
		get_or_set = generic_table_field_get;
		memset(entry, 0, sizeof(*entry));
	} else {
		get_or_set = generic_table_field_set;
		memset(buf, 0, size);
	}
	get_or_set(buf, &entry->type,      63, 58, size);
	get_or_set(buf, &entry->maxlen,    62, 52, size);
	get_or_set(buf, &entry->sharindx,  51, 42, size);
	loge("ALA");
	if (entry->type == 0) {
		loge("BALA");
		get_or_set(buf, &entry->bag,    41, 28, size);
		get_or_set(buf, &entry->jitter, 27, 18, size);
	}
}

void sja1105_vl_policing_entry_set(
		void *buf,
		struct sja1105_vl_policing_entry *entry)
{
	sja1105_vl_policing_entry_access(buf, entry, 1);
}

void sja1105_vl_policing_entry_get(
		void *buf,
		struct sja1105_vl_policing_entry *entry)
{
	sja1105_vl_policing_entry_access(buf, entry, 0);
}

void sja1105_vl_policing_entry_fmt_show(
		char *print_buf,
		char *fmt,
		struct sja1105_vl_policing_entry *entry)
{
	formatted_append(print_buf, fmt, "TYPE      0x%" PRIX64, entry->type);
	formatted_append(print_buf, fmt, "MAXLEN    0x%" PRIX64, entry->maxlen);
	formatted_append(print_buf, fmt, "SHARINDX  0x%" PRIX64, entry->sharindx);
	if (entry->type == 0) {
		formatted_append(print_buf, fmt, "BAG     0x%" PRIX64, entry->bag);
		formatted_append(print_buf, fmt, "JITTER  0x%" PRIX64, entry->jitter);
	}
}

void sja1105_vl_policing_entry_show(struct sja1105_vl_policing_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_vl_policing_entry_fmt_show(print_buf, fmt, entry);
	fprintf(stdout, print_buf);
}
