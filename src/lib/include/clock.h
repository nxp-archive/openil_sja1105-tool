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

#include "spi.h"
#include "agu-tables.h"
#include "cgu-tables.h"
#include "static-config.h"

void sja1105_cgu_pll_control_show(struct sja1105_cgu_pll_control*);
void sja1105_cgu_pll_control_pack(void*, struct sja1105_cgu_pll_control*);
void sja1105_cgu_pll_control_unpack(void*, struct sja1105_cgu_pll_control*);
void sja1105_cgu_mii_control_pack(void*, struct sja1105_cgu_mii_control*);
void sja1105_cgu_mii_control_unpack(void*, struct sja1105_cgu_mii_control*);
void sja1105_cgu_mii_control_show(struct sja1105_cgu_mii_control *mii_control);
int sja1105_cgu_rmii_ref_clk_config(struct sja1105_spi_setup*, int);
int sja1105_cgu_rmii_ext_tx_clk_config(struct sja1105_spi_setup*, int);
int sja1105_cgu_mii_tx_clk_config(struct sja1105_spi_setup*, int, int);
int sja1105_cgu_mii_rx_clk_config(struct sja1105_spi_setup*, int);
int sja1105_cgu_mii_ext_tx_clk_config(struct sja1105_spi_setup*, int);
int sja1105_cgu_mii_ext_rx_clk_config(struct sja1105_spi_setup*, int);
int sja1105_cgu_rgmii_tx_clk_config(struct sja1105_spi_setup*, int, int);
int sja1105_rgmii_cfg_pad_tx_config(struct sja1105_spi_setup *spi_setup, int port);
void sja1105_cgu_idiv_pack(void*, struct sja1105_cgu_idiv*);
void sja1105_cgu_idiv_unpack(void*, struct sja1105_cgu_idiv*);
void sja1105_cgu_idiv_show(void*, struct sja1105_cgu_idiv*);
int sja1105_cgu_idiv_config(struct sja1105_spi_setup*, int, int, int);
int sja1105_clocking_setup(struct sja1105_spi_setup*, struct sja1105_xmii_params_table*,
                           struct sja1105_mac_config_entry*);

int mii_clocking_setup(struct sja1105_spi_setup *spi_setup, int port,
                       int mii_mode);
int rmii_clocking_setup(struct sja1105_spi_setup *spi_setup, int port,
                        int rmii_mode);
int rgmii_clocking_setup(struct sja1105_spi_setup *spi_setup,
                         int port, int speed_mbps);

void sja1105_cfg_pad_mii_tx_pack(void*, struct sja1105_cfg_pad_mii_tx*);
void sja1105_cfg_pad_mii_tx_unpack(void*, struct sja1105_cfg_pad_mii_tx*);
void sja1105_cfg_pad_mii_tx_show(struct sja1105_cfg_pad_mii_tx*);

#endif
