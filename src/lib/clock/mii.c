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

int sja1105_cgu_mii_tx_clk_config(struct sja1105_spi_setup *spi_setup,
                                  int port, int mii_mode)
{
	const int BUF_LEN = 4;
	uint8_t packed_buf[BUF_LEN];
	struct  sja1105_cgu_mii_control mii_tx_clk;
#ifdef SJA1105PQRS
	const int mii_tx_clk_offsets[] = {0x13, 0x19, 0x1F, 0x25, 0x2B}; // Table 114, UM11040
#else
	/* UM10944.pdf, Table 78, CGU Register overview */
	const int mii_tx_clk_offsets[] = {0x13, 0x1A, 0x21, 0x28, 0x2F};
#endif
	const int mac_clk_sources[] = {
		CLKSRC_MII0_TX_CLK,
		CLKSRC_MII1_TX_CLK,
		CLKSRC_MII2_TX_CLK,
		CLKSRC_MII3_TX_CLK,
		CLKSRC_MII4_TX_CLK,
	};
	const int phy_clk_sources[] = {
		CLKSRC_IDIV0,
		CLKSRC_IDIV1,
		CLKSRC_IDIV2,
		CLKSRC_IDIV3,
		CLKSRC_IDIV4,
	};
	int clksrc;

	if (mii_mode == XMII_MODE_MAC) {
		clksrc = mac_clk_sources[port];
	} else {
		clksrc = phy_clk_sources[port];
	}
	/* Payload for packed_buf */
	mii_tx_clk.clksrc    = clksrc;
	mii_tx_clk.autoblock = 1;  /* Autoblock clk while changing clksrc */
	mii_tx_clk.pd        = 0;  /* Power Down off => enabled */
	sja1105_cgu_mii_control_pack(packed_buf, &mii_tx_clk);

	return sja1105_spi_send_packed_buf(spi_setup,
	                                   SPI_WRITE,
	                                   CGU_ADDR + mii_tx_clk_offsets[port],
	                                   packed_buf,
	                                   BUF_LEN);
}

int sja1105_cgu_mii_rx_clk_config(
		struct sja1105_spi_setup *spi_setup,
		int    port)
{
	const int BUF_LEN = 4;
	uint8_t packed_buf[BUF_LEN];
	struct  sja1105_cgu_mii_control mii_rx_clk;
#ifdef SJA1105PQRS
	const int mii_rx_clk_offsets[] = {0x14, 0x1A, 0x20, 0x26, 0x2C}; // Table 114, UM11040
#else
	/* UM10944.pdf, Table 78, CGU Register overview */
	const int mii_rx_clk_offsets[] = {0x14, 0x1B, 0x22, 0x29, 0x30};
#endif
	const int clk_sources[] = {
		CLKSRC_MII0_RX_CLK,
		CLKSRC_MII1_RX_CLK,
		CLKSRC_MII2_RX_CLK,
		CLKSRC_MII3_RX_CLK,
		CLKSRC_MII4_RX_CLK,
	};

	/* Payload for packed_buf */
	mii_rx_clk.clksrc    = clk_sources[port];
	mii_rx_clk.autoblock = 1;  /* Autoblock clk while changing clksrc */
	mii_rx_clk.pd        = 0;  /* Power Down off => enabled */
	sja1105_cgu_mii_control_pack(packed_buf, &mii_rx_clk);

	return sja1105_spi_send_packed_buf(spi_setup,
	                                   SPI_WRITE,
	                                   CGU_ADDR + mii_rx_clk_offsets[port],
	                                   packed_buf,
	                                   BUF_LEN);
}

