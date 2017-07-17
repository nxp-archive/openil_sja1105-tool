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
#include <string.h>
#include <inttypes.h>
#include <errno.h>
/* These are our own include files */
#include <lib/include/ptp.h>
#include <lib/include/gtable.h>
#include <lib/include/status-tables.h>
#include <lib/include/spi.h>
#include <common.h>

/* Wrapper around sja1105_spi_send_int() using SPI_READ
 * at CORE_ADDR + reg_offset */
int sja1105_ptp_read_cmd(struct sja1105_spi_setup *spi_setup,
                         uint64_t reg_offset,
                         uint64_t *value,
                         uint64_t size_bytes)
{
	return sja1105_spi_send_int(spi_setup,
	                            SPI_READ,
	                            CORE_ADDR + reg_offset,
	                            value,
	                            size_bytes);
}

/* Wrapper around sja1105_spi_send_int() using SPI_WRITE
 * at CORE_ADDR + reg_offset */
int sja1105_ptp_write_cmd(struct sja1105_spi_setup *spi_setup,
                          uint64_t reg_offset,
                          uint64_t *value,
                          uint64_t size_bytes)
{
	return sja1105_spi_send_int(spi_setup,
	                            SPI_WRITE,
	                            CORE_ADDR + reg_offset,
	                            value,
	                            size_bytes);
}

int sja1105_ptp_ts_clk_get(struct sja1105_spi_setup *spi_setup, uint64_t *value)
{
	return sja1105_ptp_read_cmd(spi_setup,
	                            SJA1105_PTPTSCLK_ADDR,
	                            value, 8);
}

int sja1105_ptp_clk_get(struct sja1105_spi_setup *spi_setup, uint64_t *value)
{
	return sja1105_ptp_read_cmd(spi_setup,
	                            SJA1105_PTPCLKVAL_ADDR,
	                            value, 8);
}

int sja1105_ptp_clk_write(struct sja1105_spi_setup *spi_setup, uint64_t value)
{
	return sja1105_ptp_write_cmd(spi_setup,
	                            SJA1105_PTPCLKVAL_ADDR,
	                            &value, 8);
}

int sja1105_ptp_clk_set(struct sja1105_spi_setup *spi_setup, uint64_t value)
{
	int rc;

	rc = sja1105_ptp_set_add_mode(spi_setup, PTP_SET_MODE);
	if (rc < 0) {
		loge("failed configuring set mode for ptp clk");
		goto out;
	}
	rc = sja1105_ptp_clk_write(spi_setup, value);
out:
	return rc;
}

int sja1105_ptp_clk_add(struct sja1105_spi_setup *spi_setup, uint64_t value)
{
	int rc;

	rc = sja1105_ptp_set_add_mode(spi_setup, PTP_ADD_MODE);
	if (rc < 0) {
		loge("failed configuring add mode for ptp clk");
		goto out;
	}
	rc = sja1105_ptp_clk_write(spi_setup, value);
out:
	return rc;
}

int sja1105_ptp_clk_rate_set(struct sja1105_spi_setup *spi_setup,
                             uint64_t value)
{
	return sja1105_ptp_write_cmd(spi_setup,
	                            SJA1105_PTPCLKRATE_ADDR,
	                            &value, 4);
}

int sja1105_ptp_configure(struct sja1105_spi_setup *spi_setup,
                          struct sja1105_ptp_config *ptp_config)
{
	int rc = 0;
	struct sja1105_ptp_ctrl_cmd ptp_control;

	rc += sja1105_ptp_write_cmd(spi_setup,
	                           SJA1105_PTPPINDUR_ADDR,
	                           &ptp_config->pin_duration, 4);
	rc += sja1105_ptp_write_cmd(spi_setup,
	                           SJA1105_PTPPINST_ADDR,
	                           &ptp_config->pin_start, 8);
	rc += sja1105_ptp_write_cmd(spi_setup,
	                           SJA1105_PTPSCHTM_ADDR,
	                           &ptp_config->schedule_time, 8);
	rc += sja1105_ptp_write_cmd(spi_setup,
	                           SJA1105_PTPCLKCORP_ADDR,
	                           &ptp_config->schedule_correction_period, 4);
	memset(&ptp_control, 0, sizeof(ptp_control));
	/* Compose a single PTP control command */
	ptp_control.valid = 1;
	ptp_control.start_schedule     = ptp_config->schedule_autostart;
	ptp_control.start_pin_toggle   = ptp_config->pin_toggle_autostart;
	ptp_control.ts_based_on_ptpclk = ptp_config->ts_based_on_ptpclk;
	if (ptp_config->pin_toggle_autostart ||
	    ptp_config->schedule_autostart) {
		logv("Resetting PTP clock");
		ptp_control.reset = 1;
	}
	rc += sja1105_ptp_ctrl_cmd_send(spi_setup, &ptp_control);
	return rc;
}

