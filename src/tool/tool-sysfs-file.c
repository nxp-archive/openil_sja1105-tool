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
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <common.h>
#include <lib/include/spi.h>


/*
 * Read data from a sysfs file.
 * return: >0: number of bytes read, -1: failed
 */
int sysfs_read(struct sja1105_spi_setup *spi_setup, char* name,
               char* buf, size_t len)
{
	int rc;
	int fd;
	char file_name[PATH_MAX];

	snprintf(file_name, PATH_MAX, "%s/%s",
	         spi_setup->device, name);
	fd = open(file_name, O_RDONLY);
	if (fd < 0) {
		logv("%s: could not open file %s", __FUNCTION__, file_name);
		rc = -1;
		goto out;
	}

	rc = read(fd, buf, len);
	if (rc <= 0) {
		logv("%s: could not read file %s", __FUNCTION__, file_name);
		rc = -1;
		goto out_close;
	}
out_close:
	close(fd);
out:
	return rc;
}

/*
 * Write data to a sysfs file.
 * Return: 0: ok, -1: failed
 */
int sysfs_write(struct sja1105_spi_setup *spi_setup, char* name,
                char* buf, size_t len)
{
	int rc;
	int fd;
	char file_name[PATH_MAX];

	snprintf(file_name, PATH_MAX, "%s/%s",
	         spi_setup->device, name);
	fd = open(file_name, O_WRONLY);
	if (fd < 0) {
		logv("%s: could not open file %s", __FUNCTION__, file_name);
		rc = -1;
		goto out;
	}

	rc = write(fd, buf, len);
	if (rc != (int)len) {
		logv("%s: could not write file %s", __FUNCTION__, file_name);
		rc = -1;
		goto out_close;
	}
	rc = 0;
out_close:
	close(fd);
out:
	return rc;
}
