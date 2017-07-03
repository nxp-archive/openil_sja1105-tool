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
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* These are our own includes */
#include <lib/include/config.h>
#include <lib/include/gtable.h>
#include <lib/include/spi.h>

static void sja1105_cfg_pad_mii_tx_access(
		void *buf,
		struct sja1105_cfg_pad_mii_tx *pad_mii_tx,
		int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = 4;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(pad_mii_tx, 0, sizeof(*pad_mii_tx));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &pad_mii_tx->d32_os,   28, 27, 4);
	pack_or_unpack(buf, &pad_mii_tx->d32_ipud, 25, 24, 4);
	pack_or_unpack(buf, &pad_mii_tx->d10_os,   20, 19, 4);
	pack_or_unpack(buf, &pad_mii_tx->d10_ipud, 17, 16, 4);
	pack_or_unpack(buf, &pad_mii_tx->ctrl_os,  12, 11, 4);
	pack_or_unpack(buf, &pad_mii_tx->ctrl_ipud, 9,  8, 4);
	pack_or_unpack(buf, &pad_mii_tx->clk_os,    4,  3, 4);
	pack_or_unpack(buf, &pad_mii_tx->clk_ih,    2,  2, 4);
	pack_or_unpack(buf, &pad_mii_tx->clk_ipud,  1,  0, 4);
}

void sja1105_cfg_pad_mii_tx_set(void *buf, struct sja1105_cfg_pad_mii_tx *pad_mii_tx)
{
	sja1105_cfg_pad_mii_tx_access(buf, pad_mii_tx, 1);
}

void sja1105_cfg_pad_mii_tx_get(void *buf, struct sja1105_cfg_pad_mii_tx *pad_mii_tx)
{
	sja1105_cfg_pad_mii_tx_access(buf, pad_mii_tx, 0);
}

void sja1105_cfg_pad_mii_tx_show(struct sja1105_cfg_pad_mii_tx *pad_mii_tx)
{
	printf("D32_OS    %" PRIX64 "\n", pad_mii_tx->d32_os);
	printf("D32_IPUD  %" PRIX64 "\n", pad_mii_tx->d32_ipud);
	printf("D10_OS    %" PRIX64 "\n", pad_mii_tx->d10_os);
	printf("D10_IPUD  %" PRIX64 "\n", pad_mii_tx->d10_ipud);
	printf("CTRL_OS   %" PRIX64 "\n", pad_mii_tx->ctrl_os);
	printf("CTRL_IPUD %" PRIX64 "\n", pad_mii_tx->ctrl_ipud);
	printf("CLK_OS    %" PRIX64 "\n", pad_mii_tx->clk_os);
	printf("CLK_IH    %" PRIX64 "\n", pad_mii_tx->clk_ih);
	printf("CLK_IPUD  %" PRIX64 "\n", pad_mii_tx->clk_ipud);
}

int sja1105_rgmii_cfg_pad_tx_config(struct sja1105_spi_setup *spi_setup, int port)
{
#define MSG_SIZE SIZE_SPI_MSG_HEADER + 4
	struct  sja1105_spi_message msg;
	struct  sja1105_cfg_pad_mii_tx pad_mii_tx;
	uint8_t tx_buf[MSG_SIZE];
	uint8_t rx_buf[MSG_SIZE];
	/* UM10944.pdf, Table 86, AGU Register overview */
	int     pad_mii_tx_offsets[] = {0x00, 0x02, 0x04, 0x06, 0x08};

	memset(tx_buf, 0, MSG_SIZE);
	memset(rx_buf, 0, MSG_SIZE);

	/* Header */
	msg.access     = SPI_WRITE;
	msg.read_count = 0;
	msg.address    = AGU_ADDR + pad_mii_tx_offsets[port];
	sja1105_spi_message_set(tx_buf, &msg);

	/* Payload */
	pad_mii_tx.d32_os    = 3; /* TXD[3:2] output stage: high noise/high speed */
	pad_mii_tx.d32_ipud  = 2; /* TXD[3:2] input stage: plain input (default) */
	pad_mii_tx.d10_os    = 3; /* TXD[1:0] output stage: high noise/high speed */
	pad_mii_tx.d10_ipud  = 2; /* TXD[1:0] input stage: plain input (default) */
	pad_mii_tx.ctrl_os   = 3; /* TX_CTL / TX_ER output stage */
	pad_mii_tx.ctrl_ipud = 2; /* TX_CTL / TX_ER input stage (default) */
	pad_mii_tx.clk_os    = 3; /* TX_CLK output stage */
	pad_mii_tx.clk_ih    = 0; /* TX_CLK input hysteresis (default) */
	pad_mii_tx.clk_ipud  = 2; /* TX_CLK input stage (default) */
	sja1105_cfg_pad_mii_tx_set(tx_buf + SIZE_SPI_MSG_HEADER, &pad_mii_tx);

	return sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, MSG_SIZE);
}


