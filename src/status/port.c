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
#include "internal.h"

void sja1105_port_status_show(
		struct sja1105_port_status *status,
		int    port,
		char  *print_buf)
{
	char *fmt = "%-30s\n";
	formatted_append(print_buf, fmt, "Port %d",          port);
	formatted_append(print_buf, fmt, "");
	formatted_append(print_buf, fmt, "MAC-Level Diagnostic Counters");
	formatted_append(print_buf, fmt, "N_RUNT          %" PRIu64, status->n_runt);
	formatted_append(print_buf, fmt, "N_SOFERR        %" PRIu64, status->n_soferr);
	formatted_append(print_buf, fmt, "N_ALIGNERR      %" PRIu64, status->n_alignerr);
	formatted_append(print_buf, fmt, "N_MIIERR        %" PRIu64, status->n_miierr);
	formatted_append(print_buf, fmt, "");
	formatted_append(print_buf, fmt, "MAC-Level Diagnostic Flags");
	formatted_append(print_buf, fmt, "TYPEERR         %" PRIX64, status->typeerr);
	formatted_append(print_buf, fmt, "SIZEERR         %" PRIX64, status->sizeerr);
	formatted_append(print_buf, fmt, "TCTIMEOUT       %" PRIX64, status->tctimeout);
	formatted_append(print_buf, fmt, "PRIORERR        %" PRIX64, status->priorerr);
	formatted_append(print_buf, fmt, "NOMASTER        %" PRIX64, status->nomaster);
	formatted_append(print_buf, fmt, "MEMOV           %" PRIX64, status->memov);
	formatted_append(print_buf, fmt, "MEMERR          %" PRIX64, status->memerr);
	formatted_append(print_buf, fmt, "INVTYP          %" PRIX64, status->invtyp);
	formatted_append(print_buf, fmt, "INTCYOV         %" PRIX64, status->intcyov);
	formatted_append(print_buf, fmt, "DOMERR          %" PRIX64, status->domerr);
	formatted_append(print_buf, fmt, "PCFBAGDROP      %" PRIX64, status->pcfbagdrop);
	formatted_append(print_buf, fmt, "SPCPRIOR        %" PRIX64, status->spcprior);
	formatted_append(print_buf, fmt, "AGEPRIOR        %" PRIX64, status->ageprior);
	formatted_append(print_buf, fmt, "PORTDROP        %" PRIX64, status->portdrop);
	formatted_append(print_buf, fmt, "LENDROP         %" PRIX64, status->lendrop);
	formatted_append(print_buf, fmt, "BAGDROP         %" PRIX64, status->bagdrop);
	formatted_append(print_buf, fmt, "POLICEERR       %" PRIX64, status->policeerr);
	formatted_append(print_buf, fmt, "DRPNON664ERR    %" PRIX64, status->drpnona664err);
	formatted_append(print_buf, fmt, "SPCERR          %" PRIX64, status->spcerr);
	formatted_append(print_buf, fmt, "AGEDRP          %" PRIX64, status->agedrp);
	formatted_append(print_buf, fmt, "");
	formatted_append(print_buf, fmt, "High-Level Diagnostic Counters");
	formatted_append(print_buf, fmt, "N_N664ERR       %" PRIu64, status->n_n664err);
	formatted_append(print_buf, fmt, "N_VLANERR       %" PRIu64, status->n_vlanerr);
	formatted_append(print_buf, fmt, "N_UNRELEASED    %" PRIu64, status->n_unreleased);
	formatted_append(print_buf, fmt, "N_SIZERR        %" PRIu64, status->n_sizerr);
	formatted_append(print_buf, fmt, "N_CRCERR        %" PRIu64, status->n_crcerr);
	formatted_append(print_buf, fmt, "N_VLNOTFOUND    %" PRIu64, status->n_vlnotfound);
	formatted_append(print_buf, fmt, "N_BEPOLERR      %" PRIu64, status->n_bepolerr);
	formatted_append(print_buf, fmt, "N_POLERR        %" PRIu64, status->n_polerr);
	formatted_append(print_buf, fmt, "N_RXFRM         %" PRIu64, status->n_rxfrm);
	formatted_append(print_buf, fmt, "N_RXBYTE        %" PRIu64, status->n_rxbyte);
	formatted_append(print_buf, fmt, "N_TXFRM         %" PRIu64, status->n_txfrm);
	formatted_append(print_buf, fmt, "N_TXBYTE        %" PRIu64, status->n_txbyte);
	formatted_append(print_buf, fmt, "N_QFULL         %" PRIu64, status->n_qfull);
	formatted_append(print_buf, fmt, "N_PART_DROP     %" PRIu64, status->n_part_drop);
	formatted_append(print_buf, fmt, "N_EGR_DISABLED  %" PRIu64, status->n_egr_disabled);
	formatted_append(print_buf, fmt, "N_NOT_REACH     %" PRIu64, status->n_not_reach);
	formatted_append(print_buf, fmt, "");
}

