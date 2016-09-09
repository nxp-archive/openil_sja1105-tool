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

static int entry_get(xmlNode *node, struct sja1105_mac_config_entry *entry)
{
	int rc = 0;

	rc  = xml_read_array(&entry->top, 8, "top", node);
	rc += xml_read_array(&entry->base, 8, "base", node);
	rc += xml_read_array(&entry->enabled, 8, "enabled", node);
	if (rc != 8 * 3) {
		fprintf(stderr, "Must have exactly 8 entries for TOP, BASE and ENABLED!\n");
		rc = -1;
		goto out;
	}
	rc  = xml_read_field(&entry->ifg, "ifg", node);
	rc |= xml_read_field(&entry->speed, "speed", node);
	rc |= xml_read_field(&entry->tp_delin, "tp_delin", node);
	rc |= xml_read_field(&entry->tp_delout, "tp_delout", node);
	rc |= xml_read_field(&entry->maxage, "maxage", node);
	rc |= xml_read_field(&entry->vlanprio, "vlanprio", node);
	rc |= xml_read_field(&entry->vlanid, "vlanid", node);
	rc |= xml_read_field(&entry->ing_mirr, "ing_mirr", node);
	rc |= xml_read_field(&entry->egr_mirr, "egr_mirr", node);
	rc |= xml_read_field(&entry->drpnona664, "drpnona664", node);
	rc |= xml_read_field(&entry->drpdtag, "drpdtag", node);
	rc |= xml_read_field(&entry->drpuntag, "drpuntag", node);
	rc |= xml_read_field(&entry->retag, "retag", node);
	rc |= xml_read_field(&entry->dyn_learn, "dyn_learn", node);
	rc |= xml_read_field(&entry->egress, "egress", node);
	rc |= xml_read_field(&entry->ingress, "ingress", node);
out:
	return rc;
}

static int parse_entry(xmlNode *node, struct sja1105_config *config)
{
	struct sja1105_mac_config_entry entry;
	int rc;

	if (config->mac_config_count >= MAX_MAC_CONFIG_COUNT) {
		fprintf(stderr, "Cannot have more than %d MAC Configuration "
		        "Table entries!\n", MAX_MAC_CONFIG_COUNT);
		rc = -1;
		goto out;
	}
	memset(&entry, 0, sizeof(entry));
	rc = entry_get(node, &entry);
	config->mac_config[config->mac_config_count++] = entry;
out:
	return rc;
}

int mac_configuration_table_parse(xmlNode *node, struct sja1105_config *config)
{
	xmlNode *c;
	int rc = 0;

	if (node->type != XML_ELEMENT_NODE) {
		fprintf(stderr, "MAC Configuration Table node must be "
		        "of element type!\n");
		rc = -1;
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
out:
	return rc;
}

