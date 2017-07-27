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
/* These are our own include files */
#include <lib/include/static-config.h>
#include <lib/include/gtable.h>
#include <lib/include/spi.h>
#include <common.h>

int sja1105_reset(const struct sja1105_spi_setup *spi_setup, struct sja1105_reset_ctrl *reset)
{
	struct sja1105_spi_message msg;
	uint8_t tx_buf[RGU_MSG_LEN];
	uint8_t rx_buf[RGU_MSG_LEN];
	uint8_t *reset_ctrl_ptr;

	msg.access     = SPI_WRITE;
	msg.read_count = 0;
	msg.address    = RGU_ADDR;

	sja1105_spi_message_pack(tx_buf, &msg);
	reset_ctrl_ptr = tx_buf + SIZE_SPI_MSG_HEADER;
	gtable_pack(reset_ctrl_ptr, &reset->rst_ctrl, 8, 0, 4);

	logv("%s resetting switch",
	    (reset->rst_ctrl == RGU_WARM) ? "Warm" : "Cold");
	return sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, RGU_MSG_LEN);
}
