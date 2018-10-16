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
#include <common.h>
#include "helpers.h"

int __attribute__((weak)) SJA1105_DEBUG_CONDITION   = 0;
int __attribute__((weak)) SJA1105_VERBOSE_CONDITION = 0;

void mac_addr_sprintf(char *buf, uint64_t mac_hexval)
{
	snprintf(buf, MAC_ADDR_SIZE, "%.02x:%.02x:%.02x:%.02x:%.02x:%.02x",
	        (unsigned) (mac_hexval >> 40) & 0xff,
	        (unsigned) (mac_hexval >> 32) & 0xff,
	        (unsigned) (mac_hexval >> 24) & 0xff,
	        (unsigned) (mac_hexval >> 16) & 0xff,
	        (unsigned) (mac_hexval >>  8) & 0xff,
	        (unsigned) (mac_hexval >>  0) & 0xff);
}

const char *sja1105_err_code_to_string(int rc)
{
	switch (rc) {
	case SJA1105_ERR_OK:
		return "SJA1105_ERR_OK";
	case SJA1105_ERR_USAGE:
		return "SJA1105_ERR_USAGE";
	case SJA1105_ERR_CMDLINE_PARSE:
		return "SJA1105_ERR_CMDLINE_PARSE";
	case SJA1105_ERR_HW_NOT_RESPONDING:
		return "SJA1105_ERR_HW_NOT_RESPONDING";
	case SJA1105_ERR_HW_NOT_RESPONDING_STAGING_AREA_DIRTY:
		return "SJA1105_ERR_HW_NOT_RESPONDING_STAGING_AREA_DIRTY";
	case SJA1105_ERR_UPLOAD_FAILED_HW_LEFT_FLOATING_STAGING_AREA_DIRTY:
		return "SJA1105_ERR_UPLOAD_FAILED_HW_LEFT_FLOATING_STAGING_AREA_DIRTY";
	case SJA1105_ERR_UPLOAD_FAILED_HW_LEFT_FLOATING:
		return "SJA1105_ERR_UPLOAD_FAILED_HW_LEFT_FLOATING";
	case SJA1105_ERR_STAGING_AREA_INVALID:
		return "SJA1105_ERR_STAGING_AREA_INVALID";
	case SJA1105_ERR_INVALID_XML:
		return "SJA1105_ERR_INVALID_XML";
	case SJA1105_ERR_FILESYSTEM:
		return "SJA1105_ERR_FILESYSTEM";
	default:
		return "unknown error code";
	}
}

