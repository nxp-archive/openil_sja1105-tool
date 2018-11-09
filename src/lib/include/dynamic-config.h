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
#ifndef _DYN_CFG_H
#define _DYN_CFG_H

#include "spi.h"
#include "static-config.h"

#define SJA1105_MGMT_ROUTE_COUNT 4

struct sja1105_mgmt_entry {
	uint64_t ts_regid;
	uint64_t egr_ts;
	uint64_t macaddr;
	uint64_t destports;
	uint64_t enfport;
	uint64_t index;
};

union sja1105_dyn_l2_lookup_entry {
	struct sja1105_l2_lookup_entry l2;
	struct sja1105_mgmt_entry mgmt;
};

struct sja1105_dyn_l2_lookup_cmd {
	uint64_t valid;
	uint64_t rdwrset;
	uint64_t errors;
	uint64_t lockeds;
	uint64_t valident;
	uint64_t mgmtroute;
	union sja1105_dyn_l2_lookup_entry entry;
};

void sja1105_dyn_l2_lookup_cmd_pack(void *buf, struct
                                    sja1105_dyn_l2_lookup_cmd *cmd);
void sja1105_dyn_l2_lookup_cmd_unpack(void *buf, struct
                                      sja1105_dyn_l2_lookup_cmd *cmd);
int sja1105_mgmt_route_get(struct sja1105_spi_setup*, struct sja1105_mgmt_entry*, int index);
int sja1105_mgmt_route_set(struct sja1105_spi_setup*, struct sja1105_mgmt_entry*, int index);
void sja1105_mgmt_entry_show(struct sja1105_mgmt_entry *entry);

#endif
