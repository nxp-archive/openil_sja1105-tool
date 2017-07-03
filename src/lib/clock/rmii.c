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
#include <lib/include/gtable.h>
#include <lib/include/clock.h>
#include <lib/include/spi.h>
#include <common.h>

int sja1105_cgu_rmii_ref_clk_config(
		struct sja1105_spi_setup *spi_setup,
		int    port)
{
#define MSG_SIZE SIZE_SPI_MSG_HEADER + 4
	struct  sja1105_cgu_mii_control ref_clk;
	struct  sja1105_spi_message msg;
	uint8_t tx_buf[MSG_SIZE];
	uint8_t rx_buf[MSG_SIZE];
	/* UM10944.pdf, Table 78, CGU Register overview */
	int     ref_clk_offsets[] = {0x15, 0x1C, 0x23, 0x2A, 0x31};
	int     clk_sources[] = {
		CLKSRC_MII0_TX_CLK,
		CLKSRC_MII1_TX_CLK,
		CLKSRC_MII2_TX_CLK,
		CLKSRC_MII3_TX_CLK,
		CLKSRC_MII4_TX_CLK,
	};

	memset(tx_buf, 0, MSG_SIZE);
	memset(rx_buf, 0, MSG_SIZE);

	/* Header */
	msg.access     = SPI_WRITE;
	msg.read_count = 0;
	msg.address    = CGU_ADDR + ref_clk_offsets[port];
	sja1105_spi_message_set(tx_buf, &msg);

	/* Payload */
	ref_clk.clksrc    = clk_sources[port];
	ref_clk.autoblock = 1;           /* Autoblock clk while changing clksrc */
	ref_clk.pd        = 0;           /* Power Down off => enabled */
	sja1105_cgu_mii_control_set(tx_buf + SIZE_SPI_MSG_HEADER, &ref_clk);

	return sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, MSG_SIZE);
}

int sja1105_cgu_rmii_ext_tx_clk_config(
		struct sja1105_spi_setup *spi_setup,
		int    port)
{
#define MSG_SIZE SIZE_SPI_MSG_HEADER + 4
	struct  sja1105_cgu_mii_control ext_tx_clk;
	struct  sja1105_spi_message msg;
	uint8_t tx_buf[MSG_SIZE];
	uint8_t rx_buf[MSG_SIZE];
	/* UM10944.pdf, Table 78, CGU Register overview */
	int     ext_tx_clk_offsets[] = {0x18, 0x1F, 0x26, 0x2D, 0x34};

	memset(tx_buf, 0, MSG_SIZE);
	memset(rx_buf, 0, MSG_SIZE);

	/* Header */
	msg.access     = SPI_WRITE;
	msg.read_count = 0;
	msg.address    = CGU_ADDR + ext_tx_clk_offsets[port];
	sja1105_spi_message_set(tx_buf, &msg);

	/* Payload */
	ext_tx_clk.clksrc    = CLKSRC_PLL1; /* XXX Did not check this */
	ext_tx_clk.autoblock = 1;           /* Autoblock clk while changing clksrc */
	ext_tx_clk.pd        = 0;           /* Power Down off => enabled */
	sja1105_cgu_mii_control_set(tx_buf + SIZE_SPI_MSG_HEADER, &ext_tx_clk);

	return sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, MSG_SIZE);
}

int rmii_clocking_setup(struct sja1105_spi_setup *spi_setup, int port,
                        int rmii_mode)
{
	int rc;

	if (rmii_mode != XMII_MODE_MAC && rmii_mode != XMII_MODE_PHY) {
		goto error;
	}
	logv("configuring rmii clocking for port %d...", port);
	rc = sja1105_cgu_idiv_config(spi_setup, port, 0, 1);
	if (rc < 0) {
		goto error;
	}
	rc = sja1105_cgu_rmii_ref_clk_config(spi_setup, port);
	if (rc < 0) {
		goto error;
	}
	if (rmii_mode == XMII_MODE_MAC) {
		rc = sja1105_cgu_rmii_ext_tx_clk_config(spi_setup, port);
		if (rc < 0) {
			goto error;
		}
	}
	return 0;
error:
	return -1;
}

