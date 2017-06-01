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
#ifndef _SPI_EXTERNAL_H
#define _SPI_EXTERNAL_H

#include <linux/spi/spidev.h>
#include <stdint.h>

struct spi_setup {
	const char *device;
	uint8_t     mode;
	uint8_t     bits;
	uint32_t    speed;
	uint16_t    delay;
	int         cs_change;
	int         dry_run;
	const char *staging_area;
	int         flush;
	int         fd;
};

struct sja1105_spi_message {
	uint64_t access;
	uint64_t read_count;
	uint64_t address;
};

struct sja1105_spi_chunk {
	struct sja1105_spi_message msg;
	char *buf;
	int   size;
};

int spi_transfer(const struct spi_setup*, const void *tx, void *rx, int size);
int configure_spi(struct spi_setup *spi_setup);
void sja1105_spi_message_get(void*, struct sja1105_spi_message*);
void sja1105_spi_message_set(void*, struct sja1105_spi_message*);
void sja1105_spi_message_show(struct sja1105_spi_message*);
void spi_get_chunks(char*, int, struct sja1105_spi_chunk*, int*);
void spi_message_aggregate(char*, struct sja1105_spi_message*, char*, int);

#define SIZE_SJA1105_DEVICE_ID 4
#define SIZE_SPI_MSG_HEADER    4
#define SIZE_SPI_MSG_MAXLEN    64 * 4

#define SPI_READ  0
#define SPI_WRITE 1

#endif
