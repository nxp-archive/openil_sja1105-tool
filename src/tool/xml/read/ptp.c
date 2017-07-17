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
#include "internal.h"

int
parse_ptp_config(xmlNode *node, struct sja1105_ptp_config *config)
{
	int rc = 0;

	if (node->type != XML_ELEMENT_NODE) {
		loge("PTP Configuration node must be of element type!");
		rc = -1;
		goto out;
	}
	memset(config, 0, sizeof(*config));
	rc |= xml_read_field(&config->pin_duration, "pin_duration", node);
	rc |= xml_read_field(&config->pin_start, "pin_start", node);
	rc |= xml_read_field(&config->schedule_time, "schedule_time", node);
	rc |= xml_read_field(&config->schedule_correction_period,
	                     "schedule_correction_period", node);
	rc |= xml_read_field(&config->ts_based_on_ptpclk,
	                     "ts_based_on_ptpclk", node);
	rc |= xml_read_field(&config->schedule_autostart,
	                     "schedule_autostart", node);
	rc |= xml_read_field(&config->pin_toggle_autostart,
	                     "pin_toggle_autostart", node);
	if (rc) {
		loge("PTP Configuration is incomplete!");
		goto out;
	}
	logv("read PTP Configuration");
out:
	return rc;
}

