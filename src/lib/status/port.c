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

void sja1105_port_status_show(struct sja1105_port_status *status,
                              int port, char *print_buf, size_t len,
                              uint64_t device_id)
{
	char *fmt = "%-30s\n";
	int i;
	formatted_append(print_buf, len, fmt, "Port %d",          port);
	formatted_append(print_buf, len, fmt, "");
	formatted_append(print_buf, len, fmt, "MAC-Level Diagnostic Counters");
	formatted_append(print_buf, len, fmt, "N_RUNT          %" PRIu64, status->mac.n_runt);
	formatted_append(print_buf, len, fmt, "N_SOFERR        %" PRIu64, status->mac.n_soferr);
	formatted_append(print_buf, len, fmt, "N_ALIGNERR      %" PRIu64, status->mac.n_alignerr);
	formatted_append(print_buf, len, fmt, "N_MIIERR        %" PRIu64, status->mac.n_miierr);
	formatted_append(print_buf, len, fmt, "");
	formatted_append(print_buf, len, fmt, "MAC-Level Diagnostic Flags");
	formatted_append(print_buf, len, fmt, "TYPEERR         %" PRIX64, status->mac.typeerr);
	formatted_append(print_buf, len, fmt, "SIZEERR         %" PRIX64, status->mac.sizeerr);
	formatted_append(print_buf, len, fmt, "TCTIMEOUT       %" PRIX64, status->mac.tctimeout);
	formatted_append(print_buf, len, fmt, "PRIORERR        %" PRIX64, status->mac.priorerr);
	formatted_append(print_buf, len, fmt, "NOMASTER        %" PRIX64, status->mac.nomaster);
	formatted_append(print_buf, len, fmt, "MEMOV           %" PRIX64, status->mac.memov);
	formatted_append(print_buf, len, fmt, "MEMERR          %" PRIX64, status->mac.memerr);
	formatted_append(print_buf, len, fmt, "INVTYP          %" PRIX64, status->mac.invtyp);
	formatted_append(print_buf, len, fmt, "INTCYOV         %" PRIX64, status->mac.intcyov);
	formatted_append(print_buf, len, fmt, "DOMERR          %" PRIX64, status->mac.domerr);
	formatted_append(print_buf, len, fmt, "PCFBAGDROP      %" PRIX64, status->mac.pcfbagdrop);
	formatted_append(print_buf, len, fmt, "SPCPRIOR        %" PRIX64, status->mac.spcprior);
	formatted_append(print_buf, len, fmt, "AGEPRIOR        %" PRIX64, status->mac.ageprior);
	formatted_append(print_buf, len, fmt, "PORTDROP        %" PRIX64, status->mac.portdrop);
	formatted_append(print_buf, len, fmt, "LENDROP         %" PRIX64, status->mac.lendrop);
	formatted_append(print_buf, len, fmt, "BAGDROP         %" PRIX64, status->mac.bagdrop);
	formatted_append(print_buf, len, fmt, "POLICEERR       %" PRIX64, status->mac.policeerr);
	formatted_append(print_buf, len, fmt, "DRPNON664ERR    %" PRIX64, status->mac.drpnona664err);
	formatted_append(print_buf, len, fmt, "SPCERR          %" PRIX64, status->mac.spcerr);
	formatted_append(print_buf, len, fmt, "AGEDRP          %" PRIX64, status->mac.agedrp);
	formatted_append(print_buf, len, fmt, "");
	formatted_append(print_buf, len, fmt, "High-Level Diagnostic Counters");
	formatted_append(print_buf, len, fmt, "N_N664ERR       %" PRIu64, status->hl1.n_n664err);
	formatted_append(print_buf, len, fmt, "N_VLANERR       %" PRIu64, status->hl1.n_vlanerr);
	formatted_append(print_buf, len, fmt, "N_UNRELEASED    %" PRIu64, status->hl1.n_unreleased);
	formatted_append(print_buf, len, fmt, "N_SIZERR        %" PRIu64, status->hl1.n_sizerr);
	formatted_append(print_buf, len, fmt, "N_CRCERR        %" PRIu64, status->hl1.n_crcerr);
	formatted_append(print_buf, len, fmt, "N_VLNOTFOUND    %" PRIu64, status->hl1.n_vlnotfound);
	formatted_append(print_buf, len, fmt, "N_CTPOLERR      %" PRIu64, status->hl1.n_ctpolerr);
	formatted_append(print_buf, len, fmt, "N_POLERR        %" PRIu64, status->hl1.n_polerr);
	formatted_append(print_buf, len, fmt, "N_RXFRM         %" PRIu64, status->hl1.n_rxfrm);
	formatted_append(print_buf, len, fmt, "N_RXBYTE        %" PRIu64, status->hl1.n_rxbyte);
	formatted_append(print_buf, len, fmt, "N_TXFRM         %" PRIu64, status->hl1.n_txfrm);
	formatted_append(print_buf, len, fmt, "N_TXBYTE        %" PRIu64, status->hl1.n_txbyte);
	formatted_append(print_buf, len, fmt, "N_QFULL         %" PRIu64, status->hl2.n_qfull);
	formatted_append(print_buf, len, fmt, "N_PART_DROP     %" PRIu64, status->hl2.n_part_drop);
	formatted_append(print_buf, len, fmt, "N_EGR_DISABLED  %" PRIu64, status->hl2.n_egr_disabled);
	formatted_append(print_buf, len, fmt, "N_NOT_REACH     %" PRIu64, status->hl2.n_not_reach);
	formatted_append(print_buf, len, fmt, "");
	if (IS_PQRS(device_id)) {
		formatted_append(print_buf, len, fmt, "Queue Levels");
		for (i = 0; i < 8; i++) {
			formatted_append(print_buf, len, fmt, "QLEVEL_HWM_%d   %" PRIu64, i, status->hl2.qlevel_hwm[i]);
			formatted_append(print_buf, len, fmt, "QLEVEL_%d       %" PRIu64, i, status->hl2.qlevel[i]);
		}
		formatted_append(print_buf, len, fmt, "");
	}
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
	rc = sja1105_spi_send_packed_buf(spi_setup,
	                                 SPI_READ,
	                                 CORE_ADDR + mac_base_addr[port],
	                                 packed_buf,
	                                 SIZE_MAC_AREA);
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

	rc = sja1105_spi_send_packed_buf(spi_setup,
	                                 SPI_READ,
	                                 CORE_ADDR + high_level_1_base_addr[port],
	                                 packed_buf,
	                                 SIZE_HL1_AREA);
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

	rc = sja1105_spi_send_packed_buf(spi_setup,
	                                 SPI_READ,
	                                 CORE_ADDR + high_level_2_base_addr[port],
	                                 packed_buf,
	                                 SIZE_HL2_AREA);
	if (rc < 0) {
		loge("failed to read high-level 2 registers");
		goto out;
	}
	sja1105_port_status_hl2_unpack(packed_buf, status);

	if (IS_ET(spi_setup->device_id)) {
		/* Code below is strictly P/Q/R/S specific. */
		goto out;
	}
	rc = sja1105_spi_send_packed_buf(spi_setup,
	                                 SPI_READ,
	                                 CORE_ADDR + qlevel_base_addr[port],
	                                 packed_buf,
	                                 SIZE_QLEVEL_AREA);
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
	if (rc < 0) {
		goto out;
	}

	rc = sja1105_port_status_get_hl1(spi_setup, &status->hl1, port);
	if (rc < 0) {
		goto out;
	}

	rc = sja1105_port_status_get_hl2(spi_setup, &status->hl2, port);
	if (rc < 0) {
		goto out;
	}

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
	rc = sja1105_spi_send_packed_buf(spi_setup,
	                                 SPI_READ,
	                                 CORE_ADDR + PORT_STATUS_CTRL_ADDR,
	                                 packed_buf,
	                                 BUF_LEN);
	if (rc < 0) {
		loge("failed to clear mac level counters");
		goto out;
	}
out:
	return rc;

}
