/*
 * SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
 *
 * Copyright (c) 2016-2018, NXP Semiconductors
 */
#include <lib/include/static-config.h>
#include <lib/include/status.h>
#include <lib/include/gtable.h>
#include <lib/include/spi.h>
#include <common.h>
#include "sja1105.h"

int sja1105_device_id_get(struct sja1105_spi_setup *spi_setup,
                          uint64_t *device_id, uint64_t *part_nr)
{
	uint64_t compatible_device_ids[] = {
		SJA1105E_DEVICE_ID,
		SJA1105T_DEVICE_ID,
		SJA1105PR_DEVICE_ID,
		SJA1105QS_DEVICE_ID,
	};
	uint64_t tmp_device_id;
	uint64_t tmp_part_nr;
	unsigned int i;
	int rc;

	if (spi_setup->dry_run) {
		/* These checks simply cannot pass (and do not even
		 * make sense to have) if we are in dry run mode */
		rc = 0;
		goto out_found;
	}
	rc = sja1105_spi_send_int(spi_setup, SPI_READ,CORE_ADDR + 0x00,
	                          &tmp_device_id, SIZE_SJA1105_DEVICE_ID);
	if (rc < 0) {
		loge("sja1105_spi_send_int failed");
		goto out_error;
	}
	*device_id = SJA1105_NO_DEVICE_ID;
	for (i = 0; i < ARRAY_SIZE(compatible_device_ids); i++) {
		if (tmp_device_id == compatible_device_ids[i]) {
			*device_id = compatible_device_ids[i];
			break;
		}
	}
	if (*device_id == SJA1105_NO_DEVICE_ID) {
		loge("Unrecognized Device ID 0x%08" PRIx64, tmp_device_id);
		rc = -EINVAL;
		goto out_error;
	}
	if (IS_PQRS(*device_id)) {
		/* 0x100BC3 relative to 0x100800 */
		const int PROD_ID_ADDR = 0x3C3;
		rc = sja1105_spi_send_int(spi_setup, SPI_READ,
		                          ACU_ADDR + PROD_ID_ADDR,
		                          &tmp_part_nr, 4);
		if (rc < 0) {
			loge("sja1105_spi_send_int failed");
			goto out_error;
		}
		gtable_unpack(&tmp_part_nr, part_nr, 19, 4, 4);
	}
	logv("%s Device ID detected.",
	     sja1105_device_id_string_get(*device_id, *part_nr));
out_error:
out_found:
	return rc;
}

