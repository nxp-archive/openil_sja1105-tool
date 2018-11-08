/******************************************************************************
 * Copyright (c) 2018, NXP Semiconductors
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

static void
sja1105_sgmii_entry_access(void *buf,
                           struct sja1105_sgmii_entry *entry,
                           int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = SIZE_SGMII_ENTRY;
	uint64_t tmp;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(entry, 0, sizeof(*entry));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &entry->digital_error_cnt, 1151, 1120, size);
	pack_or_unpack(buf, &entry->digital_control_2, 1119, 1088, size);
	pack_or_unpack(buf, &entry->debug_control,      383,  352, size);
	pack_or_unpack(buf, &entry->test_control,       351,  320, size);
	pack_or_unpack(buf, &entry->autoneg_control,    287,  256, size);
	pack_or_unpack(buf, &entry->digital_control_1,  255,  224, size);
	pack_or_unpack(buf, &entry->autoneg_adv,        223,  192, size);
	pack_or_unpack(buf, &entry->basic_control,      191,  160, size);
	/* Reserved areas */
	if (write == 1) {
		tmp = 0x00000000ull; gtable_pack(buf, &tmp, 1087, 1056, size);
		tmp = 0x00000000ull; gtable_pack(buf, &tmp, 1055, 1024, size);
		tmp = 0x00000000ull; gtable_pack(buf, &tmp, 1023,  992, size);
		tmp = 0x00000100ull; gtable_pack(buf, &tmp,  991,  960, size);
		tmp = 0x0000023Full; gtable_pack(buf, &tmp,  959,  928, size);
		tmp = 0x0000000Aull; gtable_pack(buf, &tmp,  927,  896, size);
		tmp = 0x00001C22ull; gtable_pack(buf, &tmp,  895,  864, size);
		tmp = 0x00000001ull; gtable_pack(buf, &tmp,  863,  832, size);
		tmp = 0x00000003ull; gtable_pack(buf, &tmp,  831,  800, size);
		tmp = 0x00000000ull; gtable_pack(buf, &tmp,  799,  768, size);
		tmp = 0x00000001ull; gtable_pack(buf, &tmp,  767,  736, size);
		tmp = 0x00000005ull; gtable_pack(buf, &tmp,  735,  704, size);
		tmp = 0x00000101ull; gtable_pack(buf, &tmp,  703,  672, size);
		tmp = 0x00000000ull; gtable_pack(buf, &tmp,  671,  640, size);
		tmp = 0x00000001ull; gtable_pack(buf, &tmp,  639,  608, size);
		tmp = 0x00000000ull; gtable_pack(buf, &tmp,  607,  576, size);
		tmp = 0x0000000Aull; gtable_pack(buf, &tmp,  575,  544, size);
		tmp = 0x00000000ull; gtable_pack(buf, &tmp,  543,  512, size);
		tmp = 0x00000000ull; gtable_pack(buf, &tmp,  511,  480, size);
		tmp = 0x00000000ull; gtable_pack(buf, &tmp,  479,  448, size);
		tmp = 0x00000000ull; gtable_pack(buf, &tmp,  447,  416, size);
		tmp = 0x0000899Cull; gtable_pack(buf, &tmp,  415,  384, size);
		tmp = 0x0000000Aull; gtable_pack(buf, &tmp,  319,  288, size);
		tmp = 0x00000004ull; gtable_pack(buf, &tmp,  159,  128, size);
		tmp = 0x00000000ull; gtable_pack(buf, &tmp,  127,   96, size);
		tmp = 0x00000000ull; gtable_pack(buf, &tmp,   95,   64, size);
		tmp = 0x00000000ull; gtable_pack(buf, &tmp,   63,   32, size);
		tmp = 0x00000000ull; gtable_pack(buf, &tmp,   31,    0, size);
	}
}
/*
 * sja1105_sgmii_entry_pack
 * sja1105_sgmii_entry_unpack
 */
DEFINE_COMMON_PACK_UNPACK_ACCESSORS(sgmii);

void
sja1105_sgmii_entry_fmt_show(char *print_buf,
		size_t len,
		char *fmt,
		struct sja1105_sgmii_entry *entry)
{
	formatted_append(print_buf, len, fmt, "DIGITAL_ERROR_CNT 0x%" PRIX64, entry->digital_error_cnt);
	formatted_append(print_buf, len, fmt, "DIGITAL_CONTROL_2 0x%" PRIX64, entry->digital_control_2);
	formatted_append(print_buf, len, fmt, "DEBUG_CONTROL     0x%" PRIX64, entry->debug_control);
	formatted_append(print_buf, len, fmt, "TEST_CONTROL      0x%" PRIX64, entry->test_control);
	formatted_append(print_buf, len, fmt, "AUTONEG_CONTROL   0x%" PRIX64, entry->autoneg_control);
	formatted_append(print_buf, len, fmt, "DIGITAL_CONTROL_1 0x%" PRIX64, entry->digital_control_1);
	formatted_append(print_buf, len, fmt, "AUTONEG_ADV       0x%" PRIX64, entry->autoneg_adv);
	formatted_append(print_buf, len, fmt, "BASIC_CONTROL     0x%" PRIX64, entry->basic_control);
}

void sja1105_sgmii_entry_show(struct sja1105_sgmii_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_sgmii_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

