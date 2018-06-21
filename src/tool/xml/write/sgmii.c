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

int
sgmii_table_write(xmlTextWriterPtr writer,
                  struct sja1105_static_config *config)
{
	int rc = 0;
	int i;

	logv("writing %d SGMII Table entries", config->sgmii_count);
	for (i = 0; i < config->sgmii_count; i++) {
		rc |= xmlTextWriterStartElement(writer, BAD_CAST "entry");
		rc |= xml_write_field(writer, "index",       i);
		rc |= xml_write_field(writer, "digital_error_cnt", config->sgmii[i].digital_error_cnt);
		rc |= xml_write_field(writer, "digital_control_2", config->sgmii[i].digital_control_2);
		rc |= xml_write_field(writer, "debug_control", config->sgmii[i].debug_control);
		rc |= xml_write_field(writer, "test_control", config->sgmii[i].test_control);
		rc |= xml_write_field(writer, "autoneg_control", config->sgmii[i].autoneg_control);
		rc |= xml_write_field(writer, "digital_control_1", config->sgmii[i].digital_control_1);
		rc |= xml_write_field(writer, "autoneg_adv", config->sgmii[i].autoneg_adv);
		rc |= xml_write_field(writer, "basic_control", config->sgmii[i].basic_control);
		rc |= xmlTextWriterEndElement(writer);
		if (rc < 0) {
			loge("error while writing SGMII Table element %d", i);
			return -EINVAL;
		}
	}
	return 0;
}


