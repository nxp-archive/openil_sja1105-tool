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
#ifndef _SJA1105_TOOL_INTERNAL
#define _SJA1105_TOOL_INTERNAL

#include <common.h>
#include <lib/include/staging-area.h>
#include <lib/include/spi.h>
#include <lib/include/static-config.h>
#include <lib/include/dynamic-config.h>
#include <lib/include/port-control.h>
#include <lib/include/gtable.h>

struct general_config {
	char *staging_area;
	int   screen_width;
	int   entries_per_line;
	int   verbose;
	int   debug;
};

/* defined in src/tool/sja1105-config.c */
extern struct general_config general_config;
extern int SJA1105_VERBOSE_CONDITION;
extern int SJA1105_DEBUG_CONDITION;

int read_config_file(char*, struct sja1105_spi_setup*, struct general_config*);
int rgu_parse_args(struct sja1105_spi_setup*, int argc, char **argv);
int ptp_parse_args(struct sja1105_spi_setup*, int argc, char **argv);
int config_parse_args(struct sja1105_spi_setup*, int argc, char **argv);
int status_parse_args(struct sja1105_spi_setup*, int argc, char **argv);
int reg_parse_args(struct sja1105_spi_setup*, int argc, char **argv);
int staging_area_modify(struct sja1105_staging_area*, char*, char*, char*);
int staging_area_modify_parse(struct sja1105_staging_area*,
                              int *argc, char ***argv);
int sja1105_staging_area_show(struct sja1105_staging_area*, char *table_name);

int staging_area_load(const char*, struct sja1105_staging_area*);
int staging_area_save(const char*, struct sja1105_staging_area*);
int staging_area_flush(struct sja1105_spi_setup*,
                       struct sja1105_staging_area*);
int staging_area_hexdump(const char*);

/* From strings.c, mainly */
char *trimwhitespace(char *str);
int   matches(const char*, const char*);
int   get_match(const char*, const char**, int);
int   get_multiline_buf_width(char *buf);
int   get_entry_count_to_fit_screen(char **print_bufs, int count);
void  show_print_bufs(char **print_bufs, int count);
void  linewise_concat(char **buffers, int count);
int   read_array(char *array_str, uint64_t *array_val, int max_count);
int   reliable_uint64_from_string(uint64_t *to, char *from, char**);
int   reliable_double_from_string(double *to, char *from, char**);


#define TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(table)                           \
	void sja1105_##table##_entry_show(struct sja1105_##table##_entry*);        \

/* show table functions */
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(l2_forwarding_params);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(l2_forwarding);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(l2_policing);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(mac_config);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(schedule_entry_points_params);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(schedule_entry_points);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(schedule_params);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(schedule);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(vlan_lookup);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(xmii_params);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(sgmii);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(vl_forwarding_params);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(vl_forwarding);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(vl_policing);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(vl_lookup);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(avb_params);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(general_params);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(mac_config);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(l2_lookup);
TOOL_DEFINE_HEADERS_FOR_CONFIG_TABLE(l2_lookup_params);

void sja1105_table_header_show(struct sja1105_table_header *hdr);
void sja1105_mgmt_entry_show(struct sja1105_mgmt_entry *entry);
void sja1105_dyn_l2_lookup_cmd_show(struct sja1105_dyn_l2_lookup_cmd *cmd);
void sja1105_cbs_show(struct sja1105_cbs *cbs);
void sja1105_cgu_pll_control_show(struct sja1105_cgu_pll_control *pll_control,
                                  uint64_t device_id);
void sja1105_cgu_mii_control_show(struct sja1105_cgu_mii_control *mii_control);
void sja1105_cgu_idiv_show(struct sja1105_cgu_idiv *idiv);
void sja1105_cfg_pad_mii_tx_show(struct sja1105_cfg_pad_mii_tx *pad_mii_tx);
int sja1105_static_config_hexdump(void *buf);
void gtable_hexdump(void *table, int len);
void gtable_bitdump(void *table, int len);

#define SJA1105_NETCONF_ROOT "sja1105"
#define SJA1105_NETCONF_NS   "http://nxp.com/ns/yang/tsn/sja1105"

#endif
