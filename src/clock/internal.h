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
#ifndef _CGU_INTERNAL_H
#define _CGU_INTERNAL_H

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include "../spi/external.h"
#include "../tables/external.h"
#include "../common.h"

void sja1105_cgu_idiv_set(void*, struct sja1105_cgu_idiv*);
void sja1105_cgu_idiv_get(void*, struct sja1105_cgu_idiv*);
void sja1105_cgu_idiv_show(struct sja1105_cgu_idiv*);
int  sja1105_cgu_idiv_config(struct spi_setup*, int, int, int);
void sja1105_cgu_mii_control_set(void*, struct sja1105_cgu_mii_control*);
void sja1105_cgu_mii_control_get(void*, struct sja1105_cgu_mii_control*);
void sja1105_cgu_mii_control_show(struct sja1105_cgu_mii_control*);
int  sja1105_cgu_rgmii_tx_clk_config(struct spi_setup*, int, int);
int  sja1105_cgu_rmii_ref_clk_config(struct spi_setup*, int);
int  sja1105_cgu_rmii_ext_tx_clk_config(struct spi_setup*, int);
int  sja1105_cgu_mii_tx_clk_config(struct spi_setup*, int);
int  sja1105_cgu_mii_rx_clk_config(struct spi_setup*, int);
int  sja1105_cgu_mii_ext_tx_clk_config(struct spi_setup*, int);
int  sja1105_cgu_mii_ext_rx_clk_config(struct spi_setup*, int);
void sja1105_cgu_pll_control_set(void*, struct sja1105_cgu_pll_control*);
void sja1105_cgu_pll_control_get(void*, struct sja1105_cgu_pll_control*);
void sja1105_cgu_pll_control_show(struct sja1105_cgu_pll_control*);
int  sja1105_rgmii_cfg_pad_tx_config(struct spi_setup*, int);

#endif
