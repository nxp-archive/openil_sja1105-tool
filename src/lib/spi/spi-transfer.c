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
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
/* These are our own libraries */
#include <lib/include/config.h>
#include <lib/include/gtable.h>
#include <lib/include/spi.h>
#include <common.h>

int sja1105_spi_configure(struct sja1105_spi_setup *spi_setup)
{
	int ret = 0;
	int fd;

	logv("configuring device %s", spi_setup->device);
	fd = open(spi_setup->device, O_RDWR);
	if (fd < 0) {
		loge("can't open device");
		ret = fd;
		goto out_1;
	}
	/* spi mode */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &spi_setup->mode);
	if (ret == -1) {
		loge("can't set spi mode");
		goto out_2;
	}
	ret = ioctl(fd, SPI_IOC_RD_MODE, &spi_setup->mode);
	if (ret == -1) {
		loge("can't get spi mode");
		goto out_2;
	}
	/* bits per word */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spi_setup->bits);
	if (ret == -1) {
		loge("can't set bits per word");
		goto out_2;
	}
	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &spi_setup->bits);
	if (ret == -1) {
		loge("can't get bits per word");
		goto out_2;
	}
	/* max speed hz */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_setup->speed);
	if (ret == -1) {
		loge("can't set max speed hz");
		goto out_2;
	}
	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_setup->speed);
	if (ret == -1) {
		loge("can't get max speed hz");
		goto out_2;
	}
	spi_setup->fd = fd;
	logv("spi mode: %d",      spi_setup->mode);
	logv("bits per word: %d", spi_setup->bits);
	logv("max speed: %d KHz", spi_setup->speed / 1000);
	return fd;
out_2:
	close(fd);
out_1:
	return fd;
}

int sja1105_spi_transfer(const struct sja1105_spi_setup *spi_setup,
                         const void *tx, void *rx, int size)
{
	struct spi_ioc_transfer tr = {
		.tx_buf        = (unsigned long)tx,
		.rx_buf        = (unsigned long)rx,
		.len           = size,
		.delay_usecs   = spi_setup->delay,
		.speed_hz      = spi_setup->speed,
		.bits_per_word = spi_setup->bits,
		.cs_change     = spi_setup->cs_change,
	};

	if (spi_setup->dry_run) {
		printf("spi-transfer: size %d bytes\n", size);
		gtable_hexdump((void*) tx, size);
		return 0;
	} else {
		memset(rx, 0, size);
		return ioctl(spi_setup->fd, SPI_IOC_MESSAGE(1), &tr);
	}
}

/* If read_or_write is:
 *     * SPI_WRITE: creates and sends an SPI write message at absolute
 *                  address reg_addr, taking size_bytes from *packed_buf
 *     * SPI_READ: creates and sends an SPI read message from absolute
 *                 address reg_addr, writing size_bytes into *packed_buf
 */
int sja1105_spi_send_packed_buf(struct sja1105_spi_setup *spi_setup,
                                uint64_t read_or_write,
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
	sja1105_spi_message_set(tx_buf, &msg);

	if (read_or_write == SPI_READ) {
		memset(tx_buf + SIZE_SPI_MSG_HEADER, 0, size_bytes);
	} else if (read_or_write == SPI_WRITE) {
		memcpy(tx_buf + SIZE_SPI_MSG_HEADER, /* dest */
		       packed_buf,                   /* src */
		       size_bytes);                  /* size */
	} else {
		loge("read_or_write must be SPI_READ or SPI_WRITE");
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
int sja1105_spi_send_int(struct sja1105_spi_setup *spi_setup,
                         uint64_t read_or_write,
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

