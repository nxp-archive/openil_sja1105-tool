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
#include "internal.h"

static int mii_clocking_setup(int fd, struct spi_setup *spi_setup, int port, int mii_mode)
{
	int rc;

	if (mii_mode != XMII_MODE_MAC && mii_mode != XMII_MODE_PHY) {
		goto error;
	}
	if (general_config.verbose) {
		printf("configuring mii clocking for port %d...\n", port);
	}
	/* If xmii_mode is MAC, then we have to configure:
	 *     * MII_TX_CLK
	 *     * MII_RX_CLK
	 * If xmii_mode is PHY, we also have to configure:
	 *     * EXT_TX_CLK
	 *     * EXT_RX_CLK
	 */
	rc = sja1105_cgu_idiv_config(fd, spi_setup, port);
	if (rc < 0) {
		goto error;
	}
	rc = sja1105_cgu_mii_tx_clk_config(fd, spi_setup, port);
	if (rc < 0) {
		goto error;
	}
	rc = sja1105_cgu_mii_rx_clk_config(fd, spi_setup, port);
	if (rc < 0) {
		goto error;
	}
	if (mii_mode == XMII_MODE_PHY) {
		rc = sja1105_cgu_mii_ext_tx_clk_config(fd, spi_setup, port);
		if (rc < 0) {
			goto error;
		}
		rc = sja1105_cgu_mii_ext_rx_clk_config(fd, spi_setup, port);
		if (rc < 0) {
			goto error;
		}
	}
	return 0;
error:
	return -1;
}

static int rmii_clocking_setup(int fd, struct spi_setup *spi_setup, int port, int rmii_mode)
{
	int rc;

	if (rmii_mode != XMII_MODE_MAC && rmii_mode != XMII_MODE_PHY) {
		goto error;
	}
	if (general_config.verbose) {
		printf("configuring rmii clocking for port %d...\n", port);
	}
	rc = sja1105_cgu_idiv_config(fd, spi_setup, port);
	if (rc < 0) {
		goto error;
	}
	rc = sja1105_cgu_rmii_ref_clk_config(fd, spi_setup, port);
	if (rc < 0) {
		goto error;
	}
	if (rmii_mode == XMII_MODE_MAC) {
		rc = sja1105_cgu_rmii_ext_tx_clk_config(fd, spi_setup, port);
		if (rc < 0) {
			goto error;
		}
	}
	return 0;
error:
	return -1;
}

static int rgmii_clocking_setup(int fd, struct spi_setup *spi_setup, int port)
{
	int rc;

	if (general_config.verbose) {
		printf("configuring rgmii clocking for port %d...\n", port);
	}
	rc = sja1105_cgu_idiv_config(fd, spi_setup, port);
	if (rc < 0) {
		goto error;
	}
	rc = sja1105_cgu_rgmii_tx_clk_config(fd, spi_setup, port);
	if (rc < 0) {
		goto error;
	}
	return 0;
error:
	return -1;
}

int sja1105_clocking_setup(struct spi_setup *spi_setup, struct sja1105_xmii_params_table *params)
{
	int rc;
	int fd;
	int i;

	fd = configure_spi(spi_setup);
	if (fd < 0) {
		goto out;
	}

	for (i = 0; i < 5; i++) {
		if (params->xmii_mode[i] == XMII_SPEED_MII) {
			mii_clocking_setup(fd, spi_setup, i, params->phy_mac[i]);
		} else if (params->xmii_mode[i] == XMII_SPEED_RMII) {
			rmii_clocking_setup(fd, spi_setup, i, params->phy_mac[i]);
		} else if (params->xmii_mode[i] == XMII_SPEED_RGMII) {
			rgmii_clocking_setup(fd, spi_setup, i);
		} else {
			fprintf(stderr, "Invalid xmii_mode for port %d specified: %"
			        PRIu64 "\n", i, params->xmii_mode[i]);
			rc = -1;
			goto out_1;
		}
	}
	rc = 0;
out_1:
	close(fd);
out:
	return rc;
}

