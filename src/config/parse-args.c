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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../clock/external.h"
#include "xml/read/external.h"
#include "xml/write/external.h"

static void print_usage(const char *prog)
{
	printf("Usage: %s configure <command> [<options>] \n", prog);
	printf("<command> can be:\n");
	printf("* load <filename.xml>\n");
	printf("* save <filename.xml>\n");
	printf("* default <config>, which can be:\n");
	printf("    * ls1021atsn - load a built-in config compatible with the NXP LS1021ATSN board\n");
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
			fprintf(stderr, "could not write to file\n");
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
			fprintf(stderr, "read failed\n");
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
		fprintf(stderr, "Config file %s does not exist!\n", config_file);
		rc = fd;
		goto out_1;
	}
	rc = fstat(fd, &stat);
	if (rc < 0) {
		fprintf(stderr, "could not read file size\n");
		goto out_2;
	}
	len = stat.st_size;
	buf = (char*) malloc(len * sizeof(char));
	if (!buf) {
		fprintf(stderr, "malloc failed\n");
		goto out_2;
	}
	rc = reliable_read(fd, buf, len);
	if (rc < 0) {
		goto out_3;
	}
	rc = sja1105_config_hexdump(buf);
	if (rc < 0) {
		fprintf(stderr, "error while interpreting config\n");
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
		fprintf(stderr, "Config file %s does not exist!\n", config_file);
		rc = fd;
		goto out_1;
	}
	rc = fstat(fd, &stat);
	if (rc < 0) {
		fprintf(stderr, "could not read file size\n");
		goto out_2;
	}
	len = stat.st_size;
	buf = (char*) malloc(len * sizeof(char));
	if (!buf) {
		fprintf(stderr, "malloc failed\n");
		goto out_2;
	}
	rc = reliable_read(fd, buf, len);
	if (rc < 0) {
		goto out_3;
	}
	rc = sja1105_config_get(buf, config);
	if (rc < 0) {
		fprintf(stderr, "error while interpreting config\n");
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
	int fd, rc;
	char *buf;
	int len;

	len = sja1105_config_get_length(config);

	buf = (char*) malloc(len * sizeof(char));
	if (!buf) {
		fprintf(stderr, "malloc failed\n");
		goto out_1;
	}
	sja1105_config_set(buf, config);

	fd = open(config_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0) {
		fprintf(stderr, "could not open %s for write\n", config_file);
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

int config_upload(struct spi_setup *spi_setup, struct sja1105_config *config)
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
	int    fd;
	int    rc;
	int    i;

	fd = configure_spi(spi_setup);
	if (fd < 0) {
		fprintf(stderr, "failed to open spi device\n");
		goto out_1;
	}

	config_buf_len = sja1105_config_get_length(config) + SIZE_SJA1105_DEVICE_ID;
	config_buf = (char*) malloc(config_buf_len * sizeof(char));
	if (!config_buf) {
		fprintf(stderr, "malloc failed\n");
		goto out_2;
	}
	/* Write Device ID to first 4 bytes of config_buf */
	generic_table_field_set(config_buf, &device_id, 31, 0, SIZE_SJA1105_DEVICE_ID);
	/* Write config tables to config_buf */
	sja1105_config_set(config_buf + SIZE_SJA1105_DEVICE_ID, config);
	/* Recalculate CRC of the last header */
	/* Don't include the CRC field itself */
	crc_len = config_buf_len - 4;
	/* Read the whole table header */
	final_header_ptr = config_buf + config_buf_len - SIZE_TABLE_HEADER;
	sja1105_table_header_get(final_header_ptr, &final_header);
	/* Modify */
	final_header.crc = ether_crc32_le(config_buf, crc_len);
	/* Rewrite */
	sja1105_table_header_set(final_header_ptr, &final_header);

	/* Fill chunks array with chunk_count pointers */
	spi_get_chunks(config_buf, config_buf_len, chunks, &chunk_count);

	for (i = 0; i < chunk_count; i++) {
		/* Combine chunks[i].msg and chunks[i].buf into tx_buf */
		spi_message_aggregate(tx_buf, &chunks[i].msg, chunks[i].buf, chunks[i].size);
		/* Send it out */
		rc = spi_transfer(fd, spi_setup, tx_buf, rx_buf,
		                  SIZE_SPI_MSG_HEADER + chunks[i].size);
		if (rc < 0) {
			fprintf(stderr, "spi_transfer failed\n");
			goto out_3;
		}
	}
	rc = 0;
out_3:
	free(config_buf);
out_2:
	close(fd);
out_1:
	return rc;
}

void config_parse_args(struct spi_setup *spi_setup, int argc, char **argv)
{
	const char *options[] = {
		"load",
		"save",
		"default",
		"modify",
		"new",
		"upload",
		"show",
		"hexdump",
	};
	int match;
	struct sja1105_config config;
	int rc;

	if (argc < 3) {
		goto parse_error;
	}
	match = get_match(argv[2], options, ARRAY_SIZE(options));
	if (match < 0) {
		goto parse_error;
	} else if (strcmp(options[match], "load") == 0) {
		if (argc != 4) {
			goto parse_error;
		}
		rc = sja1105_config_read_from_xml(argv[3], &config);
		if (rc < 0) {
			goto error;
		}
		rc = config_save(spi_setup->staging_area, &config);
		if (rc < 0) {
			goto error;
		}
	} else if (strcmp(options[match], "save") == 0) {
		if (argc != 4) {
			goto parse_error;
		}
		rc = config_load(spi_setup->staging_area, &config);
		if (rc < 0) {
			goto error;
		}
		rc = sja1105_config_write_to_xml(argv[3], &config);
		if (rc < 0) {
			goto error;
		}
	} else if (strcmp(options[match], "default") == 0) {
		if (argc != 4) {
			goto parse_error;
		}
		rc = config_default(&config, argv[3]);
		if (rc < 0) {
			goto error;
		}
		rc = config_save(spi_setup->staging_area, &config);
		if (rc < 0) {
			goto error;
		}
	} else if (strcmp(options[match], "upload") == 0) {
		struct sja1105_reset_ctrl reset = {.rst_ctrl = RGU_COLD};

		rc = config_load(spi_setup->staging_area, &config);
		if (rc < 0) {
			goto error;
		}
		rc = sja1105_reset(spi_setup, &reset);
		if (rc < 0) {
			goto error;
		}
		rc = config_upload(spi_setup, &config);
		if (rc < 0) {
			goto error;
		}
		rc = sja1105_clocking_setup(spi_setup, &config.xmii_params[0]);
		if (rc < 0) {
			goto error;
		}
	} else if (strcmp(options[match], "modify") == 0) {
		/*if (argc != 6) {*/
			/*goto parse_error;*/
		/*}*/
		rc = config_load(spi_setup->staging_area, &config);
		if (rc < 0) {
			goto error;
		}
		rc = config_table_entry_modify(&config, argv[3], argv[4], argv[5]);
		if (rc < 0) {
			goto error;
		}
		rc = config_save(spi_setup->staging_area, &config);
		if (rc < 0) {
			goto error;
		}
	} else if (strcmp(options[match], "new") == 0) {
		memset(&config, 0, sizeof(config));
		rc = config_save(spi_setup->staging_area, &config);
		if (rc < 0) {
			goto error;
		}
	} else if (strcmp(options[match], "show") == 0) {
		rc = config_load(spi_setup->staging_area, &config);
		if (rc < 0) {
			goto error;
		}
		rc = sja1105_config_show(&config, argv[3]);
		if (rc < 0) {
			goto error;
		}
	} else if (strcmp(options[match], "hexdump") == 0) {
		rc = config_hexdump(spi_setup->staging_area);
		if (rc < 0) {
			goto error;
		}
	} else {
		goto parse_error;
	}
	return;
parse_error:
	print_usage(argv[0]);
error:
	exit(-1);
}
