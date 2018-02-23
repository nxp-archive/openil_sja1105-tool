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
#ifndef _CONFIG_TABLES_H
#define _CONFIG_TABLES_H

#include <stdint.h>

#define CONFIG_ADDR 0x20000

#define SIZE_TABLE_HEADER                       12
#define SIZE_SCHEDULE_ENTRY                     8
#define SIZE_SCHEDULE_ENTRY_POINTS_ENTRY        4
#define SIZE_VL_LOOKUP_ENTRY                    12
#define SIZE_VL_POLICING_ENTRY                  8
#define SIZE_VL_FORWARDING_ENTRY                4
#define SIZE_L2_LOOKUP_ENTRY                    12
#define SIZE_L2_POLICING_ENTRY                  8
#define SIZE_VLAN_LOOKUP_ENTRY                  8
#define SIZE_L2_FORWARDING_ENTRY                8
#define SIZE_MAC_CONFIG_ENTRY                   28
#define SIZE_SCHEDULE_PARAMS_ENTRY              12
#define SIZE_SCHEDULE_ENTRY_POINTS_PARAMS_ENTRY 4
#define SIZE_VL_FORWARDING_PARAMS_ENTRY         12
#define SIZE_L2_LOOKUP_PARAMS_TABLE             4
#define SIZE_L2_FORWARDING_PARAMS_TABLE         12
#define SIZE_CLK_SYNC_PARAMS_TABLE              52
#define SIZE_AVB_PARAMS_TABLE                   12
#define SIZE_GENERAL_PARAMS_TABLE               40
#define SIZE_RETAGGING_ENTRY                    8
#define SIZE_XMII_MODE_PARAMS_TABLE             4

/* UM10944.pdf Page 11, Table 2. Configuration Blocks */
#define BLKID_SCHEDULE_TABLE                     0x00
#define BLKID_SCHEDULE_ENTRY_POINTS_TABLE        0x01
#define BLKID_VL_LOOKUP_TABLE                    0x02
#define BLKID_VL_POLICING_TABLE                  0x03
#define BLKID_VL_FORWARDING_TABLE                0x04
#define BLKID_L2_LOOKUP_TABLE                    0x05
#define BLKID_L2_POLICING_TABLE                  0x06
#define BLKID_VLAN_LOOKUP_TABLE                  0x07
#define BLKID_L2_FORWARDING_TABLE                0x08
#define BLKID_MAC_CONFIG_TABLE                   0x09
#define BLKID_SCHEDULE_PARAMS_TABLE              0x0A
#define BLKID_SCHEDULE_ENTRY_POINTS_PARAMS_TABLE 0x0B
#define BLKID_VL_FORWARDING_PARAMS_TABLE         0x0C
#define BLKID_L2_LOOKUP_PARAMS_TABLE             0x0D
#define BLKID_L2_FORWARDING_PARAMS_TABLE         0x0E
#define BLKID_CLK_SYNC_PARAMS_TABLE              0x0F
#define BLKID_AVB_PARAMS_TABLE                   0x10
#define BLKID_GENERAL_PARAMS_TABLE               0x11
#define BLKID_RETAGGING_TABLE                    0x12
#define BLKID_XMII_MODE_PARAMS_TABLE             0x4E

#define MAX_SCHEDULE_COUNT                       1024
#define MAX_SCHEDULE_ENTRY_POINTS_COUNT          2048
#define MAX_VL_LOOKUP_COUNT                      1024
#define MAX_VL_POLICING_COUNT                    1024
#define MAX_VL_FORWARDING_COUNT                  1024
#define MAX_L2_LOOKUP_COUNT                      1024
#define MAX_L2_POLICING_COUNT                    45
#define MAX_VLAN_LOOKUP_COUNT                    4096
#define MAX_L2_FORWARDING_COUNT                  13
#define MAX_MAC_CONFIG_COUNT                     5
#define MAX_SCHEDULE_PARAMS_COUNT                1
#define MAX_SCHEDULE_ENTRY_POINTS_PARAMS_COUNT   1
#define MAX_VL_FORWARDING_PARAMS_COUNT           1
#define MAX_L2_LOOKUP_PARAMS_COUNT               1
#define MAX_L2_FORWARDING_PARAMS_COUNT           1
#define MAX_GENERAL_PARAMS_COUNT                 1
#define MAX_RETAGGING_COUNT                      32
#define MAX_XMII_PARAMS_COUNT                    1
#define MAX_AVB_PARAMS_COUNT                     1
#define MAX_CLK_SYNC_COUNT                       1

#define MAX_FRAME_MEMORY                         929
#define MAX_FRAME_MEMORY_RETAGGING               910

