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

static void print_usage()
{
	printf("Usage: sja1105-tool reset [ cold | warm ] \n");
}

int sja1105_reset(const struct spi_setup *spi_setup, struct sja1105_reset_ctrl *reset)
{
	struct sja1105_spi_message msg;
	uint8_t tx_buf[RGU_MSG_LEN];
	uint8_t rx_buf[RGU_MSG_LEN];
	uint8_t *reset_ctrl_ptr;
	int fd;

	fd = configure_spi(spi_setup);
	if (fd < 0) {
		goto out_1;
	}

	msg.access     = SPI_WRITE;
	msg.read_count = 0;
	msg.address    = RGU_ADDR;

	sja1105_spi_message_set(tx_buf, &msg);
	reset_ctrl_ptr = tx_buf + SIZE_SPI_MSG_HEADER;
	generic_table_field_set(reset_ctrl_ptr, &reset->rst_ctrl, 8, 0, 4);

	logv("%s resetting switch",
	    (reset->rst_ctrl == RGU_WARM) ? "Warm" : "Cold");
	spi_transfer(fd, spi_setup, tx_buf, rx_buf, RGU_MSG_LEN);
	close(fd);
	return 0;
out_1:
	return -1;
}

int rgu_parse_args(struct spi_setup *spi_setup, int argc, char **argv)
{
	struct sja1105_reset_ctrl reset;

	if (argc < 1) {
		goto parse_error;
	}
	if (matches(argv[0], "warm") == 0) {
		reset.rst_ctrl = RGU_WARM;
	} else if (matches(argv[0], "cold") == 0) {
		reset.rst_ctrl = RGU_COLD;
	} else {
		goto parse_error;
	}
	return sja1105_reset(spi_setup, &reset);
parse_error:
	print_usage();
	return -1;
}

