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
/* These are our own include files */
#include <lib/include/ptp.h>
#include <lib/include/gtable.h>
#include <lib/include/status-tables.h>
#include <lib/include/spi.h>
#include <common.h>

static int sja1105_ptp_config_cmd(struct sja1105_spi_setup *spi_setup,
                                  uint64_t read_or_write,
                                  uint64_t reg_offset,
                                  uint64_t *value,
                                  uint64_t size_bytes)
{
	const int MSG_LEN = size_bytes + SIZE_SPI_MSG_HEADER;
	struct sja1105_spi_message msg;
	uint8_t tx_buf[MSG_LEN];
	uint8_t rx_buf[MSG_LEN];
	int rc;

	memset(rx_buf, 0, MSG_LEN);

	msg.access     = read_or_write;
	msg.read_count = (read_or_write == SPI_READ) ? (size_bytes / 4) : 0;
	msg.address    = CORE_ADDR + reg_offset;
	sja1105_spi_message_set(tx_buf, &msg);

	if (read_or_write == SPI_READ) {
		memset(tx_buf + SIZE_SPI_MSG_HEADER, 0, size_bytes);
	} else if (read_or_write == SPI_WRITE) {
		gtable_pack(tx_buf + SIZE_SPI_MSG_HEADER,
		            value, 8 * size_bytes - 1, 0,
		            size_bytes);
	} else {
		loge("read_or_write must be SPI_READ or SPI_WRITE");
		goto out;
	}

	rc = sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, MSG_LEN);
	if (rc < 0) {
		loge("sja1105_spi_transfer failed");
		goto out;
	}
	if (read_or_write == SPI_READ) {
		gtable_unpack(rx_buf + SIZE_SPI_MSG_HEADER,
		              value, 8 * size_bytes - 1, 0,
		              size_bytes);
	}
out:
	return rc;
}

int sja1105_ptp_ts_clk_get(struct sja1105_spi_setup *spi_setup, uint64_t *value)
{
	return sja1105_ptp_config_cmd(spi_setup, SPI_READ,
	                              SJA1105_PTPTSCLK_ADDR,
	                              value, 8);
}

int sja1105_ptp_clk_get(struct sja1105_spi_setup *spi_setup, uint64_t *value)
{
	return sja1105_ptp_config_cmd(spi_setup, SPI_READ,
	                              SJA1105_PTPCLKVAL_ADDR,
	                              value, 8);
}

int sja1105_ptp_clk_write(struct sja1105_spi_setup *spi_setup, uint64_t value)
{
	return sja1105_ptp_config_cmd(spi_setup, SPI_WRITE,
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
	return sja1105_ptp_config_cmd(spi_setup, SPI_WRITE,
	                              SJA1105_PTPCLKRATE_ADDR,
	                              &value, 4);
}

int sja1105_ptp_configure(struct sja1105_spi_setup *spi_setup,
                          struct sja1105_ptp_config *ptp_config)
{
	int rc = 0;
	struct sja1105_ptp_ctrl_cmd ptp_control;

	rc += sja1105_ptp_config_cmd(spi_setup, SPI_WRITE,
	                             SJA1105_PTPPINDUR_ADDR,
	                             &ptp_config->pin_duration, 4);
	rc += sja1105_ptp_config_cmd(spi_setup, SPI_WRITE,
	                             SJA1105_PTPPINST_ADDR,
	                             &ptp_config->pin_start, 8);
	rc += sja1105_ptp_config_cmd(spi_setup, SPI_WRITE,
	                             SJA1105_PTPSCHTM_ADDR,
	                             &ptp_config->schedule_time, 8);
	rc += sja1105_ptp_config_cmd(spi_setup, SPI_WRITE,
	                             SJA1105_PTPCLKCORP_ADDR,
	                             &ptp_config->schedule_correction_period, 4);
	memset(&ptp_control, 0, sizeof(ptp_control));
	ptp_control.valid = 1;
	ptp_control.start_schedule     = ptp_config->schedule_autostart;
	ptp_control.start_pin_toggle   = ptp_config->pin_toggle_autostart;
	ptp_control.ts_based_on_ptpclk = ptp_config->ts_based_on_ptpclk;
	rc += sja1105_ptp_ctrl_cmd_send(spi_setup, &ptp_control);
	return rc;
}