#define SJA1105T_DEVICE_ID                       0x9E00030Eull

struct sja1105_schedule_entry {
	uint64_t winstindex;
	uint64_t winend;
	uint64_t winst;
	uint64_t destports;
	uint64_t setvalid;
	uint64_t txen;
	uint64_t resmedia_en;
	uint64_t resmedia;
	uint64_t vlindex;
	uint64_t delta;
};

struct sja1105_schedule_params_entry {
	uint64_t subscheind[8];
};

struct sja1105_general_params_table {
	uint64_t vllupformat;
	uint64_t mirr_ptacu;
	uint64_t switchid;
	uint64_t hostprio;
	uint64_t mac_fltres1;
	uint64_t mac_fltres0;
	uint64_t mac_flt1;
	uint64_t mac_flt0;
	uint64_t incl_srcpt1;
	uint64_t incl_srcpt0;
	uint64_t send_meta1;
	uint64_t send_meta0;
	uint64_t casc_port;
	uint64_t host_port;
	uint64_t mirr_port;
	uint64_t vlmarker;
	uint64_t vlmask;
	uint64_t tpid;
	uint64_t ignore2stf;
	uint64_t tpid2;
};

struct sja1105_schedule_entry_points_entry {
	uint64_t subschindx;
	uint64_t delta;
	uint64_t address;
};

struct sja1105_schedule_entry_points_params {
	uint64_t clksrc;
	uint64_t actsubsch;
};

struct sja1105_table_header {
	uint64_t block_id;
	uint64_t len;
	uint64_t crc;
};

struct sja1105_vlan_lookup_entry {
	uint64_t ving_mirr;
	uint64_t vegr_mirr;
	uint64_t vmemb_port;
	uint64_t vlan_bc;
	uint64_t tag_port;
	uint64_t vlanid;
};

struct sja1105_l2_lookup_entry {
	uint64_t vlanid;
	uint64_t macaddr;
	uint64_t destports;
	uint64_t enfport;
	uint64_t index;
};

struct sja1105_l2_lookup_params_table {
	uint64_t maxage;
	uint64_t dyn_tbsz;
	uint64_t poly;
	uint64_t shared_learn;
	uint64_t no_enf_hostprt;
	uint64_t no_mgmt_learn;
};

struct sja1105_l2_forwarding_entry {
	uint64_t bc_domain;
	uint64_t reach_port;
	uint64_t fl_domain;
	uint64_t vlan_pmap[8];
};

struct sja1105_l2_forwarding_params_table {
	uint64_t max_dynp;
	uint64_t part_spc[8];
};

struct sja1105_l2_policing_entry {
	uint64_t sharindx;
	uint64_t smax;
	uint64_t rate;
	uint64_t maxlen;
	uint64_t partition;
};

struct sja1105_mac_config_entry {
	uint64_t top[8];
	uint64_t base[8];
	uint64_t enabled[8];
	uint64_t ifg;
	uint64_t speed;
	uint64_t tp_delin;
	uint64_t tp_delout;
	uint64_t maxage;
	uint64_t vlanprio;
	uint64_t vlanid;
	uint64_t ing_mirr;
	uint64_t egr_mirr;
	uint64_t drpnona664;
	uint64_t drpdtag;
	uint64_t drpuntag;
	uint64_t retag;
	uint64_t dyn_learn;
	uint64_t egress;
	uint64_t ingress;
};

struct sja1105_xmii_params_table {
	uint64_t phy_mac[5];
	uint64_t xmii_mode[5];
};

struct sja1105_avb_params_table {
	uint64_t destmeta;
	uint64_t srcmeta;
};

struct sja1105_vl_lookup_entry {
	uint64_t format;
	uint64_t port;
	union {
		/* format == 0 */
		struct {
			uint64_t destports;
			uint64_t iscritical;
			uint64_t macaddr;
			uint64_t vlanid;
			uint64_t vlanprior;
		};
		/* format == 1 */
		struct {
			uint64_t egrmirr;
			uint64_t ingrmirr;
			uint64_t vlid;
		};
	};
};

struct sja1105_vl_policing_entry {
	uint64_t type;
	uint64_t maxlen;
	uint64_t sharindx;
	uint64_t bag;
	uint64_t jitter;
};

struct sja1105_vl_forwarding_entry {
	uint64_t type;
	uint64_t priority;
	uint64_t partition;
	uint64_t destports;
};

struct sja1105_vl_forwarding_params_table {
	uint64_t partspc[8];
	uint64_t debugen;
};

