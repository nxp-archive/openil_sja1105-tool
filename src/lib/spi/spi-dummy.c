/******************************************************************************
 * Copyright (c) 2017, NXP Semiconductors
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
#include <lib/include/spi.h>

/*
 * TODO:
 * Dummy implementation of SPI functions for sja1105-tool/libsja1105 build target.
 * For this build target, all library functions that call one of this SPI
 * functions shall be replaced with functions that get the required
 * information from the kernel module.
 */

int sja1105_spi_send_packed_buf(struct sja1105_spi_setup *spi_setup,
                                enum sja1105_spi_access_mode read_or_write,
                                uint64_t reg_addr,
                                void    *packed_buf,
                                uint64_t size_bytes)
{
	(void)spi_setup;
	(void)read_or_write;
	(void)reg_addr;
	(void)packed_buf;
	(void)size_bytes;
	return -1;
}

int sja1105_spi_send_int(struct sja1105_spi_setup *spi_setup,
                         enum sja1105_spi_access_mode read_or_write,
                         uint64_t reg_offset,
                         uint64_t *value,
                         uint64_t size_bytes)
{
	(void)spi_setup;
	(void)read_or_write;
	(void)reg_offset;
	(void)value;
	(void)size_bytes;
	return -1;
}

int sja1105_spi_send_long_packed_buf(struct sja1105_spi_setup *spi_setup,
                                     enum sja1105_spi_access_mode read_or_write,
                                     uint64_t base_addr,
                                     char    *packed_buf,
                                     uint64_t size_bytes)
{
	(void)spi_setup;
	(void)read_or_write;
	(void)base_addr;
	(void)packed_buf;
	(void)size_bytes;
	return -1;
}
