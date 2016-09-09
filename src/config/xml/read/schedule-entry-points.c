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

static int entry_get(xmlNode *node, struct sja1105_schedule_entry_points_entry *entry)
{
	int rc = 0;
	rc |= xml_read_field(&entry->subschindx, "subschindx", node);
	rc |= xml_read_field(&entry->delta, "delta", node);
	rc |= xml_read_field(&entry->address, "address", node);
	return rc;
}

static int parse_entry(xmlNode *node, struct sja1105_config *config)
{
	struct sja1105_schedule_entry_points_entry entry;
	int rc;

	if (config->schedule_entry_points_count >= MAX_SCHEDULE_ENTRY_POINTS_COUNT) {
		fprintf(stderr, "Cannot have more than %d Schedule Entry Points "
		        "Table entries!\n", MAX_SCHEDULE_ENTRY_POINTS_COUNT);
		rc = -1;
		goto out;
	}
	memset(&entry, 0, sizeof(entry));
	rc = entry_get(node, &entry);
	config->schedule_entry_points[config->schedule_entry_points_count++] = entry;
out:
	return rc;
}

int schedule_entry_points_table_parse(xmlNode *node, struct sja1105_config *config)
{
	xmlNode *c;
	int rc = 0;

	if (node->type != XML_ELEMENT_NODE) {
		fprintf(stderr, "Schedule Entry Points Table node must be "
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

