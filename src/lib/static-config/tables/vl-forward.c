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
#include <lib/include/static-config.h>
#include <lib/include/gtable.h>
#include <common.h>

static void sja1105_vl_forwarding_entry_access(
		void *buf,
		struct sja1105_vl_forwarding_entry *entry,
		int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = SIZE_VL_FORWARDING_ENTRY;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(entry, 0, sizeof(*entry));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &entry->type,      31, 31, size);
	pack_or_unpack(buf, &entry->priority,  30, 28, size);
	pack_or_unpack(buf, &entry->partition, 27, 25, size);
	pack_or_unpack(buf, &entry->destports, 24, 20, size);
}
/*
 * sja1105_vl_forwarding_entry_pack
 * sja1105_vl_forwarding_entry_unpack
 */
DEFINE_COMMON_PACK_UNPACK_ACCESSORS(vl_forwarding);

void sja1105_vl_forwarding_entry_fmt_show(
		char *print_buf,
		size_t len,
		char *fmt,
		struct sja1105_vl_forwarding_entry *entry)
{
	formatted_append(print_buf, len, fmt, "TYPE      0x%" PRIX64, entry->type);
	formatted_append(print_buf, len, fmt, "PRIORITY  0x%" PRIX64, entry->priority);
	formatted_append(print_buf, len, fmt, "PARTITION 0x%" PRIX64, entry->partition);
	formatted_append(print_buf, len, fmt, "DESTPORTS 0x%" PRIX64, entry->destports);
}

void sja1105_vl_forwarding_entry_show(struct sja1105_vl_forwarding_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_vl_forwarding_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

