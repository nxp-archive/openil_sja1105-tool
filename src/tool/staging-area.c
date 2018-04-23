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
		goto out_1;
	}
	rc = fstat(fd, &stat);
	if (rc < 0) {
		loge("could not read file size");
		goto out_2;
	}
	len = stat.st_size;
	buf = (char*) malloc(len * sizeof(char));
	if (!buf) {
		loge("malloc failed");
		goto out_2;
	}
	rc = reliable_read(fd, buf, len);
	if (rc < 0) {
		goto out_3;
	}
	printf("Static configuration:\n");
	/* Returns number of bytes dumped */
	rc = sja1105_static_config_hexdump(buf);
	if (rc < 0) {
		loge("error while interpreting config");
		goto out_3;
	}
	logi("static config: dumped %d bytes", rc);
out_3:
	free(buf);
out_2:
	close(fd);
out_1:
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
		goto out_1;
	}
	rc = fstat(fd, &stat);
	if (rc < 0) {
		loge("could not read file size");
		goto out_2;
	}
	staging_area_len = stat.st_size;
	buf = (char*) malloc(staging_area_len * sizeof(char));
	if (!buf) {
		loge("malloc failed");
		goto out_2;
	}
	rc = reliable_read(fd, buf, staging_area_len);
	if (rc < 0) {
		goto out_3;
	}
	/* Static config */
	rc = sja1105_static_config_unpack(buf, static_config);
	if (rc < 0) {
		loge("error while interpreting config");
		goto out_3;
	}
	rc = 0;
out_3:
	free(buf);
out_2:
	close(fd);
out_1:
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

static int
static_config_upload(struct sja1105_spi_setup *spi_setup,
                     struct sja1105_static_config *config)
{
	struct   sja1105_table_header final_header;
	char    *final_header_ptr;
	char  *config_buf;
	int    config_buf_len;
	int    crc_len;
	int    rc;

	config_buf_len = sja1105_static_config_get_length(config);
	config_buf = (char*) malloc(config_buf_len * sizeof(char));
	if (!config_buf) {
		loge("malloc failed");
		rc = -errno;
		goto out;
	}
	/* Write Device ID and config tables to config_buf */
	rc = sja1105_static_config_pack(config_buf, config);
	if (rc < 0) {
		loge("sja1105_static_config_pack failed");
		goto out_free;
	}
	/* Recalculate CRC of the last header */
	/* Don't include the CRC field itself */
	crc_len = config_buf_len - 4;
	/* Read the whole table header */
	final_header_ptr = config_buf + config_buf_len - SIZE_TABLE_HEADER;
	sja1105_table_header_unpack(final_header_ptr, &final_header);
	/* Modify */
	final_header.crc = ether_crc32_le(config_buf, crc_len);
	/* Rewrite */
	sja1105_table_header_pack(final_header_ptr, &final_header);

	rc = sja1105_spi_send_long_packed_buf(spi_setup,
	                                      SPI_WRITE,
	                                      CONFIG_ADDR,
	                                      config_buf,
	                                      config_buf_len);
out_free:
	free(config_buf);
out:
	return rc;
}

int static_config_flush(struct sja1105_spi_setup *spi_setup,
                        struct sja1105_static_config *config)
{
	struct sja1105_general_status status;
	struct sja1105_egress_port_mask port_mask;
	int i, rc;

	rc = sja1105_static_config_check_valid(config);
	if (rc < 0) {
		loge("cannot upload config, because it is not valid");
		goto out;
	}
	/* Workaround for PHY jabbering during switch reset */
	memset(&port_mask, 0, sizeof(port_mask));
	for (i = 0; i < SJA1105T_NUM_PORTS; i++) {
		port_mask.inhibit_tx[i] = 1;
	}
	rc = sja1105_inhibit_tx(spi_setup, &port_mask);
	if (rc < 0) {
		loge("sja1105_set_egress_port_mask failed");
		goto out;
	}
	/* Wait for an eventual egress packet to finish transmission
	 * (reach IFG). It is guaranteed that a second one will not
	 * follow, and that switch cold reset is thus safe
	 */
	usleep(1000);
	/* Put the SJA1105 in programming mode */
	rc = sja1105_cold_reset(spi_setup);
	if (rc < 0) {
		loge("sja1105_reset failed");
		goto out;
	}
	rc = static_config_upload(spi_setup, config);
	if (rc < 0) {
		loge("static_config_upload failed");
		goto out;
	}
	/* Configure the CGU (PHY link modes and speeds) */
	rc = sja1105_clocking_setup(spi_setup, &config->xmii_params[0],
	                           &config->mac_config[0]);
	if (rc < 0) {
		loge("sja1105_clocking_setup failed");
		goto out;
	}
	/* Check that SJA1105 responded well to the config upload */
	if (spi_setup->dry_run == 0) {
		/* These checks simply cannot pass (and do not even
		 * make sense to have) if we are in dry run mode */
		rc = sja1105_general_status_get(spi_setup, &status);
		if (rc < 0) {
			goto out;
		}
		if (status.ids == 1) {
			loge("Mismatch between hardware and staging area "
			     "device id. Wrote 0x%" PRIx64 ", wants 0x%" PRIx64,
			     config->device_id, spi_setup->device_id);
			goto out;
		}
		if (status.crcchkl == 1) {
			loge("local crc failed while uploading config");
			rc = -EINVAL;
			goto out;
		}
		if (status.crcchkg == 1) {
			loge("global crc failed while uploading config");
			rc = -EINVAL;
			goto out;
		}
		if (status.configs == 0) {
			loge("configuration is invalid");
			rc = -EINVAL;
		}
	}
out:
	return rc;
}

int
staging_area_flush(struct sja1105_spi_setup *spi_setup,
                   struct sja1105_staging_area *staging_area)
{
	int rc;

	rc = static_config_flush(spi_setup, &staging_area->static_config);
	if (rc < 0) {
		loge("static_config_flush failed");
		goto out;
	}
out:
	return rc;
}

