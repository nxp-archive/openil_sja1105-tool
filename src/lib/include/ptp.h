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

#include <common.h>
#include "spi.h"

#define SJA1105PQRS_PTPSYNCTS_ADDR  0x1F
#define SJA1105QS_PTPCLKCORP_ADDR   0x1E
#define SJA1105PQRS_PTPTSCLK_ADDR   0x1C
#define SJA1105PQRS_PTPCLKRATE_ADDR 0x1B
#define SJA1105PQRS_PTPCLKVAL_ADDR  0x19
#define SJA1105PQRS_PTPPINDUR_ADDR  0x17
#define SJA1105PQRS_PTPPINST_ADDR   0x15
#define SJA1105QS_PTPSCHTM_ADDR     0x13

#define SJA1105T_PTPCLKCORP_ADDR    0x1D
#define SJA1105ET_PTPTSCLK_ADDR     0x1B
#define SJA1105ET_PTPCLKRATE_ADDR   0x1A
#define SJA1105ET_PTPCLKVAL_ADDR    0x18
#define SJA1105ET_PTPPINDUR_ADDR    0x16
#define SJA1105ET_PTPPINST_ADDR     0x14
#define SJA1105T_PTPSCHTM_ADDR      0x12

#define SIZE_PTP_CONFIG         (7*8)
#define PTP_ADDR                0x0   /* Offset into CORE_ADDR */

enum sja1105_ptp_clk_add_mode {
	PTP_SET_MODE = 0,
	PTP_ADD_MODE,
};

enum sja1105_ptpegr_ts_source {
	TS_PTPTSCLK = 0,
	TS_PTPCLK = 1
};

struct sja1105_ptp_cmd {
	uint64_t ptpstrtsch;   /* start schedule */
	uint64_t ptpstopsch;   /* stop schedule */
	uint64_t startptpcp;   /* start pin toggle  */
	uint64_t stopptpcp;    /* stop pin toggle */
	uint64_t cassync;      /* if cascaded master, trigger a toggle of the
	                          PTP_CLK pin, and store the timestamp of its
	                          1588 clock (ptpclk or ptptsclk, depending on
	                          corrclk4ts), in ptpsyncts.
	                          only for P/Q/R/S series */
	uint64_t resptp;       /* reset */
	uint64_t corrclk4ts;   /* if (1) timestamps are based on ptpclk,
	                          if (0) timestamps are based on ptptsclk */
	uint64_t ptpclksub;    /* only for P/Q/R/S series */
	uint64_t ptpclkadd;    /* enum sja1105_ptp_clk_add_mode */
};

void sja1105_timespec_to_ptp_time(const struct timespec *ts, uint64_t *ptp_time);
void sja1105_ptp_time_to_timespec(struct timespec *ts, uint64_t ptp_time);

int  sja1105_ptp_ts_clk_get(struct sja1105_spi_setup*, struct timespec *ts);
int  sja1105_ptp_clk_get(struct sja1105_spi_setup*, struct timespec *ts);
int  sja1105_ptp_clk_set(struct sja1105_spi_setup*, const struct timespec *ts);
int  sja1105_ptp_clk_add(struct sja1105_spi_setup*, const struct timespec *ts);
int  sja1105_ptp_clk_rate_set(struct sja1105_spi_setup*, double ratio);

void sja1105_ptp_cmd_unpack(void *buf, struct sja1105_ptp_cmd*, uint64_t);
void sja1105_ptp_cmd_pack(void *buf, struct sja1105_ptp_cmd*, uint64_t);
void sja1105_ptp_cmd_show(struct sja1105_ptp_cmd*, uint64_t);
int  sja1105_ptp_cmd_commit(struct sja1105_spi_setup*,
                            struct sja1105_ptp_cmd*);

int  sja1105_ptp_pin_toggle_start(struct sja1105_spi_setup*);
int  sja1105_ptp_pin_toggle_stop(struct sja1105_spi_setup*);
int  sja1105_ptp_reset(struct sja1105_spi_setup*);

int  sja1105_ptp_pin_duration_set(struct sja1105_spi_setup *spi_setup,
                                  const struct timespec *ts);
int  sja1105_ptp_pin_start_time_set(struct sja1105_spi_setup *spi_setup,
                                    const struct timespec *ts);
int  sja1105_ptp_qbv_correction_period_set(struct sja1105_spi_setup *spi_setup,
                                           const struct timespec *ts);
int  sja1105_ptp_qbv_start_time_set(struct sja1105_spi_setup *spi_setup,
                                    const struct timespec *ts);
int  sja1105_ptp_qbv_start(struct sja1105_spi_setup*);
int  sja1105_ptp_qbv_stop(struct sja1105_spi_setup*);
int  sja1105_ptp_qbv_running(struct sja1105_spi_setup*);
int  sja1105_ptp_corrclk4ts_set(struct sja1105_spi_setup *,
                                enum sja1105_ptpegr_ts_source);
int  sja1105_ptpegr_ts_poll(struct sja1105_spi_setup *spi_setup,
                            enum sja1105_ptpegr_ts_source source,
                            int port, int ts_regid,
                            struct timespec *ts);

#endif
