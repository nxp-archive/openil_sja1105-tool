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
#include <lib/include/gtable.h>
#include <lib/include/clock.h>
#include <lib/include/spi.h>
#include <common.h>

int sja1105_cgu_rmii_ref_clk_config(struct sja1105_spi_setup *spi_setup,
                                    int    port)
{
	const int BUF_LEN = 4;
	struct  sja1105_cgu_mii_control ref_clk;
	uint8_t packed_buf[BUF_LEN];
	/* UM10944.pdf, Table 78, CGU Register overview */
	const int ref_clk_offsets_et[] = {0x15, 0x1C, 0x23, 0x2A, 0x31};
	/* UM11040.pdf, Table 114, CGU Register overview */
	const int ref_clk_offsets_pqrs[] = {0x15, 0x1B, 0x21, 0x27, 0x2D};
	const int *ref_clk_offsets;
	const int clk_sources[] = {
		CLKSRC_MII0_TX_CLK,
		CLKSRC_MII1_TX_CLK,
		CLKSRC_MII2_TX_CLK,
		CLKSRC_MII3_TX_CLK,
		CLKSRC_MII4_TX_CLK,
	};

	/* E/T and P/Q/R/S compatibility */
	ref_clk_offsets = IS_ET(spi_setup->device_id) ?
	                     ref_clk_offsets_et :
	                     ref_clk_offsets_pqrs;

	/* Payload for packed_buf */
	ref_clk.clksrc    = clk_sources[port];
	ref_clk.autoblock = 1;      /* Autoblock clk while changing clksrc */
	ref_clk.pd        = 0;      /* Power Down off => enabled */
	sja1105_cgu_mii_control_pack(packed_buf, &ref_clk);

	return sja1105_spi_send_packed_buf(spi_setup,
	                                   SPI_WRITE,
	                                   CGU_ADDR + ref_clk_offsets[port],
	                                   packed_buf,
	                                   BUF_LEN);
}

int sja1105_cgu_rmii_ext_tx_clk_config(struct sja1105_spi_setup *spi_setup,
                                       int    port)
{
	const int BUF_LEN = 4;
	struct  sja1105_cgu_mii_control ext_tx_clk;
	uint8_t packed_buf[BUF_LEN];
	/* UM10944.pdf, Table 78, CGU Register overview */
	const int ext_tx_clk_offsets_et[] = {0x18, 0x1F, 0x26, 0x2D, 0x34};
	/* UM11040.pdf, Table 114, CGU Register overview */
	const int ext_tx_clk_offsets_pqrs[] = {0x17, 0x1D, 0x23, 0x29, 0x2F};
	const int *ext_tx_clk_offsets;

	/* E/T and P/Q/R/S compatibility */
	ext_tx_clk_offsets = IS_ET(spi_setup->device_id) ?
	                     ext_tx_clk_offsets_et :
	                     ext_tx_clk_offsets_pqrs;

	/* Payload for packed_buf */
	ext_tx_clk.clksrc    = CLKSRC_PLL1;
	ext_tx_clk.autoblock = 1;   /* Autoblock clk while changing clksrc */
	ext_tx_clk.pd        = 0;   /* Power Down off => enabled */
	sja1105_cgu_mii_control_pack(packed_buf, &ext_tx_clk);

	return sja1105_spi_send_packed_buf(spi_setup,
	                                   SPI_WRITE,
	                                   CGU_ADDR + ext_tx_clk_offsets[port],
	                                   packed_buf,
	                                   BUF_LEN);
}

static int sja1105_cgu_rmii_pll_config(struct sja1105_spi_setup *spi_setup)
{
	const int BUF_LEN = 4;
	const int PLL1_OFFSET = 0x0A;
	struct  sja1105_cgu_pll_control pll;
	uint8_t packed_buf[BUF_LEN];
	int     rc;

	/* PLL1 must be enabled and output 50 Mhz.
	 * This is done by writing first 0x0A010941 to
	 * the PLL_1_C register and then deasserting
	 * power down (PD) 0x0A010940. */

	/* Step 1: PLL1 setup for 50Mhz */
	pll.pllclksrc = 0xA;
	pll.msel      = 0x1;
	pll.autoblock = 0x1;
	pll.psel      = 0x1;
	pll.direct    = 0x0;
	pll.fbsel     = 0x1;
	pll.bypass    = 0x0;
	pll.pd        = 0x1;
	/* P/Q/R/S only */
	pll.nsel      = 0x0; /* PLL pre-divider is 1 (nsel + 1) */
	pll.p23en     = 0x0; /* disable 120 and 240 degree phase PLL outputs */

	sja1105_cgu_pll_control_pack(packed_buf, &pll, spi_setup->device_id);
	rc = sja1105_spi_send_packed_buf(spi_setup,
	                                 SPI_WRITE,
	                                 CGU_ADDR + PLL1_OFFSET,
	                                 packed_buf,
	                                 BUF_LEN);
	if (rc < 0) {
		loge("failed to configure PLL1 for 50MHz");
		goto out;
	}

	/* Step 2: Enable PLL1 */
	pll.pd        = 0x0;

	sja1105_cgu_pll_control_pack(packed_buf, &pll, spi_setup->device_id);
	rc = sja1105_spi_send_packed_buf(spi_setup,
	                                 SPI_WRITE,
	                                 CGU_ADDR + PLL1_OFFSET,
	                                 packed_buf,
	                                 BUF_LEN);
	if (rc < 0) {
		loge("failed to enable PLL1");
		goto out;
	}
out:
	return rc;
}

int rmii_clocking_setup(struct sja1105_spi_setup *spi_setup, int port,
                        int rmii_mode)
{
	int rc;

	if (rmii_mode != XMII_MODE_MAC && rmii_mode != XMII_MODE_PHY) {
		loge("RMII mode must either be MAC or PHY");
		rc = -EINVAL;
		goto out;
	}
	logv("Configuring RMII-%s clocking for port %d",
	    (rmii_mode == XMII_MODE_MAC) ? "MAC" : "PHY", port);
	/* AH1601.pdf chapter 2.5.1. Sources */
	if (rmii_mode == XMII_MODE_MAC) {
		/* Configure and enable PLL1 for 50Mhz output */
		rc = sja1105_cgu_rmii_pll_config(spi_setup);
		if (rc < 0) {
			goto out;
		}
	}
	/* Disable IDIV for this port */
	rc = sja1105_cgu_idiv_config(spi_setup, port, 0, 1);
	if (rc < 0) {
		goto out;
	}
	/* Source to sink mappings */
	rc = sja1105_cgu_rmii_ref_clk_config(spi_setup, port);
	if (rc < 0) {
		goto out;
	}
	if (rmii_mode == XMII_MODE_MAC) {
		rc = sja1105_cgu_rmii_ext_tx_clk_config(spi_setup, port);
		if (rc < 0) {
			goto out;
		}
	}
out:
	return rc;
}

