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

static int entry_get(xmlNode *node, struct sja1105_l2_forwarding_entry *entry)
{
	int rc = 0;
	rc |= xml_read_field(&entry->bc_domain, "bc_domain", node);
	if (rc < 0) {
		fprintf(stderr, "bc_domain read failed\n");
		goto error;
	}
	rc |= xml_read_field(&entry->reach_port, "reach_port", node);
	if (rc < 0) {
		fprintf(stderr, "reach_port read failed\n");
		goto error;
	}
	rc |= xml_read_field(&entry->fl_domain, "fl_domain", node);
	if (rc < 0) {
		fprintf(stderr, "fl_domain read failed\n");
		goto error;
	}
	rc = xml_read_array(&entry->vlan_pmap, 8, "vlan_pmap", node);
	if (rc != 8) {
		fprintf(stderr, "Must have exactly 8 VLAN_PMAP entries!\n");
		goto error;
	}
error:
	if (rc) {
		fprintf(stderr, "L2 Forwarding entry is incomplete!\n");
	}
	return rc;
}

static int parse_entry(xmlNode *node, struct sja1105_config *config)
{
	struct sja1105_l2_forwarding_entry entry;
	int rc;

	if (config->l2_forwarding_count >= MAX_L2_FORWARDING_COUNT) {
		fprintf(stderr, "Cannot have more than %d L2 Forwarding "
		        "Table entries!\n", MAX_L2_FORWARDING_COUNT);
		return -1;
	}
	memset(&entry, 0, sizeof(entry));
	rc = entry_get(node, &entry);
	if (rc != 0) {
		goto error;
	}
	config->l2_forwarding[config->l2_forwarding_count++] = entry;
	return 0;
error:
	return -1;
}

int l2_forwarding_table_parse(xmlNode *node, struct sja1105_config *config)
{
	xmlNode *c;

	if (node->type != XML_ELEMENT_NODE) {
		fprintf(stderr, "L2 Forwarding Table node must be "
		        "of element type!\n");
		return -1;
	}
	for (c = node->children; c != NULL; c = c->next) {
		if (c->type != XML_ELEMENT_NODE) {
			continue;
		}
		parse_entry(c, config);
	}
	return 0;
}

