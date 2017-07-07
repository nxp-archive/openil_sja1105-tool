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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "xml/read/external.h"
#include "xml/write/external.h"
#include "internal.h"
/* From libsja1105 */
#include <lib/include/config.h>
#include <lib/include/gtable.h>
#include <lib/include/spi.h>
#include <lib/include/ptp.h>
#include <lib/include/status.h>
#include <lib/include/reset.h>
#include <lib/include/clock.h>
#include <common.h>

static void print_usage()
{
	printf("Usage: sja1105-tool config <command> [<options>] \n");
	printf("<command> can be:\n");
	printf("* new\n");
	printf("* load [-f|--flush] <filename.xml>\n");
	printf("* save <filename.xml>\n");
	printf("* default [-f|--flush] <config>, which can be:\n");
	printf("    * ls1021atsn - load a built-in config compatible with the NXP LS1021ATSN board\n");
	printf("* modify [-f|--flush] <table>[<entry_index>] <field> <value>\n");
	printf("* upload\n");
	printf("* show [<table>]. If no table is specified, shows entire config.\n");
	printf("* hexdump [<table>]. If no table is specified, dumps entire config.\n");
}

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

int config_hexdump(const char *config_file)
{
	struct stat stat;
	unsigned int len;
	char *buf;
	int fd;
	int rc;

	fd = open(config_file, O_RDONLY);
	if (fd < 0) {
		loge("Staging area %s does not exist!", config_file);
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
	rc = sja1105_config_hexdump(buf);
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

int config_load(const char *config_file, struct sja1105_config *config)
{
	struct stat stat;
	unsigned int len;
	char *buf;
	int fd;
	int rc;

	fd = open(config_file, O_RDONLY);
	if (fd < 0) {
		loge("Staging area %s does not exist!", config_file);
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
	rc = sja1105_config_unpack(buf, config);
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

int config_save(const char *config_file, struct sja1105_config *config)
{
	int   rc = 0;
	char *buf;
	int   len;
	int   fd;

	len = sja1105_config_get_length(config);

	buf = (char*) malloc(len * sizeof(char));
	if (!buf) {
		loge("malloc failed");
		goto out_1;
	}
	sja1105_config_pack(buf, config);

	fd = open(config_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0) {
		loge("could not open %s for write", config_file);
		rc = fd;
		goto out_2;
	}

	rc = reliable_write(fd, buf, len);
	if (rc < 0) {
		goto out_2;
	}

	close(fd);
out_2:
	free(buf);
out_1:
	return rc;
}

int config_upload(struct sja1105_spi_setup *spi_setup, struct sja1105_config *config)
{
	struct   sja1105_table_header final_header;
	char    *final_header_ptr;
	uint64_t device_id = SJA1105_DEVICE_ID;
	/* XXX: Maybe 100 is not the best number of chunks here */
	struct sja1105_spi_chunk chunks[100];
	int    chunk_count;
	char   tx_buf[SIZE_SPI_MSG_HEADER + SIZE_SPI_MSG_MAXLEN];
	char   rx_buf[SIZE_SPI_MSG_MAXLEN + SIZE_SPI_MSG_HEADER];
	char  *config_buf;
	int    config_buf_len;
	int    crc_len;
	int    rc;
	int    i;

	config_buf_len = sja1105_config_get_length(config) + SIZE_SJA1105_DEVICE_ID;
	config_buf = (char*) malloc(config_buf_len * sizeof(char));
	if (!config_buf) {
		loge("malloc failed");
		goto out;
	}
	/* Write Device ID to first 4 bytes of config_buf */
	rc = gtable_pack(config_buf, &device_id, 31, 0, SIZE_SJA1105_DEVICE_ID);
	if (rc < 0) {
		loge("failed to write device id to buffer");
		goto out_free;
	}
	/* Write config tables to config_buf */
	sja1105_config_pack(config_buf + SIZE_SJA1105_DEVICE_ID, config);
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

	/* Fill chunks array with chunk_count pointers */
	spi_get_chunks(config_buf, config_buf_len, chunks, &chunk_count);

	for (i = 0; i < chunk_count; i++) {
		/* Combine chunks[i].msg and chunks[i].buf into tx_buf */
		spi_message_aggregate(tx_buf, &chunks[i].msg, chunks[i].buf, chunks[i].size);
		/* Send it out */
		rc = sja1105_spi_transfer(spi_setup, tx_buf, rx_buf,
		                          SIZE_SPI_MSG_HEADER + chunks[i].size);
		if (rc < 0) {
			loge("sja1105_spi_transfer failed");
			goto out_free;
		}
	}
out_free:
	free(config_buf);
out:
	return rc;
}

void get_flush_mode(struct sja1105_spi_setup *spi_setup, int *argc, char ***argv)
{
	if ((*argc) && ((strcmp(*argv[0], "-f") == 0 ||
	                (strcmp(*argv[0], "--flush") == 0)))) {
		spi_setup->flush = 1;
		(*argc)--; (*argv)++;
	}
}

int ptp_init(struct sja1105_spi_setup *spi_setup)
{
	struct sja1105_ptp_config ptp_config;
	int rc;

	printf("ptp_init\n");
	memset(&ptp_config, 0, sizeof(ptp_config));
	ptp_config.schedule_time = 1;
	/*ptp_config.schedule_autostart = 1;*/
	/*ptp_config.pin_toggle_autostart = 1;*/
	rc = sja1105_ptp_configure(spi_setup, &ptp_config);
	if (rc < 0) {
		loge("sja1105_ptp_configure failed");
		goto out;
	}
	rc = sja1105_ptp_start_schedule(spi_setup);
	if (rc < 0) {
		loge("sja1105_ptp_start_schedule failed");
		goto out;
	}
	rc = sja1105_ptp_reset(spi_setup);
	if (rc < 0) {
		loge("sja1105_ptp_reset failed");
		goto out;
	}
out:
	return rc;
}


int config_flush(struct sja1105_spi_setup *spi_setup, struct sja1105_config *config)
{
	struct sja1105_reset_ctrl     reset = {.rst_ctrl = RGU_COLD};
	struct sja1105_general_status status;
	uint64_t expected_device_id = SJA1105_DEVICE_ID;
	int rc;

	/* Check that we are talking with the right device over SPI */
	rc = sja1105_general_status_get(spi_setup, &status);
	if (rc < 0) {
		goto out;
	}
	if (spi_setup->dry_run == 0) {
		/* These checks simply cannot pass (and do not even
		 * make sense to have) if we are in dry run mode */
		if (status.device_id != expected_device_id) {
			loge("read device id %" PRIx64 ", expected %" PRIx64,
			     status.device_id, expected_device_id);
			goto out;
		}
	}
	rc = sja1105_config_check_valid(config);
	if (rc < 0) {
		loge("cannot upload config, because it is not valid");
		goto out;
	}
	rc = sja1105_reset(spi_setup, &reset);
	if (rc < 0) {
		loge("sja1105_reset failed");
		goto out;
	}
	rc = config_upload(spi_setup, config);
	if (rc < 0) {
		loge("config_upload failed");
		goto out;
	}
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
			loge("not responding to configured device id");
			goto out;
		}
		if (status.crcchkl == 1) {
			loge("local crc failed while uploading config");
			goto out;
		}
		if (status.crcchkg == 1) {
			loge("global crc failed while uploading config");
			goto out;
		}
		if (status.configs == 0) {
			loge("configuration is invalid");
		}
	}
	ptp_init(spi_setup);
out:
	return rc;
}

int config_parse_args(struct sja1105_spi_setup *spi_setup, int argc, char **argv)
{
	const char *options[] = {
		"help",
		"load",
		"save",
		"default",
		"modify",
		"new",
		"upload",
		"show",
		"hexdump",
	};
	struct sja1105_config config;
	int match;
	int rc;

	if (argc < 1) {
		goto parse_error;
	}
	match = get_match(argv[0], options, ARRAY_SIZE(options));
	argc--; argv++;
	if (match < 0) {
		goto parse_error;
	} else if (strcmp(options[match], "help") == 0) {
		print_usage();
	} else if (strcmp(options[match], "load") == 0) {
		get_flush_mode(spi_setup, &argc, &argv);
		if (argc != 1) {
			goto parse_error;
		}
		rc = sja1105_config_read_from_xml(argv[0], &config);
		if (rc < 0) {
			goto error;
		}
		rc = config_save(spi_setup->staging_area, &config);
		if (rc < 0) {
			goto error;
		}
		if (spi_setup->flush) {
			rc = sja1105_spi_configure(spi_setup);
			if (rc < 0) {
				loge("sja1105_spi_configure failed");
				goto error;
			}
			rc = config_flush(spi_setup, &config);
			if (rc < 0) {
				loge("config_flush failed");
				goto error;
			}
		}
	} else if (strcmp(options[match], "save") == 0) {
		if (argc != 1) {
			goto parse_error;
		}
		rc = config_load(spi_setup->staging_area, &config);
		if (rc < 0) {
			goto error;
		}
		rc = sja1105_config_write_to_xml(argv[0], &config);
		if (rc < 0) {
			goto error;
		}
	} else if (strcmp(options[match], "default") == 0) {
		const char *default_config_options[] = {
			"ls1021atsn",
		};
		enum sja1105_default_config default_configs[] = {
			LS1021ATSN,
		};
		get_flush_mode(spi_setup, &argc, &argv);
		if (argc != 1) {
			goto parse_error;
		}
		match = get_match(argv[0], default_config_options,
		                  ARRAY_SIZE(default_config_options));
		if (match < 0) {
			loge("Unrecognized default config %s", argv[0]);
			goto error;
		}
		rc = sja1105_config_default(&config, default_configs[match]);
		if (rc < 0) {
			goto error;
		}
		rc = config_save(spi_setup->staging_area, &config);
		if (rc < 0) {
			goto error;
		}
		if (spi_setup->flush) {
			rc = sja1105_spi_configure(spi_setup);
			if (rc < 0) {
				loge("sja1105_spi_configure failed");
				goto error;
			}
			rc = config_flush(spi_setup, &config);
			if (rc < 0) {
				goto error;
			}
		}
	} else if (strcmp(options[match], "upload") == 0) {
		if (argc != 0) {
			goto parse_error;
		}
		rc = config_load(spi_setup->staging_area, &config);
		if (rc < 0) {
			goto error;
		}
		rc = sja1105_spi_configure(spi_setup);
		if (rc < 0) {
			loge("sja1105_spi_configure failed");
			goto error;
		}
		rc = config_flush(spi_setup, &config);
		if (rc < 0) {
			goto error;
		}
	} else if (strcmp(options[match], "modify") == 0) {
		get_flush_mode(spi_setup, &argc, &argv);
		/*if (argc != 6) {*/
			/*goto parse_error;*/
		/*}*/
		rc = config_load(spi_setup->staging_area, &config);
		if (rc < 0) {
			goto error;
		}
		rc = config_table_entry_modify(&config, argv[0], argv[1], argv[2]);
		if (rc < 0) {
			goto error;
		}
		rc = config_save(spi_setup->staging_area, &config);
		if (rc < 0) {
			goto error;
		}
		if (spi_setup->flush) {
			rc = sja1105_spi_configure(spi_setup);
			if (rc < 0) {
				loge("sja1105_spi_configure failed");
				goto error;
			}
			rc = config_flush(spi_setup, &config);
			if (rc < 0) {
				goto error;
			}
		}
	} else if (strcmp(options[match], "new") == 0) {
		if (argc != 0) {
			goto parse_error;
		}
		memset(&config, 0, sizeof(config));
		rc = config_save(spi_setup->staging_area, &config);
		if (rc < 0) {
			goto error;
		}
	} else if (strcmp(options[match], "show") == 0) {
		if (argc != 0 && argc != 1) {
			goto parse_error;
		}
		rc = config_load(spi_setup->staging_area, &config);
		if (rc < 0) {
			goto error;
		}
		rc = sja1105_config_show(&config, argv[0]);
		if (rc < 0) {
			goto error;
		}
	} else if (strcmp(options[match], "hexdump") == 0) {
		if (argc != 0) {
			goto parse_error;
		}
		rc = config_hexdump(spi_setup->staging_area);
		if (rc < 0) {
			goto error;
		}
	} else {
		goto parse_error;
	}
	return 0;
parse_error:
	print_usage();
error:
	return -1;
}
