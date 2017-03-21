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

/* TODO: change usage */
static void print_usage(const char *prog)
{
	printf("Usage: %s status [ type ] [ options ]\n", prog);
	printf("[ type ] can be:\n");
	printf(" * general -> General Status Information Register\n");
	/*printf(" * sync    -> Synchronization Status Register\n");*/
	/*printf(" * vl      -> Virtual Link Status\n");*/
	/*printf(" * memory  -> Memory Partition Information Register\n" \*/
	       /*"              Give Partition No. as argument [0-7]\n" \*/
	       /*"              default partition 0\n" \*/
	/*printf(" * vl-mem  -> VL Memory Partition Information Register\n" \*/
	       /*"              Provide Partition# as argument [0-7]\n");*/
	printf(" * port    -> Port status Information Register\n" \
	       "              Provide Port No. as argument [0-4]\n");
	/*printf(" * ptp     -> PTP Control Register Values\n");*/
}

void status_parse_args(struct spi_setup *spi_setup, int argc, char **argv)
{
	const char *options[] = {
		"general",
		/*"sync",*/
		/*"vl",*/
		/*"memory",*/
		/*"vl-memory",*/
		"ports",
		/*"ptp",*/
	};
	/*int partition;*/
	int port_no;
	int match;

	if (argc < 3) {
		goto error;
	}
	match = get_match(argv[2], options, ARRAY_SIZE(options));
	if (match < 0) {
		goto error;
	} else if (strcmp(options[match], "general") == 0) {
		status_general(spi_setup);
	/*} else if (strcmp(options[match], "sync") == 0) {*/
		/*status_sync(spi_setup);*/
	/*} else if (strcmp(options[match], "vl") == 0) {*/
		/*status_vl(spi_setup, argv[2]);*/
	/*} else if (strcmp(options[match], "memory") == 0) {*/
		/*if (argc < 4) {*/
			/*partition = 0;*/
			/*printf("showing for default partition %d\n");*/
		/*} else {*/
			/*sscanf(argv[3], "%d", &partition);*/
		/*}*/
		/*status_memory(spi_setup, partition);*/
	/*} else if (strcmp(options[match], "vl-memory") == 0) {*/
		/*if (argc < 4) {*/
			/*partition = 0;*/
			/*printf("showing for default partition %d\n");*/
		/*} else {*/
			/*sscanf(argv[3], "%d", &partition);*/
		/*}*/
		/*status_vl_memory(spi_setup, partition);*/
	} else if (strcmp(options[match], "ports") == 0) {
		if (argc < 4) {
			port_no = -1;
			logv("showing for all ports");
		} else {
			sscanf(argv[3], "%d", &port_no);
		}
		status_ports(spi_setup, port_no);
	/*} else if (strcmp(options[match], "ptp") == 0) {*/
		/*status_ptp(spi_setup);*/
	} else {
		goto error;
	}
	return;
error:
	print_usage(argv[0]);
	exit(0);
}

