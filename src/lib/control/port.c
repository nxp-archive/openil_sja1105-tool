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
#include <lib/include/gtable.h>
#include <lib/include/port-control.h>
#include <lib/include/status.h>
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

/* This function just packs/unpacks the structure for writing to
 * UM10944.pdf Table 62 (address 30h), which commits a change of
 * CBS parameters.
 * Care must be taken that a prior write be performed to Table 63
 * (address 2Fh to 2Ch) in order to set the CBS parameters to
 * the desired values. */
static void
sja1105_cbs_cmd_access(void *buf,
                       struct sja1105_cbs *cbs,
                       int write)
{
	int  (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int    size = 4;
	/* No point to keeping the "valid" bit as part of the
	 * structure, since we will never use it for anything
	 * else than triggering a write */
	uint64_t valid = 1;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(cbs, 0, sizeof(*cbs));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &valid,      31, 31, 4);
	pack_or_unpack(buf, &cbs->index, 19, 16, 4);
	pack_or_unpack(buf, &cbs->port,   5,  3, 4);
	pack_or_unpack(buf, &cbs->prio,   2,  0, 4);
}

void sja1105_cbs_cmd_pack(void *buf,
                          struct sja1105_cbs *cbs)
{
	sja1105_cbs_cmd_access(buf, cbs, 1);
}

void sja1105_cbs_cmd_unpack(void *buf,
                            struct sja1105_cbs *cbs)
{
	sja1105_cbs_cmd_access(buf, cbs, 0);
}

void sja1105_cbs_show(struct sja1105_cbs *cbs)
{
	printf("SHAPER_ID  %" PRIX64 "\n", cbs->index);
	printf("CBS_PORT   %" PRIX64 "\n", cbs->port);
	printf("CBS_PRIO   %" PRIX64 "\n", cbs->prio);
	printf("CREDIT_LO  %" PRIX64 "\n", cbs->credit_lo);
	printf("CREDIT_HI  %" PRIX64 "\n", cbs->credit_hi);
	printf("SEND_SLOPE %" PRIX64 "\n", cbs->send_slope);
	printf("IDLE_SLOPE %" PRIX64 "\n", cbs->idle_slope);
}

/* Wrapper around sja1105_spi_send_packed_buf() */
int sja1105_cbs_commit(struct sja1105_spi_setup *spi_setup,
                       struct sja1105_cbs *cbs)
{
	const int CBS_CONTROL_ADDR = 0x30;
	const int CBS_DATA_ADDR    = 0x2C;
	/* packed_buf will accomodate 2 transfers, one of
	 * 32 bytes and one of 4 bytes. */
	const int BUF_LEN = 32;
	uint8_t packed_buf[BUF_LEN];
	/* Make pointer arithmetic work on 4 bytes */
	uint32_t *p = (uint32_t*) packed_buf;
	int rc;

	/* Data portion of transaction */
	gtable_pack(p + 0, &cbs->idle_slope, 31, 0, 4);
	gtable_pack(p + 1, &cbs->send_slope, 31, 0, 4);
	gtable_pack(p + 2, &cbs->credit_hi,  31, 0, 4);
	gtable_pack(p + 3, &cbs->credit_lo,  31, 0, 4);
	rc = sja1105_spi_send_packed_buf(spi_setup,
	                                 SPI_WRITE,
	                                 CORE_ADDR + CBS_DATA_ADDR,
	                                 p,
	                                 BUF_LEN);
	if (rc < 0) {
		loge("spi_send_packed_buf failed for cbs data transaction");
		goto out;
	}
	/* Command portion of transaction */
	sja1105_cbs_cmd_pack(packed_buf, cbs);
	rc = sja1105_spi_send_packed_buf(spi_setup,
	                                 SPI_WRITE,
	                                 CORE_ADDR + CBS_CONTROL_ADDR,
	                                 packed_buf,
	                                 4);
	if (rc < 0) {
		loge("spi_send_packed_buf failed for cbs command transaction");
		goto out;
	}
out:
	return rc;
}

