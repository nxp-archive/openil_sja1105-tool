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
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* These are our own includes */
#include <lib/include/config.h>
#include <lib/include/gtable.h>
#include <lib/include/spi.h>

static void sja1105_cgu_mii_control_access(void *buf, struct sja1105_cgu_mii_control *mii_control, int write)
{
	int (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int   size = 4;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(mii_control, 0, sizeof(*mii_control));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &mii_control->clksrc,    28, 24, 4);
	pack_or_unpack(buf, &mii_control->autoblock, 11, 11, 4);
	pack_or_unpack(buf, &mii_control->pd,         0,  0, 4);
}

void sja1105_cgu_mii_control_set(void *buf, struct sja1105_cgu_mii_control *mii_control)
{
	sja1105_cgu_mii_control_access(buf, mii_control, 1);
}

void sja1105_cgu_mii_control_get(void *buf, struct sja1105_cgu_mii_control *mii_control)
{
	sja1105_cgu_mii_control_access(buf, mii_control, 0);
}

void sja1105_cgu_mii_control_show(struct sja1105_cgu_mii_control *mii_control)
{
	printf("CLKSEL    %" PRIX64 "\n", mii_control->clksrc);
	printf("AUTOBLOCK %" PRIX64 "\n", mii_control->autoblock);
	printf("PD        %" PRIX64 "\n", mii_control->pd);
}

int sja1105_cgu_rgmii_tx_clk_config(
		struct sja1105_spi_setup *spi_setup,
		int    port,
		int    speed_mbps)
{
#define MSG_SIZE SIZE_SPI_MSG_HEADER + 4
	struct  sja1105_cgu_mii_control txc;
	struct  sja1105_spi_message msg;
	uint8_t tx_buf[MSG_SIZE];
	uint8_t rx_buf[MSG_SIZE];
	/* UM10944.pdf, Table 78, CGU Register overview */
	int     txc_offsets[] = {0x16, 0x1D, 0x24, 0x2B, 0x32};
	int     clksrc;

	if (speed_mbps == 1000) {
		clksrc = CLKSRC_PLL0;
	} else {
		int clk_sources[] = {CLKSRC_IDIV0, CLKSRC_IDIV1, CLKSRC_IDIV2,
		                     CLKSRC_IDIV3, CLKSRC_IDIV4};
		clksrc = clk_sources[port];
	}

	memset(tx_buf, 0, MSG_SIZE);
	memset(rx_buf, 0, MSG_SIZE);

	/* Header */
	msg.access     = SPI_WRITE;
	msg.read_count = 0;
	msg.address    = CGU_ADDR + txc_offsets[port];
	sja1105_spi_message_set(tx_buf, &msg);

	/* Payload */
	txc.clksrc    = clksrc; /* RGMII: 125MHz for 1000, 25MHz for 100, 2.5MHz for 10 */
	txc.autoblock = 1;      /* Autoblock clk while changing clksrc */
	txc.pd        = 0;      /* Power Down off => enabled */
	sja1105_cgu_mii_control_set(tx_buf + SIZE_SPI_MSG_HEADER, &txc);

	return sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, MSG_SIZE);
}

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
	sja1105_cgu_mii_control_set(tx_buf + SIZE_SPI_MSG_HEADER, &mii_tx_clk);

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
	sja1105_cgu_mii_control_set(tx_buf + SIZE_SPI_MSG_HEADER, &mii_rx_clk);

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
	sja1105_cgu_mii_control_set(tx_buf + SIZE_SPI_MSG_HEADER, &mii_ext_tx_clk);

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
	sja1105_cgu_mii_control_set(tx_buf + SIZE_SPI_MSG_HEADER, &mii_ext_rx_clk);

	return sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, MSG_SIZE);
}

