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
#include <lib/include/static-config.h>
#include <lib/include/clock.h>
#include <lib/include/spi.h>
#include <common.h>

int sja1105_clocking_setup(struct sja1105_spi_setup *spi_setup,
                           struct sja1105_xmii_params_table *params,
                           struct sja1105_mac_config_entry  *mac_config)
{
	int speed_mbps;
	int rc = 0;
	int i;

	for (i = 0; i < 5; i++) {
		switch (mac_config[i].speed) {
		case 1: speed_mbps = 1000; break;
		case 2: speed_mbps = 100;  break;
		case 3: speed_mbps = 10;   break;
		default: loge("auto speed not yet supported"); return -1;
		}
		if (params->xmii_mode[i] == XMII_SPEED_MII) {
			mii_clocking_setup(spi_setup, i, params->phy_mac[i]);
		} else if (params->xmii_mode[i] == XMII_SPEED_RMII) {
			rmii_clocking_setup(spi_setup, i, params->phy_mac[i]);
		} else if (params->xmii_mode[i] == XMII_SPEED_RGMII) {
			rgmii_clocking_setup(spi_setup, i, speed_mbps);
		} else {
			loge("Invalid xmii_mode for port %d specified: %" PRIu64,
			     i, params->xmii_mode[i]);
			rc = -EINVAL;
			goto out;
		}
	}
out:
	return rc;
}

