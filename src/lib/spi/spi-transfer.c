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
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
/* These are our own libraries */
#include <lib/include/static-config.h>
#include <lib/include/gtable.h>
#include <lib/include/spi.h>
#include <common.h>

/* struct sja1105_spi_setup *setup is bi-directional.
 * On input, the function looks at fields:
 *   ->device (path to spidev char device)
 *   ->mode (clock phase, clock polarity)
 *   ->bits (per word, must be 8)
 *   ->speed (SPI clock in Hz)
 *   ->dry_run (see below)
 * On output, the function:
 *   - is a no-op, if dry_run is true
 *   - sets field ->fd to a ioctl-able file descriptor
 *     to the SPI device (responsibility goes to the
 *     caller to close it)
 *   - sets field ->device_id to the identified Device ID
 *     of the chip (read over SPI).
 */
int sja1105_spi_configure(struct sja1105_spi_setup *spi_setup)
{
	struct ioctl_cmd {
		int      read_ioctl;
		int      write_ioctl;
		char    *description;
		uint64_t value;
	} cmds[] = {
		{
			.read_ioctl  = SPI_IOC_RD_MODE,
			.write_ioctl = SPI_IOC_WR_MODE,
			.description = "SPI mode (clock phase, polarity)",
			.value       = spi_setup->mode,
		}, {
			.read_ioctl  = SPI_IOC_RD_BITS_PER_WORD,
			.write_ioctl = SPI_IOC_WR_BITS_PER_WORD,
			.description = "bits per word",
			.value       = spi_setup->bits,
		}, {
			.read_ioctl  = SPI_IOC_RD_MAX_SPEED_HZ,
			.write_ioctl = SPI_IOC_WR_MAX_SPEED_HZ,
			.description = "max SPI clock speed (Hz)",
			.value       = spi_setup->speed,
		}
	};
	/* Must be initialized with zero, because the read-back
	 * ioctl will not access it in 64-bit mode, so part of
	 * tmp would be junk otherwise.
	 */
	uint64_t tmp = 0;
	unsigned int i;
	int fd, rc;

	if (spi_setup->dry_run) {
		/* Pass an invalid fd, but also do not fail.
		 * As long as the caller just passes the spi_setup
		 * along to the sja1105_spi_transfer function below,
		 * and doesn't do anything crazy with it,
		 * this should be a non-issue.
		 */
		logv("%s: spi_setup is in dry run mode, no-op", __func__);
		spi_setup->fd = -1;
		rc = 0;
		goto out_dry_run;
	}

	logv("configuring device %s", spi_setup->device);
	fd = open(spi_setup->device, O_RDWR);
	if (fd < 0) {
		loge("can't open device");
		rc = fd;
		goto out_open_failed;
	}
	for (i = 0; i < ARRAY_SIZE(cmds); i++) {
		rc = ioctl(fd, cmds[i].write_ioctl, &cmds[i].value);
		if (rc < 0) {
			loge("cannot write %s %" PRIu64, cmds[i].description,
			     cmds[i].value);
			goto out_ioctl_failed;
		}
		rc = ioctl(fd, cmds[i].read_ioctl, &tmp);
		if (rc < 0) {
			loge("cannot read back %s", cmds[i].description);
			goto out_ioctl_failed;
		}
		if (cmds[i].value != tmp) {
			loge("%s: written %" PRIu64 ", read back %" PRIu64,
			     cmds[i].description, cmds[i].value, tmp);
			goto out_mismatched_read_write;
		}
	}
	spi_setup->fd = fd;
	logv("spi mode: %d",      spi_setup->mode);
	logv("bits per word: %d", spi_setup->bits);
	logv("max speed: %d KHz", spi_setup->speed / 1000);

	if (spi_setup->device_id == SJA1105_NO_DEVICE_ID) {
		/* Device ID was not overridden from sja1105.conf.
		 * Check that we are talking with a compatible
		 * device over SPI. */
		rc = sja1105_device_id_get(spi_setup, &spi_setup->device_id,
		                          &spi_setup->part_nr);
		if (rc < 0) {
			goto out_unknown_device_id;
		}
	}
	goto out_ok;
out_mismatched_read_write:
out_unknown_device_id:
out_ioctl_failed:
	close(fd);
out_open_failed:
out_dry_run:
out_ok:
	return rc;
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
	int saved_ioctl_result;
	int rc = 0;

	if (spi_setup->dry_run) {
		printf("spi-transfer: size %d bytes\n", size);
		gtable_hexdump((void*) tx, size);
		/* Do not fail */
		saved_ioctl_result = size;
	} else {
		memset(rx, 0, size);
		if (flock(spi_setup->fd, LOCK_EX) < 0) {
			loge("locking spi device failed");
			rc = -EAGAIN;
			goto out;
		}
		rc = ioctl(spi_setup->fd, SPI_IOC_MESSAGE(1), &tr);
		if (rc < 0) {
			loge("ioctl failed");
			/* Fall-through */
		}
		saved_ioctl_result = rc;
		rc = flock(spi_setup->fd, LOCK_UN);
		if (rc < 0) {
			loge("unlocking spi device failed");
			rc = -EAGAIN;
		}
	}
out:
	/* The SPI_IOC_MESSAGE ioctl does not return 0 on success, but
	 * the number of transferred bytes instead.
	 * https://github.com/openil/sja1105-tool/issues/8
	 */
	if (rc < 0) {
		return rc;
	} else {
		return (saved_ioctl_result == size) ? 0 : -EIO;
	}
}

