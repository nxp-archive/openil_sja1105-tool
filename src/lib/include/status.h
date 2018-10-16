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
#ifndef _STATUS_PARSE_ARGS_H
#define _STATUS_PARSE_ARGS_H

#include <common.h>
#include "spi.h"

#define CORE_ADDR   0x000000
#define ACU_ADDR    0x100800

struct sja1105_general_status {
	uint64_t configs;
	uint64_t crcchkl;
	uint64_t ids;
	uint64_t crcchkg;
	uint64_t nslot;
	uint64_t vlind;
	uint64_t vlparind;
	uint64_t vlroutes;
	uint64_t vlparts;
	uint64_t macaddl;
	uint64_t portenf;
	uint64_t fwds_03h;
	uint64_t macfds;
	uint64_t enffds;
	uint64_t l2busyfds;
	uint64_t l2busys;
	uint64_t macaddu;
	uint64_t macaddhcl;
	uint64_t vlanidhc;
	uint64_t hashconfs;
	uint64_t macaddhcu;
	uint64_t wpvlanid;
	uint64_t port_07h;
	uint64_t vlanbusys;
	uint64_t wrongports;
	uint64_t vnotfounds;
	uint64_t vlid;
	uint64_t portvl;
	uint64_t vlnotfound;
	uint64_t emptys;
	uint64_t buffers;
	uint64_t buflwmark; /* Only on P/Q/R/S */
	uint64_t port_0ah;
	uint64_t fwds_0ah;
	uint64_t parts;
	uint64_t ramparerrl;
	uint64_t ramparerru;
};

struct sja1105_port_status {
	uint64_t n_runt;
	uint64_t n_soferr;
	uint64_t n_alignerr;
	uint64_t n_miierr;
	uint64_t typeerr;
	uint64_t sizeerr;
	uint64_t tctimeout;
	uint64_t priorerr;
	uint64_t nomaster;
	uint64_t memov;
	uint64_t memerr;
	uint64_t invtyp;
	uint64_t intcyov;
	uint64_t domerr;
	uint64_t pcfbagdrop;
	uint64_t spcprior;
	uint64_t ageprior;
	uint64_t portdrop;
	uint64_t lendrop;
	uint64_t bagdrop;
	uint64_t policeerr;
	uint64_t drpnona664err;
	uint64_t spcerr;
	uint64_t agedrp;
	uint64_t n_n664err;
	uint64_t n_vlanerr;
	uint64_t n_unreleased;
	uint64_t n_sizerr;
	uint64_t n_crcerr;
	uint64_t n_vlnotfound;
	uint64_t n_bepolerr;
	uint64_t n_polerr;
	uint64_t n_rxfrmsh;
	uint64_t n_rxfrm;
	uint64_t n_rxbytesh;
	uint64_t n_rxbyte;
	uint64_t n_txfrmsh;
	uint64_t n_txfrm;
	uint64_t n_txbytesh;
	uint64_t n_txbyte;
	uint64_t n_qfull;
	uint64_t n_part_drop;
	uint64_t n_egr_disabled;
	uint64_t n_not_reach;
	uint64_t qlevel_hwm[8]; /* Only for P/Q/R/S */
	uint64_t qlevel[8];     /* Only for P/Q/R/S */
};

struct sja1105_ptp_status {
	uint64_t syncstate;
	uint64_t integcy;
	uint64_t actcorr;
	uint64_t maxcorr;
	uint64_t mincorr;
	uint64_t syncloss;
	uint64_t locmem;
	uint64_t minmemvar;
	uint64_t maxmemvar;
	uint64_t localsynmem0;
	uint64_t localsynmem1;
	uint64_t minasynmem;
	uint64_t maxasynmem;
	uint64_t camem;
	uint64_t synmem;
	uint64_t moffset[8];
	uint64_t timer;
	uint64_t clock;
};

int  sja1105_general_status_get(struct sja1105_spi_setup*,
                                struct sja1105_general_status*);
void sja1105_general_status_show(struct sja1105_general_status*,
                                 uint64_t device_id);
void sja1105_port_status_show(struct sja1105_port_status*,
                              int    port, char  *print_buf,
                              uint64_t device_id);
int sja1105_port_status_get(struct sja1105_spi_setup*,
                            struct sja1105_port_status*,
                            int port);
int sja1105_port_status_clear(struct sja1105_spi_setup*, int);

#endif