void sja1105_port_status_get_mac(void *buf, struct sja1105_port_status *status)
{
	/* So that additions translate to 4 bytes */
	uint32_t *p = (uint32_t*) buf;
	generic_table_field_get(p + 0x0, &status->n_runt,       31, 24, 4);
	generic_table_field_get(p + 0x0, &status->n_soferr,     23, 16, 4);
	generic_table_field_get(p + 0x0, &status->n_alignerr,   15,  8, 4);
	generic_table_field_get(p + 0x0, &status->n_miierr,      7,  0, 4);
	generic_table_field_get(p + 0x1, &status->typeerr,      27, 27, 4);
	generic_table_field_get(p + 0x1, &status->sizeerr,      26, 26, 4);
	generic_table_field_get(p + 0x1, &status->priorerr,     24, 24, 4);
	generic_table_field_get(p + 0x1, &status->nomaster,     23, 23, 4);
	generic_table_field_get(p + 0x1, &status->memov,        22, 22, 4);
	generic_table_field_get(p + 0x1, &status->memerr,       21, 21, 4);
	generic_table_field_get(p + 0x1, &status->invtyp,       19, 19, 4);
	generic_table_field_get(p + 0x1, &status->intcyov,      18, 18, 4);
	generic_table_field_get(p + 0x1, &status->domerr,       17, 17, 4);
	generic_table_field_get(p + 0x1, &status->pcfbagdrop,   16, 16, 4);
	generic_table_field_get(p + 0x1, &status->spcprior,     15, 12, 4);
	generic_table_field_get(p + 0x1, &status->ageprior,     11,  8, 4);
	generic_table_field_get(p + 0x1, &status->portdrop,      6,  6, 4);
	generic_table_field_get(p + 0x1, &status->lendrop,       5,  5, 4);
	generic_table_field_get(p + 0x1, &status->bagdrop,       4,  4, 4);
	generic_table_field_get(p + 0x1, &status->policeerr,     3,  3, 4);
	generic_table_field_get(p + 0x1, &status->drpnona664err, 2,  2, 4);
	generic_table_field_get(p + 0x1, &status->spcerr,        1,  1, 4);
	generic_table_field_get(p + 0x1, &status->agedrp,        0,  0, 4);
}

void sja1105_port_status_get_hl1(void *buf, struct sja1105_port_status *status)
{
	/* So that additions translate to 4 bytes */
	uint32_t *p = (uint32_t*) buf;
	generic_table_field_get(p + 0xF, &status->n_n664err,    31,  0, 4);
	generic_table_field_get(p + 0xE, &status->n_vlanerr,    31,  0, 4);
	generic_table_field_get(p + 0xD, &status->n_unreleased, 31,  0, 4);
	generic_table_field_get(p + 0xC, &status->n_sizerr,     31,  0, 4);
	generic_table_field_get(p + 0xB, &status->n_crcerr,     31,  0, 4);
	generic_table_field_get(p + 0xA, &status->n_vlnotfound, 31,  0, 4);
	generic_table_field_get(p + 0x9, &status->n_bepolerr,   31,  0, 4);
	generic_table_field_get(p + 0x8, &status->n_polerr,     31,  0, 4);
	generic_table_field_get(p + 0x7, &status->n_rxfrmsh,    31,  0, 4);
	generic_table_field_get(p + 0x6, &status->n_rxfrm,      31,  0, 4);
	generic_table_field_get(p + 0x5, &status->n_rxbytesh,   31,  0, 4);
	generic_table_field_get(p + 0x4, &status->n_rxbyte,     31,  0, 4);
	generic_table_field_get(p + 0x3, &status->n_txfrmsh,    31,  0, 4);
	generic_table_field_get(p + 0x2, &status->n_txfrm,      31,  0, 4);
	generic_table_field_get(p + 0x1, &status->n_txbytesh,   31,  0, 4);
	generic_table_field_get(p + 0x0, &status->n_txbyte,     31,  0, 4);
	status->n_rxfrm  += status->n_rxfrmsh  << 32;
	status->n_rxbyte += status->n_rxbytesh << 32;
	status->n_txfrm  += status->n_txfrmsh  << 32;
	status->n_txbyte += status->n_txbytesh << 32;
}

