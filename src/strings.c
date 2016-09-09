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

int matches(const char *cmd, const char *pattern)
{
	if (strlen(cmd) > strlen(pattern))
		return -1;
	return strncasecmp(pattern, cmd, strlen(cmd));
}

int get_match(const char *cmd, const char **options, int option_count)
{
	char **cmd_matches;
	int match_count = 0;
	int match_index;
	int i;

	/*if (cmd == NULL) {*/
		/*goto out_1;*/
	/*}*/
	cmd_matches = (char**) calloc(option_count, sizeof(char*));
	if (cmd_matches == NULL) {
		goto out_1;
	}
	for (i = 0; i < option_count; i++) {
		if (matches(cmd, options[i]) == 0) {
			match_index = i;
			cmd_matches[match_count++] = strdup(options[i]);
			if (strlen(cmd) == strlen(options[i])) {
				/* This is an absolute exact match, stop the search */
				match_count = 1;
				goto out;
			}
		}
	}
	if (match_count == 0) {
		printf("Token \"%s\" matches no known command. Try:\n", cmd);
		for (i = 0; i < option_count; i++) {
			printf("   * %s\n", options[i]);
		}
	} else if (match_count > 1) {
		printf("Token \"%s\" is ambiguous. It can refer to:\n", cmd);
		for (i = 0; i < match_count; i++) {
			printf("   * %s\n", cmd_matches[i]);
		}
	};
	for (i = 0; i < match_count; i++) {
		free(cmd_matches[i]);
	}
out:
	free(cmd_matches);
out_1:
	return (match_count == 1) ? match_index : -1;
}

void formatted_append(char *buffer, char *width_fmt, char *fmt, ...)
{
	char temp_buf[MAX_LINE_SIZE];
	va_list args;
	va_start(args, fmt);

	/* Print the args into temp_buf according to fmt */
	vsprintf(temp_buf, fmt, args);
	/* Append the temp_buf to the output buffer width-formatted */
	sprintf(buffer + strlen(buffer), width_fmt, temp_buf);

	va_end(args);
}

int reliable_uint64_from_string(uint64_t *to, char *from, char **endptr)
{
	int   errno_saved = errno;
	int   rc = 0;
	char *p;

	errno = 0;
	*to = strtoull(from, &p, 0);
	if (endptr != NULL) {
		*endptr = p;
	}
	if (errno) {
		fprintf(stderr, "Integer overflow occured while reading \"%s\"\n", from);
		rc = -1;
		goto out;
	}
	if (from == p) {
		/* Read nothing */
		fprintf(stderr, "No integer stored at \"%s\"\n", from);
		rc = -1;
		goto out;
	}
	errno = errno_saved;
out:
	return rc;
}

void print_array(char *print_buf, uint64_t *array, int count)
{
	int written;
	char *p;
	int i;

	written = snprintf(print_buf, MAX_LINE_SIZE, "[");
	p = print_buf + written;
	for (i = 0; i < count; i++) {
		written += snprintf(p, MAX_LINE_SIZE - written, "0x%" PRIX64 " ", array[i]);
		p = print_buf + written;
	}
	snprintf(p, MAX_LINE_SIZE, "]");
}

int read_array(char *array_str, uint64_t *array_val, int max_count)
{
	int   count;
	int   rc;
	char *p;

	p = trimwhitespace(array_str);
	if (p[0] != '[') {
		fprintf(stderr, "Array must contain space-separated elements "
		        "inside brackets, like [1 2 3]\n");
		rc = -1;
		goto out;
	}
	/* Strip initial '[' character */
	p++;
	count = 0;
	while (p != NULL && *p != ']') {
		rc = reliable_uint64_from_string(&array_val[count], p, &p);
		if (rc != 0) {
			goto out;
		}
		p = trimwhitespace(p);
		if (p == NULL) {
			fprintf(stderr, "End of array detected but no \"]\" present!\n");
			goto out;
		}
		count++;
		if (count > max_count) {
			fprintf(stderr, "Input array larger than %d elements!\n", max_count);
			rc = -1;
			goto out;
		}
	}
	rc = count;
out:
	return rc;
}

void linewise_concat(char **buffers, int count)
{
	char **current_line, **next_line;
	int i;

	next_line    = (char**) calloc(count, sizeof(char*));
	current_line = (char**) calloc(count, sizeof(char*));

	for (i = 0; i < count; i++) {
		current_line[i] = buffers[i];
	}
	while (current_line[0]) {
		for (i = 0; i < count; i++) {
			next_line[i] = strchr(current_line[i], '\n');
			if (next_line[i])
				*next_line[i] = '\0';

			if (strlen(current_line[i])) {
				if (i == 0) {
					printf("|| ");
				}
				printf("%s || ", current_line[i]);
			}

			if (next_line[i])
				current_line[i] = next_line[i] + 1;
			else
				current_line[i] = NULL;
		}
		if (current_line[0]) {
			printf("\n");
		}
	}
	free(next_line);
	free(current_line);
}

/*
 * Courtesy of:
 * http://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
 * Note: This function returns a pointer to a substring of the original string.
 * If the given string was allocated dynamically, the caller must not overwrite
 * that pointer with the returned value, since the original pointer must be
 * deallocated using the same allocator with which it was allocated.  The return
 * value must NOT be deallocated using free() etc.
 */
char *trimwhitespace(char *str)
{
	char *end;

	if (str == NULL) {
		return NULL;
	}
	/* Trim leading space */
	while (isspace((unsigned char) *str)) {
		str++;
	}
	/* All spaces? */
	if (*str == 0) {
		return str;
	}
	/* Trim trailing space */
	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char) *end)) {
		end--;
	}
	/* Write new null terminator */
	*(end + 1) = 0;
	return str;
}

int get_multiline_buf_width(char *buf)
{
	int max_len = 0;
	int line_len;
	char *p;

	while (*buf) {
		p = strchrnul(buf, '\n');
		line_len = (p - buf);
		if (max_len < line_len) {
			max_len = line_len;
		}
		buf = (*p == '\n' ? p + 1 : p);
	}
	return max_len;
}

int get_entry_count_to_fit_screen(char **print_bufs, int count)
{
	int num_entries;
	int total_line_width;
	int line_width;

	if (count == 0) {
		return 0;
	}
	total_line_width = get_multiline_buf_width(print_bufs[0]);
	num_entries      = 1;
	while (num_entries < count) {
		line_width = get_multiline_buf_width(print_bufs[num_entries]);
		if (total_line_width + line_width < general_config.screen_width) {
			total_line_width += line_width;
			num_entries++;
		} else {
			break;
		}
	}
	return num_entries;
}

void show_print_bufs(char **print_bufs, int count)
{
	int max_entries_to_fit_screen;
	int increment;
	int i = 0;

	while (i < count) {
		max_entries_to_fit_screen = get_entry_count_to_fit_screen(print_bufs + i, count - i);
		increment = min(count - i, general_config.entries_per_line);
		increment = min(increment, max_entries_to_fit_screen);
		linewise_concat(print_bufs + i, increment);
		i += increment;
	}
}