static void
sja1105_general_status_unpack(void *buf, struct sja1105_general_status *status,
                              uint64_t device_id)
{
	/* So that addition translates to 4 bytes */
	uint32_t *p = (uint32_t*) buf;
	memset(status, 0, sizeof(*status));
	/* device_id is missing from the buffer, but we don't
	 * want to diverge from the manual definition of the
	 * register addresses, so we'll back off one step with
	 * the register pointer, and never access p[0].
	 */
	p--;
	gtable_unpack(p + 0x1, &status->configs,   31, 31, 4);
	gtable_unpack(p + 0x1, &status->crcchkl,   30, 30, 4);
	gtable_unpack(p + 0x1, &status->ids,       29, 29, 4);
	gtable_unpack(p + 0x1, &status->crcchkg,   28, 28, 4);
	gtable_unpack(p + 0x1, &status->nslot,      3,  0, 4);
	gtable_unpack(p + 0x2, &status->vlind,     31, 16, 4);
	gtable_unpack(p + 0x2, &status->vlparind,  15,  8, 4);
	gtable_unpack(p + 0x2, &status->vlroutes,   1,  1, 4);
	gtable_unpack(p + 0x2, &status->vlparts,    0,  0, 4);
	gtable_unpack(p + 0x3, &status->macaddl,   31, 16, 4);
	gtable_unpack(p + 0x3, &status->portenf,   15,  8, 4);
	gtable_unpack(p + 0x3, &status->fwds_03h,   4,  4, 4);
	gtable_unpack(p + 0x3, &status->macfds,     3,  3, 4);
	gtable_unpack(p + 0x3, &status->enffds,     2,  2, 4);
	gtable_unpack(p + 0x3, &status->l2busyfds,  1,  1, 4);
	gtable_unpack(p + 0x3, &status->l2busys,    0,  0, 4);
	gtable_unpack(p + 0x4, &status->macaddu,   31,  0, 4);
	gtable_unpack(p + 0x5, &status->macaddhcl, 31, 16, 4);
	gtable_unpack(p + 0x5, &status->vlanidhc,  15,  4, 4);
	gtable_unpack(p + 0x5, &status->hashconfs,  0,  0, 4);
	gtable_unpack(p + 0x6, &status->macaddhcu, 31,  0, 4);
	gtable_unpack(p + 0x7, &status->wpvlanid,  31, 16, 4);
	gtable_unpack(p + 0x7, &status->port_07h,  15,  8, 4);
	gtable_unpack(p + 0x7, &status->vlanbusys,  4,  4, 4);
	gtable_unpack(p + 0x7, &status->wrongports, 3,  3, 4);
	gtable_unpack(p + 0x7, &status->vnotfounds, 2,  2, 4);
	gtable_unpack(p + 0x8, &status->vlid,      31, 16, 4);
	gtable_unpack(p + 0x8, &status->portvl,    15,  8, 4);
	gtable_unpack(p + 0x8, &status->vlnotfound, 0,  0, 4);
	gtable_unpack(p + 0x9, &status->emptys,    31, 31, 4);
	gtable_unpack(p + 0x9, &status->buffers,   30,  0, 4);
	if (IS_ET(device_id)) {
		gtable_unpack(p + 0xA, &status->port_0ah,  15,  8, 4);
		gtable_unpack(p + 0xA, &status->fwds_0ah,   1,  1, 4);
		gtable_unpack(p + 0xA, &status->parts,      0,  0, 4);
		gtable_unpack(p + 0xB, &status->ramparerrl,20,  0, 4);
		gtable_unpack(p + 0xC, &status->ramparerru, 4,  0, 4);
	} else {
		gtable_unpack(p + 0xA, &status->buflwmark, 30,  0, 4);
		gtable_unpack(p + 0xB, &status->port_0ah,  15,  8, 4);
		gtable_unpack(p + 0xB, &status->fwds_0ah,   1,  1, 4);
		gtable_unpack(p + 0xB, &status->parts,      0,  0, 4);
		gtable_unpack(p + 0xC, &status->ramparerrl,22,  0, 4);
		gtable_unpack(p + 0xD, &status->ramparerru, 4,  0, 4);
	}
}

void sja1105_general_status_show(struct sja1105_general_status *status,
		char *print_buf,
		size_t len,
		uint64_t device_id)
{
	char *fmt = "%s\n";

	formatted_append(print_buf, len, fmt, "CONFIGS    %" PRIX64, status->configs);
	formatted_append(print_buf, len, fmt, "CRCCHKL    %" PRIX64, status->crcchkl);
	formatted_append(print_buf, len, fmt, "IDS        %" PRIX64, status->ids);
	formatted_append(print_buf, len, fmt, "CRCCHKG    %" PRIX64, status->crcchkg);
	formatted_append(print_buf, len, fmt, "NSLOT      %" PRIX64, status->nslot);
	formatted_append(print_buf, len, fmt, "VLIND      %" PRIX64, status->vlind);
	formatted_append(print_buf, len, fmt, "VLPARIND   %" PRIX64, status->vlparind);
	formatted_append(print_buf, len, fmt, "VLROUTES   %" PRIX64, status->vlroutes);
	formatted_append(print_buf, len, fmt, "VLPARTS    %" PRIX64, status->vlparts);
	formatted_append(print_buf, len, fmt, "MACADDL    %" PRIX64, status->macaddl);
	formatted_append(print_buf, len, fmt, "PORTENF    %" PRIX64, status->portenf);
	formatted_append(print_buf, len, fmt, "FWDS_03h   %" PRIX64, status->fwds_03h);
	formatted_append(print_buf, len, fmt, "MACFDS     %" PRIX64, status->macfds);
	formatted_append(print_buf, len, fmt, "ENFFDS     %" PRIX64, status->enffds);
	formatted_append(print_buf, len, fmt, "L2BUSYFDS  %" PRIX64, status->l2busyfds);
	formatted_append(print_buf, len, fmt, "L2BUSYS    %" PRIX64, status->l2busys);
	formatted_append(print_buf, len, fmt, "MACADDU    %" PRIX64, status->macaddu);
	formatted_append(print_buf, len, fmt, "MACADDHCL  %" PRIX64, status->macaddhcl);
	formatted_append(print_buf, len, fmt, "VLANIDHC   %" PRIX64, status->vlanidhc);
	formatted_append(print_buf, len, fmt, "HASHCONFS  %" PRIX64, status->hashconfs);
	formatted_append(print_buf, len, fmt, "MACADDHCU  %" PRIX64, status->macaddhcu);
	formatted_append(print_buf, len, fmt, "WPVLANID   %" PRIX64, status->wpvlanid);
	formatted_append(print_buf, len, fmt, "PORT_07h   %" PRIX64, status->port_07h);
	formatted_append(print_buf, len, fmt, "VLAN_BUSYS %" PRIX64, status->vlanbusys);
	formatted_append(print_buf, len, fmt, "WRONGPORTS %" PRIX64, status->wrongports);
	formatted_append(print_buf, len, fmt, "VNOTFOUNDS %" PRIX64, status->vnotfounds);
	formatted_append(print_buf, len, fmt, "VLID       %" PRIX64, status->vlid);
	formatted_append(print_buf, len, fmt, "PORTVL     %" PRIX64, status->portvl);
	formatted_append(print_buf, len, fmt, "VLNOTFOUND %" PRIX64, status->vlnotfound);
	formatted_append(print_buf, len, fmt, "EMPTYS     %" PRIX64, status->emptys);
	formatted_append(print_buf, len, fmt, "BUFFERS    %" PRIX64, status->buffers);
	if (IS_PQRS(device_id)) {
		formatted_append(print_buf, len, fmt, "BUFLWMARK  %" PRIX64, status->buflwmark);
	}
	formatted_append(print_buf, len, fmt, "PORT_0Ah   %" PRIX64, status->port_0ah);
	formatted_append(print_buf, len, fmt, "FWDS_0Ah   %" PRIX64, status->fwds_0ah);
	formatted_append(print_buf, len, fmt, "PARTS      %" PRIX64, status->parts);
	formatted_append(print_buf, len, fmt, "RAMPARERRL %" PRIX64, status->ramparerrl);
	formatted_append(print_buf, len, fmt, "RAMPARERRU %" PRIX64, status->ramparerru);
}

