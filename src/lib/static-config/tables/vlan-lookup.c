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

static void sja1105_vlan_lookup_entry_access(
		void *buf,
		struct sja1105_vlan_lookup_entry *entry,
		int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = SIZE_VLAN_LOOKUP_ENTRY;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(entry, 0, sizeof(*entry));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &entry->ving_mirr,  63, 59, size);
	pack_or_unpack(buf, &entry->vegr_mirr,  58, 54, size);
	pack_or_unpack(buf, &entry->vmemb_port, 53, 49, size);
	pack_or_unpack(buf, &entry->vlan_bc,    48, 44, size);
	pack_or_unpack(buf, &entry->tag_port,   43, 39, size);
	pack_or_unpack(buf, &entry->vlanid,     38, 27, size);
}

/*
 * sja1105_vlan_lookup_entry_pack
 * sja1105_vlan_lookup_entry_unpack
 */
DEFINE_COMMON_PACK_UNPACK_ACCESSORS(vlan_lookup);

void sja1105_vlan_lookup_entry_fmt_show(
		char *print_buf,
		char *fmt,
		struct sja1105_vlan_lookup_entry *entry)
{
	formatted_append(print_buf, fmt, "VING_MIRR  0x%" PRIX64, entry->ving_mirr);
	formatted_append(print_buf, fmt, "VEGR_MIRR  0x%" PRIX64, entry->vegr_mirr);
	formatted_append(print_buf, fmt, "VMEMB_PORT 0x%" PRIX64, entry->vmemb_port);
	formatted_append(print_buf, fmt, "VLAN_BC    0x%" PRIX64, entry->vlan_bc);
	formatted_append(print_buf, fmt, "TAG_PORT   0x%" PRIX64, entry->tag_port);
	formatted_append(print_buf, fmt, "VLANID     0x%" PRIX64, entry->vlanid);
}

void sja1105_vlan_lookup_entry_show(struct sja1105_vlan_lookup_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_vlan_lookup_entry_fmt_show(print_buf, fmt, entry);
	puts(print_buf);
}

