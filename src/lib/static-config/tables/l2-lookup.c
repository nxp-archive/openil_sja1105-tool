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

void sja1105et_l2_lookup_entry_access(void *buf,
                                      struct sja1105_l2_lookup_entry *entry,
                                      int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = SIZE_L2_LOOKUP_ENTRY_ET;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(entry, 0, sizeof(*entry));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &entry->vlanid,    95, 84, size);
	pack_or_unpack(buf, &entry->macaddr,   83, 36, size);
	pack_or_unpack(buf, &entry->destports, 35, 31, size);
	pack_or_unpack(buf, &entry->enfport,   30, 30, size);
	pack_or_unpack(buf, &entry->index,     29, 20, size);
}

void sja1105pqrs_l2_lookup_entry_access(void *buf,
                                        struct sja1105_l2_lookup_entry *entry,
                                        int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = SIZE_L2_LOOKUP_ENTRY_PQRS;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(entry, 0, sizeof(*entry));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	/* These are static L2 lookup entries, so the structure
	 * should match UM11040 Table 16/17 definitions when
	 * LOCKEDS is 1.
	 */
	pack_or_unpack(buf, &entry->tsreg,        159, 159, size);
	pack_or_unpack(buf, &entry->mirrvlan,     158, 147, size);
	pack_or_unpack(buf, &entry->takets,       146, 146, size);
	pack_or_unpack(buf, &entry->mirr,         145, 145, size);
	pack_or_unpack(buf, &entry->retag,        144, 144, size);
	pack_or_unpack(buf, &entry->mask_iotag,   143, 143, size);
	pack_or_unpack(buf, &entry->mask_vlanid,  142, 131, size);
	pack_or_unpack(buf, &entry->mask_macaddr, 130,  83, size);
	pack_or_unpack(buf, &entry->iotag,         82,  82, size);
	pack_or_unpack(buf, &entry->vlanid,        81,  70, size);
	pack_or_unpack(buf, &entry->macaddr,       69,  22, size);
	pack_or_unpack(buf, &entry->destports,     21,  17, size);
	pack_or_unpack(buf, &entry->enfport,       16,  16, size);
	pack_or_unpack(buf, &entry->index,         15,   6, size);
}

/*
 * sja1105et_l2_lookup_entry_pack
 * sja1105et_l2_lookup_entry_unpack
 * sja1105pqrs_l2_lookup_entry_pack
 * sja1105pqrs_l2_lookup_entry_unpack
 */
DEFINE_SEPARATE_PACK_UNPACK_ACCESSORS(l2_lookup);

/* Common functions */
void sja1105_l2_lookup_entry_fmt_show(
		char *print_buf,
		size_t len,
		char *fmt,
		struct sja1105_l2_lookup_entry *entry)
{
	char mac_buf[MAC_ADDR_SIZE];

	formatted_append(print_buf, len, fmt, "VLANID    0x%" PRIX64, entry->vlanid);
	memset(mac_buf, 0, sizeof(mac_buf));
	mac_addr_sprintf(mac_buf, entry->macaddr);
	formatted_append(print_buf, len, fmt, "MACADDR   %s", mac_buf);
	formatted_append(print_buf, len, fmt, "DESTPORTS 0x%" PRIX64, entry->destports);
	formatted_append(print_buf, len, fmt, "ENFPORT   0x%" PRIX64, entry->enfport);
	formatted_append(print_buf, len, fmt, "INDEX     0x%" PRIX64, entry->index);
}

void sja1105_l2_lookup_entry_show(struct sja1105_l2_lookup_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_l2_lookup_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

