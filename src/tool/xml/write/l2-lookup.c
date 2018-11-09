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
l2_address_lookup_table_write(xmlTextWriterPtr writer,
                              struct sja1105_static_config *config)
{
	int rc = 0;
	int i;

	logv("writing %d L2 Lookup entries", config->l2_lookup_count);
	for (i = 0; i < config->l2_lookup_count; i++) {
		rc |= xmlTextWriterStartElement(writer, BAD_CAST "entry");
		rc |= xml_write_field(writer, "tsreg",    config->l2_lookup[i].tsreg);
		rc |= xml_write_field(writer, "mirrvlan",    config->l2_lookup[i].mirrvlan);
		rc |= xml_write_field(writer, "takets",    config->l2_lookup[i].takets);
		rc |= xml_write_field(writer, "mirr",    config->l2_lookup[i].mirr);
		rc |= xml_write_field(writer, "retag",    config->l2_lookup[i].retag);
		rc |= xml_write_field(writer, "mask_iotag",    config->l2_lookup[i].mask_iotag);
		rc |= xml_write_field(writer, "mask_vlanid",    config->l2_lookup[i].mask_vlanid);
		rc |= xml_write_field(writer, "mask_macaddr",    config->l2_lookup[i].mask_macaddr);
		rc |= xml_write_field(writer, "iotag",    config->l2_lookup[i].iotag);
		rc |= xml_write_field(writer, "vlanid",    config->l2_lookup[i].vlanid);
		rc |= xml_write_field(writer, "macaddr",   config->l2_lookup[i].macaddr);
		rc |= xml_write_field(writer, "destports", config->l2_lookup[i].destports);
		rc |= xml_write_field(writer, "enfport",   config->l2_lookup[i].enfport);
		rc |= xml_write_field(writer, "index",     config->l2_lookup[i].index);
		rc |= xmlTextWriterEndElement(writer);
		if (rc < 0) {
			loge("error while writing l2_lookup Table element %d", i);
			return -EINVAL;
		}
	}
	return 0;
}

