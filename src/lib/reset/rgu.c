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
#include <lib/include/static-config.h>
#include <lib/include/gtable.h>
#include <lib/include/spi.h>
#include <common.h>

static void
sja1105_reset_cmd_access(void *buf,
                          struct sja1105_reset_cmd *reset,
                          int write,
                          uint64_t device_id)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = 4;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(reset, 0, sizeof(*reset));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	if (IS_ET(device_id)) {
		pack_or_unpack(buf, &reset->cold_rst,   3, 3, 4);
		pack_or_unpack(buf, &reset->warm_rst,   2, 2, 4);
	} else {
		pack_or_unpack(buf, &reset->switch_rst, 8, 8, 4);
		pack_or_unpack(buf, &reset->cfg_rst,    7, 7, 4);
		pack_or_unpack(buf, &reset->car_rst,    5, 5, 4);
		pack_or_unpack(buf, &reset->otp_rst,    4, 4, 4);
		pack_or_unpack(buf, &reset->warm_rst,   3, 3, 4);
		pack_or_unpack(buf, &reset->cold_rst,   2, 2, 4);
		pack_or_unpack(buf, &reset->por_rst,    1, 1, 4);
	}
}

void sja1105_reset_cmd_pack(void *buf,
                             struct sja1105_reset_cmd *reset,
                             uint64_t device_id)
{
	sja1105_reset_cmd_access(buf, reset, 1, device_id);
}

void sja1105_reset_cmd_unpack(void *buf,
                               struct sja1105_reset_cmd *reset,
                               uint64_t device_id)
{
	sja1105_reset_cmd_access(buf, reset, 0, device_id);
}

void sja1105_reset_cmd_show(struct sja1105_reset_cmd *reset)
{
	if (reset->switch_rst) {
		logv("Main reset for all functional modules requested");
	}
	if (reset->cfg_rst) {
		logv("Chip configuration reset requested");
	}
	if (reset->car_rst) {
		logv("Clock and reset control logic reset requested");
	}
	if (reset->otp_rst) {
		logv("OTP read cycle for reading product "
		     "config settings requested");
	}
	if (reset->warm_rst) {
		logv("Warm reset requested");
	}
	if (reset->cold_rst) {
		logv("Cold reset requested");
	}
	if (reset->por_rst) {
		logv("Power-on reset requested");
	}
}

int sja1105_reset_cmd_commit(struct sja1105_spi_setup *spi_setup,
                             struct sja1105_reset_cmd *reset)
{
	const int BUF_LEN = 4;
	uint8_t packed_buf[BUF_LEN];
	int rc;

	sja1105_reset_cmd_show(reset);
	if (IS_ET(spi_setup->device_id) &&
	    (reset->switch_rst ||
	     reset->cfg_rst ||
	     reset->car_rst ||
	     reset->otp_rst ||
	     reset->por_rst)) {
		loge("Only warm and cold reset is supported for SJA1105 E/T!");
		rc = -EINVAL;
		goto out;
	}
	sja1105_reset_cmd_pack(packed_buf, reset, spi_setup->device_id);

	rc = sja1105_spi_send_packed_buf(spi_setup,
	                                 SPI_WRITE,
	                                 RGU_ADDR,
	                                 packed_buf,
	                                 BUF_LEN);
out:
	return rc;
}

int sja1105_switch_core_reset(struct sja1105_spi_setup *spi_setup)
{
	struct sja1105_reset_cmd reset = {0};

	reset.switch_rst = 1;
	return sja1105_reset_cmd_commit(spi_setup, &reset);
}

int sja1105_config_reset(struct sja1105_spi_setup *spi_setup)
{
	struct sja1105_reset_cmd reset = {0};

	reset.cfg_rst = 1;
	return sja1105_reset_cmd_commit(spi_setup, &reset);
}

int sja1105_clocking_reset(struct sja1105_spi_setup *spi_setup)
{
	struct sja1105_reset_cmd reset = {0};

	reset.car_rst = 1;
	return sja1105_reset_cmd_commit(spi_setup, &reset);
}

int sja1105_otp_reset(struct sja1105_spi_setup *spi_setup)
{
	struct sja1105_reset_cmd reset = {0};

	reset.otp_rst = 1;
	return sja1105_reset_cmd_commit(spi_setup, &reset);
}

int sja1105_warm_reset(struct sja1105_spi_setup *spi_setup)
{
	struct sja1105_reset_cmd reset = {0};

	reset.warm_rst = 1;
	return sja1105_reset_cmd_commit(spi_setup, &reset);
}

int sja1105_cold_reset(struct sja1105_spi_setup *spi_setup)
{
	struct sja1105_reset_cmd reset = {0};

	reset.cold_rst = 1;
	return sja1105_reset_cmd_commit(spi_setup, &reset);
}

int sja1105_por_reset(struct sja1105_spi_setup *spi_setup)
{
	struct sja1105_reset_cmd reset = {0};

	reset.por_rst = 1;
	return sja1105_reset_cmd_commit(spi_setup, &reset);
}

