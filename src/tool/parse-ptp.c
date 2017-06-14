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
#include <inttypes.h>
#include <stdio.h>
#include <lib/include/ptp.h>
#include <common.h>
#include "internal.h"

static void print_usage()
{
	printf("Usage:\n");
	printf(" * sja1105-tool ptp get {clk | ts-clk}\n");
	printf(" * sja1105-tool ptp set {clk | ts-clk} <value>\n");
	printf(" * sja1105-tool ptp start {schedule | pin-toggle}\n");
	printf(" * sja1105-tool ptp stop {schedule | pin-toggle}\n");
	printf(" * sja1105-tool ptp update\n");
	printf(" * sja1105-tool ptp reset\n");
}

int sja1105_ptp_update(struct sja1105_spi_setup *spi_setup)
{
	/**
	 * TODO:
	 * Open staging area.
	 * Read the fake "struct sja1105_ptp_config" that's inside.
	 * Run sja1105_ptp_config_cmd() with that struct.
	 */
	loge("unimplemented");
	return -1;
}

int ptp_parse_args(struct sja1105_spi_setup *spi_setup, int argc, char **argv)
{
	uint64_t tmp;
	int rc;

	if (argc < 1) {
		goto parse_error;
	}
	if (matches(argv[0], "get") == 0) {
		if (argc != 2) {
			loge("Expecting one argument");
			goto parse_error;
		}
		if (matches(argv[1], "ts-clk") == 0) {
			rc = sja1105_ptp_ts_clk_get(spi_setup, &tmp);
			printf("%f\n", (float)((tmp * 8) / 1000000000.0));
		}
		else if (matches(argv[1], "clk") == 0) {
			rc = sja1105_ptp_clk_get(spi_setup, &tmp);
			printf("%f\n", (float)((tmp * 8) / 1000000000.0));
		} else {
			loge("unknown token \"%s\"", argv[1]);
			goto parse_error;
		}
	} else if (matches(argv[0], "set") == 0) {
		if (argc != 3) {
			loge("Expecting 2 arguments");
			goto parse_error;
		}
		if (matches(argv[1], "clk") == 0) {
			/* TODO: parse argument as float */
			rc = reliable_uint64_from_string(&tmp, argv[2], NULL);
			if (rc < 0) {
				loge("invalid int at \"%s\"", argv[2]);
				goto parse_error;
			}
			rc = sja1105_ptp_clk_set(spi_setup, tmp);
		} else {
			loge("unknown token \"%s\"", argv[1]);
		}
	} else if (matches(argv[0], "add") == 0) {
		if (argc != 3) {
			loge("Expecting 2 arguments");
			goto parse_error;
		}
		if (matches(argv[1], "clk") == 0) {
			/* TODO: parse argument as float */
			rc = reliable_uint64_from_string(&tmp, argv[2], NULL);
			if (rc < 0) {
				loge("invalid int at \"%s\"", argv[2]);
				goto parse_error;
			}
			rc = sja1105_ptp_clk_add(spi_setup, tmp);
		} else {
			loge("unknown token \"%s\"", argv[1]);
		}
	} else if (matches(argv[0], "reset") == 0) {
		if (argc != 1) {
			loge("Expecting no arguments");
			goto parse_error;
		}
		rc = sja1105_ptp_reset(spi_setup);
	} else if (matches(argv[0], "start") == 0) {
		if (argc != 2) {
			loge("Expecting one argument");
			goto parse_error;
		}
		if (matches(argv[1], "schedule") == 0) {
			rc = sja1105_ptp_start_schedule(spi_setup);
		} else if (matches(argv[1], "pin-toggle") == 0) {
			rc = sja1105_ptp_start_pin_toggle(spi_setup);
		} else {
			loge("unknown token \"%s\"", argv[1]);
		}
	} else if (matches(argv[0], "stop") == 0) {
		if (argc != 2) {
			loge("Expecting one argument");
			goto parse_error;
		}
		if (matches(argv[1], "schedule") == 0) {
			rc = sja1105_ptp_stop_schedule(spi_setup);
		} else if (matches(argv[1], "pin-toggle") == 0) {
			rc = sja1105_ptp_stop_pin_toggle(spi_setup);
		} else {
			loge("unknown token \"%s\"", argv[1]);
		}
	} else if (matches(argv[0], "update") == 0) {
		if (argc != 1) {
			loge("Expecting no arguments");
			goto parse_error;
		}
		rc = sja1105_ptp_update(spi_setup);
	} else {
		loge("unknown token \"%s\"", argv[0]);
		goto parse_error;
	}
	return rc;
parse_error:
	print_usage();
	return -1;
}

