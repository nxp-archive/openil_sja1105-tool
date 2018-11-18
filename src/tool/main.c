/******************************************************************************
 * Copyright (c) 2016, NXP Semiconductors
 * Copyright (c) 2015, VVDN Technologies
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <lib/include/static-config.h>
#include <lib/include/gtable.h>
#include <common.h>
#include "internal.h"

const char *default_sja1105_conf_file = "/etc/sja1105/sja1105.conf";

void print_usage()
{
	printf("Usage: sja1105-tool [-c|--config-file] [command] [options] \n"
	       "command can be one of:\n"
	       "   * config\n"
	       "   * status\n"
	       "   * reset\n"
	       "   * reg\n"
	       "   * help | -h | --help\n"
	       "   * version | -V | --version\n");
	printf("\n");
	printf("Type \"sja1105-tool [command] help\" to see more details.\n");
	printf("\n");
}

void print_version()
{
	char buf[256];
	sja1105_lib_get_version(buf);
	printf("libsja1105 version: %s\n", buf);
	sja1105_lib_get_build_date(buf);
	printf("libsja1105 build date: %s\n", buf);
	printf("sja1105-tool version: %s\n", VERSION);
	printf("sja1105-tool build date: %s\n", __DATE__ " " __TIME__);
}

static int parse_special_args(int *argc, char ***argv,
                              char **sja1105_conf_file)
{
	int more_special_args;
	char *arg;
	int rc = 0;

	do {
		more_special_args = 0;
		arg = *argv[0];
		if (matches(arg, "-V") == 0 ||
		    matches(arg, "version") == 0 ||
		    matches(arg, "--version") == 0) {
			/* Version parsed */
			print_version();
			more_special_args = 1;
			(*argc)--; (*argv)++;
			/* Do not continue to run */
			sja1105_err_remap(rc, SJA1105_ERR_USAGE);
		} else if (matches(arg, "-h") == 0 ||
		           matches(arg, "help") == 0 ||
		           matches(arg, "--help") == 0) {
			/* Help parsed */
			print_usage();
			more_special_args = 1;
			(*argc)--; (*argv)++;
			/* Do not continue to run */
			sja1105_err_remap(rc, SJA1105_ERR_USAGE);
		} else if (matches(arg, "-c") == 0 ||
		           matches(arg, "--config-file") == 0) {
			/* Parse non-default config file */
			*sja1105_conf_file = (*argv)[1];
			more_special_args = 1;
			/* Consume 2 arguments */
			(*argc)--; (*argv)++;
			(*argc)--; (*argv)++;
			/* Continue to run */
			sja1105_err_remap(rc, SJA1105_ERR_OK);
		}
	} while (more_special_args && (*argc));

	return rc;
}

static int parse_args(struct sja1105_spi_setup *spi_setup, int argc, char **argv)
{
	const char *options[] = {
		"configure",
		"status",
		"reg",
	};
	int (*next_parse_args[])(struct sja1105_spi_setup*, int, char**) = {
		config_parse_args,
		status_parse_args,
		reg_parse_args,
	};
	int  rc;

	if (argc < 1) {
		rc = -EINVAL;
		goto error;
	}
	rc = get_match(argv[0], options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto error;
	}
	argc--; argv++;
	return next_parse_args[rc](spi_setup, argc, argv);
error:
	print_usage();
	return rc;
}

void cleanup(struct sja1105_spi_setup *spi_setup)
{
	extern const char *default_device;
	extern const char *default_staging_area;

	if (spi_setup->device && spi_setup->device != default_device) {
		free((char*) spi_setup->device);
	}
	if (spi_setup->staging_area &&
	    spi_setup->staging_area != default_staging_area) {
		free((char*) spi_setup->staging_area);
	}
}

static int reinterpreted_return_code(int rc)
{
	return -rc;
}

int main(int argc, char *argv[])
{
	char *sja1105_conf_file = (char*) default_sja1105_conf_file;
	struct sja1105_spi_setup spi_setup;
	int rc = SJA1105_ERR_OK;

	/* discard program name */
	argc--; argv++;
	if (argc == 0) {
		print_usage();
		goto out;
	}
	rc = parse_special_args(&argc, &argv, &sja1105_conf_file);
	if (rc < 0) {
		goto out;
	}
	read_config_file(sja1105_conf_file, &spi_setup, &general_config);
	/* Adjust gtable for SJA1105 SPI memory layout */
	gtable_configure(QUIRK_LSW32_IS_FIRST);
	rc = parse_args(&spi_setup, argc, argv);
	if (rc == SJA1105_ERR_OK) {
		logv("ok");
	}
	cleanup(&spi_setup);
out:
	return reinterpreted_return_code(rc);
}
