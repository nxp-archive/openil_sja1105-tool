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
#ifndef _SJA1105_TOOL_COMMON_H
#define _SJA1105_TOOL_COMMON_H

 #include <inttypes.h>
 #include <stdint.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include <stdarg.h>
 #include <errno.h>
 #include <stddef.h>
 #include <unistd.h>

/* These are our own error codes */
#include <lib/include/errors.h>

/* Since remapping is used internally, and many checks
 * search for a negative return code, we do that here.
 * The sign is flipped again when returning the error
 * to userspace.
 */
#define sja1105_err_remap(old_err, new_err)           \
	do {                                          \
		logv("Remapping error code %d to %d", \
		    (old_err), (new_err));            \
		old_err = -new_err;                   \
	} while (0);

#define MAX_LINE_SIZE 2048

/* Macros for conditional, error, verbose and debug logging */
extern int SJA1105_DEBUG_CONDITION;
extern int SJA1105_VERBOSE_CONDITION;

#define _log(file, fmt, ...) do {                       \
	if (SJA1105_DEBUG_CONDITION) {                  \
		fprintf(file, "%s@%d: %s: " fmt "\n",   \
		        __FILE__, __LINE__, __func__,   \
		        ##__VA_ARGS__);                 \
	} else {                                        \
		fprintf(file, fmt "\n", ##__VA_ARGS__); \
	}                                               \
} while(0);

#define logc(file, condition, ...) do {                 \
	if (condition) {                                \
		_log(file, __VA_ARGS__);                \
	}                                               \
} while(0);

#define loge(...) _log(stderr, __VA_ARGS__)
#define logi(...) _log(stdout, __VA_ARGS__)
#define logv(...) logc(stdout, SJA1105_VERBOSE_CONDITION, __VA_ARGS__);

void formatted_append(char *buffer, size_t len, char *width_fmt, char *fmt, ...);
void print_array(char *print_buf, uint64_t *array, int count);

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define min(x, y) (((x) < (y)) ? (x) : (y))


#define DEFINE_PACK_UNPACK_ACCESSORS(device, table)                                \
                                                                                   \
	void sja1105##device##_##table##_entry_pack(void *buf,                     \
	                           struct sja1105_##table##_entry *entry)          \
	{                                                                          \
		sja1105##device##_##table##_entry_access(buf, entry, 1);           \
	}                                                                          \
                                                                                   \
	void sja1105##device##_##table##_entry_unpack(void *buf,                   \
	                           struct sja1105_##table##_entry *entry)          \
	{                                                                          \
		sja1105##device##_##table##_entry_access(buf, entry, 0);           \
	}

#define DEFINE_COMMON_PACK_UNPACK_ACCESSORS(table)                                 \
	DEFINE_PACK_UNPACK_ACCESSORS(, table);                                     \

#define DEFINE_SEPARATE_PACK_UNPACK_ACCESSORS(table)                               \
	DEFINE_PACK_UNPACK_ACCESSORS(et, table);                                   \
	DEFINE_PACK_UNPACK_ACCESSORS(pqrs, table);                                 \

#endif
