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
#ifndef _PORT_H
#define _PORT_H

#include "spi.h"

#define SJA1105T_NUM_PORTS 5

struct sja1105_cbs {
	/* UM10944.pdf Table 62. Credit-based shaping block
	 * register 1 (address 30h).
	 * Used as identification */
	uint64_t index;
	uint64_t port;
	uint64_t prio;
	/* UM10944.pdf Table 63. Credit-based shaping block
	 * registers 2 to 5 (address 2Fh to 2Ch).
	 * Used for actual configuration. */
	uint64_t credit_lo;
	uint64_t credit_hi;
	uint64_t send_slope;
	uint64_t idle_slope;
};

struct sja1105_egress_port_mask {
	uint64_t inhibit_tx[SJA1105T_NUM_PORTS];
};

int sja1105_inhibit_tx(struct sja1105_spi_setup *spi_setup,
                       struct sja1105_egress_port_mask *port_mask);

#endif

