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
#include <stdio.h>
#include <common.h>
#include "internal.h"

/* TOOL_TODO
static void print_usage()
{
	printf("Usage:\n");
	printf(" * sja1105-tool reset core\n");
	printf(" * sja1105-tool reset config\n");
	printf(" * sja1105-tool reset clocking\n");
	printf(" * sja1105-tool reset otp\n");
	printf(" * sja1105-tool reset warm\n");
	printf(" * sja1105-tool reset cold\n");
	printf(" * sja1105-tool reset por\n");
}
*/

int rgu_parse_args(struct sja1105_spi_setup *spi_setup, int argc, char **argv)
{
/*
	const char *reset_options[] = {
		"core",
		"config",
		"clocking",
		"otp",
		"warm",
		"cold",
		"por",
	};
	int (*sja1105_reset_fn[])(struct sja1105_spi_setup*) = {
		sja1105_switch_core_reset,
		sja1105_config_reset,
		sja1105_clocking_reset,
		sja1105_otp_reset,
		sja1105_warm_reset,
		sja1105_cold_reset,
		sja1105_por_reset,
	};
	int match;
	int rc;

	if (argc < 1) {
		goto out_parse_error_usage;
	}
	match = get_match(argv[0], reset_options, ARRAY_SIZE(reset_options));
	if (match < 0) {
		goto out_parse_error;
	}
	rc = sja1105_spi_configure(spi_setup);
	if (rc < 0) {
		loge("failed to open spi device");
		goto out_spi_configure_failed;
	}
	rc = sja1105_reset_fn[match](spi_setup);
	if (rc < 0) {
		goto out_reset_failed;
	}
	goto out_ok;

out_parse_error_usage:
	print_usage();
out_parse_error:
	rc = -EINVAL;
out_spi_configure_failed:
out_reset_failed:
out_ok:
	return rc;
*/
	(void)spi_setup;
	(void)argc;
	(void)argv;
	return -1;
}

