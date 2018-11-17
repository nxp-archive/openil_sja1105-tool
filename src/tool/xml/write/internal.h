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
#ifndef _CONFIG_XML_WRITE_INTERNAL
#define _CONFIG_XML_WRITE_INTERNAL

#include <stdio.h>
#include <libxml/xmlwriter.h>
#include <stdint.h>
#include <inttypes.h>
/* These are our include files */
#include <lib/include/static-config.h>
#include <common.h>
/* This is the top-level _SJA1105_TOOL_INTERNAL header */
#include <tool/internal.h>

int xml_write_field(xmlTextWriterPtr, char*, uint64_t);
int xml_write_array(xmlTextWriterPtr, char*, uint64_t*, int);
int schedule_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int schedule_entry_points_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int schedule_entry_points_parameters_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int vl_lookup_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int vl_policing_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int vl_forwarding_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int l2_address_lookup_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int l2_policing_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int vlan_lookup_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int l2_forwarding_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int mac_configuration_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int schedule_parameters_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int schedule_entry_points_parameters_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int vl_forwarding_parameters_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int l2_address_lookup_parameters_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int l2_forwarding_parameters_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int avb_parameters_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int clock_synchronization_parameters_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int general_parameters_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);
int retagging_table_write(xmlTextWriterPtr, struct sja1105_static_config *config);

#endif
