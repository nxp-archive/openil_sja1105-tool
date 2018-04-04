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

static int entry_get(xmlNode *node, struct sja1105_general_params_table *entry)
{
	int rc = 0;
	rc |= xml_read_field(&entry->vllupformat, "vllupformat", node);
	rc |= xml_read_field(&entry->mirr_ptacu,  "mirr_ptacu", node);
	rc |= xml_read_field(&entry->switchid,    "switchid", node);
	rc |= xml_read_field(&entry->hostprio,    "hostprio", node);
	rc |= xml_read_field(&entry->mac_fltres1, "mac_fltres1", node);
	rc |= xml_read_field(&entry->mac_fltres0, "mac_fltres0", node);
	rc |= xml_read_field(&entry->mac_flt1,    "mac_flt1", node);
	rc |= xml_read_field(&entry->mac_flt0,    "mac_flt0", node);
	rc |= xml_read_field(&entry->incl_srcpt1, "incl_srcpt1", node);
	rc |= xml_read_field(&entry->incl_srcpt0, "incl_srcpt0", node);
	rc |= xml_read_field(&entry->send_meta1,  "send_meta1", node);
	rc |= xml_read_field(&entry->send_meta0,  "send_meta0", node);
	rc |= xml_read_field(&entry->casc_port,   "casc_port", node);
	rc |= xml_read_field(&entry->host_port,   "host_port", node);
	rc |= xml_read_field(&entry->mirr_port,   "mirr_port", node);
	rc |= xml_read_field(&entry->vlmarker,    "vlmarker", node);
	rc |= xml_read_field(&entry->vlmask,      "vlmask", node);
	rc |= xml_read_field(&entry->tpid,        "tpid", node);
	rc |= xml_read_field(&entry->ignore2stf,  "ignore2stf", node);
	rc |= xml_read_field(&entry->tpid2,       "tpid2", node);
	if (rc) {
		loge("General Parameters entry is incomplete!");
	}
	return rc;
}

static int parse_entry(xmlNode *node, struct sja1105_static_config *config)
{
	struct sja1105_general_params_table entry;
	int rc;

	if (config->general_params_count >= MAX_GENERAL_PARAMS_COUNT) {
		loge("Cannot have more than %d General Parameters Table entries!",
		     MAX_GENERAL_PARAMS_COUNT);
		rc = -ERANGE;
		goto out;
	}
	memset(&entry, 0, sizeof(entry));
	rc = entry_get(node, &entry);
	config->general_params[config->general_params_count++] = entry;
out:
	return rc;
}

int general_parameters_table_parse(xmlNode *node, struct sja1105_static_config *config)
{
	xmlNode *c;
	int rc = 0;

	if (node->type != XML_ELEMENT_NODE) {
		loge("General Parameters Table node must be of element type!");
		rc = -EINVAL;
		goto out;
	}
	for (c = node->children; c != NULL; c = c->next) {
		if (c->type != XML_ELEMENT_NODE) {
			continue;
		}
		rc = parse_entry(c, config);
		if (rc < 0) {
			goto out;
		}
	}
	logv("read %d General Parameters entries", config->general_params_count);
out:
	return rc;
}

