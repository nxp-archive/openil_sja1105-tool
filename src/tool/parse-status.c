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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include "internal.h"
#include <lib/include/status.h>

static void print_usage()
{
	printf("Usage: sja1105-tool status [ type ] [ options ]\n");
	printf("[ type ] can be:\n");
	printf(" * general -> General Status Information Register\n");
	printf(" * port    -> Port status Information Register\n" \
	       "              Provide Port No. as argument [0-4]\n");
}

static int get_general_status(struct sja1105_spi_setup *spi_setup,
                              char* buf, size_t len)
{
	int rc;

	rc = sysfs_read(spi_setup, "general_status", buf, len);

	return (rc > 0) ? 0 : -1;
}

static int get_port_status(struct sja1105_spi_setup *spi_setup, int port,
                           char* buf, size_t len)
{
	int rc;
	char value[2];

	snprintf(value, sizeof(value), "%i", port);
	rc = sysfs_write(spi_setup, "port_status", value, 2);
	if (rc)
		goto out;

	rc = sysfs_read(spi_setup, "port_status", buf, len);
	rc = (rc > 0) ? 0 : -1;;

out:
	return rc;
}

static int clear_port_status(struct sja1105_spi_setup *spi_setup, int port)
{
	int rc;
	char value[8];

	if (port == -1) {
		snprintf(value, sizeof(value), "all");
	} else if (port < 5) {
		snprintf(value, sizeof(value), "%i", port);
	} else {
		loge("invalid port number %d", port);
		rc = -1;
		goto out;
	}

	rc = sysfs_write(spi_setup, "port_status_clear", value, strlen(value));

out:
	return rc;
}

static int status_ports(struct sja1105_spi_setup *spi_setup,
                        int port_no)
{
	char *print_buf[5];
	/* XXX Maybe not quite right? */
	int   size = 10 * MAX_LINE_SIZE;
	int   rc;
	int   i;

	if (port_no == -1) {
		/* Show for all ports */
		for (i = 0; i < 5; i++) {
			print_buf[i] = (char*) calloc(size, sizeof(char));
		}
		for (i = 0; i < 5; i++) {
			rc = get_port_status(spi_setup, i, print_buf[i], size);
			if (rc < 0) {
				loge("sja1105_port_status_get failed");
				goto out;
			}
		}
		linewise_concat(print_buf, 5);

		for (i = 0; i < 5; i++) {
			free(print_buf[i]);
		}
	} else {
		/* Show for single port */
		print_buf[0] = (char*) calloc(size, sizeof(char));
		rc = get_port_status(spi_setup, port_no, print_buf[0], size);
		if (rc < 0) {
			loge("sja1105_port_status_get failed");
			goto out;
		}
		printf("%s\n", print_buf[0]);
		free(print_buf[0]);
	}
out:
	return rc;
}

int status_parse_args(struct sja1105_spi_setup *spi_setup,
                      int argc, char **argv)
{
	const char *options[] = {
		"general",
		"ports",
	};
	uint64_t tmp;
	int clear = 0;
	int port_no;
	int match;
	int rc = 0;
	char print_buf[2048];

	if (argc < 1) {
		rc = -EINVAL;
		goto parse_error;
	}
	match = get_match(argv[0], options, ARRAY_SIZE(options));
	if (match < 0) {
		rc = -EINVAL;
		goto parse_error;
	} else if (matches(options[match], "general") == 0) {
		rc = get_general_status(spi_setup, print_buf, 2048);
		if (rc) {
			loge("failed to get general status");
			goto error;
		}
		printf("%s", print_buf);
	} else if (matches(options[match], "ports") == 0) {
		/* Consume "ports" */
		argc--; argv++;
		/* Clear requested? Set flag, continue parsing,
		 * run command later. Consume "clear". */
		if (argc && matches(argv[0], "clear") == 0) {
			clear = 1;
			argc--; argv++;
		}
		if (argc == 0) {
			port_no = -1;
			logv("all ports");
		} else if (argc == 1) {
			rc = reliable_uint64_from_string(&tmp, argv[0], NULL);
			if (rc < 0) {
				loge("no int stored at %s", argv[0]);
				goto error;
			}
			port_no = (int) tmp;
		} else {
			rc = -EINVAL;
			goto parse_error;
		}

		if (clear) {
			rc = clear_port_status(spi_setup, port_no);
			if (rc < 0) {
				loge("failed to clear port status");
				goto error;
			}
		} else {
			rc = status_ports(spi_setup, port_no);
			if (rc < 0) {
				loge("failed to get port status");
				goto error;
			}
		}
	} else {
		rc = -EINVAL;
		goto parse_error;
	}
	return 0;
parse_error:
	print_usage();
error:
	return rc;
}

