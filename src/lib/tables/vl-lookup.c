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
#include <lib/include/config.h>
#include <lib/include/gtable.h>
#include <lib/helpers.h>
#include <common.h>

static void sja1105_vl_lookup_entry_access(
		void *buf,
		struct sja1105_vl_lookup_entry *entry,
		int write)
{
	int  (*get_or_set)(void*, uint64_t*, int, int, int);
	int    size = SIZE_VL_LOOKUP_ENTRY;

	if (write == 0) {
		get_or_set = generic_table_field_get;
		memset(entry, 0, sizeof(*entry));
	} else {
		get_or_set = generic_table_field_set;
		memset(buf, 0, size);
	}
	if (entry->format == 0) {
		logv("Interpreting vllupformat as 0");
		get_or_set(buf, &entry->destports,  95, 91, size);
		get_or_set(buf, &entry->iscritical, 90, 90, size);
		get_or_set(buf, &entry->macaddr,    89, 42, size);
		get_or_set(buf, &entry->vlanid,     41, 30, size);
		get_or_set(buf, &entry->port,       29, 27, size);
		get_or_set(buf, &entry->vlanprior,  26, 24, size);
	} else {
		logv("Interpreting vllupformat as 1");
		get_or_set(buf, &entry->egrmirr,    95, 91, size);
		get_or_set(buf, &entry->ingrmirr,   90, 90, size);
		get_or_set(buf, &entry->vlid,       57, 42, size);
		get_or_set(buf, &entry->port,       29, 27, size);
	}
}

void sja1105_vl_lookup_entry_set(
		void *buf,
		struct sja1105_vl_lookup_entry *entry)
{
	sja1105_vl_lookup_entry_access(buf, entry, 1);
}

void sja1105_vl_lookup_entry_get(
		void *buf,
		struct sja1105_vl_lookup_entry *entry)
{
	sja1105_vl_lookup_entry_access(buf, entry, 0);
}

void sja1105_vl_lookup_entry_fmt_show(
		char *print_buf,
		char *fmt,
		struct sja1105_vl_lookup_entry *entry)
{
	char mac_buf[MAC_ADDR_SIZE];

	if (entry->format == 0) {
		logv("Interpreting vllupformat as 0");
		memset(mac_buf, 0, sizeof(mac_buf));
		mac_addr_sprintf(mac_buf, entry->macaddr);
		formatted_append(print_buf, fmt, "DESTPORTS  0x%" PRIX64, entry->destports);
		formatted_append(print_buf, fmt, "ISCRITICAL 0x%" PRIX64, entry->iscritical);
		formatted_append(print_buf, fmt, "MACADDR    %s", mac_buf);
		formatted_append(print_buf, fmt, "VLANID     0x%" PRIX64, entry->vlanid);
		formatted_append(print_buf, fmt, "PORT       0x%" PRIX64, entry->port);
		formatted_append(print_buf, fmt, "VLANPRIOR  0x%" PRIX64, entry->vlanprior);
	} else {
		logv("Interpreting vllupformat as 1");
		formatted_append(print_buf, fmt, "EGRMIRR    0x%" PRIX64, entry->egrmirr);
		formatted_append(print_buf, fmt, "INGRMIRR   0x%" PRIX64, entry->ingrmirr);
		formatted_append(print_buf, fmt, "VLID       0x%" PRIX64, entry->vlid);
		formatted_append(print_buf, fmt, "PORT       0x%" PRIX64, entry->port);
	}
}

void sja1105_vl_lookup_entry_show(struct sja1105_vl_lookup_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_vl_lookup_entry_fmt_show(print_buf, fmt, entry);
	fprintf(stdout, print_buf);
}