static void
sja1105_ptp_config_access(void *buf,
                          struct sja1105_ptp_config *config,
                          int write)
{
	int (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int size = SIZE_PTP_CONFIG;
	/* Pointer arithmetic on 8 bytes */
	uint64_t *p = buf;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(config, 0, sizeof(*config));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(p + 0, &config->pin_duration,  63, 0, 8);
	pack_or_unpack(p + 1, &config->pin_start,     63, 0, 8);
	pack_or_unpack(p + 2, &config->schedule_time, 63, 0, 8);
	pack_or_unpack(p + 3, &config->schedule_correction_period,
	                                              63, 0, 8);
	pack_or_unpack(p + 4, &config->ts_based_on_ptpclk,
	                                              63, 0, 8);
	pack_or_unpack(p + 5, &config->schedule_autostart,
	                                              63, 0, 8);
	pack_or_unpack(p + 6, &config->pin_toggle_autostart,
	                                              63, 0, 8);
}

void
sja1105_ptp_config_unpack(void *buf,
                          struct sja1105_ptp_config *config)
{
	sja1105_ptp_config_access(buf, config, 0);
}

void
sja1105_ptp_config_pack(void *buf,
                        struct sja1105_ptp_config *config)
{
	sja1105_ptp_config_access(buf, config, 1);
}

void
sja1105_ptp_config_fmt_show(char *print_buf,
                            char *fmt,
                            struct sja1105_ptp_config *config)
{
	formatted_append(print_buf, fmt, "PTP Configuration:");
	formatted_append(print_buf, fmt, "PTPPINDUR (Pin Duration): %" PRIx64,
	                 config->pin_duration);
	formatted_append(print_buf, fmt, "PTPPINST (Pin Start): %" PRIx64,
	                 config->pin_start);
	formatted_append(print_buf, fmt, "PTPSCHTM (Schedule Time): %" PRIx64,
	                 config->schedule_time);
	formatted_append(print_buf, fmt, "PTPCLKCORP (Schedule Correction "
	                 "Period): %" PRIx64,
	                 config->schedule_correction_period);
	formatted_append(print_buf, fmt, "CORRCLK4TS (TS based on PTPCLK): %"
	                 PRIx64, config->ts_based_on_ptpclk);
	formatted_append(print_buf, fmt, "PTPSTRTSCH (Schedule Autostart): %"
	                 PRIx64, config->schedule_autostart);
	formatted_append(print_buf, fmt, "STARTPTPCP (Pin Toggle Autostart): %"
	                 PRIx64, config->pin_toggle_autostart);
}

void
sja1105_ptp_config_show(struct sja1105_ptp_config *config)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_ptp_config_fmt_show(print_buf, fmt, config);
	puts(print_buf);
}

/* IEEE 754 (double precision): fractional part is 52 bits.
 * Take most significant 31 bits of that.
 */
#define DOUBLE_FRACTION_AS_UINT32(double_ptr) \
	(uint32_t)(((uint64_t) *double_ptr >> (52 - 31)) & 0x7fffffff)

int sja1105_ptpclkrate_from_ratio(double ratio, uint32_t *ptpclkrate)
{
	uint64_t *ratio_ptr;
	int rc = 0;

	if (ratio <= 0 || ratio >= 2) {
		loge("ratio %lf outside of range", ratio);
		rc = -ERANGE;
		goto out;
	}
	ratio_ptr = (uint64_t *)(&ratio);
	if (ratio < 1) {
		/* Put fractional part of proper ratio (larger than 1)
		 * into result. Integer part (msb) is zero */
		ratio += 1;
		*ptpclkrate = DOUBLE_FRACTION_AS_UINT32(ratio_ptr);
	} else {
		/* Put fractional part of ratio into result.
		 * Make integer part (msb) 1 */
		*ptpclkrate = DOUBLE_FRACTION_AS_UINT32(ratio_ptr);
		*ptpclkrate |= 0x80000000;
	}
out:
	return rc;
}

