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

static void sja1105et_l2_lookup_params_entry_access(
		void *buf,
		struct sja1105_l2_lookup_params_entry *entry,
		int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = SIZE_L2_LOOKUP_PARAMS_ENTRY_ET;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(entry, 0, sizeof(*entry));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &entry->maxage,         31, 17, size);
	pack_or_unpack(buf, &entry->dyn_tbsz,       16, 14, size);
	pack_or_unpack(buf, &entry->poly,           13,  6, size);
	pack_or_unpack(buf, &entry->shared_learn,    5,  5, size);
	pack_or_unpack(buf, &entry->no_enf_hostprt,  4,  4, size);
	pack_or_unpack(buf, &entry->no_mgmt_learn,   3,  3, size);
}

static void sja1105pqrs_l2_lookup_params_entry_access(
		void *buf,
		struct sja1105_l2_lookup_params_entry *entry,
		int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = SIZE_L2_LOOKUP_PARAMS_ENTRY_PQRS;
	int    offset;
	int    i;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(entry, 0, sizeof(*entry));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &entry->drpbc,          127, 123, size);
	pack_or_unpack(buf, &entry->drpmc,          122, 118, size);
	pack_or_unpack(buf, &entry->drpuni,         117, 113, size);
	offset = 58;
	for (i = 0; i < 5; i++) {
		pack_or_unpack(buf, &entry->maxaddrp[i], offset + 10, offset + 0, size);
		offset += 11;
	}
	pack_or_unpack(buf, &entry->maxage,          57,  43, size);
	pack_or_unpack(buf, &entry->start_dynspc,    42,  33, size);
	pack_or_unpack(buf, &entry->drpnolearn,      32,  28, size);
	pack_or_unpack(buf, &entry->shared_learn,    27,  27, size);
	pack_or_unpack(buf, &entry->no_enf_hostprt,  26,  26, size);
	pack_or_unpack(buf, &entry->no_mgmt_learn,   25,  25, size);
	pack_or_unpack(buf, &entry->use_static,      24,  24, size);
	pack_or_unpack(buf, &entry->owr_dyn,         23,  23, size);
	pack_or_unpack(buf, &entry->learn_once,      22,  22, size);
}

/*
 * sja1105et_l2_lookup_params_entry_pack
 * sja1105et_l2_lookup_params_entry_unpack
 * sja1105pqrs_l2_lookup_params_entry_pack
 * sja1105pqrs_l2_lookup_params_entry_unpack
 */
DEFINE_SEPARATE_PACK_UNPACK_ACCESSORS(l2_lookup_params);

void sja1105_l2_lookup_params_entry_fmt_show(
		char *print_buf,
		size_t len,
		char *fmt,
		struct sja1105_l2_lookup_params_entry *entry)
{
	formatted_append(print_buf, len, fmt, "MAXAGE         0x%" PRIX64, entry->maxage);
	formatted_append(print_buf, len, fmt, "DYN_TBSZ       0x%" PRIX64, entry->dyn_tbsz);
	formatted_append(print_buf, len, fmt, "POLY           0x%" PRIX64, entry->poly);
	formatted_append(print_buf, len, fmt, "SHARED_LEARN   0x%" PRIX64, entry->shared_learn);
	formatted_append(print_buf, len, fmt, "NO_ENF_HOSTPRT 0x%" PRIX64, entry->no_enf_hostprt);
	formatted_append(print_buf, len, fmt, "NO_MGMT_LEARN  0x%" PRIX64, entry->no_mgmt_learn);
}
