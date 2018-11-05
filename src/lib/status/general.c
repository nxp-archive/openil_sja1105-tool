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
#include <lib/include/static-config.h>
#include <lib/include/status.h>
#include <lib/include/gtable.h>
#include <lib/include/spi.h>
#include <common.h>

const char *SJA1105E_DEVICE_ID_STR        = "SJA1105E";
const char *SJA1105T_DEVICE_ID_STR        = "SJA1105T";
const char *SJA1105P_DEVICE_ID_STR        = "SJA1105P";
const char *SJA1105Q_DEVICE_ID_STR        = "SJA1105Q";
const char *SJA1105R_DEVICE_ID_STR        = "SJA1105R";
const char *SJA1105S_DEVICE_ID_STR        = "SJA1105S";
const char *SJA1105PR_DEVICE_ID_STR       = "SJA1105P or SJA1105R";
const char *SJA1105QS_DEVICE_ID_STR       = "SJA1105Q or SJA1105S";
const char *SJA1105_NO_DEVICE_ID_STR      = "None";

const char *sja1105_device_id_string_get(uint64_t device_id, uint64_t part_nr)
{
	if (device_id == SJA1105E_DEVICE_ID) {
		return SJA1105E_DEVICE_ID_STR;
	}
	if (device_id == SJA1105T_DEVICE_ID) {
		return SJA1105T_DEVICE_ID_STR;
	}
	/* P and R have same Device ID, and differ by Part Number.
	 * Same do Q and S.
	 */
	if (IS_P(device_id, part_nr)) {
		return SJA1105P_DEVICE_ID_STR;
	}
	if (IS_Q(device_id, part_nr)) {
		return SJA1105Q_DEVICE_ID_STR;
	}
	if (IS_R(device_id, part_nr)) {
		return SJA1105P_DEVICE_ID_STR;
	}
	if (IS_S(device_id, part_nr)) {
		return SJA1105S_DEVICE_ID_STR;
	}
	/* Fallback: if we don't know/care what the part_nr is, and we
	 * have a P/R, we can simply pass -1 to part_nr and have this
	 * function say it's either P or R, instead of reporting it
	 * as invalid.
	 */
	if (device_id == SJA1105PR_DEVICE_ID) {
		return SJA1105PR_DEVICE_ID_STR;
	}
	if (device_id == SJA1105QS_DEVICE_ID) {
		return SJA1105QS_DEVICE_ID_STR;
	}
	return SJA1105_NO_DEVICE_ID_STR;
}

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
	rc = sja1105_spi_send_int(spi_setup,
	                          SPI_READ,
	                          CORE_ADDR + 0x00,
	                          &tmp_device_id,
	                          SIZE_SJA1105_DEVICE_ID);
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
		rc = sja1105_spi_send_int(spi_setup,
		                          SPI_READ,
		                          ACU_ADDR + PROD_ID_ADDR,
		                          &tmp_part_nr,
		                          4);
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
	rc = sja1105_spi_send_packed_buf(spi_setup,
	                                 SPI_READ,
	                                 CORE_ADDR + 0x01,
	                                 packed_buf,
	                                 SIZE_GENERAL_STATUS);
	if (rc < 0) {
		loge("spi read failed");
		goto out;
	}
	sja1105_general_status_unpack(packed_buf, status,
	                              spi_setup->device_id);
out:
	return rc;
}

