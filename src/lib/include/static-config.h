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
#ifndef _TABLES_EXTERNAL_H
#define _TABLES_EXTERNAL_H

#include <common.h>

#define CONFIG_ADDR 0x20000

#define SIZE_TABLE_HEADER                       12
#define SIZE_SCHEDULE_ENTRY                     8
#define SIZE_SCHEDULE_ENTRY_POINTS_ENTRY        4
#define SIZE_VL_LOOKUP_ENTRY                    12
#define SIZE_VL_POLICING_ENTRY                  8
#define SIZE_VL_FORWARDING_ENTRY                4
#define SIZE_L2_LOOKUP_ENTRY_ET                 12
#define SIZE_L2_LOOKUP_ENTRY_PQRS               20
#define SIZE_L2_POLICING_ENTRY                  8
#define SIZE_VLAN_LOOKUP_ENTRY                  8
#define SIZE_L2_FORWARDING_ENTRY                8
#define SIZE_MAC_CONFIG_ENTRY_ET                28
#define SIZE_MAC_CONFIG_ENTRY_PQRS              32
#define SIZE_SCHEDULE_PARAMS_ENTRY              12
#define SIZE_SCHEDULE_ENTRY_POINTS_PARAMS_ENTRY 4
#define SIZE_VL_FORWARDING_PARAMS_ENTRY         12
#define SIZE_L2_LOOKUP_PARAMS_ENTRY_ET          4
#define SIZE_L2_LOOKUP_PARAMS_ENTRY_PQRS        16
#define SIZE_L2_FORWARDING_PARAMS_ENTRY         12
#define SIZE_CLK_SYNC_PARAMS_ENTRY              52
#define SIZE_AVB_PARAMS_ENTRY_ET                12
#define SIZE_AVB_PARAMS_ENTRY_PQRS              16
#define SIZE_GENERAL_PARAMS_ENTRY_ET            40
#define SIZE_GENERAL_PARAMS_ENTRY_PQRS          44
#define SIZE_RETAGGING_ENTRY                    8
#define SIZE_XMII_MODE_PARAMS_ENTRY             4
#define SIZE_SGMII_ENTRY                        144

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
#define BLKID_SGMII_TABLE                        0xC8

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
#define MAX_SGMII_COUNT                          1
#define MAX_AVB_PARAMS_COUNT                     1
#define MAX_CLK_SYNC_COUNT                       1

#define MAX_FRAME_MEMORY                         929
#define MAX_FRAME_MEMORY_RETAGGING               910

#define SJA1105E_DEVICE_ID         0x9C00000Cull
#define SJA1105T_DEVICE_ID         0x9E00030Eull
#define SJA1105PR_DEVICE_ID        0xAF00030Eull
#define SJA1105QS_DEVICE_ID        0xAE00030Eull
#define SJA1105_NO_DEVICE_ID       0x00000000ull

#define SJA1105P_PART_NR           0x9A84
#define SJA1105Q_PART_NR           0x9A85
#define SJA1105R_PART_NR           0x9A86
#define SJA1105S_PART_NR           0x9A87
#define SJA1105_PART_NR_DONT_CARE  0xFFFF

#define IS_PQRS(device_id) \
	(((device_id) == SJA1105PR_DEVICE_ID) || \
	 ((device_id) == SJA1105QS_DEVICE_ID))
#define IS_ET(device_id) \
	(((device_id) == SJA1105E_DEVICE_ID) || \
	 ((device_id) == SJA1105T_DEVICE_ID))
/* P and R have same Device ID, and differ by Part Number */
#define IS_P(device_id, part_nr) \
	(((device_id) == SJA1105PR_DEVICE_ID) && \
	 ((part_nr) == SJA1105P_PART_NR))
#define IS_R(device_id, part_nr) \
	(((device_id) == SJA1105PR_DEVICE_ID) && \
	 ((part_nr) == SJA1105R_PART_NR))
/* Same do Q and S */
#define IS_Q(device_id, part_nr) \
	(((device_id) == SJA1105QS_DEVICE_ID) && \
	 ((part_nr) == SJA1105Q_PART_NR))
#define IS_S(device_id, part_nr) \
	(((device_id) == SJA1105QS_DEVICE_ID) && \
	 ((part_nr) == SJA1105S_PART_NR))
#define DEVICE_ID_VALID(device_id) \
	(IS_ET(device_id) || IS_PQRS(device_id))
#define SUPPORTS_TSN(device_id) \
	(((device_id) == SJA1105T_DEVICE_ID) || \
	 ((device_id) == SJA1105QS_DEVICE_ID))

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

struct sja1105_general_params_entry {
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
	/* P/Q/R/S only */
	uint64_t queue_ts;
	uint64_t egrmirrvid;
	uint64_t egrmirrpcp;
	uint64_t egrmirrdei;
	uint64_t replay_port;
};

struct sja1105_schedule_entry_points_entry {
	uint64_t subschindx;
	uint64_t delta;
	uint64_t address;
};

