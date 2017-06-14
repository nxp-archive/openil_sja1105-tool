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
#include <lib/include/spi.h>
#include <common.h>
#include "internal.h"

void print_usage()
{
	printf("Usage: sja1105-tool [command] [options] \n"
	       "command can be one of:\n"
	       "   * config\n"
	       "   * status\n"
	       "   * reset\n"
	       "   * version | -V | --version\n");
	printf("\n");
	printf("Type \"sja1105-tool [command] help\" to see more details.\n");
	printf("\n");
}

int print_version(__attribute__((unused)) struct sja1105_spi_setup *spi_setup,
                  __attribute__((unused)) int argc,
                  __attribute__((unused)) char** argv)
{
	printf("%s\n", VERSION);
	return 0;
}

static int parse_args(struct sja1105_spi_setup *spi_setup, int argc, char **argv)
{
	const char *options[] = {
		"configure",
		"status",
		"reset",
		"ptp",
		"-V",
		"version",
		"--version",
	};
	int (*next_parse_args[])(struct sja1105_spi_setup*, int, char**) = {
		config_parse_args,
		status_parse_args,
		rgu_parse_args,
		ptp_parse_args,
		print_version,
		print_version,
		print_version,
	};
	int  rc;

	if (argc < 1) {
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
	return -1;
}

void cleanup(struct sja1105_spi_setup *spi_setup)
{
	if (spi_setup->device) {
		free((char*) spi_setup->device);
	}
	if (spi_setup->staging_area) {
		free((char*) spi_setup->staging_area);
	}
	if (spi_setup->fd) {
		close(spi_setup->fd);
	}
}

int main(int argc, char *argv[])
{
	struct sja1105_spi_setup spi_setup;
	int rc;

	/* discard program name */
	argc--; argv++;
	read_config_file(SJA1105_CONF_FILE, &spi_setup, &general_config);
	rc = parse_args(&spi_setup, argc, argv);
	if (rc == 0) {
		logv("ok");
	}
	cleanup(&spi_setup);
	return rc;
}
