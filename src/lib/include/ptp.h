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
#ifndef _PTP_H
#define _PTP_H

#include "ptp-tables.h"
#include "spi.h"

void sja1105_ptp_config_get(void *buf, struct sja1105_ptp_config *ptp_config);
void sja1105_ptp_config_set(void *buf, struct sja1105_ptp_config *ptp_config);
void sja1105_ptp_config_show(struct sja1105_ptp_config *ptp_config);
int  sja1105_ptp_config_cmd(struct spi_setup *spi_setup,
                            struct sja1105_ptp_config *ptp_config);
void sja1105_ptp_control_get(void *buf, struct sja1105_ptp_control *ptp_control);
void sja1105_ptp_control_set(void *buf, struct sja1105_ptp_control *ptp_control);
void sja1105_ptp_control_show(struct sja1105_ptp_control *ptp_control);
int  sja1105_ptp_control_cmd(struct spi_setup *spi_setup,
                             struct sja1105_ptp_control *ptp_control);

#endif
