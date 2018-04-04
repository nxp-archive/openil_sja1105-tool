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

static int entry_get(xmlNode *node, struct sja1105_l2_lookup_params_table *entry)
{
	int rc = 0;
	rc |= xml_read_field(&entry->maxage, "maxage", node);
	rc |= xml_read_field(&entry->dyn_tbsz, "dyn_tbsz", node);
	rc |= xml_read_field(&entry->poly, "poly", node);
	rc |= xml_read_field(&entry->shared_learn, "shared_learn", node);
	rc |= xml_read_field(&entry->no_enf_hostprt, "no_enf_hostprt", node);
	rc |= xml_read_field(&entry->no_mgmt_learn, "no_mgmt_learn", node);
	if (rc) {
		loge("L2 Lookup Parameters entry is incomplete!");
	}
	return rc;
}

static int parse_entry(xmlNode *node, struct sja1105_static_config *config)
{
	struct sja1105_l2_lookup_params_table entry;
	int rc;

	if (config->l2_lookup_params_count >= MAX_L2_LOOKUP_PARAMS_COUNT) {
		loge("Cannot have more than %d L2 Lookup "
		     "Parameters Table entries!", MAX_L2_LOOKUP_PARAMS_COUNT);
		rc = -ERANGE;
		goto out;
	}
	memset(&entry, 0, sizeof(entry));
	rc = entry_get(node, &entry);
	config->l2_lookup_params[config->l2_lookup_params_count++] = entry;
out:
	return rc;
}

int l2_address_lookup_parameters_table_parse(xmlNode *node, struct sja1105_static_config *config)
{
	xmlNode *c;
	int rc = 0;

	if (node->type != XML_ELEMENT_NODE) {
		loge("L2 Lookup Parameters Table node must be "
		     "of element type!");
		rc = -EINVAL;
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
	logv("read %d L2 Lookup Parameters entries",
	     config->l2_lookup_params_count);
out:
	return rc;
}

