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
l2_address_lookup_parameters_table_write(xmlTextWriterPtr writer,
                                         struct sja1105_static_config *config)
{
	int rc = 0;
	int i;

	logv("writing %d L2 Lookup Parameters entries", config->l2_lookup_params_count);
	for (i = 0; i < config->l2_lookup_params_count; i++) {
		rc |= xmlTextWriterStartElement(writer, BAD_CAST "entry");
		rc |= xml_write_field(writer, "index", i);
		rc |= xml_write_field(writer, "maxage", config->l2_lookup_params[i].maxage);
		rc |= xml_write_field(writer, "dyn_tbsz", config->l2_lookup_params[i].dyn_tbsz);
		rc |= xml_write_field(writer, "poly", config->l2_lookup_params[i].poly);
		rc |= xml_write_field(writer, "shared_learn", config->l2_lookup_params[i].shared_learn);
		rc |= xml_write_field(writer, "no_enf_hostprt", config->l2_lookup_params[i].no_enf_hostprt);
		rc |= xml_write_field(writer, "no_mgmt_learn", config->l2_lookup_params[i].no_mgmt_learn);
		rc |= xmlTextWriterEndElement(writer);
		if (rc < 0) {
			loge("error while writing l2_lookup_params Table element %d", i);
			goto out;
		}
	}
out:
	return rc;
}

