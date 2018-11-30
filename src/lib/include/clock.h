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
#ifndef _CGU_EXTERNAL_H
#define _CGU_EXTERNAL_H

#include <common.h>
#include "spi.h"
#include "static-config.h"

/* AGU */
#define AGU_ADDR    0x100800

struct sja1105_cfg_pad_mii_tx {
	uint64_t d32_os;
	uint64_t d32_ipud;
	uint64_t d10_os;
	uint64_t d10_ipud;
	uint64_t ctrl_os;
	uint64_t ctrl_ipud;
	uint64_t clk_os;
	uint64_t clk_ih;
	uint64_t clk_ipud;
};


/* CGU */
#define CGU_ADDR    0x100000

/* UM10944 Table 82.
 * IDIV_0_C to IDIV_4_C control registers
 * (addr. 10000Bh to 10000Fh) */
struct sja1105_cgu_idiv {
	uint64_t clksrc;
	uint64_t autoblock;
	uint64_t idiv;
	uint64_t pd;
};

/* UM10944 Table 80.
 * PLL_x_S clock status registers 0 and 1
 * (address 100007h and 100009h) */
struct sja1105_cgu_pll_status {
	uint64_t lock;
};

/*
 * PLL_1_C control register
 *
 * SJA1105 E/T: UM10944 Table 81 (address 10000Ah)
 * SJA1105 P/Q/R/S: UM11040 Table 116 (address 10000Ah)
 *
 */
struct sja1105_cgu_pll_control {
	uint64_t pllclksrc;
	uint64_t msel;
	uint64_t nsel; /* Only for P/Q/R/S series */
	uint64_t autoblock;
	uint64_t psel;
	uint64_t direct;
	uint64_t fbsel;
	uint64_t p23en; /* Only for P/Q/R/S series */
	uint64_t bypass;
	uint64_t pd;
};

#define CLKSRC_MII0_TX_CLK 0x00
#define CLKSRC_MII0_RX_CLK 0x01
#define CLKSRC_MII1_TX_CLK 0x02
#define CLKSRC_MII1_RX_CLK 0x03
#define CLKSRC_MII2_TX_CLK 0x04
#define CLKSRC_MII2_RX_CLK 0x05
#define CLKSRC_MII3_TX_CLK 0x06
#define CLKSRC_MII3_RX_CLK 0x07
#define CLKSRC_MII4_TX_CLK 0x08
#define CLKSRC_MII4_RX_CLK 0x09
#define CLKSRC_PLL0        0x0B
#define CLKSRC_PLL1        0x0E
#define CLKSRC_IDIV0       0x11
#define CLKSRC_IDIV1       0x12
#define CLKSRC_IDIV2       0x13
#define CLKSRC_IDIV3       0x14
#define CLKSRC_IDIV4       0x15

/* UM10944 Table 83.
 * MIIx clock control registers 1 to 30
 * (addresses 100013h to 100035h) */
struct sja1105_cgu_mii_control {
	uint64_t clksrc;
	uint64_t autoblock;
	uint64_t pd;
};

#define XMII_MODE_MAC    0ull
#define XMII_MODE_PHY    1ull
#define XMII_SPEED_MII   0ull
#define XMII_SPEED_RMII  1ull
#define XMII_SPEED_RGMII 2ull
#define XMII_SPEED_SGMII 3ull /* Only available for port 4 on R/S */

#endif
