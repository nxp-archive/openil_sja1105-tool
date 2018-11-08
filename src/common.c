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
#include "common.h"

void print_array(char *print_buf, uint64_t *array, int count)
{
	int written;
	char *p;
	int i;

	written = snprintf(print_buf, MAX_LINE_SIZE, "[");
	p = print_buf + written;
	for (i = 0; i < count; i++) {
		written += snprintf(p, MAX_LINE_SIZE - written,
		                    "0x%" PRIX64 " ", array[i]);
		p = print_buf + written;
	}
	snprintf(p, MAX_LINE_SIZE, "]");
}

void formatted_append(char *buffer, size_t len, char *width_fmt, char *fmt, ...)
{
	char *temp_buf;
	size_t old_len;
	va_list args;

	/* Allocate temp_buf from heap to save stack size on kmod build */
	temp_buf = calloc(1, MAX_LINE_SIZE);
	if (!temp_buf)
		return;

	va_start(args, fmt);

	/* Print the args into temp_buf according to fmt */
	vsnprintf(temp_buf, MAX_LINE_SIZE, fmt, args);
	/* Append the temp_buf to the output buffer width-formatted */
	old_len = strlen(buffer);
	snprintf(buffer + old_len, len - old_len, width_fmt, temp_buf);

	va_end(args);

	free(temp_buf);
}

