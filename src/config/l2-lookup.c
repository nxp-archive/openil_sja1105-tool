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

static void sja1105_l2_lookup_entry_access(
		void *buf,
		struct sja1105_l2_lookup_entry *entry,
		int write)
{
	int  (*get_or_set)(void*, uint64_t*, int, int, int);
	int    size = SIZE_L2_LOOKUP_ENTRY;

	if (write == 0) {
		get_or_set = generic_table_field_get;
		memset(entry, 0, sizeof(*entry));
	} else {
		get_or_set = generic_table_field_set;
		memset(buf, 0, size);
	}
	get_or_set(buf, &entry->vlanid,    95, 84, size);
	get_or_set(buf, &entry->macaddr,   83, 36, size);
	get_or_set(buf, &entry->destports, 35, 31, size);
	get_or_set(buf, &entry->enfport,   30, 30, size);
	get_or_set(buf, &entry->index,     29, 20, size);
}

void sja1105_l2_lookup_entry_set(
		void *buf,
		struct sja1105_l2_lookup_entry *entry)
{
	sja1105_l2_lookup_entry_access(buf, entry, 1);
}

void sja1105_l2_lookup_entry_get(
		void *buf,
		struct sja1105_l2_lookup_entry *entry)
{
	sja1105_l2_lookup_entry_access(buf, entry, 0);
}

void sja1105_l2_lookup_entry_fmt_show(
		char *print_buf,
		char *fmt,
		struct sja1105_l2_lookup_entry *entry)
{
	formatted_append(print_buf, fmt, "VLANID    0x%" PRIX64, entry->vlanid);
	formatted_append(print_buf, fmt, "MACADDR   0x%" PRIX64, entry->macaddr);
	formatted_append(print_buf, fmt, "DESTPORTS 0x%" PRIX64, entry->destports);
	formatted_append(print_buf, fmt, "ENFPORT   0x%" PRIX64, entry->enfport);
	formatted_append(print_buf, fmt, "INDEX     0x%" PRIX64, entry->index);
}

void sja1105_l2_lookup_entry_show(struct sja1105_l2_lookup_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_l2_lookup_entry_fmt_show(print_buf, fmt, entry);
	fprintf(stdout, print_buf);
}

