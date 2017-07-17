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
#include <lib/include/staging-area.h>
#include <lib/include/ptp.h>
#include <common.h>
#include "internal.h"

static void print_usage()
{
	printf("Usage:\n");
	printf(" * sja1105-tool ptp get {clk | ts-clk}\n");
	printf(" * sja1105-tool ptp set {clk | clk-rate} <value>\n");
	printf(" * sja1105-tool ptp start {schedule | pin-toggle}\n");
	printf(" * sja1105-tool ptp stop {schedule | pin-toggle}\n");
	printf(" * sja1105-tool ptp update\n");
	printf(" * sja1105-tool ptp reset\n");
}

int sja1105_ptp_update(struct sja1105_spi_setup *spi_setup)
{
	struct sja1105_staging_area staging_area;
	struct sja1105_ptp_config  *ptp_config;
	int rc;

	rc = staging_area_load(spi_setup->staging_area, &staging_area);
	if (rc < 0) {
		loge("staging_area_load failed");
		goto out;
	}
	ptp_config = &staging_area.ptp_config;
	rc = sja1105_ptp_configure(spi_setup, ptp_config);
out:
	return rc;
}

int ptp_parse_args(struct sja1105_spi_setup *spi_setup, int argc, char **argv)
{
	double tmp_double;
	uint64_t tmp;
	uint32_t ptpclkrate;
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
			rc = sja1105_spi_configure(spi_setup);
			if (rc < 0) {
				loge("sja1105_spi_configure failed");
				goto error;
			}
			rc = sja1105_ptp_ts_clk_get(spi_setup, &tmp);
			printf("%lf\n", (double)((tmp * 8) / 1000000000.0));
		}
		else if (matches(argv[1], "clk") == 0) {
			rc = sja1105_spi_configure(spi_setup);
			if (rc < 0) {
				loge("sja1105_spi_configure failed");
				goto error;
			}
			rc = sja1105_ptp_clk_get(spi_setup, &tmp);
			printf("%lf\n", (double)((tmp * 8) / 1000000000.0));
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
			rc = reliable_double_from_string(&tmp_double, argv[2],
			                                 NULL);
			if (rc < 0) {
				loge("invalid double at \"%s\"", argv[2]);
				goto parse_error;
			}
			rc = sja1105_spi_configure(spi_setup);
			if (rc < 0) {
				loge("sja1105_spi_configure failed");
				goto error;
			}
			tmp = (uint64_t)((tmp_double * 1000000000.0) / 8);
			rc = sja1105_ptp_clk_set(spi_setup, tmp);
		} else if (matches(argv[1], "clk-rate") == 0) {
			rc = reliable_double_from_string(&tmp_double, argv[2],
			                                 NULL);
			if (rc < 0) {
				loge("invalid double at \"%s\"", argv[2]);
				goto parse_error;
			}
			rc = sja1105_ptpclkrate_from_ratio(tmp_double,
			                                   &ptpclkrate);
			if (rc < 0) {
				loge("ptpclkrate_from_ratio failed");
				goto error;
			}
			rc = sja1105_spi_configure(spi_setup);
			if (rc < 0) {
				loge("sja1105_spi_configure failed");
				goto error;
			}
			logv("setting ptpclkrate to 0x%" PRIx32, ptpclkrate);
			rc = sja1105_ptp_clk_rate_set(spi_setup, ptpclkrate);
		} else {
			loge("unknown token \"%s\"", argv[1]);
		}
	} else if (matches(argv[0], "add") == 0) {
		if (argc != 3) {
			loge("Expecting 2 arguments");
			goto parse_error;
		}
		if (matches(argv[1], "clk") == 0) {
			rc = reliable_double_from_string(&tmp_double, argv[2],
			                                 NULL);
			if (rc < 0) {
				loge("invalid double at \"%s\"", argv[2]);
				goto parse_error;
			}
			rc = sja1105_spi_configure(spi_setup);
			if (rc < 0) {
				loge("sja1105_spi_configure failed");
				goto error;
			}
			tmp = (uint64_t)((tmp_double * 1000000000.0) / 8);
			rc = sja1105_ptp_clk_add(spi_setup, tmp);
		} else {
			loge("unknown token \"%s\"", argv[1]);
		}
	} else if (matches(argv[0], "reset") == 0) {
		if (argc != 1) {
			loge("Expecting no arguments");
			goto parse_error;
		}
		rc = sja1105_spi_configure(spi_setup);
		if (rc < 0) {
			loge("sja1105_spi_configure failed");
			goto error;
		}
		rc = sja1105_ptp_reset(spi_setup);
	} else if (matches(argv[0], "start") == 0) {
		if (argc != 2) {
			loge("Expecting one argument");
			goto parse_error;
		}
		if (matches(argv[1], "schedule") == 0) {
			rc = sja1105_spi_configure(spi_setup);
			if (rc < 0) {
				loge("sja1105_spi_configure failed");
				goto error;
			}
			rc = sja1105_ptp_start_schedule(spi_setup);
		} else if (matches(argv[1], "pin-toggle") == 0) {
			rc = sja1105_spi_configure(spi_setup);
			if (rc < 0) {
				loge("sja1105_spi_configure failed");
				goto error;
			}
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
			rc = sja1105_spi_configure(spi_setup);
			if (rc < 0) {
				loge("sja1105_spi_configure failed");
				goto error;
			}
			rc = sja1105_ptp_stop_schedule(spi_setup);
		} else if (matches(argv[1], "pin-toggle") == 0) {
			rc = sja1105_spi_configure(spi_setup);
			if (rc < 0) {
				loge("sja1105_spi_configure failed");
				goto error;
			}
			rc = sja1105_ptp_stop_pin_toggle(spi_setup);
		} else {
			loge("unknown token \"%s\"", argv[1]);
		}
	} else if (matches(argv[0], "update") == 0) {
		if (argc != 1) {
			loge("Expecting no arguments");
			goto parse_error;
		}
		rc = sja1105_spi_configure(spi_setup);
		if (rc < 0) {
			loge("sja1105_spi_configure failed");
			goto error;
		}
		rc = sja1105_ptp_update(spi_setup);
	} else {
		loge("unknown token \"%s\"", argv[0]);
		goto parse_error;
	}
	return rc;
parse_error:
	print_usage();
error:
	return -1;
}

