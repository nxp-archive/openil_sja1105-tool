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
#include <common.h>
#include <lib/helpers.h>
#include <tool/internal.h>


void sja1105_xmii_params_entry_show(struct sja1105_xmii_params_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_xmii_params_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_vlan_lookup_entry_show(struct sja1105_vlan_lookup_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_vlan_lookup_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_vl_policing_entry_show(struct sja1105_vl_policing_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_vl_policing_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_vl_lookup_entry_show(struct sja1105_vl_lookup_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_vl_lookup_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_vl_forwarding_entry_show(struct sja1105_vl_forwarding_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_vl_forwarding_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_vl_forwarding_params_entry_show(struct sja1105_vl_forwarding_params_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_vl_forwarding_params_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_table_header_show(struct sja1105_table_header *hdr)
{
	char print_buf[MAX_LINE_SIZE];

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_table_header_fmt_show(print_buf, MAX_LINE_SIZE, hdr);
	puts(print_buf);
}

void sja1105_sgmii_entry_show(struct sja1105_sgmii_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_sgmii_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_schedule_entry_show(struct sja1105_schedule_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_schedule_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_schedule_params_entry_show(struct sja1105_schedule_params_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_schedule_params_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_schedule_entry_points_entry_show(struct sja1105_schedule_entry_points_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_schedule_entry_points_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_schedule_entry_points_params_entry_show(struct sja1105_schedule_entry_points_params_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_schedule_entry_points_params_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_mac_config_entry_show(struct sja1105_mac_config_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_mac_config_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_l2_policing_entry_show(struct sja1105_l2_policing_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_l2_policing_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_l2_lookup_entry_show(struct sja1105_l2_lookup_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_l2_lookup_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_l2_lookup_params_entry_show(struct sja1105_l2_lookup_params_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_l2_lookup_params_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_l2_forwarding_entry_show(struct sja1105_l2_forwarding_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_l2_forwarding_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_l2_forwarding_params_entry_show(struct sja1105_l2_forwarding_params_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_l2_forwarding_params_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_general_params_entry_show(struct sja1105_general_params_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_general_params_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_avb_params_entry_show(struct sja1105_avb_params_entry *entry)
{
	char print_buf[MAX_LINE_SIZE];
	char *fmt = "%s\n";

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_avb_params_entry_fmt_show(print_buf, MAX_LINE_SIZE, fmt, entry);
	puts(print_buf);
}

void sja1105_mgmt_entry_show(struct sja1105_mgmt_entry *entry)
{
	printf("TS_REGID  %" PRIX64 "\n", entry->ts_regid);
	printf("EGR_TS    %" PRIX64 "\n", entry->egr_ts);
	printf("MACADDR   %" PRIX64 "\n", entry->macaddr);
	printf("DESTPORTS %" PRIX64 "\n", entry->destports);
	printf("ENFPORT   %" PRIX64 "\n", entry->enfport);
	printf("INDEX     %" PRIX64 "\n", entry->index);
}

void sja1105_dyn_l2_lookup_cmd_show(struct sja1105_dyn_l2_lookup_cmd *cmd)
{
	printf("VALID     %" PRIX64 "\n", cmd->valid);
	printf("RDWRSET   %" PRIX64 "\n", cmd->rdwrset);
	printf("ERRORS    %" PRIX64 "\n", cmd->errors);
	printf("LOCKEDS   %" PRIX64 "\n", cmd->lockeds);
	printf("VALIDENT  %" PRIX64 "\n", cmd->valident);
	printf("MGMTROUTE %" PRIX64 "\n", cmd->mgmtroute);
	if (cmd->mgmtroute) {
		sja1105_mgmt_entry_show(&cmd->entry.mgmt);
	} else {
		sja1105_l2_lookup_entry_show(&cmd->entry.l2);
	}
}

void sja1105_cbs_show(struct sja1105_cbs *cbs)
{
	printf("SHAPER_ID  %" PRIX64 "\n", cbs->index);
	printf("CBS_PORT   %" PRIX64 "\n", cbs->port);
	printf("CBS_PRIO   %" PRIX64 "\n", cbs->prio);
	printf("CREDIT_LO  %" PRIX64 "\n", cbs->credit_lo);
	printf("CREDIT_HI  %" PRIX64 "\n", cbs->credit_hi);
	printf("SEND_SLOPE %" PRIX64 "\n", cbs->send_slope);
	printf("IDLE_SLOPE %" PRIX64 "\n", cbs->idle_slope);
}

void sja1105_cgu_pll_control_show(struct sja1105_cgu_pll_control *pll_control,
                                  uint64_t device_id)
{
	printf("PLLCLKSEL %" PRIX64 "\n", pll_control->pllclksrc);
	printf("MSEL      %" PRIX64 "\n", pll_control->msel);
	if (IS_PQRS(device_id)) {
		printf("NSEL      %" PRIX64 "\n", pll_control->nsel);
	}
	printf("AUTOBLOCK %" PRIX64 "\n", pll_control->autoblock);
	printf("PSEL      %" PRIX64 "\n", pll_control->psel);
	printf("DIRECT    %" PRIX64 "\n", pll_control->direct);
	printf("FBSEL     %" PRIX64 "\n", pll_control->fbsel);
	printf("BYPASS    %" PRIX64 "\n", pll_control->bypass);
	printf("PD        %" PRIX64 "\n", pll_control->pd);
}

void sja1105_cgu_mii_control_show(struct sja1105_cgu_mii_control *mii_control)
{
	printf("CLKSEL    %" PRIX64 "\n", mii_control->clksrc);
	printf("AUTOBLOCK %" PRIX64 "\n", mii_control->autoblock);
	printf("PD        %" PRIX64 "\n", mii_control->pd);
}

void sja1105_cgu_idiv_show(struct sja1105_cgu_idiv *idiv)
{
	printf("CLKSRC    %" PRIX64 "\n", idiv->clksrc);
	printf("AUTOBLOCK %" PRIX64 "\n", idiv->autoblock);
	printf("IDIV      %" PRIX64 "\n", idiv->idiv);
	printf("PD        %" PRIX64 "\n", idiv->pd);
}

void sja1105_cfg_pad_mii_tx_show(struct sja1105_cfg_pad_mii_tx *pad_mii_tx)
{
	printf("D32_OS    %" PRIX64 "\n", pad_mii_tx->d32_os);
	printf("D32_IPUD  %" PRIX64 "\n", pad_mii_tx->d32_ipud);
	printf("D10_OS    %" PRIX64 "\n", pad_mii_tx->d10_os);
	printf("D10_IPUD  %" PRIX64 "\n", pad_mii_tx->d10_ipud);
	printf("CTRL_OS   %" PRIX64 "\n", pad_mii_tx->ctrl_os);
	printf("CTRL_IPUD %" PRIX64 "\n", pad_mii_tx->ctrl_ipud);
	printf("CLK_OS    %" PRIX64 "\n", pad_mii_tx->clk_os);
	printf("CLK_IH    %" PRIX64 "\n", pad_mii_tx->clk_ih);
	printf("CLK_IPUD  %" PRIX64 "\n", pad_mii_tx->clk_ipud);
}


/* Returns number of bytes that were dumped
 * (length of static config) */
int sja1105_static_config_hexdump(void *buf)
{
	struct sja1105_table_header hdr;
	struct sja1105_static_config config;
	char *p = buf;
	char *table_end;
	int bytes;

	memset(&config, 0, sizeof(config));
	/* Retrieve device_id from first 4 bytes of packed buffer */
	gtable_unpack(p, &config.device_id, 31, 0, 4);
	printf("Device ID is 0x%08" PRIx64 " (%s)\n",
	       config.device_id, sja1105_device_id_string_get(
	       config.device_id, SJA1105_PART_NR_DONT_CARE));
	p += SIZE_SJA1105_DEVICE_ID;

	while (1) {
		sja1105_table_header_unpack(p, &hdr);
		/* This should match on last table header */
		if (hdr.len == 0) {
			break;
		}
		sja1105_table_header_show(&hdr);
		printf("Header:\n");
		gtable_hexdump(p, SIZE_TABLE_HEADER);
		p += SIZE_TABLE_HEADER;

		table_end = p + hdr.len * 4;
		while (p < table_end) {
			bytes = sja1105_static_config_add_entry(&hdr, p,
			                                        &config);
			if (bytes < 0) {
				goto error;
			}
			printf("Entry (%d bytes):\n", bytes);
			gtable_hexdump(p, bytes);
			p += bytes;
		};
		if (p != table_end) {
			loge("WARNING: Incorrect table length specified in header!");
			printf("Extra:\n");
			gtable_hexdump(p, (ptrdiff_t) (table_end - p));
			p = table_end;
		}
		printf("Table Data CRC:\n");
		gtable_hexdump(p, 4);
		p += 4;
		printf("\n");
	}
	return ((ptrdiff_t) (p - (char*) buf)) * sizeof(*buf);
error:
	return -1;
}

void gtable_hexdump(void *table, int len)
{
	uint8_t *p = (uint8_t*) table;
	int i;

	for (i = 0; i < len; i++) {
		if (i && (i % 4 == 0)) {
			printf(" ");
		}
		if (i % 8 == 0) {
			if (i) {
				printf("\n");
			}
			printf("(%.4X): ", i);
		}
		printf("%.2X ", p[i]);
	}
	printf("\n");
}

void gtable_bitdump(void *table, int len)
{
	uint8_t *p = (uint8_t*) table;
	int i, bit;

	for (i = 0; i < len; i++) {
		if (i && (i % 4 == 0)) {
			printf(" ");
		}
		if (i % 8 == 0) {
			if (i) {
				printf("\n");
			}
			printf("(%.4X): ", i);
		}
		for (bit = 7; bit >= 0; bit--) {
			printf("%d", (p[i] & (1 << bit)) >> bit);
		}
		printf(" ");
	}
	printf("\n");
}
