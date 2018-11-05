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

static void sja1105_schedule_entry_access(
		void *buf,
		struct sja1105_schedule_entry *entry,
		int write)
{
	int (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int size = SIZE_SCHEDULE_ENTRY;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(entry, 0, sizeof(*entry));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &entry->winstindex,  63, 54, size);
	pack_or_unpack(buf, &entry->winend,      53, 53, size);
	pack_or_unpack(buf, &entry->winst,       52, 52, size);
	pack_or_unpack(buf, &entry->destports,   51, 47, size);
	pack_or_unpack(buf, &entry->setvalid,    46, 46, size);
	pack_or_unpack(buf, &entry->txen,        45, 45, size);
	pack_or_unpack(buf, &entry->resmedia_en, 44, 44, size);
	pack_or_unpack(buf, &entry->resmedia,    43, 36, size);
	pack_or_unpack(buf, &entry->vlindex,     35, 26, size);
	pack_or_unpack(buf, &entry->delta,       25, 8,  size);
}
/*
 * sja1105_schedule_entry_pack
 * sja1105_schedule_entry_unpack
 */
DEFINE_COMMON_PACK_UNPACK_ACCESSORS(schedule);

void sja1105_schedule_entry_fmt_show(
		char *print_buf,
		size_t len,
		char *fmt,
		struct sja1105_schedule_entry *entry)
{
	formatted_append(print_buf, len, fmt, "WINSTINDEX  0x%" PRIX64, entry->winstindex);
	formatted_append(print_buf, len, fmt, "WINEND      0x%" PRIX64, entry->winend);
	formatted_append(print_buf, len, fmt, "WINST       0x%" PRIX64, entry->winst);
	formatted_append(print_buf, len, fmt, "DESTPORTS   0x%" PRIX64, entry->destports);
	formatted_append(print_buf, len, fmt, "SETVALID    0x%" PRIX64, entry->setvalid);
	formatted_append(print_buf, len, fmt, "TXEN        0x%" PRIX64, entry->txen);
	formatted_append(print_buf, len, fmt, "RESMEDIA_EN 0x%" PRIX64, entry->resmedia_en);
	formatted_append(print_buf, len, fmt, "RESMEDIA    0x%" PRIX64, entry->resmedia);
	formatted_append(print_buf, len, fmt, "VLINDEX     0x%" PRIX64, entry->vlindex);
	formatted_append(print_buf, len, fmt, "DELTA       0x%" PRIX64, entry->delta);
}