struct sja1105_schedule_entry_points_params_entry {
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
	uint64_t tsreg;         /* P/Q/R/S only - LOCKEDS=1 */
	uint64_t mirrvlan;      /* P/Q/R/S only - LOCKEDS=1 */
	uint64_t takets;        /* P/Q/R/S only - LOCKEDS=1 */
	uint64_t mirr;          /* P/Q/R/S only - LOCKEDS=1 */
	uint64_t retag;         /* P/Q/R/S only - LOCKEDS=1 */
	uint64_t mask_iotag;    /* P/Q/R/S only */
	uint64_t mask_vlanid;   /* P/Q/R/S only */
	uint64_t mask_macaddr;  /* P/Q/R/S only */
	uint64_t iotag;         /* P/Q/R/S only */
	uint64_t vlanid;
	uint64_t macaddr;
	uint64_t destports;
	uint64_t enfport;
	uint64_t index;
};

struct sja1105_l2_lookup_params_entry {
	uint64_t drpbc;           /* P/Q/R/S only */
	uint64_t drpmc;           /* P/Q/R/S only */
	uint64_t drpuni;          /* P/Q/R/S only */
	uint64_t maxaddrp[5];     /* P/Q/R/S only */
	uint64_t start_dynspc;    /* P/Q/R/S only */
	uint64_t drpnolearn;      /* P/Q/R/S only */
	uint64_t use_static;      /* P/Q/R/S only */
	uint64_t owr_dyn;         /* P/Q/R/S only */
	uint64_t learn_once;      /* P/Q/R/S only */
	uint64_t maxage;          /* Shared */
	uint64_t dyn_tbsz;        /* E/T only */
	uint64_t poly;            /* E/T only */
	uint64_t shared_learn;    /* Shared */
	uint64_t no_enf_hostprt;  /* Shared */
	uint64_t no_mgmt_learn;   /* Shared */
};

struct sja1105_l2_forwarding_entry {
	uint64_t bc_domain;
	uint64_t reach_port;
	uint64_t fl_domain;
	uint64_t vlan_pmap[8];
};

struct sja1105_l2_forwarding_params_entry {
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
	uint64_t drpsotag;   /* only on P/Q/R/S */
	uint64_t drpsitag;   /* only on P/Q/R/S */
	uint64_t drpuntag;
	uint64_t retag;
	uint64_t dyn_learn;
	uint64_t egress;
	uint64_t ingress;
	uint64_t mirrcie;    /* only on P/Q/R/S */
	uint64_t mirrcetag;  /* only on P/Q/R/S */
	uint64_t ingmirrvid; /* only on P/Q/R/S */
	uint64_t ingmirrpcp; /* only on P/Q/R/S */
	uint64_t ingmirrdei; /* only on P/Q/R/S */
};

struct sja1105_xmii_params_entry {
	uint64_t phy_mac[5];
	uint64_t xmii_mode[5];
};

struct sja1105_avb_params_entry {
	uint64_t l2cbs; /* only on P/Q/R/S */
	uint64_t cas_master; /* only on P/Q/R/S */
	uint64_t destmeta;
	uint64_t srcmeta;
};