int sja1105_general_status_get(struct sja1105_spi_setup *spi_setup,
                               struct sja1105_general_status *status)
{
	const int SIZE_GENERAL_STATUS = IS_ET(spi_setup->device_id) ?
	                                0x0C * 4 : /* 0x01 to 0x0C */
	                                0x0D * 4;  /* 0x01 to 0x0D */
	uint8_t packed_buf[SIZE_GENERAL_STATUS];
	int rc;

	/* The base address is off-by-1 compared to UM10944,
	 * because we are skipping device_id from the readout. */
	rc = sja1105_spi_send_packed_buf(spi_setup, SPI_READ, CORE_ADDR + 0x01,
	                                 packed_buf, SIZE_GENERAL_STATUS);
	if (rc < 0) {
		loge("spi read failed");
		goto out;
	}
	sja1105_general_status_unpack(packed_buf, status, spi_setup->device_id);
out:
	return rc;
}

static void
sja1105_port_status_mac_unpack(void *buf, struct
                               sja1105_port_status_mac *status)
{
	/* So that additions translate to 4 bytes */
	uint32_t *p = (uint32_t*) buf;
	gtable_unpack(p + 0x0, &status->n_runt,       31, 24, 4);
	gtable_unpack(p + 0x0, &status->n_soferr,     23, 16, 4);
	gtable_unpack(p + 0x0, &status->n_alignerr,   15,  8, 4);
	gtable_unpack(p + 0x0, &status->n_miierr,      7,  0, 4);
	gtable_unpack(p + 0x1, &status->typeerr,      27, 27, 4);
	gtable_unpack(p + 0x1, &status->sizeerr,      26, 26, 4);
	gtable_unpack(p + 0x1, &status->tctimeout,    25, 25, 4);
	gtable_unpack(p + 0x1, &status->priorerr,     24, 24, 4);
	gtable_unpack(p + 0x1, &status->nomaster,     23, 23, 4);
	gtable_unpack(p + 0x1, &status->memov,        22, 22, 4);
	gtable_unpack(p + 0x1, &status->memerr,       21, 21, 4);
	gtable_unpack(p + 0x1, &status->invtyp,       19, 19, 4);
	gtable_unpack(p + 0x1, &status->intcyov,      18, 18, 4);
	gtable_unpack(p + 0x1, &status->domerr,       17, 17, 4);
	gtable_unpack(p + 0x1, &status->pcfbagdrop,   16, 16, 4);
	gtable_unpack(p + 0x1, &status->spcprior,     15, 12, 4);
	gtable_unpack(p + 0x1, &status->ageprior,     11,  8, 4);
	gtable_unpack(p + 0x1, &status->portdrop,      6,  6, 4);
	gtable_unpack(p + 0x1, &status->lendrop,       5,  5, 4);
	gtable_unpack(p + 0x1, &status->bagdrop,       4,  4, 4);
	gtable_unpack(p + 0x1, &status->policeerr,     3,  3, 4);
	gtable_unpack(p + 0x1, &status->drpnona664err, 2,  2, 4);
	gtable_unpack(p + 0x1, &status->spcerr,        1,  1, 4);
	gtable_unpack(p + 0x1, &status->agedrp,        0,  0, 4);
}

