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

static void sja1105_l2_forwarding_entry_access(
		void *buf,
		struct sja1105_l2_forwarding_entry *entry,
		int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = SIZE_L2_FORWARDING_ENTRY;
	int    offset;
	int    i;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(entry, 0, sizeof(*entry));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &entry->bc_domain,  63, 59, size);
	pack_or_unpack(buf, &entry->reach_port, 58, 54, size);
	pack_or_unpack(buf, &entry->fl_domain,  53, 49, size);
	offset = 25;
	for (i = 0; i < 8; i++) {
		pack_or_unpack(buf, &entry->vlan_pmap[i], offset + 2, offset + 0, size);
		offset += 3;
	}
}
/*
 * sja1105_l2_forwarding_entry_pack
 * sja1105_l2_forwarding_entry_unpack
 */
DEFINE_COMMON_PACK_UNPACK_ACCESSORS(l2_forwarding);

void sja1105_l2_forwarding_entry_fmt_show(
		char *print_buf,
		char *fmt,
		struct sja1105_l2_forwarding_entry *entry)
{
	char vlan_pmap_buf[MAX_LINE_SIZE];

	print_array(vlan_pmap_buf, entry->vlan_pmap, 8);
	formatted_append(print_buf, fmt, "BC_DOMAIN  0x%" PRIX64, entry->bc_domain);
	formatted_append(print_buf, fmt, "REACH_PORT 0x%" PRIX64, entry->reach_port);
	formatted_append(print_buf, fmt, "FL_DOMAIN  0x%" PRIX64, entry->fl_domain);
	formatted_append(print_buf, fmt, "VLAN_PMAP %s", vlan_pmap_buf);
}

void sja1105_l2_forwarding_entry_show(struct sja1105_l2_forwarding_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_l2_forwarding_entry_fmt_show(print_buf, fmt, entry);
	puts(print_buf);
}

