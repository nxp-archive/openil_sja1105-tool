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
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "internal.h"
/* From libsja1105 */
#include <lib/include/static-config.h>
#include <common.h>

const char *default_staging_area = "/lib/firmware/sja1105.bin";
const char *default_device = "/sys/bus/spi/drivers/sja1105/spi0.1";
const uint64_t default_device_id = SJA1105_NO_DEVICE_ID;
/* default_device_id of SJA1105_NO_DEVICE_ID signals
 * to sja1105_spi_configure that it should attempt to read
 * the real Device ID over SPI. Its absence from sja1105.conf
 * is not an error - its presence is merely an override of the
 * auto-detection behavior.
 */

struct general_config general_config;
int SJA1105_VERBOSE_CONDITION;
int SJA1105_DEBUG_CONDITION;

struct fields_set {
	int device_id;
	int device;
	int staging_area;
	int flush;
	int verbose;
	int debug;
	int entries_per_line;
	int screen_width;
};

static void
config_set_defaults(struct sja1105_spi_setup *spi_setup,
                    struct general_config *general_conf,
                    struct fields_set *fields_set)
{
#define SET_DEFAULT_VAL(struct_ptr, field, value, log, fmt) \
	if (!fields_set->field) { \
		log("%s field not defined in config file, setting default " \
		    "value " fmt, #field, value); \
		struct_ptr->field = value; \
	}
	SET_DEFAULT_VAL(spi_setup, device_id, default_device_id, logv, "0x%" PRIx64);
	SET_DEFAULT_VAL(spi_setup, device, default_device, logi, "%s");
	SET_DEFAULT_VAL(spi_setup, staging_area, default_staging_area, logi, "%s");
	SET_DEFAULT_VAL(spi_setup, flush, 0, logi, "%d");
	SET_DEFAULT_VAL(general_conf, verbose, 0, logi, "%d");
	SET_DEFAULT_VAL(general_conf, debug, 0, logi, "%d");
	SET_DEFAULT_VAL(general_conf, entries_per_line, 1, logi, "%d");
	SET_DEFAULT_VAL(general_conf, screen_width, 80, logi, "%d");
}

static inline int
parse_spi_setup(struct sja1105_spi_setup *spi_setup, char *key, char *value,
                struct fields_set *fields_set)
{
	int rc;
	uint64_t tmp;

	if (strcmp(key, "device") == 0) {
		spi_setup->device = strdup(value);
		fields_set->device = 1;
	} else if (strcmp(key, "device_id") == 0) {
		rc = reliable_uint64_from_string(&tmp, value, NULL);
		if (rc < 0) {
			goto error;
		}
		spi_setup->device_id = tmp;
		fields_set->device_id = 1;
	} else if (strcmp(key, "auto_flush") == 0) {
		if (strcmp(value, "false") == 0) {
			spi_setup->flush = 0;
		} else if (strcmp(value, "true") == 0) {
			spi_setup->flush = 1;
		} else {
			loge("Invalid value \"%s\" for autoflush. "
			     "Expected true or false.", value);
			return -1;
		}
		fields_set->flush = 1;
	} else if (strcmp(key, "staging_area") == 0) {
		spi_setup->staging_area = strdup(value);
		fields_set->staging_area = 1;
	} else {
		loge("Invalid key \"%s\"", key);
		return -1;
	}
	return 0;
error:
	loge("Invalid value \"%s\" for key \"%s\" in config file", value, key);
	return rc;
}

static inline int
parse_general_config(struct general_config *general_conf,
                     char *key, char *value, struct fields_set *fields_set)
{
	uint64_t tmp;
	int rc;

	if (strcmp(key, "verbose") == 0) {
		if (strcmp(value, "false") == 0) {
			general_conf->verbose = 0;
		} else if (strcmp(value, "true") == 0) {
			general_conf->verbose = 1;
		} else {
			loge("Invalid value \"%s\" for verbose. "
			     "Expected true or false.", value);
			return -1;
		}
		fields_set->verbose = 1;
	} else if (strcmp(key, "debug") == 0) {
		if (strcmp(value, "false") == 0) {
			general_conf->debug = 0;
		} else if (strcmp(value, "true") == 0) {
			general_conf->debug = 1;
		} else {
			loge("Invalid value \"%s\" for debug. "
			     "Expected true or false.", value);
			return -1;
		}
		fields_set->debug = 1;
	} else if (strcmp(key, "entries_per_line") == 0) {
		rc = reliable_uint64_from_string(&tmp, value, NULL);
		if (rc < 0) {
			goto error;
		}
		general_conf->entries_per_line = tmp;
		fields_set->entries_per_line = 1;
	} else if (strcmp(key, "screen_width") == 0) {
		rc = reliable_uint64_from_string(&tmp, value, NULL);
		if (rc < 0) {
			goto error;
		}
		general_conf->screen_width = tmp;
		fields_set->screen_width = 1;
	} else {
		loge("Invalid key \"%s\"", key);
		return -1;
	}
	return 0;
error:
	loge("Invalid value \"%s\" for key \"%s\" in config file", value, key);
	return rc;
}

static inline int parse_key_val(struct sja1105_spi_setup *spi_setup,
                                struct general_config *general_conf,
                                char *key, char *value, char *section_hdr,
                                struct fields_set *fields_set)
{
	if (strcmp(section_hdr, "[setup]") == 0) {
		parse_spi_setup(spi_setup, key, value, fields_set);
	} else if (strcmp(section_hdr, "[general]") == 0) {
		parse_general_config(general_conf, key, value, fields_set);
		SJA1105_VERBOSE_CONDITION = general_conf->verbose;
		SJA1105_DEBUG_CONDITION   = general_conf->debug;
	} else {
		loge("Invalid section header \"%s\"", section_hdr);
		return -1;
	}
	return 0;
}

int read_config_file(char *filename, struct sja1105_spi_setup *spi_setup,
                     struct general_config *general_conf)
{
	struct fields_set fields_set;
	char  line[MAX_LINE_SIZE];
	int   line_num = 0;
	int   rc;
	char *section_hdr = NULL;
	char *key, *value;
	char *p;
	FILE *fd;

	fd = fopen(filename, "r");
	if (!fd) {
		printf("%s not present, loading default config\n", filename);
		rc = -ENOENT;
		goto default_conf;
	}
	memset(spi_setup, 0, sizeof(*spi_setup));
	memset(general_conf, 0, sizeof(*general_conf));
	memset(&fields_set, 0, sizeof(fields_set));
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
			loge("Invalid format for line %d: \"%s\"", line_num, line);
			loge("Accepted line format: \"<key> = <value>\"");
			rc = -EINVAL;
			goto out;
		}
		key   = trimwhitespace(p);
		value = trimwhitespace(value);
		rc = parse_key_val(spi_setup, general_conf,
		                   key, value, section_hdr, &fields_set);
		if (rc < 0) {
			loge("Could not parse line %d: \"%s\"", line_num, line);
			rc = -EINVAL;
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
		loge("Invalid config file, using defaults.\n");
		/* Fall-through. We're using default values anyway for
		 * entries not specified in the config file. */
	}
	config_set_defaults(spi_setup, general_conf, &fields_set);
	return rc;
}

