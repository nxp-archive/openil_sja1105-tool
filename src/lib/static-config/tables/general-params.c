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

static void sja1105et_general_params_entry_access(
		void *buf,
		struct sja1105_general_params_entry *entry,
		int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = SIZE_GENERAL_PARAMS_ENTRY_ET;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(entry, 0, sizeof(*entry));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &entry->vllupformat, 319, 319, size);
	pack_or_unpack(buf, &entry->mirr_ptacu,  318, 318, size);
	pack_or_unpack(buf, &entry->switchid,    317, 315, size);
	pack_or_unpack(buf, &entry->hostprio,    314, 312, size);
	pack_or_unpack(buf, &entry->mac_fltres1, 311, 264, size);
	pack_or_unpack(buf, &entry->mac_fltres0, 263, 216, size);
	pack_or_unpack(buf, &entry->mac_flt1,    215, 168, size);
	pack_or_unpack(buf, &entry->mac_flt0,    167, 120, size);
	pack_or_unpack(buf, &entry->incl_srcpt1, 119, 119, size);
	pack_or_unpack(buf, &entry->incl_srcpt0, 118, 118, size);
	pack_or_unpack(buf, &entry->send_meta1,  117, 117, size);
	pack_or_unpack(buf, &entry->send_meta0,  116, 116, size);
	pack_or_unpack(buf, &entry->casc_port,   115, 113, size);
	pack_or_unpack(buf, &entry->host_port,   112, 110, size);
	pack_or_unpack(buf, &entry->mirr_port,   109, 107, size);
	pack_or_unpack(buf, &entry->vlmarker,    106,  75, size);
	pack_or_unpack(buf, &entry->vlmask,       74,  43, size);
	pack_or_unpack(buf, &entry->tpid,         42,  27, size);
	pack_or_unpack(buf, &entry->ignore2stf,   26,  26, size);
	pack_or_unpack(buf, &entry->tpid2,        25,  10, size);
}

static void sja1105pqrs_general_params_entry_access(
		void *buf,
		struct sja1105_general_params_entry *entry,
		int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = SIZE_GENERAL_PARAMS_ENTRY_PQRS;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(entry, 0, sizeof(*entry));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &entry->vllupformat, 351, 351, size);
	pack_or_unpack(buf, &entry->mirr_ptacu,  350, 350, size);
	pack_or_unpack(buf, &entry->switchid,    349, 347, size);
	pack_or_unpack(buf, &entry->hostprio,    346, 344, size);
	pack_or_unpack(buf, &entry->mac_fltres1, 343, 296, size);
	pack_or_unpack(buf, &entry->mac_fltres0, 295, 248, size);
	pack_or_unpack(buf, &entry->mac_flt1,    247, 200, size);
	pack_or_unpack(buf, &entry->mac_flt0,    199, 152, size);
	pack_or_unpack(buf, &entry->incl_srcpt1, 151, 151, size);
	pack_or_unpack(buf, &entry->incl_srcpt0, 150, 150, size);
	pack_or_unpack(buf, &entry->send_meta1,  149, 149, size);
	pack_or_unpack(buf, &entry->send_meta0,  148, 148, size);
	pack_or_unpack(buf, &entry->casc_port,   147, 145, size);
	pack_or_unpack(buf, &entry->host_port,   144, 142, size);
	pack_or_unpack(buf, &entry->mirr_port,   141, 139, size);
	pack_or_unpack(buf, &entry->vlmarker,    138, 107, size);
	pack_or_unpack(buf, &entry->vlmask,      106,  75, size);
	pack_or_unpack(buf, &entry->tpid,         74,  59, size);
	pack_or_unpack(buf, &entry->ignore2stf,   58,  58, size);
	pack_or_unpack(buf, &entry->tpid2,        57,  42, size);
	pack_or_unpack(buf, &entry->queue_ts,     41,  41, size);
	pack_or_unpack(buf, &entry->egrmirrvid,   40,  29, size);
	pack_or_unpack(buf, &entry->egrmirrpcp,   28,  26, size);
	pack_or_unpack(buf, &entry->egrmirrdei,   25,  25, size);
	pack_or_unpack(buf, &entry->replay_port,  24,  22, size);
}
/* Device-specific pack/unpack accessors
 * sja1105et_general_params_entry_pack
 * sja1105et_general_params_entry_unpack
 * sja1105pqrs_general_params_entry_pack
 * sja1105pqrs_general_params_entry_unpack
 */
DEFINE_SEPARATE_PACK_UNPACK_ACCESSORS(general_params);

void sja1105_general_params_entry_fmt_show(
		char *print_buf,
		size_t len,
		char *fmt,
		struct sja1105_general_params_entry *entry)
{
	char mac_buf[MAC_ADDR_SIZE];

	formatted_append(print_buf, len, fmt, "VLLUPFORMAT 0x%" PRIX64, entry->vllupformat);
	formatted_append(print_buf, len, fmt, "MIRR_PTACU  0x%" PRIX64, entry->mirr_ptacu);
	formatted_append(print_buf, len, fmt, "SWITCHID    0x%" PRIX64, entry->switchid);
	formatted_append(print_buf, len, fmt, "HOSTPRIO    0x%" PRIX64, entry->hostprio);
	memset(mac_buf, 0, sizeof(mac_buf));
	mac_addr_sprintf(mac_buf, entry->mac_fltres1);
	formatted_append(print_buf, len, fmt, "MAC_FLTRES1 %s", mac_buf);
	memset(mac_buf, 0, sizeof(mac_buf));
	mac_addr_sprintf(mac_buf, entry->mac_fltres0);
	formatted_append(print_buf, len, fmt, "MAC_FLTRES0 %s", mac_buf);
	memset(mac_buf, 0, sizeof(mac_buf));
	mac_addr_sprintf(mac_buf, entry->mac_flt1);
	formatted_append(print_buf, len, fmt, "MAC_FLT1    %s", mac_buf);
	memset(mac_buf, 0, sizeof(mac_buf));
	mac_addr_sprintf(mac_buf, entry->mac_flt0);
	formatted_append(print_buf, len, fmt, "MAC_FLT0    %s", mac_buf);
	formatted_append(print_buf, len, fmt, "INCL_SRCPT1 0x%" PRIX64, entry->incl_srcpt1);
	formatted_append(print_buf, len, fmt, "INCL_SRCPT0 0x%" PRIX64, entry->incl_srcpt0);
	formatted_append(print_buf, len, fmt, "SEND_META1  0x%" PRIX64, entry->send_meta1);
	formatted_append(print_buf, len, fmt, "SEND_META0  0x%" PRIX64, entry->send_meta0);
	formatted_append(print_buf, len, fmt, "CASC_PORT   0x%" PRIX64, entry->casc_port);
	formatted_append(print_buf, len, fmt, "HOST_PORT   0x%" PRIX64, entry->host_port);
	formatted_append(print_buf, len, fmt, "MIRR_PORT   0x%" PRIX64, entry->mirr_port);
	formatted_append(print_buf, len, fmt, "VLMARKER    0x%" PRIX64, entry->vlmarker);
	formatted_append(print_buf, len, fmt, "VLMASK      0x%" PRIX64, entry->vlmask);
	formatted_append(print_buf, len, fmt, "TPID        0x%" PRIX64, entry->tpid);
	formatted_append(print_buf, len, fmt, "IGNORE2STF  0x%" PRIX64, entry->ignore2stf);
	formatted_append(print_buf, len, fmt, "TPID2       0x%" PRIX64, entry->tpid2);
}
