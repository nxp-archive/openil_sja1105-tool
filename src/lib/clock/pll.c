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
#include <lib/include/gtable.h>
#include <common.h>

static void
sja1105_cgu_pll_control_access(void *buf,
                               struct sja1105_cgu_pll_control *pll_control,
                               int write,
                               uint64_t device_id)
{
	int (*pack_or_unpack)(void*, uint64_t*, int, int, int);
	int   size = 4;

	if (write == 0) {
		pack_or_unpack = gtable_unpack;
		memset(pll_control, 0, sizeof(*pll_control));
	} else {
		pack_or_unpack = gtable_pack;
		memset(buf, 0, size);
	}
	pack_or_unpack(buf, &pll_control->pllclksrc, 28, 24, 4);
	pack_or_unpack(buf, &pll_control->msel,      23, 16, 4);
	pack_or_unpack(buf, &pll_control->autoblock, 11, 11, 4);
	pack_or_unpack(buf, &pll_control->psel,       9,  8, 4);
	pack_or_unpack(buf, &pll_control->direct,     7,  7, 4);
	pack_or_unpack(buf, &pll_control->fbsel,      6,  6, 4);
	pack_or_unpack(buf, &pll_control->bypass,     1,  1, 4);
	pack_or_unpack(buf, &pll_control->pd,         0,  0, 4);
	if (IS_PQRS(device_id)) {
		pack_or_unpack(buf, &pll_control->nsel, 13, 12, 4);
		pack_or_unpack(buf, &pll_control->p23en, 2,  2, 4);
	}
}

void sja1105_cgu_pll_control_pack(void *buf,
                                  struct sja1105_cgu_pll_control *pll_control,
                                  uint64_t device_id)
{
	sja1105_cgu_pll_control_access(buf, pll_control, 1, device_id);
}

void sja1105_cgu_pll_control_unpack(void *buf, struct
                                    sja1105_cgu_pll_control *pll_control,
                                    uint64_t device_id)
{
	sja1105_cgu_pll_control_access(buf, pll_control, 0, device_id);
}

