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
general_parameters_table_write(xmlTextWriterPtr writer,
                               struct sja1105_static_config *config)
{
	int rc = 0;
	int i;

	logv("writing %d General Parameters entries", config->general_params_count);
	for (i = 0; i < config->general_params_count; i++) {
		rc |= xmlTextWriterStartElement(writer, BAD_CAST "entry");
		rc |= xml_write_field(writer, "index",       i);
		rc |= xml_write_field(writer, "vllupformat", config->general_params[i].vllupformat);
		rc |= xml_write_field(writer, "mirr_ptacu", config->general_params[i].mirr_ptacu);
		rc |= xml_write_field(writer, "switchid", config->general_params[i].switchid);
		rc |= xml_write_field(writer, "hostprio", config->general_params[i].hostprio);
		rc |= xml_write_field(writer, "mac_fltres1", config->general_params[i].mac_fltres1);
		rc |= xml_write_field(writer, "mac_fltres0", config->general_params[i].mac_fltres0);
		rc |= xml_write_field(writer, "mac_flt1", config->general_params[i].mac_flt1);
		rc |= xml_write_field(writer, "mac_flt0", config->general_params[i].mac_flt0);
		rc |= xml_write_field(writer, "incl_srcpt1", config->general_params[i].incl_srcpt1);
		rc |= xml_write_field(writer, "incl_srcpt0", config->general_params[i].incl_srcpt0);
		rc |= xml_write_field(writer, "send_meta1", config->general_params[i].send_meta1);
		rc |= xml_write_field(writer, "send_meta0", config->general_params[i].send_meta0);
		rc |= xml_write_field(writer, "casc_port", config->general_params[i].casc_port);
		rc |= xml_write_field(writer, "host_port", config->general_params[i].host_port);
		rc |= xml_write_field(writer, "mirr_port", config->general_params[i].mirr_port);
		rc |= xml_write_field(writer, "vlmarker", config->general_params[i].vlmarker);
		rc |= xml_write_field(writer, "vlmask", config->general_params[i].vlmask);
		rc |= xml_write_field(writer, "tpid", config->general_params[i].tpid);
		rc |= xml_write_field(writer, "ignore2stf", config->general_params[i].ignore2stf);
		rc |= xml_write_field(writer, "tpid2", config->general_params[i].tpid2);
		rc |= xml_write_field(writer, "queue_ts", config->general_params[i].queue_ts);
		rc |= xml_write_field(writer, "egrmirrvid", config->general_params[i].egrmirrvid);
		rc |= xml_write_field(writer, "egrmirrpcp", config->general_params[i].egrmirrpcp);
		rc |= xml_write_field(writer, "egrmirrdei", config->general_params[i].egrmirrdei);
		rc |= xml_write_field(writer, "replay_port", config->general_params[i].replay_port);
		rc |= xmlTextWriterEndElement(writer);
		if (rc < 0) {
			loge("error while writing general_params Table element %d", i);
			return -EINVAL;
		}
	}
	return 0;
}


