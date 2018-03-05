/******************************************************************************
 * Copyright (c) 2018, NXP Semiconductors
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
#include <lib/include/gtable.h>
#include <lib/include/status-tables.h>
#include <lib/include/port.h>
#include <lib/include/spi.h>
#include <common.h>

int sja1105_inhibit_tx(struct sja1105_spi_setup *spi_setup,
                       struct sja1105_egress_port_mask *port_mask)
{
	const int ETH_PORT_CONTROL_ADDR = 0x11;
	const int BUF_LEN = 4;
	uint8_t packed_buf[BUF_LEN];
	int i;

	for (i = 0; i < SJA1105T_NUM_PORTS; i++) {
		gtable_pack(packed_buf, &port_mask->inhibit_tx[i], i, i, 4);
	}
	return sja1105_spi_send_packed_buf(spi_setup,
	                                   SPI_WRITE,
	                                   CORE_ADDR + ETH_PORT_CONTROL_ADDR,
	                                   packed_buf,
	                                   4);
}

