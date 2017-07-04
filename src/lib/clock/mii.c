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

int sja1105_cgu_mii_tx_clk_config(
		struct sja1105_spi_setup *spi_setup,
		int    port)
{
#define MSG_SIZE SIZE_SPI_MSG_HEADER + 4
	struct  sja1105_cgu_mii_control mii_tx_clk;
	struct  sja1105_spi_message msg;
	uint8_t tx_buf[MSG_SIZE];
	uint8_t rx_buf[MSG_SIZE];
	/* UM10944.pdf, Table 78, CGU Register overview */
	int     mii_tx_clk_offsets[] = {0x13, 0x1A, 0x21, 0x28, 0x2F};

	memset(tx_buf, 0, MSG_SIZE);
	memset(rx_buf, 0, MSG_SIZE);

	/* Header */
	msg.access     = SPI_WRITE;
	msg.read_count = 0;
	msg.address    = CGU_ADDR + mii_tx_clk_offsets[port];
	sja1105_spi_message_set(tx_buf, &msg);

	/* Payload */
	mii_tx_clk.clksrc    = CLKSRC_PLL1; /* XXX This is surely wrong */
	mii_tx_clk.autoblock = 1;           /* Autoblock clk while changing clksrc */
	mii_tx_clk.pd        = 0;           /* Power Down off => enabled */
	sja1105_cgu_mii_control_pack(tx_buf + SIZE_SPI_MSG_HEADER, &mii_tx_clk);

	return sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, MSG_SIZE);
}

int sja1105_cgu_mii_rx_clk_config(
		struct sja1105_spi_setup *spi_setup,
		int    port)
{
#define MSG_SIZE SIZE_SPI_MSG_HEADER + 4
	struct  sja1105_cgu_mii_control mii_rx_clk;
	struct  sja1105_spi_message msg;
	uint8_t tx_buf[MSG_SIZE];
	uint8_t rx_buf[MSG_SIZE];
	/* UM10944.pdf, Table 78, CGU Register overview */
	int     mii_rx_clk_offsets[] = {0x14, 0x1B, 0x22, 0x29, 0x30};

	memset(tx_buf, 0, MSG_SIZE);
	memset(rx_buf, 0, MSG_SIZE);

	/* Header */
	msg.access     = SPI_WRITE;
	msg.read_count = 0;
	msg.address    = CGU_ADDR + mii_rx_clk_offsets[port];
	sja1105_spi_message_set(tx_buf, &msg);

	/* Payload */
	mii_rx_clk.clksrc    = CLKSRC_PLL1;
	mii_rx_clk.autoblock = 1;           /* Autoblock clk while changing clksrc */
	mii_rx_clk.pd        = 0;           /* Power Down off => enabled */
	sja1105_cgu_mii_control_pack(tx_buf + SIZE_SPI_MSG_HEADER, &mii_rx_clk);

	return sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, MSG_SIZE);
}

int sja1105_cgu_mii_ext_tx_clk_config(
		struct sja1105_spi_setup *spi_setup,
		int    port)
{
#define MSG_SIZE SIZE_SPI_MSG_HEADER + 4
	struct  sja1105_cgu_mii_control mii_ext_tx_clk;
	struct  sja1105_spi_message msg;
	uint8_t tx_buf[MSG_SIZE];
	uint8_t rx_buf[MSG_SIZE];
	/* UM10944.pdf, Table 78, CGU Register overview */
	int     mii_ext_tx_clk_offsets[] = {0x18, 0x1F, 0x26, 0x2D, 0x34};
	int     clk_sources[] = {
		CLKSRC_IDIV0,
		CLKSRC_IDIV1,
		CLKSRC_IDIV2,
		CLKSRC_IDIV3,
		CLKSRC_IDIV4,
	};

	memset(tx_buf, 0, MSG_SIZE);
	memset(rx_buf, 0, MSG_SIZE);

	/* Header */
	msg.access     = SPI_WRITE;
	msg.read_count = 0;
	msg.address    = CGU_ADDR + mii_ext_tx_clk_offsets[port];
	sja1105_spi_message_set(tx_buf, &msg);

	/* Payload */
	mii_ext_tx_clk.clksrc    = clk_sources[port];
	mii_ext_tx_clk.autoblock = 1;           /* Autoblock clk while changing clksrc */
	mii_ext_tx_clk.pd        = 0;           /* Power Down off => enabled */
	sja1105_cgu_mii_control_pack(tx_buf + SIZE_SPI_MSG_HEADER,
	                            &mii_ext_tx_clk);

	return sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, MSG_SIZE);
}

int sja1105_cgu_mii_ext_rx_clk_config(
		struct sja1105_spi_setup *spi_setup,
		int    port)
{
#define MSG_SIZE SIZE_SPI_MSG_HEADER + 4
	struct  sja1105_cgu_mii_control mii_ext_rx_clk;
	struct  sja1105_spi_message msg;
	uint8_t tx_buf[MSG_SIZE];
	uint8_t rx_buf[MSG_SIZE];
	/* UM10944.pdf, Table 78, CGU Register overview */
	int     mii_ext_rx_clk_offsets[] = {0x19, 0x20, 0x27, 0x2E, 0x35};
	int     clk_sources[] = {
		CLKSRC_IDIV0,
		CLKSRC_IDIV1,
		CLKSRC_IDIV2,
		CLKSRC_IDIV3,
		CLKSRC_IDIV4,
	};

	memset(tx_buf, 0, MSG_SIZE);
	memset(rx_buf, 0, MSG_SIZE);

	/* Header */
	msg.access     = SPI_WRITE;
	msg.read_count = 0;
	msg.address    = CGU_ADDR + mii_ext_rx_clk_offsets[port];
	sja1105_spi_message_set(tx_buf, &msg);

	/* Payload */
	mii_ext_rx_clk.clksrc    = clk_sources[port];
	mii_ext_rx_clk.autoblock = 1;           /* Autoblock clk while changing clksrc */
	mii_ext_rx_clk.pd        = 0;           /* Power Down off => enabled */
	sja1105_cgu_mii_control_pack(tx_buf + SIZE_SPI_MSG_HEADER,
	                            &mii_ext_rx_clk);

	return sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, MSG_SIZE);
}


int mii_clocking_setup(struct sja1105_spi_setup *spi_setup, int port,
                       int mii_mode)
{
	int rc;

	if (mii_mode != XMII_MODE_MAC && mii_mode != XMII_MODE_PHY) {
		goto error;
	}
	logv("configuring mii clocking for port %d...", port);
	/* If xmii_mode is MAC, then we have to configure:
	 *     * MII_TX_CLK
	 *     * MII_RX_CLK
	 * If xmii_mode is PHY, we also have to configure:
	 *     * EXT_TX_CLK
	 *     * EXT_RX_CLK
	 */
	rc = sja1105_cgu_idiv_config(spi_setup, port,
	                            (mii_mode == XMII_MODE_PHY), 1);
	if (rc < 0) {
		goto error;
	}
	rc = sja1105_cgu_mii_tx_clk_config(spi_setup, port);
	if (rc < 0) {
		goto error;
	}
	rc = sja1105_cgu_mii_rx_clk_config(spi_setup, port);
	if (rc < 0) {
		goto error;
	}
	if (mii_mode == XMII_MODE_PHY) {
		rc = sja1105_cgu_mii_ext_tx_clk_config(spi_setup, port);
		if (rc < 0) {
			goto error;
		}
		rc = sja1105_cgu_mii_ext_rx_clk_config(spi_setup, port);
		if (rc < 0) {
			goto error;
		}
	}
	return 0;
error:
	return -1;
}