struct sja1105_sgmii_entry {
	uint64_t digital_error_cnt;
	uint64_t digital_control_2;
	uint64_t debug_control;
	uint64_t test_control;
	uint64_t autoneg_control;
	uint64_t digital_control_1;
	uint64_t autoneg_adv;
	uint64_t basic_control;
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

struct sja1105_vl_forwarding_params_entry {
	uint64_t partspc[8];
	uint64_t debugen;
};

struct sja1105_clk_sync_params_entry {
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

#define STATIC_CONFIG_MEMBER(table, size)           \
	struct sja1105_##table##_entry table[size]; \
	int table##_count;                          \

struct sja1105_static_config {
	uint64_t device_id;
	STATIC_CONFIG_MEMBER(l2_forwarding_params, MAX_L2_FORWARDING_PARAMS_COUNT);
	STATIC_CONFIG_MEMBER(l2_forwarding, MAX_L2_FORWARDING_COUNT);
	STATIC_CONFIG_MEMBER(l2_lookup, MAX_L2_LOOKUP_COUNT);
	STATIC_CONFIG_MEMBER(l2_lookup_params, MAX_L2_LOOKUP_PARAMS_COUNT);
	STATIC_CONFIG_MEMBER(l2_policing, MAX_L2_POLICING_COUNT);
	STATIC_CONFIG_MEMBER(mac_config, MAX_MAC_CONFIG_COUNT);
	STATIC_CONFIG_MEMBER(schedule_entry_points_params, MAX_SCHEDULE_ENTRY_POINTS_PARAMS_COUNT);
	STATIC_CONFIG_MEMBER(schedule_entry_points, MAX_SCHEDULE_ENTRY_POINTS_COUNT);
	STATIC_CONFIG_MEMBER(schedule_params, MAX_SCHEDULE_PARAMS_COUNT);
	STATIC_CONFIG_MEMBER(schedule, MAX_SCHEDULE_COUNT);
	STATIC_CONFIG_MEMBER(vlan_lookup, MAX_VLAN_LOOKUP_COUNT);
	STATIC_CONFIG_MEMBER(xmii_params, MAX_XMII_PARAMS_COUNT);
	STATIC_CONFIG_MEMBER(general_params, MAX_GENERAL_PARAMS_COUNT);
	STATIC_CONFIG_MEMBER(avb_params, MAX_AVB_PARAMS_COUNT);
	STATIC_CONFIG_MEMBER(vl_forwarding_params, MAX_VL_FORWARDING_PARAMS_COUNT);
	STATIC_CONFIG_MEMBER(vl_forwarding, MAX_VL_FORWARDING_COUNT);
	STATIC_CONFIG_MEMBER(vl_policing, MAX_VL_POLICING_COUNT);
	STATIC_CONFIG_MEMBER(vl_lookup, MAX_VL_LOOKUP_COUNT);
	STATIC_CONFIG_MEMBER(retagging, MAX_RETAGGING_COUNT);
	STATIC_CONFIG_MEMBER(sgmii, MAX_SGMII_COUNT);
};

#define DEFINE_HEADERS_FOR_CONFIG_TABLE(device, table)                                         \
	void sja1105_##table##_entry_fmt_show(char*, size_t, char*, struct sja1105_##table##_entry*);  \
	void sja1105##device##_##table##_entry_pack(void*, struct sja1105_##table##_entry*);   \
	void sja1105##device##_##table##_entry_unpack(void*, struct sja1105_##table##_entry*); \

#define DEFINE_COMMON_HEADERS_FOR_CONFIG_TABLE(table)                                          \
	DEFINE_HEADERS_FOR_CONFIG_TABLE(, table)                                               \

#define DEFINE_SEPARATE_HEADERS_FOR_CONFIG_TABLE(table)                                        \
	DEFINE_HEADERS_FOR_CONFIG_TABLE(et, table)                                             \
	DEFINE_HEADERS_FOR_CONFIG_TABLE(pqrs, table)                                           \

DEFINE_COMMON_HEADERS_FOR_CONFIG_TABLE(l2_forwarding_params);
DEFINE_COMMON_HEADERS_FOR_CONFIG_TABLE(l2_forwarding);
DEFINE_COMMON_HEADERS_FOR_CONFIG_TABLE(l2_policing);
DEFINE_COMMON_HEADERS_FOR_CONFIG_TABLE(mac_config);
DEFINE_COMMON_HEADERS_FOR_CONFIG_TABLE(schedule_entry_points_params);
DEFINE_COMMON_HEADERS_FOR_CONFIG_TABLE(schedule_entry_points);
DEFINE_COMMON_HEADERS_FOR_CONFIG_TABLE(schedule_params);
DEFINE_COMMON_HEADERS_FOR_CONFIG_TABLE(schedule);
DEFINE_COMMON_HEADERS_FOR_CONFIG_TABLE(vlan_lookup);
DEFINE_COMMON_HEADERS_FOR_CONFIG_TABLE(xmii_params);
DEFINE_COMMON_HEADERS_FOR_CONFIG_TABLE(sgmii);
DEFINE_COMMON_HEADERS_FOR_CONFIG_TABLE(vl_forwarding_params);
DEFINE_COMMON_HEADERS_FOR_CONFIG_TABLE(vl_forwarding);
DEFINE_COMMON_HEADERS_FOR_CONFIG_TABLE(vl_policing);
DEFINE_COMMON_HEADERS_FOR_CONFIG_TABLE(vl_lookup);
DEFINE_SEPARATE_HEADERS_FOR_CONFIG_TABLE(avb_params);
DEFINE_SEPARATE_HEADERS_FOR_CONFIG_TABLE(general_params);
DEFINE_SEPARATE_HEADERS_FOR_CONFIG_TABLE(mac_config);
DEFINE_SEPARATE_HEADERS_FOR_CONFIG_TABLE(l2_lookup);
DEFINE_SEPARATE_HEADERS_FOR_CONFIG_TABLE(l2_lookup_params);

/* These can't be summarized using the DEFINE_HEADERS_FOR_CONFIG_TABLE macro */
void sja1105_table_header_pack(void*, struct sja1105_table_header*);
void sja1105_table_header_unpack(void*, struct sja1105_table_header*);
void sja1105_table_header_pack_with_crc(void*, struct sja1105_table_header *hdr);
void sja1105_table_header_fmt_show(char *print_buf,size_t len,
                                   struct sja1105_table_header *hdr);

/* From static-config.c */
unsigned int sja1105_static_config_get_length(struct sja1105_static_config*);
int  sja1105_static_config_add_entry(struct sja1105_table_header*, void *,
                                     struct sja1105_static_config*);
int  sja1105_static_config_check_valid(struct sja1105_static_config*);
int  sja1105_static_config_pack(void*, struct sja1105_static_config*);
int  sja1105_static_config_unpack(void*, ssize_t, struct sja1105_static_config*);

const char *sja1105_device_id_string_get(uint64_t device_id, uint64_t part_nr);

void sja1105_lib_get_build_date(char *buf);
void sja1105_lib_get_version(char *buf);

#endif
