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
#include <string.h>
#include "xml/write/external.h"
#include <common.h>
#include "internal.h"

#if !defined(LIBXML_WRITER_ENABLED) || !defined(LIBXML_OUTPUT_ENABLED)

int
sja1105_staging_area_to_xml(__attribute__((unused)) char *xml_file,
                            __attribute__((unused)) struct sja1105_staging_area
                            *staging_area)
{
	loge("Writer or output support is not compiled in libxml!");
	return -1;
}

#else

int xml_write_field(xmlTextWriterPtr writer, char *field, uint64_t value)
{
	char print_buf[MAX_LINE_SIZE];

	if (!strcmp(field, "index"))
		snprintf(print_buf, MAX_LINE_SIZE, "%" PRIu64, value);
	else
		snprintf(print_buf, MAX_LINE_SIZE, "0x%" PRIX64, value);

	/* Returns the bytes written (may be 0 because of buffering) or -1 in case of error.
	 * Had to read the libxml2 code for that.
	 */
	return xmlTextWriterWriteElement(writer, BAD_CAST field, BAD_CAST print_buf);
}

int xml_write_array(xmlTextWriterPtr writer, char *field, uint64_t *values, int count)
{
	char print_buf[MAX_LINE_SIZE];

	print_array(print_buf, values, count);
	return xmlTextWriterWriteElement(writer, BAD_CAST field, BAD_CAST print_buf);
}

static int device_id_write(xmlTextWriterPtr writer, uint64_t device_id)
{
	logv("writing device_id");
	return xml_write_field(writer, "device-id", device_id);
}

static int
static_config_write(xmlTextWriterPtr writer,
                    struct sja1105_static_config *config)
{
	const char *options[] = {
		"schedule-table",
		"schedule-entry-points-table",
		"vl-lookup-table",
		"vl-policing-table",
		"vl-forwarding-table",
		"l2-address-lookup-table",
		"l2-policing-table",
		"vlan-lookup-table",
		"l2-forwarding-table",
		"mac-configuration-table",
		"schedule-parameters-table",
		"schedule-entry-points-parameters-table",
		"vl-forwarding-parameters-table",
		"l2-address-lookup-parameters-table",
		"l2-forwarding-parameters-table",
		"clock-synchronization-parameters-table",
		"avb-parameters-table",
		"general-parameters-table",
		"retagging-table",
	};
	int (*next_write_config_table[])(xmlTextWriterPtr,
	                                 struct sja1105_static_config *) = {
		schedule_table_write,
		schedule_entry_points_table_write,
		vl_lookup_table_write,
		vl_policing_table_write,
		vl_forwarding_table_write,
		l2_address_lookup_table_write,
		l2_policing_table_write,
		vlan_lookup_table_write,
		l2_forwarding_table_write,
		mac_configuration_table_write,
		schedule_parameters_table_write,
		schedule_entry_points_parameters_table_write,
		vl_forwarding_parameters_table_write,
		l2_address_lookup_parameters_table_write,
		l2_forwarding_parameters_table_write,
		clock_synchronization_parameters_table_write,
		avb_parameters_table_write,
		general_parameters_table_write,
		retagging_table_write,
	};
	int rc = 0;
	unsigned int i;

	rc = xmlTextWriterStartElement(writer, BAD_CAST "static");
	if (rc < 0) {
		loge("could not create root element for static config");
		goto out;
	}
	for (i = 0; i < ARRAY_SIZE(options); i++) {
		rc |= xmlTextWriterStartElement(writer, BAD_CAST options[i]);
		rc |= next_write_config_table[i](writer, config);
		rc |= xmlTextWriterEndElement(writer);
		if (rc < 0) {
			return -EINVAL;
		}
	}
	rc = xmlTextWriterEndElement(writer);
	if (rc < 0) {
		loge("could not end root element for static config");
		goto out;
	}
out:
	return rc;
}

int
sja1105_staging_area_to_xml(char *xml_file,
                            struct sja1105_staging_area *staging_area)
{
	int rc = 0;
	xmlTextWriterPtr writer;

	/*
	 * this initializes the library and checks potential ABI mismatches
	 * between the version it was compiled for and the actual shared
	 * library used.
	 */
	LIBXML_TEST_VERSION

	writer = xmlNewTextWriterFilename(xml_file, 0);
	if (writer == NULL) {
		loge("cannot create xml writer");
		rc = -errno;
		goto out;
	}
	rc |= xmlTextWriterSetIndent(writer, 1);
	rc |= xmlTextWriterSetIndentString(writer, (const xmlChar*) "\t");
	if (rc < 0) {
		loge("could not set xml indentation");
		goto out;
	}
	rc = xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
	if (rc < 0) {
		goto out;
	}
	/* Root element */
	rc = xmlTextWriterStartElement(writer, BAD_CAST SJA1105_NETCONF_ROOT);
	if (rc < 0) {
		goto out;
	}
	rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "xmlns",
	                                 BAD_CAST SJA1105_NETCONF_NS);
	/* Device ID */
	device_id_write(writer, staging_area->static_config.device_id);
	/* Static config */
	rc = static_config_write(writer, &staging_area->static_config);
	if (rc < 0) {
		loge("could not write config tables");
		goto out;
	}
	/* End root element */
	rc = xmlTextWriterEndElement(writer);
	if (rc < 0) {
		goto out;
	}
	rc = xmlTextWriterEndDocument(writer);
	if (rc < 0) {
		loge("could not write xml document");
		goto out;
	}
	/*
	 * Cleanup function for the XML library.
	 */
	xmlCleanupParser();
out:
	return 0;
}

#endif
