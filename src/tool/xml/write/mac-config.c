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
mac_configuration_table_write(xmlTextWriterPtr writer,
                              struct sja1105_static_config *config)
{
	int rc = 0;
	int i;

	logv("writing %d MAC Configuration entries", config->mac_config_count);
	for (i = 0; i < config->mac_config_count; i++) {
		rc |= xmlTextWriterStartElement(writer, BAD_CAST "entry");
		rc |= xml_write_field(writer, "index", i);
		rc |= xml_write_array(writer, "top", config->mac_config[i].top, 8);
		rc |= xml_write_array(writer, "base", config->mac_config[i].base, 8);
		rc |= xml_write_array(writer, "enabled", config->mac_config[i].enabled, 8);
		rc |= xml_write_field(writer, "ifg", config->mac_config[i].ifg);
		rc |= xml_write_field(writer, "tp_delin", config->mac_config[i].tp_delin);
		rc |= xml_write_field(writer, "tp_delout", config->mac_config[i].tp_delout);
		rc |= xml_write_field(writer, "maxage", config->mac_config[i].maxage);
		rc |= xml_write_field(writer, "vlanprio", config->mac_config[i].vlanprio);
		rc |= xml_write_field(writer, "vlanid", config->mac_config[i].vlanid);
		rc |= xml_write_field(writer, "ing_mirr", config->mac_config[i].ing_mirr);
		rc |= xml_write_field(writer, "egr_mirr", config->mac_config[i].egr_mirr);
		rc |= xml_write_field(writer, "drpnona664", config->mac_config[i].drpnona664);
		rc |= xml_write_field(writer, "drpdtag", config->mac_config[i].drpdtag);
		rc |= xml_write_field(writer, "drpsotag", config->mac_config[i].drpsotag);
		rc |= xml_write_field(writer, "drpsitag", config->mac_config[i].drpsitag);
		rc |= xml_write_field(writer, "drpuntag", config->mac_config[i].drpuntag);
		rc |= xml_write_field(writer, "retag", config->mac_config[i].retag);
		rc |= xml_write_field(writer, "dyn_learn", config->mac_config[i].dyn_learn);
		rc |= xml_write_field(writer, "egress", config->mac_config[i].egress);
		rc |= xml_write_field(writer, "ingress", config->mac_config[i].ingress);
		rc |= xml_write_field(writer, "mirrcie", config->mac_config[i].mirrcie);
		rc |= xml_write_field(writer, "mirrcetag", config->mac_config[i].mirrcetag);
		rc |= xml_write_field(writer, "ingmirrvid", config->mac_config[i].ingmirrvid);
		rc |= xml_write_field(writer, "ingmirrpcp", config->mac_config[i].ingmirrpcp);
		rc |= xml_write_field(writer, "ingmirrdei", config->mac_config[i].ingmirrdei);
		rc |= xmlTextWriterEndElement(writer);
		if (rc < 0) {
			loge("error while writing mac_config Table element %d", i);
			return -EINVAL;
		}
	}
	return 0;
}

