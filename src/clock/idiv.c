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

static void sja1105_cgu_idiv_access(void *buf, struct sja1105_cgu_idiv *idiv, int write)
{
	int  (*get_or_set)(void*, uint64_t*, int, int, int);
	int    size = 4;

	if (write == 0) {
		get_or_set = generic_table_field_get;
		memset(idiv, 0, sizeof(*idiv));
	} else {
		get_or_set = generic_table_field_set;
		memset(buf, 0, size);
	}
	get_or_set(buf, &idiv->clksrc,    28, 24, 4);
	get_or_set(buf, &idiv->autoblock, 11, 11, 4);
	get_or_set(buf, &idiv->idiv,       5,  2, 4);
	get_or_set(buf, &idiv->pd,         0,  0, 4);
}

void sja1105_cgu_idiv_set(void *buf, struct sja1105_cgu_idiv *idiv)
{
	sja1105_cgu_idiv_access(buf, idiv, 1);
}

void sja1105_cgu_idiv_get(void *buf, struct sja1105_cgu_idiv *idiv)
{
	sja1105_cgu_idiv_access(buf, idiv, 0);
}

void sja1105_cgu_idiv_show(struct sja1105_cgu_idiv *idiv)
{
	printf("CLKSRC    %" PRIX64 "\n", idiv->clksrc);
	printf("AUTOBLOCK %" PRIX64 "\n", idiv->autoblock);
	printf("IDIV      %" PRIX64 "\n", idiv->idiv);
	printf("PD        %" PRIX64 "\n", idiv->pd);
}

int sja1105_cgu_idiv_config(int fd, struct spi_setup *spi_setup,
                            int port, int enabled, int factor)
{
#define MSG_SIZE SIZE_SPI_MSG_HEADER + 4
	struct  sja1105_spi_message msg;
	struct  sja1105_cgu_idiv idiv;
	uint8_t tx_buf[MSG_SIZE];
	uint8_t rx_buf[MSG_SIZE];
	/* UM10944.pdf, Table 78, CGU Register overview */
	int     idiv_offsets[] = {0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

	if (enabled != 0 && enabled != 1) {
		loge("idiv enabled must be true or false");
		return -1;
	}
	if (enabled == 1 && factor != 1 && factor != 10) {
		loge("idiv factor must be 1 or 10");
		return -1;
	}

	memset(tx_buf, 0, MSG_SIZE);
	memset(rx_buf, 0, MSG_SIZE);

	/* Header */
	msg.access     = SPI_WRITE;
	msg.read_count = 0;
	msg.address    = CGU_ADDR + idiv_offsets[port];
	sja1105_spi_message_set(tx_buf, &msg);

	/* Payload */
	idiv.clksrc    = 0x0A;            /* 25MHz */
	idiv.autoblock = 1;               /* Block clk automatically */
	idiv.idiv      = factor - 1;      /* Divide by 1 or 10 */
	idiv.pd        = enabled ? 0 : 1; /* Power down? */
	sja1105_cgu_idiv_set(tx_buf + SIZE_SPI_MSG_HEADER, &idiv);

	return spi_transfer(fd, spi_setup, tx_buf, rx_buf, MSG_SIZE);
}

