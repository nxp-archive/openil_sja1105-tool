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

static void sja1105_ptp_config_access(
		void *buf,
		struct sja1105_ptp_config *ptp_config,
		int write)
{
	int  (*get_or_set)(void*, uint64_t*, int, int, int);
	int    size = 4;
	/* So that pointer additions translate to 4 bytes */
	uint32_t *p = (uint32_t*) buf;

	if (write == 0) {
		get_or_set = generic_table_field_get;
		memset(ptp_config, 0, sizeof(*ptp_config));
	} else {
		get_or_set = generic_table_field_set;
		memset(buf, 0, size);
	}
	/* 0x1D, PTPCLKCORP */
	get_or_set(p + 11, &ptp_config->schedule_correction_period,
	                                               31, 0, 4);
	/* 0x1A, PTPCLKRATE */
	get_or_set(p + 8,  &ptp_config->clk_rate,      31, 0, 4);
	/* 0x16, PTPPINDUR */
	get_or_set(p + 4,  &ptp_config->pin_duration,  31, 0, 4);
	/* 0x14, PTPPINST */
	get_or_set(p + 2,  &ptp_config->pin_start,     63, 0, 8);
	/* 0x12, PTPSCHTM */
	get_or_set(p + 0,  &ptp_config->schedule_time, 63, 0, 8);
}

void sja1105_ptp_config_set(void *buf,
                            struct sja1105_ptp_config *ptp_config)
{
	sja1105_ptp_config_access(buf, ptp_config, 1);
}

void sja1105_ptp_config_get(void *buf,
                            struct sja1105_ptp_config *ptp_config)
{
	sja1105_ptp_config_access(buf, ptp_config, 0);
}

void sja1105_ptp_config_show(struct sja1105_ptp_config *ptp_config)
{
	printf("PTPPINDUR  %" PRIX64 "\n", ptp_config->pin_duration);
	printf("PTPPINST   %" PRIX64 "\n", ptp_config->pin_start);
	printf("PTPSCHTM   %" PRIX64 "\n", ptp_config->schedule_time);
	printf("PTPCLKCORP %" PRIX64 "\n", ptp_config->
	                                   schedule_correction_period);
	printf("PTPCLKRATE %" PRIX64 "\n", ptp_config->clk_rate);
}

int sja1105_ptp_config_cmd(struct sja1105_spi_setup *spi_setup,
                           struct sja1105_ptp_config *ptp_config)
{
	const int PTP_CONFIG_ADDR = 0x12;
	const int SIZE_PTP_CONFIG = (0x1D - 0x12 + 1) * 4;
	const int MSG_LEN         = SIZE_PTP_CONFIG + SIZE_SPI_MSG_HEADER;
	struct sja1105_spi_message msg;
	uint8_t tx_buf[MSG_LEN];
	uint8_t rx_buf[MSG_LEN];
	int rc;

	memset(rx_buf, 0, MSG_LEN);

	msg.access     = SPI_WRITE;
	msg.read_count = SIZE_PTP_CONFIG / 4;
	msg.address    = CORE_ADDR + PTP_CONFIG_ADDR;
	sja1105_spi_message_set(tx_buf, &msg);

	sja1105_ptp_config_set(tx_buf + SIZE_SPI_MSG_HEADER, ptp_config);

	rc = sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, MSG_LEN);
	if (rc < 0) {
		loge("sja1105_spi_transfer failed");
		goto out;
	}
	/* TODO: examine rx_buf.
	 * start_schedule on read is found 1 if clksrc is set to ptp (11)
	 * and the schedule is running correctly
	 * stop_schedule is found 1 on read if clksrc is set to ptp
	 * and the schedule stopped successfully
	 **/
out:
	return rc;
}

int sja1105_ptp_ts_clk_get(struct sja1105_spi_setup *spi_setup, uint64_t *value)
{
	const int PTPTSCLK_ADDR = 0x1B;
	const int MSG_LEN = SIZE_SPI_MSG_HEADER + 8;
	struct sja1105_spi_message msg;
	uint8_t tx_buf[MSG_LEN];
	uint8_t rx_buf[MSG_LEN];
	int rc;

	memset(rx_buf, 0, MSG_LEN);

	msg.access     = SPI_READ;
	msg.read_count = 2;
	msg.address    = CORE_ADDR + PTPTSCLK_ADDR;
	sja1105_spi_message_set(tx_buf, &msg);

	rc = sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, MSG_LEN);
	if (rc < 0) {
		loge("sja1105_spi_transfer failed");
		goto out;
	}
	*value = *(uint64_t*)(rx_buf + SIZE_SPI_MSG_HEADER);
out:
	return rc;
}

int sja1105_ptp_clk_get(struct sja1105_spi_setup *spi_setup, uint64_t *value)
{
	const int PTPCLKVAL_ADDR = 0x18;
	const int MSG_LEN = SIZE_SPI_MSG_HEADER + 8;
	struct sja1105_spi_message msg;
	uint8_t tx_buf[MSG_LEN];
	uint8_t rx_buf[MSG_LEN];
	int rc;

	memset(rx_buf, 0, MSG_LEN);

	msg.access     = SPI_READ;
	msg.read_count = 2;
	msg.address    = CORE_ADDR + PTPCLKVAL_ADDR;
	sja1105_spi_message_set(tx_buf, &msg);

	rc = sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, MSG_LEN);
	if (rc < 0) {
		loge("sja1105_spi_transfer failed");
		goto out;
	}
	generic_table_field_get(rx_buf + SIZE_SPI_MSG_HEADER, value, 63, 0, 8);
out:
	return rc;
}

int sja1105_ptp_clk_write(struct sja1105_spi_setup *spi_setup, uint64_t value)
{
	const int PTPCLKVAL_ADDR = 0x18;
	const int MSG_LEN = SIZE_SPI_MSG_HEADER + 8;
	struct sja1105_spi_message msg;
	uint8_t tx_buf[MSG_LEN];
	uint8_t rx_buf[MSG_LEN];
	int rc;

	memset(rx_buf, 0, MSG_LEN);

	msg.access     = SPI_WRITE;
	msg.read_count = 0;
	msg.address    = CORE_ADDR + PTPCLKVAL_ADDR;
	sja1105_spi_message_set(tx_buf, &msg);

	generic_table_field_set(tx_buf + SIZE_SPI_MSG_HEADER, &value, 63, 0, 8);

	rc = sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, MSG_LEN);
	if (rc < 0) {
		loge("sja1105_spi_transfer failed");
		goto out;
	}
out:
	return rc;
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
	const int PTPCLKRATE_ADDR = 0x1A;
	const int MSG_LEN = SIZE_SPI_MSG_HEADER + 4;
	struct sja1105_spi_message msg;
	uint8_t tx_buf[MSG_LEN];
	uint8_t rx_buf[MSG_LEN];
	int rc;

	memset(rx_buf, 0, MSG_LEN);

	msg.access     = SPI_WRITE;
	msg.read_count = 0;
	msg.address    = CORE_ADDR + PTPCLKRATE_ADDR;
	sja1105_spi_message_set(tx_buf, &msg);

	generic_table_field_set(tx_buf + SIZE_SPI_MSG_HEADER, &value, 31, 0, 4);

	rc = sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, MSG_LEN);
	if (rc < 0) {
		loge("sja1105_spi_transfer failed");
		goto out;
	}
out:
	return rc;
}