struct sja1105_clk_sync_params_table {
	uint64_t etssrcpcf;
	uint64_t waitthsync;
	uint64_t wfintmout;
	uint64_t unsytotsyth;
	uint64_t unsytosyth;
	uint64_t tsytosyth;
	uint64_t tsyth;
	uint64_t tsytousyth;
	uint64_t syth;
	uint64_t sytousyth;
	uint64_t sypriority;
	uint64_t sydomain;
	uint64_t stth;
	uint64_t sttointth;
	uint64_t pcfsze;
	uint64_t pcfpriority;
	uint64_t obvwinsz;
	uint64_t numunstbcy;
	uint64_t numstbcy;
	uint64_t maxtranspclk;
	uint64_t maxintegcy;
	uint64_t listentmout;
	uint64_t intcydur;
	uint64_t inttotentth;
	uint64_t vlidout;
	uint64_t vlidimnmin;
	uint64_t vlidinmax;
	uint64_t caentmout;
	uint64_t accdevwin;
	uint64_t vlidselect;
	uint64_t tentsyrelen;
	uint64_t asytensyen;
	uint64_t sytostben;
	uint64_t syrelen;
	uint64_t sysyen;
	uint64_t syasyen;
	uint64_t ipcframesy;
	uint64_t stabasyen;
	uint64_t swmaster;
	uint64_t fullcbg;
	uint64_t srcport[8];
};

struct sja1105_retagging_entry {
	uint64_t egr_port;
	uint64_t ing_port;
	uint64_t vlan_ing;
	uint64_t vlan_egr;
	uint64_t do_not_learn;
	uint64_t use_dest_ports;
	uint64_t destports;
};

struct sja1105_static_config {
	struct sja1105_schedule_entry \
	       schedule[MAX_SCHEDULE_COUNT];
	struct sja1105_schedule_entry_points_entry \
	       schedule_entry_points[MAX_SCHEDULE_ENTRY_POINTS_COUNT];
	struct sja1105_vl_lookup_entry \
	       vl_lookup[MAX_VL_LOOKUP_COUNT];
	struct sja1105_vl_policing_entry \
	       vl_policing[MAX_VL_POLICING_COUNT];
	struct sja1105_vl_forwarding_entry \
	       vl_forwarding[MAX_VL_FORWARDING_COUNT];
	struct sja1105_l2_lookup_entry \
	       l2_lookup[MAX_L2_LOOKUP_COUNT];
	struct sja1105_l2_policing_entry \
	       l2_policing[MAX_L2_POLICING_COUNT];
	struct sja1105_vlan_lookup_entry \
	       vlan_lookup[MAX_VLAN_LOOKUP_COUNT];
	struct sja1105_l2_forwarding_entry \
	       l2_forwarding[MAX_L2_FORWARDING_COUNT];
	struct sja1105_mac_config_entry \
	       mac_config[MAX_MAC_CONFIG_COUNT];
	struct sja1105_schedule_params_entry \
	       schedule_params[MAX_SCHEDULE_PARAMS_COUNT];
	struct sja1105_schedule_entry_points_params \
	       schedule_entry_points_params[MAX_SCHEDULE_ENTRY_POINTS_PARAMS_COUNT];
	struct sja1105_vl_forwarding_params_table \
	       vl_forwarding_params[MAX_VL_FORWARDING_PARAMS_COUNT];
	struct sja1105_l2_lookup_params_table \
	       l2_lookup_params[MAX_L2_LOOKUP_PARAMS_COUNT];
	struct sja1105_l2_forwarding_params_table \
	       l2_forwarding_params[MAX_L2_FORWARDING_PARAMS_COUNT];
	struct sja1105_avb_params_table \
	       avb_params[MAX_AVB_PARAMS_COUNT];
	struct sja1105_clk_sync_params_table \
	       clk_sync_params[MAX_CLK_SYNC_COUNT];
	struct sja1105_general_params_table \
	       general_params[MAX_GENERAL_PARAMS_COUNT];
	struct sja1105_retagging_entry \
	       retagging[MAX_RETAGGING_COUNT];
	struct sja1105_xmii_params_table \
	       xmii_params[MAX_XMII_PARAMS_COUNT];
	int    schedule_count;
	int    schedule_entry_points_count;
	int    vl_lookup_count;
	int    vl_policing_count;
	int    vl_forwarding_count;
	int    l2_lookup_count;
	int    l2_policing_count;
	int    vlan_lookup_count;
	int    l2_forwarding_count;
	int    mac_config_count;
	int    schedule_params_count;
	int    schedule_entry_points_params_count;
	int    vl_forwarding_params_count;
	int    l2_lookup_params_count;
	int    l2_forwarding_params_count;
	int    avb_params_count;
	int    clk_sync_params_count;
	int    general_params_count;
	int    retagging_count;
	int    xmii_params_count;
};

#endif
