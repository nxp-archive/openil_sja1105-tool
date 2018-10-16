/******************************************************************************
 * Copyright (c) 2017, NXP Semiconductors
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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "internal.h"
/* From libsja1105 */
#include <lib/include/static-config.h>
#include <lib/include/staging-area.h>
#include <lib/include/port-control.h>
#include <lib/include/gtable.h>
#include <lib/include/spi.h>
#include <lib/include/status.h>
#include <lib/include/reset.h>
#include <lib/include/clock.h>
#include <common.h>

static int reliable_write(int fd, char *buf, int len)
{
	int bytes = 0;
	int rc;

	do {
		rc = write(fd, buf + bytes, len - bytes);
		if (rc < 0) {
			loge("could not write to file");
			goto out;
		}
		bytes += rc;
	} while (rc);
out:
	return rc;
}

static int reliable_read(int fd, char *buf, int len)
{
	int bytes = 0;
	int rc;

	do {
		rc = read(fd, buf + bytes, len - bytes);
		if (rc < 0) {
			loge("read failed");
			goto out;
		}
		bytes += rc;
	} while (rc);
out:
	return rc;
}

int
staging_area_hexdump(const char *staging_area_file)
{
	struct stat stat;
	unsigned int len;
	char *buf;
	int fd;
	int rc;

	fd = open(staging_area_file, O_RDONLY);
	if (fd < 0) {
		loge("Staging area %s does not exist!", staging_area_file);
		rc = fd;
		goto filesystem_error1;
	}
	rc = fstat(fd, &stat);
	if (rc < 0) {
		loge("could not read file size");
		goto filesystem_error2;
	}
	len = stat.st_size;
	buf = (char*) malloc(len * sizeof(char));
	if (!buf) {
		loge("malloc failed");
		goto filesystem_error2;
	}
	rc = reliable_read(fd, buf, len);
	if (rc < 0) {
		goto filesystem_error3;
	}
	printf("Static configuration:\n");
	/* Returns number of bytes dumped */
	rc = sja1105_static_config_hexdump(buf);
	if (rc < 0) {
		loge("error while interpreting config");
		goto invalid_staging_area_error;
	}
	logi("static config: dumped %d bytes", rc);
filesystem_error3:
	free(buf);
filesystem_error2:
	close(fd);
filesystem_error1:
	sja1105_err_remap(rc, SJA1105_ERR_FILESYSTEM);
	return rc;
invalid_staging_area_error:
	sja1105_err_remap(rc, SJA1105_ERR_STAGING_AREA_INVALID);
	return rc;
}

int
staging_area_load(const char *staging_area_file,
                  struct sja1105_staging_area *staging_area)
{
	struct sja1105_static_config *static_config;
	struct stat stat;
	unsigned int staging_area_len;
	char *buf;
	int fd;
	int rc;

	static_config = &staging_area->static_config;

	fd = open(staging_area_file, O_RDONLY);
	if (fd < 0) {
		loge("Staging area %s does not exist!", staging_area_file);
		rc = fd;
		goto filesystem_error1;
	}
	rc = fstat(fd, &stat);
	if (rc < 0) {
		loge("could not read file size");
		goto filesystem_error2;
	}
	staging_area_len = stat.st_size;
	buf = (char*) malloc(staging_area_len * sizeof(char));
	if (!buf) {
		loge("malloc failed");
		goto filesystem_error2;
	}
	rc = reliable_read(fd, buf, staging_area_len);
	if (rc < 0) {
		loge("failed to read staging area from file %s",
		     staging_area_file);
		goto filesystem_error3;
	}
	/* Static config */
	rc = sja1105_static_config_unpack(buf, static_config);
	if (rc < 0) {
		loge("error while interpreting config");
		goto invalid_staging_area_error;
	}
	return 0;
filesystem_error3:
	free(buf);
filesystem_error2:
	close(fd);
filesystem_error1:
	sja1105_err_remap(rc, SJA1105_ERR_FILESYSTEM);
	return rc;
invalid_staging_area_error:
	sja1105_err_remap(rc, SJA1105_ERR_STAGING_AREA_INVALID);
	return rc;
}

int
staging_area_save(const char *staging_area_file,
                  struct sja1105_staging_area *staging_area)
{
	struct sja1105_static_config *static_config;
	int   rc = 0;
	char *buf;
	int   static_config_len;
	int   staging_area_len;
	int   fd;

	static_config     = &staging_area->static_config;
	static_config_len = sja1105_static_config_get_length(static_config);
	staging_area_len  = static_config_len;

	buf = (char*) malloc(staging_area_len * sizeof(char));
	if (!buf) {
		loge("malloc failed");
		goto out_1;
	}
	logv("saving static config... %d bytes", static_config_len);
	rc = sja1105_static_config_pack(buf, static_config);
	if (rc < 0) {
		loge("sja1105_static_config_pack failed");
		goto out_2;
	}

	logv("total staging area size: %d bytes", staging_area_len);
	fd = open(staging_area_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0) {
		loge("could not open %s for write", staging_area_file);
		rc = fd;
		goto out_2;
	}

	rc = reliable_write(fd, buf, staging_area_len);
	if (rc < 0) {
		goto out_2;
	}
	logv("done");

	close(fd);
out_2:
	free(buf);
out_1:
	return rc;
}

int
staging_area_flush(struct sja1105_spi_setup *spi_setup,
                   struct sja1105_staging_area *staging_area)
{
	int rc;

	rc = sja1105_static_config_flush(spi_setup, &staging_area->static_config);
	if (rc < 0) {
		loge("static_config_flush failed");
		goto out;
	}
	/* TODO: other configuration tables?
	 */
out:
	return rc;
}

