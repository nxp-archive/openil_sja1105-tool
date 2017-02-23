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

const char *default_staging_area = "/etc/sja1105/.staging";
const char *default_device = "/dev/spidev0.1";

struct general_config general_config;

static int parse_spi_mode(struct spi_setup *spi_setup, char *mode)
{
	if (strcmp(mode, "SPI_CPHA") == 0) {
		spi_setup->mode |= SPI_CPHA;
	} else if (strcmp(mode, "SPI_CPOL") == 0) {
		spi_setup->mode |= SPI_CPOL;
	} else {
		fprintf(stderr, "Invalid value \"%s\" for mode.", mode);
		fprintf(stderr, "Expected SPI_CPHA or SPI_CPOL.\n");
		return -1;
	}
	return 0;
}

static inline int parse_spi_setup(struct spi_setup *spi_setup, char *key, char *value)
{
	char *mode;
	int rc;
	uint64_t tmp;

	if (strcmp(key, "device") == 0) {
		/* FIXME: Memory leak here */
		spi_setup->device = strdup(value);
	} else if (strcmp(key, "bits") == 0) {
		rc = reliable_uint64_from_string(&tmp, value, NULL);
		if (rc < 0) {
			goto error;
		}
		spi_setup->bits = tmp;
	} else if (strcmp(key, "speed") == 0) {
		rc = reliable_uint64_from_string(&tmp, value, NULL);
		if (rc < 0) {
			goto error;
		}
		spi_setup->speed = tmp;
	} else if (strcmp(key, "delay") == 0) {
		rc = reliable_uint64_from_string(&tmp, value, NULL);
		if (rc < 0) {
			goto error;
		}
		spi_setup->delay = tmp;
	} else if (strcmp(key, "cs_change") == 0) {
		rc = reliable_uint64_from_string(&tmp, value, NULL);
		if (rc < 0) {
			goto error;
		}
		spi_setup->cs_change = tmp;
	} else if (strcmp(key, "mode") == 0) {
		spi_setup->mode = 0;
		mode = value;
		while (value != NULL) {
			strsep(&mode, "|");
			if (mode == NULL) {
				/* Value does not contain "|" delimiter */
				parse_spi_mode(spi_setup, value);
			} else {
				parse_spi_mode(spi_setup, trimwhitespace(mode));
			}
			value = mode;
		}
	} else if (strcmp(key, "dry_run") == 0) {
		if (strcmp(value, "false") == 0) {
			spi_setup->dry_run = 0;
		} else if (strcmp(value, "true") == 0) {
			spi_setup->dry_run = 1;
		} else {
			fprintf(stderr, "Invalid value \"%s\" for dry_run. ", value);
			fprintf(stderr, "Expected true or false.\n");
			return -1;
		}
	} else if (strcmp(key, "staging-area") == 0) {
		/* FIXME: Memory leak here */
		spi_setup->staging_area = strdup(value);
	} else {
		fprintf(stderr, "Invalid key \"%s\"\n", key);
		return -1;
	}
	return 0;
error:
	fprintf(stderr, "Invalid value \"%s\" for key \"%s\" in config file\n",
	        value, key);
	return rc;
}

static inline int parse_general_config(char *key, char *value)
{
	uint64_t tmp;
	int rc;

	if (strcmp(key, "verbose") == 0) {
		if (strcmp(value, "false") == 0) {
			general_config.verbose = 0;
		} else if (strcmp(value, "true") == 0) {
			general_config.verbose = 1;
		} else {
			fprintf(stderr, "Invalid value \"%s\" for verbose. ", value);
			fprintf(stderr, "Expected true or false.\n");
			return -1;
		}
	} else if (strcmp(key, "debug") == 0) {
		if (strcmp(value, "false") == 0) {
			general_config.debug = 0;
		} else if (strcmp(value, "true") == 0) {
			general_config.debug = 1;
		} else {
			fprintf(stderr, "Invalid value \"%s\" for debug. ", value);
			fprintf(stderr, "Expected true or false.\n");
			return -1;
		}
	} else if (strcmp(key, "entries-per-line") == 0) {
		rc = reliable_uint64_from_string(&tmp, value, NULL);
		if (rc < 0) {
			goto error;
		}
		general_config.entries_per_line = tmp;
	} else if (strcmp(key, "screen-width") == 0) {
		rc = reliable_uint64_from_string(&tmp, value, NULL);
		if (rc < 0) {
			goto error;
		}
		general_config.screen_width = tmp;
	} else {
		fprintf(stderr, "Invalid key \"%s\"\n", key);
		return -1;
	}
	return 0;
error:
	fprintf(stderr, "Invalid value \"%s\" for key \"%s\" in config file\n",
	        value, key);
	return rc;
}

static inline int parse_key_val(struct spi_setup *spi_setup, char *key, char *value, char *section_hdr)
{
	if (strcmp(section_hdr, "[spi-setup]") == 0) {
		parse_spi_setup(spi_setup, key, value);
	} else if (strcmp(section_hdr, "[general]") == 0) {
		parse_general_config(key, value);
	} else {
		fprintf(stderr, "Invalid section header \"%s\"\n", section_hdr);
		return -1;
	}
	return 0;
}

int get_spi_setup(struct spi_setup *spi_setup)
{
	const char *conf_file = SJA1105_CONF_FILE;
	char  line[MAX_LINE_SIZE];
	int   line_num = 0;
	int   rc;
	char *section_hdr = NULL;
	char *key, *value;
	char *p;
	FILE *fd;

	fd = fopen(conf_file, "r");
	if (!fd) {
		printf("%s not present, loading default config\n", conf_file);
		rc = -1;
		goto default_conf;
	}
	while (fgets(line, MAX_LINE_SIZE, fd)) {
		p = trimwhitespace(line);
		if (strlen(p) == 0 || p == NULL) {
			continue;
		}
		if (p[0] == '[') {
			/* This is a section header */
			if (section_hdr != NULL) {
				free(section_hdr);
			}
			section_hdr = strdup(p);
			continue;
		}
		if (p[0] == '#') {
			/* This is a comment */
			continue;
		}
		value = p;
		strsep(&value, "=");
		if (value == NULL) {
			/* There was no equal on this line */
			fprintf(stderr, "Invalid format for line %d: \"%s\"\n",
			        line_num, line);
			fprintf(stderr, "Accepted line format: \"<key> = <value>\"\n");
			rc = -1;
			goto out;
		}
		key   = trimwhitespace(p);
		value = trimwhitespace(value);
		rc = parse_key_val(spi_setup, key, value, section_hdr);
		if (rc < 0) {
			fprintf(stderr, "Could not parse line %d: \"%s\"\n",
			        line_num, line);
			rc = -1;
			goto out;
		}
		line_num++;
	}
out:
	if (section_hdr != NULL) {
		free(section_hdr);
	}
	fclose(fd);
default_conf:
	if (rc == -1) {
		fprintf(stderr, "Invalid config file, using defaults.\n");
		spi_setup->device       = default_device;
		spi_setup->staging_area = default_staging_area;
		spi_setup->mode         = 0;
		spi_setup->bits         = 8;
		spi_setup->speed        = 1000000;
		spi_setup->delay        = 0;
		spi_setup->cs_change    = 0;
		spi_setup->mode         = SPI_CPHA;
		spi_setup->dry_run      = 0;
		general_config.verbose  = 0;
		general_config.debug    = 0;
		general_config.entries_per_line = 1;
		general_config.screen_width     = 80;
	}
	return rc;
}