void sja1105_port_status_get_hl2(void *buf, struct sja1105_port_status *status)
{
	/* So that additions translate to 4 bytes */
	uint32_t *p = (uint32_t*) buf;
	generic_table_field_get(p + 0x3, &status->n_qfull,        31,  0, 4);
	generic_table_field_get(p + 0x2, &status->n_part_drop,    31,  0, 4);
	generic_table_field_get(p + 0x1, &status->n_egr_disabled, 31,  0, 4);
	generic_table_field_get(p + 0x0, &status->n_not_reach,    31,  0, 4);
}

int sja1105_port_status_get(
		const struct spi_setup *spi_setup,
		struct sja1105_port_status *status,
		int port)
{
#define SIZE_MAC_AREA 0x02 * 4
#define SIZE_HL_AREA  0x10 * 4
#define MSG_LEN_MAC   SIZE_MAC_AREA + SIZE_SPI_MSG_HEADER
#define MSG_LEN_HL    SIZE_HL_AREA  + SIZE_SPI_MSG_HEADER
	const uint64_t mac_base_addr[]          = {0x200, 0x202, 0x204, 0x206, 0x208};
	const uint64_t high_level_1_base_addr[] = {0x400, 0x410, 0x420, 0x430, 0x440};
	const uint64_t high_level_2_base_addr[] = {0x600, 0x610, 0x620, 0x630, 0x640};
	struct sja1105_spi_message msg;
	uint8_t tx_buf[MSG_LEN_HL];
	uint8_t rx_buf[MSG_LEN_HL];
	int rc = 0;
	int fd;

	fd = configure_spi(spi_setup);
	if (fd < 0) {
		goto out;
	}
	memset(status, 0, sizeof(*status));

	/* MAC area */
	memset(tx_buf, 0, MSG_LEN_MAC);
	memset(rx_buf, 0, MSG_LEN_MAC);

	msg.access     = SPI_READ;
	msg.read_count = SIZE_MAC_AREA / 4;
	msg.address    = CORE_ADDR + mac_base_addr[port];
	sja1105_spi_message_set(tx_buf, &msg);

	rc = spi_transfer(fd, spi_setup, tx_buf, rx_buf, MSG_LEN_MAC);
	if (rc < 0) {
		loge("spi_transfer failed for mac registers");
		goto out_1;
	}
	sja1105_port_status_get_mac(rx_buf + 4, status);

	/* High-level 1 */
	memset(tx_buf, 0, MSG_LEN_HL);
	memset(rx_buf, 0, MSG_LEN_HL);

	msg.access     = SPI_READ;
	msg.read_count = SIZE_HL_AREA / 4;
	msg.address    = CORE_ADDR + high_level_1_base_addr[port];
	sja1105_spi_message_set(tx_buf, &msg);

	rc = spi_transfer(fd, spi_setup, tx_buf, rx_buf, MSG_LEN_HL);
	if (rc < 0) {
		loge("spi_transfer failed for high-level 1 registers");
		goto out_1;
	}
	sja1105_port_status_get_hl1(rx_buf + 4, status);

	/* High-level 2 */
	memset(tx_buf, 0, MSG_LEN_HL);
	memset(rx_buf, 0, MSG_LEN_HL);

	msg.access     = SPI_READ;
	msg.read_count = SIZE_HL_AREA / 4;
	msg.address    = CORE_ADDR + high_level_2_base_addr[port];
	sja1105_spi_message_set(tx_buf, &msg);

	rc = spi_transfer(fd, spi_setup, tx_buf, rx_buf, MSG_LEN_HL);
	if (rc < 0) {
		loge("spi_transfer failed for high-level 2 registers");
		goto out_1;
	}
	sja1105_port_status_get_hl2(rx_buf + 4, status);
out_1:
	close(fd);
out:
	return rc;
}

void status_ports(const struct spi_setup *spi_setup, int port_no)
{
	struct sja1105_port_status status;
	char *print_buf[5];
	/* XXX Maybe not quite right? */
	int   size = 10 * MAX_LINE_SIZE;
	int   i;

	if (port_no == -1) {
		/* Show for all ports */
		for (i = 0; i < 5; i++) {
			print_buf[i] = (char*) calloc(size, sizeof(char));
		}
		for (i = 0; i < 5; i++) {
			sja1105_port_status_get(spi_setup, &status, i);
			sja1105_port_status_show(&status, i, print_buf[i]);
		}
		linewise_concat(print_buf, 5);

		for (i = 0; i < 5; i++) {
			free(print_buf[i]);
		}
	} else {
		/* Show for single port */
		print_buf[0] = (char*) calloc(size, sizeof(char));
		sja1105_port_status_get(spi_setup, &status, port_no);
		sja1105_port_status_show(&status, port_no, print_buf[0]);
		printf("%s\n", print_buf[0]);
		free(print_buf[0]);
	}
}

