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

int configure_spi(const struct spi_setup *spi_setup)
{
	int ret = 0;
	int fd;

	logv("configuring device %s", spi_setup->device);
	fd = open(spi_setup->device, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "can't open device");
		ret = fd;
		goto out_1;
	}
	/* spi mode */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &spi_setup->mode);
	if (ret == -1) {
		fprintf(stderr, "can't set spi mode");
		goto out_2;
	}
	ret = ioctl(fd, SPI_IOC_RD_MODE, &spi_setup->mode);
	if (ret == -1) {
		fprintf(stderr, "can't get spi mode");
		goto out_2;
	}
	/* bits per word */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spi_setup->bits);
	if (ret == -1) {
		fprintf(stderr, "can't set bits per word");
		goto out_2;
	}
	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &spi_setup->bits);
	if (ret == -1) {
		fprintf(stderr, "can't get bits per word");
		goto out_2;
	}
	/* max speed hz */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_setup->speed);
	if (ret == -1) {
		fprintf(stderr, "can't set max speed hz");
		goto out_2;
	}
	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_setup->speed);
	if (ret == -1) {
		fprintf(stderr, "can't get max speed hz");
		goto out_2;
	}
	logv("spi mode: %d",      spi_setup->mode);
	logv("bits per word: %d", spi_setup->bits);
	logv("max speed: %d KHz", spi_setup->speed / 1000);
	return fd;
out_2:
	close(fd);
out_1:
	return fd;
}

int spi_transfer(int fd, const struct spi_setup *spi_setup, const void *tx, void *rx, int size)
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
		generic_table_hexdump((void*) tx, size);
		return 0;
	} else {
		memset(rx, 0, size);
		return ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	}
}