static void
sja1105_port_status_hl1_unpack(void *buf, struct
                               sja1105_port_status_hl1 *status)
{
	/* So that additions translate to 4 bytes */
	uint32_t *p = (uint32_t*) buf;
	gtable_unpack(p + 0xF, &status->n_n664err,    31,  0, 4);
	gtable_unpack(p + 0xE, &status->n_vlanerr,    31,  0, 4);
	gtable_unpack(p + 0xD, &status->n_unreleased, 31,  0, 4);
	gtable_unpack(p + 0xC, &status->n_sizerr,     31,  0, 4);
	gtable_unpack(p + 0xB, &status->n_crcerr,     31,  0, 4);
	gtable_unpack(p + 0xA, &status->n_vlnotfound, 31,  0, 4);
	gtable_unpack(p + 0x9, &status->n_ctpolerr,   31,  0, 4);
	gtable_unpack(p + 0x8, &status->n_polerr,     31,  0, 4);
	gtable_unpack(p + 0x7, &status->n_rxfrmsh,    31,  0, 4);
	gtable_unpack(p + 0x6, &status->n_rxfrm,      31,  0, 4);
	gtable_unpack(p + 0x5, &status->n_rxbytesh,   31,  0, 4);
	gtable_unpack(p + 0x4, &status->n_rxbyte,     31,  0, 4);
	gtable_unpack(p + 0x3, &status->n_txfrmsh,    31,  0, 4);
	gtable_unpack(p + 0x2, &status->n_txfrm,      31,  0, 4);
	gtable_unpack(p + 0x1, &status->n_txbytesh,   31,  0, 4);
	gtable_unpack(p + 0x0, &status->n_txbyte,     31,  0, 4);
	status->n_rxfrm  += status->n_rxfrmsh  << 32;
	status->n_rxbyte += status->n_rxbytesh << 32;
	status->n_txfrm  += status->n_txfrmsh  << 32;
	status->n_txbyte += status->n_txbytesh << 32;
}

static void
sja1105_port_status_hl2_unpack(void *buf, struct
                               sja1105_port_status_hl2 *status)
{
	/* So that additions translate to 4 bytes */
	uint32_t *p = (uint32_t*) buf;
	gtable_unpack(p + 0x3, &status->n_qfull,        31,  0, 4);
	gtable_unpack(p + 0x2, &status->n_part_drop,    31,  0, 4);
	gtable_unpack(p + 0x1, &status->n_egr_disabled, 31,  0, 4);
	gtable_unpack(p + 0x0, &status->n_not_reach,    31,  0, 4);
}

static void
sja1105pqrs_port_status_qlevel_unpack(void *buf, struct
                                      sja1105_port_status_hl2 *status)
{
	/* So that additions translate to 4 bytes */
	uint32_t *p = (uint32_t*) buf;
	int i;

	for (i = 0; i < 8; i++) {
		gtable_unpack(p + i, &status->qlevel_hwm[i], 24, 16, 4);
		gtable_unpack(p + i, &status->qlevel[i],      8,  0, 4);
	}
}

int sja1105_port_status_get_mac(struct sja1105_spi_setup *spi_setup,
                                struct sja1105_port_status_mac *status,
                                int port)
{
	const int SIZE_MAC_AREA = 0x02 * 4;
	const uint64_t mac_base_addr[] = {0x200, 0x202, 0x204, 0x206, 0x208};
	uint8_t packed_buf[SIZE_MAC_AREA];
	int rc = 0;

	memset(status, 0, sizeof(*status));

	/* MAC area */
	rc = sja1105_spi_send_packed_buf(spi_setup, SPI_READ,
	                                 CORE_ADDR + mac_base_addr[port],
	                                 packed_buf, SIZE_MAC_AREA);
	if (rc < 0) {
		loge("failed to read mac registers");
		goto out;
	}
	sja1105_port_status_mac_unpack(packed_buf, status);
out:
	return rc;
}

