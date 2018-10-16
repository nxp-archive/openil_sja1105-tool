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

#include <common.h>

struct sja1105_spi_setup {
	uint64_t    device_id;
	uint64_t    part_nr; /* Needed for P/R distinction (same switch core) */
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

enum sja1105_spi_access_mode {
	SPI_READ = 0,
	SPI_WRITE = 1,
};

const char *sja1105_device_id_string_get(uint64_t device_id, uint64_t part_nr);
int sja1105_device_id_get(struct sja1105_spi_setup *spi_setup,
                          uint64_t *device_id, uint64_t *part_nr);

int sja1105_spi_transfer(const struct sja1105_spi_setup*, const void *tx, void *rx, int size);
int sja1105_spi_configure(struct sja1105_spi_setup*);
void sja1105_spi_message_unpack(void*, struct sja1105_spi_message*);
void sja1105_spi_message_pack(void*, struct sja1105_spi_message*);
void sja1105_spi_message_show(struct sja1105_spi_message*);
int sja1105_spi_send_packed_buf(struct sja1105_spi_setup *spi_setup,
                                enum sja1105_spi_access_mode read_or_write,
                                uint64_t reg_addr,
                                void    *packed_buf,
                                uint64_t size_bytes);
int sja1105_spi_send_int(struct sja1105_spi_setup *spi_setup,
                         enum sja1105_spi_access_mode read_or_write,
                         uint64_t reg_offset,
                         uint64_t *value,
                         uint64_t size_bytes);
int sja1105_spi_send_long_packed_buf(struct sja1105_spi_setup *spi_setup,
                                     enum sja1105_spi_access_mode read_or_write,
                                     uint64_t base_addr,
                                     char    *packed_buf,
                                     uint64_t size_bytes);

#define SIZE_SJA1105_DEVICE_ID 4
#define SIZE_SPI_MSG_HEADER    4
#define SIZE_SPI_MSG_MAXLEN    64 * 4

#endif
