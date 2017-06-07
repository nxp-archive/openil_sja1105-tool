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
#ifndef _SJA1105_TOOL_INTERNAL
#define _SJA1105_TOOL_INTERNAL

#include <common.h>
#include <lib/include/config.h>
#include <lib/include/spi.h>

int read_config_file(char*, struct spi_setup*, struct general_config*);
int rgu_parse_args(struct spi_setup *spi_setup, int argc, char **argv);
int ptp_parse_args(struct spi_setup *spi_setup, int argc, char **argv);
int config_parse_args(struct spi_setup *spi_setup, int argc, char **argv);
int status_parse_args(struct spi_setup *spi_setup, int argc, char **argv);
int config_table_entry_modify(
		struct sja1105_config *config,
		char *table_name,
		char *field_name,
		char *field_val);
int sja1105_config_show(struct sja1105_config *config, char *table_name);

/* From strings.c, mainly */
char *trimwhitespace(char *str);
int   matches(const char*, const char*);
int   get_match(const char*, const char**, int);

#endif
