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

static void gtable_hexdump(void *table, int len)
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

static void sja1105_table_header_show(struct sja1105_table_header *hdr)
{
	char print_buf[MAX_LINE_SIZE];

	memset(print_buf, 0, MAX_LINE_SIZE);
	sja1105_table_header_fmt_show(print_buf, MAX_LINE_SIZE, hdr);
	puts(print_buf);
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
