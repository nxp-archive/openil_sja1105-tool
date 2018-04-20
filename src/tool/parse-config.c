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
#include "xml/read/external.h"
#include "xml/write/external.h"
#include "internal.h"
#include <string.h>

static void print_usage()
{
	printf("Usage: sja1105-tool config <command> [<options>] \n");
	printf("<command> can be:\n");
	printf("* new [-d|--device-id <value>], default 0x9e00030e (SJA1105T)\n");
	printf("* load [-f|--flush] <filename.xml>\n");
	printf("* save <filename.xml>\n");
	printf("* default [-f|--flush] <config>, which can be:\n");
	printf("    * ls1021atsn - load a built-in config compatible with the NXP LS1021ATSN board\n");
	printf("* modify [-f|--flush] <table>[<entry_index>] <field> <value>\n");
	printf("* upload\n");
	printf("* show [<table>]. If no table is specified, shows entire config.\n");
	printf("* hexdump [<table>]. If no table is specified, dumps entire config.\n");
}

static void
get_flush_mode(struct sja1105_spi_setup *spi_setup, int *argc, char ***argv)
{
	if ((*argc) && ((strcmp(*argv[0], "-f") == 0 ||
	                (strcmp(*argv[0], "--flush") == 0)))) {
		spi_setup->flush = 1;
		(*argc)--; (*argv)++;
	}
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
	struct sja1105_staging_area staging_area;
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
		rc = sja1105_staging_area_from_xml(argv[0], &staging_area);
		if (rc < 0) {
			goto error;
		}
		rc = staging_area_save(spi_setup->staging_area, &staging_area);
		if (rc < 0) {
			goto error;
		}
		if (spi_setup->flush) {
			rc = sja1105_spi_configure(spi_setup);
			if (rc < 0) {
				loge("sja1105_spi_configure failed");
				goto error;
			}
			rc = staging_area_flush(spi_setup, &staging_area);
			if (rc < 0) {
				loge("staging_area_flush failed");
				goto error;
			}
		}
	} else if (strcmp(options[match], "save") == 0) {
		if (argc != 1) {
			goto parse_error;
		}
		rc = staging_area_load(spi_setup->staging_area, &staging_area);
		if (rc < 0) {
			goto error;
		}
		rc = sja1105_staging_area_to_xml(argv[0], &staging_area);
		if (rc < 0) {
			goto error;
		}
	} else if (strcmp(options[match], "default") == 0) {
		const char *default_config_options[] = {
			"ls1021atsn",
		};
		enum sja1105_default_staging_area default_configs[] = {
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
		rc = sja1105_default_staging_area(&staging_area,
		                                  default_configs[match]);
		if (rc < 0) {
			goto error;
		}
		rc = staging_area_save(spi_setup->staging_area, &staging_area);
		if (rc < 0) {
			goto error;
		}
		if (spi_setup->flush) {
			rc = sja1105_spi_configure(spi_setup);
			if (rc < 0) {
				loge("sja1105_spi_configure failed");
				goto error;
			}
			rc = staging_area_flush(spi_setup, &staging_area);
			if (rc < 0) {
				goto error;
			}
		}
	} else if (strcmp(options[match], "upload") == 0) {
		if (argc != 0) {
			goto parse_error;
		}
		rc = staging_area_load(spi_setup->staging_area, &staging_area);
		if (rc < 0) {
			goto error;
		}
		rc = sja1105_spi_configure(spi_setup);
		if (rc < 0) {
			loge("sja1105_spi_configure failed");
			goto error;
		}
		rc = staging_area_flush(spi_setup, &staging_area);
		if (rc < 0) {
			goto error;
		}
	} else if (strcmp(options[match], "modify") == 0) {
		get_flush_mode(spi_setup, &argc, &argv);
		rc = staging_area_load(spi_setup->staging_area, &staging_area);
		if (rc < 0) {
			goto error;
		}
		rc = staging_area_modify_parse(&staging_area, &argc, &argv);
		if (rc < 0) {
			goto error;
		}
		rc = staging_area_save(spi_setup->staging_area, &staging_area);
		if (rc < 0) {
			goto error;
		}
		if (spi_setup->flush) {
			rc = sja1105_spi_configure(spi_setup);
			if (rc < 0) {
				loge("sja1105_spi_configure failed");
				goto error;
			}
			rc = staging_area_flush(spi_setup, &staging_area);
			if (rc < 0) {
				goto error;
			}
		}
	} else if (strcmp(options[match], "new") == 0) {
		if (argc != 2 && argc != 0) {
			/* The 2 forms that are allowed are:
			 * sja1105-tool config new
			 * sja1105-tool config new -d <device_id>
			 */
			goto parse_error;
		}
		memset(&staging_area, 0, sizeof(staging_area));
		if (argc == 2) {
			if ((matches(argv[0], "-d") == 0) ||
			    (matches(argv[0], "--device-id") == 0)) {
				/* sja1105-config new -d <device_id> was provided */
				rc = reliable_uint64_from_string(&staging_area.static_config.device_id,
				                                 argv[1], NULL);
				if (rc < 0) {
					loge("Invalid device id provided: %s", argv[1]);
					goto parse_error;
				}
			}
		} else {
			logv("No device id provided, defaulting to SJA1105T");
			staging_area.static_config.device_id = SJA1105T_DEVICE_ID;
		}
		rc = staging_area_save(spi_setup->staging_area, &staging_area);
		if (rc < 0) {
			goto error;
		}
	} else if (strcmp(options[match], "show") == 0) {
		if (argc != 0 && argc != 1) {
			goto parse_error;
		}
		rc = staging_area_load(spi_setup->staging_area, &staging_area);
		if (rc < 0) {
			goto error;
		}
		rc = sja1105_staging_area_show(&staging_area, argv[0]);
		if (rc < 0) {
			goto error;
		}
	} else if (strcmp(options[match], "hexdump") == 0) {
		if (argc != 0) {
			goto parse_error;
		}
		rc = staging_area_hexdump(spi_setup->staging_area);
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