int sja1105_port_status_get_hl1(struct sja1105_spi_setup *spi_setup,
                                struct sja1105_port_status_hl1 *status,
                                int port)
{
	const int SIZE_HL1_AREA = 0x10 * 4;
	const uint64_t high_level_1_base_addr[] = {0x400, 0x410, 0x420, 0x430, 0x440};
	uint8_t packed_buf[SIZE_HL1_AREA];
	int rc = 0;

	memset(status, 0, sizeof(*status));

	rc = sja1105_spi_send_packed_buf(spi_setup, SPI_READ,
	                                 CORE_ADDR + high_level_1_base_addr[port],
	                                 packed_buf, SIZE_HL1_AREA);
	if (rc < 0) {
		loge("failed to read high-level 1 registers");
		goto out;
	}
	sja1105_port_status_hl1_unpack(packed_buf, status);
out:
	return rc;
}

int sja1105_port_status_get_hl2(struct sja1105_spi_setup *spi_setup,
                                struct sja1105_port_status_hl2 *status,
                                int port)
{
	const int SIZE_HL2_AREA = 0x4 * 4;
	const int SIZE_QLEVEL_AREA =  0x8 * 4; /* 0x4 to 0xB */
	const uint64_t high_level_2_base_addr[] = {0x600, 0x610, 0x620, 0x630, 0x640};
	const uint64_t qlevel_base_addr[]       = {0x604, 0x614, 0x624, 0x634, 0x644};
	uint8_t packed_buf[SIZE_QLEVEL_AREA];
	int rc = 0;

	memset(status, 0, sizeof(*status));

	rc = sja1105_spi_send_packed_buf(spi_setup, SPI_READ,
	                                 CORE_ADDR + high_level_2_base_addr[port],
	                                 packed_buf, SIZE_HL2_AREA);
	if (rc < 0) {
		loge("failed to read high-level 2 registers");
		goto out;
	}
	sja1105_port_status_hl2_unpack(packed_buf, status);

	if (IS_ET(spi_setup->device_id))
		/* Code below is strictly P/Q/R/S specific. */
		goto out;

	rc = sja1105_spi_send_packed_buf(spi_setup, SPI_READ,
	                                 CORE_ADDR + qlevel_base_addr[port],
	                                 packed_buf, SIZE_QLEVEL_AREA);
	if (rc < 0) {
		loge("failed to read high-level 1 registers");
		goto out;
	}
	sja1105pqrs_port_status_qlevel_unpack(packed_buf, status);
out:
	return rc;
}

int sja1105_port_status_get(struct sja1105_spi_setup *spi_setup,
                            struct sja1105_port_status *status,
                            int port)
{
	int rc;

	rc = sja1105_port_status_get_mac(spi_setup, &status->mac, port);
	if (rc < 0)
		goto out;

	rc = sja1105_port_status_get_hl1(spi_setup, &status->hl1, port);
	if (rc < 0)
		goto out;

	rc = sja1105_port_status_get_hl2(spi_setup, &status->hl2, port);
	if (rc < 0)
		goto out;

out:
	return rc;
}

int sja1105_port_status_clear(struct sja1105_spi_setup *spi_setup,
                              int port)
{
	const int PORT_STATUS_CTRL_ADDR = IS_ET(spi_setup->device_id) ?
	                                  0xf : 0x10;
	const int BUF_LEN = 4;
	uint8_t   packed_buf[BUF_LEN];
	int       rc = 0;
	uint64_t  clearport;

	if (port == -1) {
		logv("clearing mac counters for all ports");
		clearport = 0x1f;
	} else if (port < 5) {
		clearport = 1 << port;
	} else {
		loge("invalid port number %d", port);
		rc = -EINVAL;
		goto out;
	}
	memset(packed_buf, 0, BUF_LEN);

	gtable_pack(packed_buf, &clearport, 4, 0, BUF_LEN);
	rc = sja1105_spi_send_packed_buf(spi_setup, SPI_WRITE,
	                                 CORE_ADDR + PORT_STATUS_CTRL_ADDR,
	                                 packed_buf, BUF_LEN);
	if (rc < 0) {
		loge("failed to clear mac level counters");
		goto out;
	}
out:
	return rc;

}

