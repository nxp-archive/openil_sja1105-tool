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
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "internal.h"
/* From libsja1105 */
#include <common.h>

static void print_usage(const char *prog)
{
	printf("%s config modify <table-name>[<entry-index>] <field-name> <field-value>\n", prog);
	printf("Please run \"%s config modify help\" to see more details\n", prog);
}

static int generic_table_entry_modify(
		uint64_t *field_addr,
		int       entry_index,
		int       entry_count,
		int       entry_field_count,
		char     *field_val)
{
	int rc;

	if (entry_index < 0 || entry_index >= entry_count) {
		loge("Index out of bounds!");
		loge("Please adjust the entry count of the table:");
		loge("* config modify <table> entry-count <value>)");
		rc = -ERANGE;
		goto out;
	}
	if (entry_field_count == 1) {
		/* Entry is single element */
		rc = reliable_uint64_from_string(field_addr, field_val, NULL);
	} else {
		/* Entry is an array */
		rc = read_array(field_val, field_addr, entry_field_count);
	}
out:
	return rc;
}

static int schedule_table_entry_modify(
		struct sja1105_static_config *config,
		int    entry_index,
		char  *field_name,
		char  *field_val)
{
	const char *options[] = {
		"winstindex",
		"winend",
		"winst",
		"destports",
		"setvalid",
		"txen",
		"resmedia_en",
		"resmedia",
		"vlindex",
		"delta",
	};
	uint64_t *fields[] = {
		&config->schedule[entry_index].winstindex,
		&config->schedule[entry_index].winend,
		&config->schedule[entry_index].winst,
		&config->schedule[entry_index].destports,
		&config->schedule[entry_index].setvalid,
		&config->schedule[entry_index].txen,
		&config->schedule[entry_index].resmedia_en,
		&config->schedule[entry_index].resmedia,
		&config->schedule[entry_index].vlindex,
		&config->schedule[entry_index].delta,
	};
	int entry_field_counts[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->schedule_count = tmp;
		goto out;
	}
	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->schedule_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

static int schedule_entry_points_table_entry_modify(
		struct sja1105_static_config *config,
		int    entry_index,
		char  *field_name,
		char  *field_val)
{
	const char *options[] = {
		"subschindx",
		"delta",
		"address",
	};
	uint64_t *fields[] = {
		&config->schedule_entry_points[entry_index].subschindx,
		&config->schedule_entry_points[entry_index].delta,
		&config->schedule_entry_points[entry_index].address,
	};
	int entry_field_counts[] = {1, 1, 1,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->schedule_entry_points_count = tmp;
		goto out;
	}
	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->schedule_entry_points_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

static int l2_lookup_table_entry_modify(
		struct sja1105_static_config *config,
		int    entry_index,
		char  *field_name,
		char  *field_val)
{
	const char *options[] = {
		"tsreg",        /* P/Q/R/S only - LOCKEDS=1 */
		"mirrvlan",     /* P/Q/R/S only - LOCKEDS=1 */
		"takets",       /* P/Q/R/S only - LOCKEDS=1 */
		"mirr",         /* P/Q/R/S only - LOCKEDS=1 */
		"retag",        /* P/Q/R/S only - LOCKEDS=1 */
		"mask_iotag",   /* P/Q/R/S only */
		"mask_vlanid",  /* P/Q/R/S only */
		"mask_macaddr", /* P/Q/R/S only */
		"iotag",        /* P/Q/R/S only */
		"vlanid",
		"macaddr",
		"destports",
		"enfport",
		"index",
	};
	uint64_t *fields[] = {
		&config->l2_lookup[entry_index].tsreg,
		&config->l2_lookup[entry_index].mirrvlan,
		&config->l2_lookup[entry_index].takets,
		&config->l2_lookup[entry_index].mirr,
		&config->l2_lookup[entry_index].retag,
		&config->l2_lookup[entry_index].mask_iotag,
		&config->l2_lookup[entry_index].mask_vlanid,
		&config->l2_lookup[entry_index].mask_macaddr,
		&config->l2_lookup[entry_index].iotag,
		&config->l2_lookup[entry_index].vlanid,
		&config->l2_lookup[entry_index].macaddr,
		&config->l2_lookup[entry_index].destports,
		&config->l2_lookup[entry_index].enfport,
		&config->l2_lookup[entry_index].index,
	};
	int entry_field_counts[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->l2_lookup_count = tmp;
		goto out;
	}
	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->l2_lookup_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

static int l2_policing_table_entry_modify(
		struct sja1105_static_config *config,
		int    entry_index,
		char  *field_name,
		char  *field_val)
{
	const char *options[] = {
		"sharindx",
		"smax",
		"rate",
		"maxlen",
		"partition",
	};
	uint64_t *fields[] = {
		&config->l2_policing[entry_index].sharindx,
		&config->l2_policing[entry_index].smax,
		&config->l2_policing[entry_index].rate,
		&config->l2_policing[entry_index].maxlen,
		&config->l2_policing[entry_index].partition,
	};
	int entry_field_counts[] = {1, 1, 1, 1, 1,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->l2_policing_count = tmp;
		goto out;
	}

	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->l2_policing_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

static int vlan_lookup_table_entry_modify(
		struct sja1105_static_config *config,
		int    entry_index,
		char  *field_name,
		char  *field_val)
{
	const char *options[] = {
		"ving_mirr",
		"vegr_mirr",
		"vmemb_port",
		"vlan_bc",
		"tag_port",
		"vlanid",
	};
	uint64_t *fields[] = {
		&config->vlan_lookup[entry_index].ving_mirr,
		&config->vlan_lookup[entry_index].vegr_mirr,
		&config->vlan_lookup[entry_index].vmemb_port,
		&config->vlan_lookup[entry_index].vlan_bc,
		&config->vlan_lookup[entry_index].tag_port,
		&config->vlan_lookup[entry_index].vlanid,
	};
	int entry_field_counts[] = {1, 1, 1, 1, 1, 1,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->vlan_lookup_count = tmp;
		goto out;
	}

	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->vlan_lookup_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

static int l2_fw_table_entry_modify(
		struct sja1105_static_config *config,
		int    entry_index,
		char  *field_name,
		char  *field_val)
{
	const char *options[] = {
		"bc_domain",
		"reach_port",
		"fl_domain",
		"vlan_pmap",
	};
	uint64_t *fields[] = {
		&config->l2_forwarding[entry_index].bc_domain,
		&config->l2_forwarding[entry_index].reach_port,
		&config->l2_forwarding[entry_index].fl_domain,
		config->l2_forwarding[entry_index].vlan_pmap,
	};
	int entry_field_counts[] = {1, 1, 1, 8, 1,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->l2_forwarding_count = tmp;
		goto out;
	}

	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->l2_forwarding_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

static int mac_config_table_entry_modify(
		struct sja1105_static_config *config,
		int    entry_index,
		char  *field_name,
		char  *field_val)
{
	const char *options[] = {
		"top",
		"base",
		"enabled",
		"ifg",
		"tp_delin",
		"tp_delout",
		"maxage",
		"vlanprio",
		"vlanid",
		"ing_mirr",
		"egr_mirr",
		"drpnona664",
		"drpdtag",
		"drpsotag", /* P/Q/R/S only */
		"drpsitag", /* P/Q/R/S only */
		"drpuntag",
		"retag",
		"dyn_learn",
		"egress",
		"ingress",
		"mirrcie", /* P/Q/R/S only */
		"mirrcetag", /* P/Q/R/S only */
		"ingmirrvid", /* P/Q/R/S only */
		"ingmirrpcp", /* P/Q/R/S only */
		"ingmirrdei", /* P/Q/R/S only */
	};
	uint64_t *fields[] = {
		config->mac_config[entry_index].top,
		config->mac_config[entry_index].base,
		config->mac_config[entry_index].enabled,
		&config->mac_config[entry_index].ifg,
		&config->mac_config[entry_index].tp_delin,
		&config->mac_config[entry_index].tp_delout,
		&config->mac_config[entry_index].maxage,
		&config->mac_config[entry_index].vlanprio,
		&config->mac_config[entry_index].vlanid,
		&config->mac_config[entry_index].ing_mirr,
		&config->mac_config[entry_index].egr_mirr,
		&config->mac_config[entry_index].drpnona664,
		&config->mac_config[entry_index].drpdtag,
		&config->mac_config[entry_index].drpsotag,
		&config->mac_config[entry_index].drpsitag,
		&config->mac_config[entry_index].drpuntag,
		&config->mac_config[entry_index].retag,
		&config->mac_config[entry_index].dyn_learn,
		&config->mac_config[entry_index].egress,
		&config->mac_config[entry_index].ingress,
		&config->mac_config[entry_index].mirrcie,
		&config->mac_config[entry_index].mirrcetag,
		&config->mac_config[entry_index].ingmirrvid,
		&config->mac_config[entry_index].ingmirrpcp,
		&config->mac_config[entry_index].ingmirrdei,
	};
	int entry_field_counts[] = {8, 8, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->mac_config_count = tmp;
		goto out;
	}
	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->mac_config_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

static int schedule_params_table_entry_modify(
		struct sja1105_static_config *config,
		int    entry_index,
		char  *field_name,
		char  *field_val)
{
	const char *options[] = {
		"subscheind",
	};
	uint64_t *fields[] = {
		config->schedule_params[entry_index].subscheind,
	};
	int entry_field_counts[] = {8,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->schedule_params_count = tmp;
		goto out;
	}
	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->schedule_params_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

static int schedule_entry_points_params_table_entry_modify(
		struct sja1105_static_config *config,
		int    entry_index,
		char  *field_name,
		char  *field_val)
{
	const char *options[] = {
		"clksrc",
		"actsubsch",
	};
	uint64_t *fields[] = {
		&config->schedule_entry_points_params[entry_index].clksrc,
		&config->schedule_entry_points_params[entry_index].actsubsch,
	};
	int entry_field_counts[] = {1, 1,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->schedule_entry_points_params_count = tmp;
		goto out;
	}
	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->schedule_entry_points_params_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

static int l2_lookup_params_table_entry_modify(
		struct sja1105_static_config *config,
		int    entry_index,
		char  *field_name,
		char  *field_val)
{
	const char *options[] = {
		"drpbc", /* P/Q/R/S only */
		"drpmc", /* P/Q/R/S only */
		"drpuni", /* P/Q/R/S only */
		"maxaddrp", /* P/Q/R/S only */
		"maxage",
		"start_dynspc", /* P/Q/R/S only */
		"drpnolearn", /* P/Q/R/S only */
		"dyn_tbsz",
		"poly",
		"shared_learn",
		"no_enf_hostprt",
		"no_mgmt_learn",
		"use_static", /* P/Q/R/S only */
		"owr_dyn", /* P/Q/R/S only */
		"learn_once", /* P/Q/R/S only */
	};
	uint64_t *fields[] = {
		&config->l2_lookup_params[entry_index].drpbc,
		&config->l2_lookup_params[entry_index].drpmc,
		&config->l2_lookup_params[entry_index].drpuni,
		config->l2_lookup_params[entry_index].maxaddrp,
		&config->l2_lookup_params[entry_index].maxage,
		&config->l2_lookup_params[entry_index].start_dynspc,
		&config->l2_lookup_params[entry_index].drpnolearn,
		&config->l2_lookup_params[entry_index].dyn_tbsz,
		&config->l2_lookup_params[entry_index].poly,
		&config->l2_lookup_params[entry_index].shared_learn,
		&config->l2_lookup_params[entry_index].no_enf_hostprt,
		&config->l2_lookup_params[entry_index].no_mgmt_learn,
		&config->l2_lookup_params[entry_index].use_static,
		&config->l2_lookup_params[entry_index].owr_dyn,
		&config->l2_lookup_params[entry_index].learn_once,
	};
	int entry_field_counts[] = {1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->l2_lookup_params_count = tmp;
		goto out;
	}
	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->l2_lookup_params_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

static int l2_fw_params_table_entry_modify(
		struct sja1105_static_config *config,
		int    entry_index,
		char  *field_name,
		char  *field_val)
{
	const char *options[] = {
		"max_dynp",
		"part_spc",
	};
	uint64_t *fields[] = {
		&config->l2_forwarding_params[entry_index].max_dynp,
		config->l2_forwarding_params[entry_index].part_spc,
	};
	int entry_field_counts[] = {1, 8,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->l2_forwarding_params_count = tmp;
		goto out;
	}
	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->l2_forwarding_params_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

static int general_params_table_entry_modify(
		struct sja1105_static_config *config,
		int    entry_index,
		char  *field_name,
		char  *field_val)
{
	const char *options[] = {
		"vllupformat",
		"mirr_ptacu",
		"switchid",
		"hostprio",
		"mac_fltres1",
		"mac_fltres0",
		"mac_flt1",
		"mac_flt0",
		"incl_srcpt1",
		"incl_srcpt0",
		"send_meta1",
		"send_meta0",
		"casc_port",
		"host_port",
		"mirr_port",
		"vlmarker",
		"vlmask",
		"tpid",
		"ignore2stf",
		"tpid2",
		"queue_ts", /* P/Q/R/S only */
		"egrmirrvid", /* P/Q/R/S only */
		"egrmirrpcp", /* P/Q/R/S only */
		"egrmirrdei", /* P/Q/R/S only */
		"replay_port", /* P/Q/R/S only */
	};
	uint64_t *fields[] = {
		&config->general_params[entry_index].vllupformat,
		&config->general_params[entry_index].mirr_ptacu,
		&config->general_params[entry_index].switchid,
		&config->general_params[entry_index].hostprio,
		&config->general_params[entry_index].mac_fltres1,
		&config->general_params[entry_index].mac_fltres0,
		&config->general_params[entry_index].mac_flt1,
		&config->general_params[entry_index].mac_flt0,
		&config->general_params[entry_index].incl_srcpt1,
		&config->general_params[entry_index].incl_srcpt0,
		&config->general_params[entry_index].send_meta1,
		&config->general_params[entry_index].send_meta0,
		&config->general_params[entry_index].casc_port,
		&config->general_params[entry_index].host_port,
		&config->general_params[entry_index].mirr_port,
		&config->general_params[entry_index].vlmarker,
		&config->general_params[entry_index].vlmask,
		&config->general_params[entry_index].tpid,
		&config->general_params[entry_index].ignore2stf,
		&config->general_params[entry_index].tpid2,
		&config->general_params[entry_index].queue_ts,
		&config->general_params[entry_index].egrmirrvid,
		&config->general_params[entry_index].egrmirrpcp,
		&config->general_params[entry_index].egrmirrdei,
		&config->general_params[entry_index].replay_port,
	};
	int entry_field_counts[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->general_params_count = tmp;
		goto out;
	}
	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->general_params_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

static int vl_lookup_table_entry_modify(
		struct sja1105_static_config *config,
		int    entry_index,
		char  *field_name,
		char  *field_val)
{
	const char *options[] = {
		"port",
		"destports",
		"iscritical",
		"macaddr",
		"vlanid",
		"vlanprior",
		"egrmirr",
		"ingrmirr",
		"vlid",
	};
	uint64_t *fields[] = {
		&config->vl_lookup[entry_index].port,
		&config->vl_lookup[entry_index].destports,
		&config->vl_lookup[entry_index].iscritical,
		&config->vl_lookup[entry_index].macaddr,
		&config->vl_lookup[entry_index].vlanid,
		&config->vl_lookup[entry_index].vlanprior,
		&config->vl_lookup[entry_index].egrmirr,
		&config->vl_lookup[entry_index].ingrmirr,
		&config->vl_lookup[entry_index].vlid,
	};
	int entry_field_counts[] = {1, 1, 1, 1, 1, 1, 1, 1, 1,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->vl_lookup_count = tmp;
		goto out;
	}
	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->vl_lookup_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

static int vl_policing_table_entry_modify(
		struct sja1105_static_config *config,
		int    entry_index,
		char  *field_name,
		char  *field_val)
{
	const char *options[] = {
		"type",
		"maxlen",
		"sharindx",
		"bag",
		"jitter",
	};
	uint64_t *fields[] = {
		&config->vl_policing[entry_index].type,
		&config->vl_policing[entry_index].maxlen,
		&config->vl_policing[entry_index].sharindx,
		&config->vl_policing[entry_index].bag,
		&config->vl_policing[entry_index].jitter,
	};
	int entry_field_counts[] = {1, 1, 1, 1, 1,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->vl_policing_count = tmp;
		goto out;
	}
	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->vl_policing_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

static int vl_fw_table_entry_modify(
		struct sja1105_static_config *config,
		int    entry_index,
		char  *field_name,
		char  *field_val)
{
	const char *options[] = {
		"type",
		"priority",
		"partition",
		"destports",
	};
	uint64_t *fields[] = {
		&config->vl_forwarding[entry_index].type,
		&config->vl_forwarding[entry_index].priority,
		&config->vl_forwarding[entry_index].partition,
		&config->vl_forwarding[entry_index].destports,
	};
	int entry_field_counts[] = {1, 1, 1, 1,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->vl_forwarding_count = tmp;
		goto out;
	}
	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->vl_forwarding_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

static int retagging_table_entry_modify(
		__attribute__((unused)) struct sja1105_static_config *config,
		__attribute__((unused)) int entry,
		__attribute__((unused)) char *field_name,
		__attribute__((unused)) char *field_val)
{
	loge("unimplemented");
	return -1;
}

static int avb_params_table_entry_modify(
		struct sja1105_static_config *config,
		int    entry_index,
		char  *field_name,
		char  *field_val)
{
	const char *options[] = {
		"l2cbs", /* P/Q/R/S only */
		"cas_master", /* P/Q/R/S only */
		"destmeta",
		"srcmeta",
	};
	uint64_t *fields[] = {
		&config->avb_params[entry_index].l2cbs,
		&config->avb_params[entry_index].cas_master,
		&config->avb_params[entry_index].destmeta,
		&config->avb_params[entry_index].srcmeta,
	};
	int entry_field_counts[] = {1, 1, 1, 1,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->avb_params_count = tmp;
		goto out;
	}
	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->avb_params_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

static int clock_sync_params_table_entry_modify(
		__attribute__((unused)) struct sja1105_static_config *config,
		__attribute__((unused)) int entry,
		__attribute__((unused)) char *field_name,
		__attribute__((unused)) char *field_val)
{
	loge("unimplemented");
	return -1;
}

static int
vl_fw_params_table_entry_modify(struct sja1105_static_config *config,
                                int    entry_index,
                                char  *field_name,
                                char  *field_val)
{
	const char *options[] = {
		"partspc",
		"debugen",
	};
	uint64_t *fields[] = {
		config->vl_forwarding_params[entry_index].partspc,
		&config->vl_forwarding_params[entry_index].debugen,
	};
	int entry_field_counts[] = {8, 1,};
	uint64_t tmp;
	int rc;

	if (matches(field_name, "entry-count") == 0) {
		rc = reliable_uint64_from_string(&tmp, field_val, NULL);
		config->vl_forwarding_params_count = tmp;
		goto out;
	}
	rc = get_match(field_name, options, ARRAY_SIZE(options));
	if (rc < 0) {
		goto out;
	}
	rc = generic_table_entry_modify(
			fields[rc],
			entry_index,
			config->vl_forwarding_params_count,
			entry_field_counts[rc],
			field_val);
out:
	return rc;
}

int
staging_area_modify(struct sja1105_staging_area *staging_area,
                    char *table_name,
                    char *field_name,
                    char *field_val)
{
	const char *static_config_options[] = {
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
	int (*next_static_table_modify[])(struct sja1105_static_config*, \
	                                  int, char*, char*) = {
		schedule_table_entry_modify,
		schedule_entry_points_table_entry_modify,
		vl_lookup_table_entry_modify,
		vl_policing_table_entry_modify,
		vl_fw_table_entry_modify,
		l2_lookup_table_entry_modify,
		l2_policing_table_entry_modify,
		vlan_lookup_table_entry_modify,
		l2_fw_table_entry_modify,
		mac_config_table_entry_modify,
		schedule_params_table_entry_modify,
		schedule_entry_points_params_table_entry_modify,
		vl_fw_params_table_entry_modify,
		l2_lookup_params_table_entry_modify,
		l2_fw_params_table_entry_modify,
		clock_sync_params_table_entry_modify,
		avb_params_table_entry_modify,
		general_params_table_entry_modify,
		retagging_table_entry_modify,
	};
	struct   sja1105_static_config *static_config;
	uint64_t entry_index;
	char    *index_ptr;
	int      rc;

	static_config = &staging_area->static_config;

	index_ptr = strchr(table_name, '[');
	if (index_ptr == NULL) {
		entry_index = 0;
	} else {
		/* Little trick to reuse the code, since the index
		 * is surrounded by [ ], same as an array would be */
		rc = read_array(index_ptr, &entry_index, 1);
		if (rc < 0) {
			goto out;
		}
		/* So we only match on the table field_name, but not on the entry index */
		*index_ptr = '\0';
	}
	rc = get_match(table_name, static_config_options,
	               ARRAY_SIZE(static_config_options));
	if (rc < 0) {
		goto out;
	}
	logv("Table %s, entry %" PRIu64", field %s, value %s",
	     static_config_options[rc], entry_index, field_name, field_val);
	if (field_name == NULL) {
		rc = -EINVAL;
		print_usage("sja1105-tool");
		goto out;
	}
	if (field_val == NULL) {
		printf("Please supply a value for field %s!\n", field_name);
		goto out;
	}
	rc = next_static_table_modify[rc](static_config, entry_index,
	                                  field_name, field_val);
	if (rc < 0) {
		loge("modify failed!");
		goto out;
	}
out:
	return rc;
}

int staging_area_modify_parse(struct sja1105_staging_area *staging_area,
                              int *argc, char ***argv)
{
	char *table_name;
	char *field_name;
	char *field_val;
	int rc = 0;

	if ((*argc) == 0) {
		print_usage("sja1105-tool");
		rc = -EINVAL;
		goto out;
	}
	table_name = (*argv)[0];
	(*argc)--; (*argv)++;
	if ((*argc) != 2) {
		/* Instead of printing usage again, try to be more helpful.
		 * At least the guy gave us the table name. Tell him what
		 * fields are available.
		 * Do that by simulating a call to staging_area_modify() with
		 * some dummy arguments (just enough for it to provide a
		 * clearer usage message
		 */
		rc = staging_area_modify(NULL, table_name, "<none>", "<none>");
		goto out;
	}
	field_name = (*argv)[0];
	field_val  = (*argv)[1];
	(*argc) -= 2; (*argv) += 2;

	rc = staging_area_modify(staging_area, table_name,
	                         field_name, field_val);
out:
	return rc;
}