int sja1105_cgu_mii_ext_tx_clk_config(
		struct sja1105_spi_setup *spi_setup,
		int    port)
{
	const int BUF_LEN = 4;
	uint8_t packed_buf[BUF_LEN];
	struct  sja1105_cgu_mii_control mii_ext_tx_clk;
#ifdef SJA1105PQRS
	const int mii_ext_tx_clk_offsets[] = {0x17, 0x1D, 0x23, 0x29, 0x2F}; // Table 114, UM11040

#else
	/* UM10944.pdf, Table 78, CGU Register overview */
	const int mii_ext_tx_clk_offsets[] = {0x18, 0x1F, 0x26, 0x2D, 0x34};
#endif
	const int clk_sources[] = {
		CLKSRC_IDIV0,
		CLKSRC_IDIV1,
		CLKSRC_IDIV2,
		CLKSRC_IDIV3,
		CLKSRC_IDIV4,
	};

	/* Payload for packed_buf */
	mii_ext_tx_clk.clksrc    = clk_sources[port];
	mii_ext_tx_clk.autoblock = 1; /* Autoblock clk while changing clksrc */
	mii_ext_tx_clk.pd        = 0; /* Power Down off => enabled */
	sja1105_cgu_mii_control_pack(packed_buf, &mii_ext_tx_clk);

	return sja1105_spi_send_packed_buf(spi_setup,
	                                   SPI_WRITE,
	                                   CGU_ADDR +
	                                       mii_ext_tx_clk_offsets[port],
	                                   packed_buf,
	                                   BUF_LEN);
}

int sja1105_cgu_mii_ext_rx_clk_config(
		struct sja1105_spi_setup *spi_setup,
		int    port)
{
	const int BUF_LEN = 4;
	uint8_t packed_buf[BUF_LEN];
	struct  sja1105_cgu_mii_control mii_ext_rx_clk;
#ifdef SJA1105PQRS
	const int mii_ext_rx_clk_offsets[] = {0x18, 0x1E, 0x24, 0x2A, 0x30}; // Table 114, UM11040
#else
	/* UM10944.pdf, Table 78, CGU Register overview */
	const int mii_ext_rx_clk_offsets[] = {0x19, 0x20, 0x27, 0x2E, 0x35};
#endif
	const int clk_sources[] = {
		CLKSRC_IDIV0,
		CLKSRC_IDIV1,
		CLKSRC_IDIV2,
		CLKSRC_IDIV3,
		CLKSRC_IDIV4,
	};

	/* Payload for packed_buf */
	mii_ext_rx_clk.clksrc    = clk_sources[port];
	mii_ext_rx_clk.autoblock = 1; /* Autoblock clk while changing clksrc */
	mii_ext_rx_clk.pd        = 0; /* Power Down off => enabled */
	sja1105_cgu_mii_control_pack(packed_buf, &mii_ext_rx_clk);

	return sja1105_spi_send_packed_buf(spi_setup,
	                                   SPI_WRITE,
	                                   CGU_ADDR +
	                                       mii_ext_rx_clk_offsets[port],
	                                   packed_buf,
	                                   BUF_LEN);
}

int mii_clocking_setup(struct sja1105_spi_setup *spi_setup, int port,
                       int mii_mode)
{
	int rc;

	if (mii_mode != XMII_MODE_MAC && mii_mode != XMII_MODE_PHY) {
		goto error;
	}
	logv("Configuring MII-%s clocking for port %d",
	    (mii_mode == XMII_MODE_MAC) ? "MAC" : "PHY", port);
	/*   * If mii_mode is MAC, disable IDIV
	 *   * If mii_mode is PHY, enable IDIV and configure for 1/1 divider
	 */
	rc = sja1105_cgu_idiv_config(spi_setup, port,
	                            (mii_mode == XMII_MODE_PHY), 1);
	if (rc < 0) {
		goto error;
	}
	/* Configure CLKSRC of MII_TX_CLK_n
	 *   * If mii_mode is MAC, select TX_CLK_n
	 *   * If mii_mode is PHY, select IDIV_n
	 */
	rc = sja1105_cgu_mii_tx_clk_config(spi_setup, port, mii_mode);
	if (rc < 0) {
		goto error;
	}
	/* Configure CLKSRC of MII_RX_CLK_n
	 * Select RX_CLK_n
	 */
	rc = sja1105_cgu_mii_rx_clk_config(spi_setup, port);
	if (rc < 0) {
		goto error;
	}
	if (mii_mode == XMII_MODE_PHY) {
		/* In MII mode the PHY (which is us) drives the TX_CLK pin */

		/* Configure CLKSRC of EXT_TX_CLK_n
		 * Select IDIV_n
		 */
		rc = sja1105_cgu_mii_ext_tx_clk_config(spi_setup, port);
		if (rc < 0) {
			goto error;
		}
		/* Configure CLKSRC of EXT_RX_CLK_n
		 * Select IDIV_n
		 */
		rc = sja1105_cgu_mii_ext_rx_clk_config(spi_setup, port);
		if (rc < 0) {
			goto error;
		}
	}
	return 0;
error:
	return -1;
}
