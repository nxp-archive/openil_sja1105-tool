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
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
/* These are our own libraries */
#include <lib/include/static-config.h>
#include <lib/include/gtable.h>
#include <lib/include/spi.h>
#include <common.h>

static void sja1105_spi_message_access(void  *buf,
                                       struct sja1105_spi_message *msg,
                                       int    write)
{
	int (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int size = SIZE_SPI_MSG_HEADER;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(msg, 0, sizeof(*msg));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &msg->access,     31, 31, size);
	pack_or_unpack(buf, &msg->read_count, 30, 25, size);
	pack_or_unpack(buf, &msg->address,    24,  4, size);
}

void sja1105_spi_message_unpack(void *buf, struct sja1105_spi_message *msg)
{
	sja1105_spi_message_access(buf, msg, 0);
}

void sja1105_spi_message_pack(void *buf, struct sja1105_spi_message *msg)
{
	sja1105_spi_message_access(buf, msg, 1);
}

void sja1105_spi_message_show(struct sja1105_spi_message *msg)
{
	printf("SPI Message Header:\n");
	if (msg->access == 1) {
		printf("Access mode write, address 0x%" PRIX64 "\n", msg->address);
	} else {
		printf("Access mode read, %" PRIu64 " 32-bit words, address 0x%"
		       PRIX64 "\n", msg->read_count, msg->address);
	}
}

void spi_message_aggregate(char  *buf,
                           struct sja1105_spi_message *hdr,
                           char  *data,
                           int    data_len)
{
	sja1105_spi_message_pack(buf, hdr);
	memcpy(buf + SIZE_SPI_MSG_HEADER, data, data_len);
}

static int spi_get_next_packet_len(char *buf, char *current, int buf_len)
{
	int distance_to_end = (int)((buf + buf_len) - current);
	if (distance_to_end > SIZE_SPI_MSG_MAXLEN) {
		return SIZE_SPI_MSG_MAXLEN;
	} else {
		return distance_to_end;
	}
}

/* Input:  config_buf and config_buf_len
 *         These represent the binary dump of all the SJA1105 configuration
 *         headers and tables, plus global CRC and device ID.
 *         See UM10944.pdf, Fig. 8, Generic loader format.
 * Output: an array of spi chunks, each telling what buffer to write,
 *         how much of it, and at what SPI address
 */
void spi_get_chunks(char *config_buf, int config_buf_len,
                    struct sja1105_spi_chunk *chunks, int *chunk_count)
{
	struct sja1105_spi_message msg;
	int    packet_len;
	char  *p;

	msg.access     = SPI_WRITE;
	msg.read_count = 0;
	msg.address    = CONFIG_ADDR;

	p = config_buf;
	packet_len = spi_get_next_packet_len(config_buf, p, config_buf_len);
	*chunk_count = 0;
	while (packet_len) {
		chunks[*chunk_count].buf  = p;
		chunks[*chunk_count].size = packet_len;
		chunks[*chunk_count].msg  = msg;
		(*chunk_count)++;
		p           += packet_len;
		msg.address += packet_len / 4;
		packet_len = spi_get_next_packet_len(config_buf, p, config_buf_len);
	}
}

/* If read_or_write is:
 *     * SPI_WRITE: creates and sends an SPI write message at absolute
 *                  address reg_addr, taking size_bytes from *packed_buf
 *     * SPI_READ: creates and sends an SPI read message from absolute
 *                 address reg_addr, writing size_bytes into *packed_buf
 */
inline int
sja1105_spi_send_packed_buf(struct sja1105_spi_setup *spi_setup,
                            enum sja1105_spi_access_mode read_or_write,
                            uint64_t reg_addr,
                            void    *packed_buf,
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
	msg.address    = reg_addr;
	sja1105_spi_message_pack(tx_buf, &msg);

	if (read_or_write == SPI_READ) {
		memset(tx_buf + SIZE_SPI_MSG_HEADER, 0, size_bytes);
	} else if (read_or_write == SPI_WRITE) {
		memcpy(tx_buf + SIZE_SPI_MSG_HEADER, /* dest */
		       packed_buf,                   /* src */
		       size_bytes);                  /* size */
	} else {
		loge("read_or_write must be SPI_READ or SPI_WRITE");
		rc = -EINVAL;
		goto out;
	}

	rc = sja1105_spi_transfer(spi_setup, tx_buf, rx_buf, MSG_LEN);
	if (rc < 0) {
		loge("sja1105_spi_transfer failed");
		goto out;
	}
	if (read_or_write == SPI_READ) {
		memcpy(packed_buf,                   /* dest */
		       rx_buf + SIZE_SPI_MSG_HEADER, /* src */
		       size_bytes);                  /* size */
	}
out:
	return rc;
}

/* If read_or_write is:
 *     * SPI_WRITE: creates and sends an SPI write message at absolute
 *                  address reg_addr, taking size_bytes from *value
 *     * SPI_READ: creates and sends an SPI read message from absolute
 *                 address reg_addr, writing size_bytes into *value
 *
 * The uint64_t *value is unpacked, meaning that it's stored in the native
 * CPU endianness and directly usable by software running on the core.
 *
 * This is a wrapper around sja1105_spi_send_packed_buf().
 *
 */
inline int
sja1105_spi_send_int(struct sja1105_spi_setup *spi_setup,
                     enum sja1105_spi_access_mode read_or_write,
                     uint64_t reg_addr,
                     uint64_t *value,
                     uint64_t size_bytes)
{
	uint8_t packed_buf[size_bytes];
	int rc;

	if (read_or_write == SPI_WRITE) {
		gtable_pack(packed_buf,
		            value, 8 * size_bytes - 1, 0,
		            size_bytes);
	}
	rc = sja1105_spi_send_packed_buf(spi_setup,
	                                 read_or_write,
	                                 reg_addr,
	                                 packed_buf,
	                                 size_bytes);
	if (read_or_write == SPI_READ) {
		gtable_unpack(packed_buf,
		              value, 8 * size_bytes - 1, 0,
		              size_bytes);
	}
	return rc;
}

