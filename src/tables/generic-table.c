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

/* these are *inclusive* */
#define ONES_TO_RIGHT_OF(x) ((1ull << ((x) + 1)) - 1)
#define ONES_TO_LEFT_OF(x) (~((1ull << (x)) - 1))

#define xchg(type, x, y) \
	{ \
		type z; \
		z = x; \
		x = y; \
		y = z; \
	}

/*
 * Bit 63 always means bit offset 7 of byte 7, albeit only logically.
 * The question is: where do we lay this bit out in memory?
 * Normally, we would do it like this:
 *
 * 63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32
 * 7                       6                       5                        4
 * 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 * 3                       2                       1                        0
 *
 *
 * If QUIRK_MSB_ON_THE_RIGHT is set, we do it like this:
 *
 * 56 57 58 59 60 61 62 63 48 49 50 51 52 53 54 55 40 41 42 43 44 45 46 47 32 33 34 35 36 37 38 39
 * 7                       6                        5                       4
 * 24 25 26 27 28 29 30 31 16 17 18 19 20 21 22 23  8  9 10 11 12 13 14 15  0  1  2  3  4  5  6  7
 * 3                       2                        1                       0
 *
 *
 * If QUIRK_LITTLE_ENDIAN is set, we do it like this:
 *
 * 39 38 37 36 35 34 33 32 47 46 45 44 43 42 41 40 55 54 53 52 51 50 49 48 63 62 61 60 59 58 57 56
 * 4                       5                       6                       7
 * 7  6  5  4  3  2  1  0  15 14 13 12 11 10  9  8 23 22 21 20 19 18 17 16 31 30 29 28 27 26 25 24
 * 0                       1                       2                       3
 *
 *
 * If QUIRK_MSB_ON_THE_RIGHT and QUIRK_LITTLE_ENDIAN are both set, we do it like this:
 *
 * 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63
 * 4                       5                       6                       7
 * 0  1  2  3  4  5  6  7  8   9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
 * 0                       1                       2                       3
 *
 *
 * If just QUIRK_LSW32_IS_FIRST is set, we do it like this:
 *
 * 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
 * 3                       2                       1                        0
 * 63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32
 * 7                       6                       5                        4
 *
 *
 * If QUIRK_LSW32_IS_FIRST and QUIRK_MSB_ON_THE_RIGHT are set, we do it like this:
 *
 * 24 25 26 27 28 29 30 31 16 17 18 19 20 21 22 23  8  7  6  2 11 10  9  8  7  6  5  4  3  2  1  0
 * 3                       2                        1                       0
 * 56 57 58 59 60 61 62 63 48 49 50 51 52 53 54 55 40 41 42 43 44 45 46 47 32 33 34 35 36 37 38 39
 * 7                       6                        5                       4
 *
 *
 * If QUIRK_LSW32_IS_FIRST and QUIRK_LITTLE_ENDIAN are set, it looks like this:
 *
 * 7  6  5  4  3  2  1  0  15 14 13 12 11 10  9  8 23 22 21 20 19 18 17 16 31 30 29 28 27 26 25 24
 * 0                       1                       2                       3
 * 39 38 37 36 35 34 33 32 47 46 45 44 43 42 41 40 55 54 53 52 51 50 49 48 63 62 61 60 59 58 57 56
 * 4                       5                       6                       7
 *
 *
 * If QUIRK_LSW32_IS_FIRST, QUIRK_LITTLE_ENDIAN and QUIRK_MSB_ON_THE_RIGHT are set, it looks like this:
 *
 * 0  1  2  3  4  5  6  7  8   9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
 * 0                       1                       2                       3
 * 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63
 * 4                       5                       6                       7
 *
 *
 * We always think of our offsets as if there were no quirk,
 * and we translate them afterwards, before accessing the table.
 */
#define QUIRK_MSB_ON_THE_RIGHT (1 << 0ull)
#define QUIRK_LITTLE_ENDIAN    (1 << 1ull)
#define QUIRK_LSW32_IS_FIRST   (1 << 2ull)

static int get_le_offset(int offset)
{
	int closest_multiple_of_4;

	closest_multiple_of_4 = (offset / 4) * 4;
	offset -= closest_multiple_of_4;
	return closest_multiple_of_4 + (3 - offset);
}

static int get_reverse_lsw32_offset(int offset, int len)
{
	int closest_multiple_of_4;
	int word_index;

	word_index = offset / 4;
	closest_multiple_of_4 = word_index * 4;
	offset -= closest_multiple_of_4;
	word_index = (len / 4) - word_index - 1;
	return word_index * 4 + offset;
}

static uint64_t bit_reverse(uint64_t val, unsigned int width)
{
	uint64_t new_val = 0;
	unsigned int bit;
	unsigned int i;

	for (i = 0; i < width; i++) {
		bit = (val & (1 << i)) != 0;
		new_val |= (bit << (width - i - 1));
	}
	return new_val;
}

static void correct_for_msb_right_quirk(
		uint64_t *to_write,
		int      *box_bit_start,
		int      *box_bit_end,
		uint8_t  *box_bit_mask)
{
	int box_bit_width = *box_bit_start - *box_bit_end + 1;
	*to_write >>= *box_bit_end;
	*to_write = bit_reverse(*to_write, box_bit_width);
	*to_write <<= *box_bit_end;
	*box_bit_start = box_bit_width - *box_bit_start - 1;
	*box_bit_end   = box_bit_width - *box_bit_end - 1;
	xchg(int, *box_bit_start, *box_bit_end);
	*box_bit_mask  = ONES_TO_RIGHT_OF(*box_bit_start) &
			 ONES_TO_LEFT_OF(*box_bit_end);
}

static int generic_table_field_access(
		void     *table,
		uint64_t *value,
		int       tbl_bit_start,
		int       tbl_bit_end,
		int       tbl_len_bytes,
		int       write,
		uint64_t  quirks)
{
	/* Number of bits for storing "value"
	 * also width of the field to access in the table */
	uint64_t value_width;
	/* Logical byte indices corresponding to the
	 * start and end of the field. */
	int      tbl_byte_start;
	int      tbl_byte_end;
	/* Bit indices into the currently accessed 8-bit box */
	int      box_bit_start;
	int      box_bit_end;
	uint8_t  box_bit_mask;
	int      box_addr;
	int      box;
	/* Corresponding bits from the value parameter */
	int      home_bit_start;
	int      home_bit_end;
	uint64_t home_bit_mask;
	uint64_t tbl_to_write;
	uint64_t value_to_write;

	tbl_byte_start = tbl_bit_start / 8;
	tbl_byte_end   = tbl_bit_end / 8;

	/* tbl_bit_start is expected to be larger than tbl_bit_end */
	if (tbl_bit_start < tbl_bit_end) {
		fprintf(stderr, "generic_table_access: invalid function call");
		return -1;
	}

	value_width = tbl_bit_start - tbl_bit_end + 1;
	if (value_width > 64) {
		fprintf(stderr, "generic_table_access: field %d-%d "
		        "too large for 64 bits!\n",
		        tbl_bit_start, tbl_bit_end);
		return -1;
	}

	if ((write == 1) && (*value >= (1ull << value_width))) {
		printf("generic_table_access: Warning, cannot store %" PRIX64
		       " inside %" PRIu64 " bits!\n",
		       *value, value_width);
		*value &= (1ull << value_width) - 1;
		printf("Truncated value to %" PRIX64 ", this may not be "
		       "what you want.\n", *value);
	}

	for (box = tbl_byte_start; box >= tbl_byte_end; box--) {
		if (box == tbl_byte_start) {
			box_bit_start = tbl_bit_start % 8;
		} else {
			box_bit_start = 7;
		}
		if (box == tbl_byte_end) {
			box_bit_end = tbl_bit_end % 8;
		} else {
			box_bit_end = 0;
		}
		home_bit_start = ((box * 8) + box_bit_start) - tbl_bit_end;
		home_bit_end   = ((box * 8) + box_bit_end) - tbl_bit_end;
		home_bit_mask = ONES_TO_RIGHT_OF(home_bit_start) &
		                ONES_TO_LEFT_OF(home_bit_end);
		box_bit_mask  = ONES_TO_RIGHT_OF(box_bit_start) &
		                ONES_TO_LEFT_OF(box_bit_end);

		box_addr = tbl_len_bytes - box - 1;
		if (quirks & QUIRK_LITTLE_ENDIAN) {
			box_addr = get_le_offset(box_addr);
		}
		if (quirks & QUIRK_LSW32_IS_FIRST) {
			box_addr = get_reverse_lsw32_offset(box_addr, tbl_len_bytes);
		}
		if (write == 0) {
			/* Read from table, write to output argument "value" */
			value_to_write = ((uint8_t*) table)[box_addr] & box_bit_mask;
			if (quirks & QUIRK_MSB_ON_THE_RIGHT) {
				correct_for_msb_right_quirk(
						&value_to_write,
						&box_bit_start,
						&box_bit_end,
						&box_bit_mask);
			}
			value_to_write >>= box_bit_end;
			value_to_write <<= home_bit_end;
			*value &= ~home_bit_mask;
			*value |= value_to_write;
		} else {
			/* Write to table, read from input argument "value" */
			tbl_to_write = (*value) & home_bit_mask;
			tbl_to_write >>= home_bit_end;
			if (quirks & QUIRK_MSB_ON_THE_RIGHT) {
				correct_for_msb_right_quirk(
						&tbl_to_write,
						&box_bit_start,
						&box_bit_end,
						&box_bit_mask);
			}
			tbl_to_write <<= box_bit_end;
			((uint8_t*) table)[box_addr] &= ~box_bit_mask;
			((uint8_t*) table)[box_addr] |= tbl_to_write;
		}
	}
	return 0;
}

int generic_table_field_get(
		void     *table,
		uint64_t *value,
		int       start,
		int       end,
		int       len_bytes)
{
	return generic_table_field_access(table, value, start, end,
	                           len_bytes, 0, QUIRK_LSW32_IS_FIRST);
}

int generic_table_field_set(
		void     *table,
		uint64_t *value,
		int       start,
		int       end,
		int       len_bytes)
{
	return generic_table_field_access(table, value, start, end,
	                           len_bytes, 1, QUIRK_LSW32_IS_FIRST);
}

void generic_table_hexdump(void *table, int len)
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

void generic_table_bitdump(void *table, int len)
{
	uint8_t *p = (uint8_t*) table;
	int i, bit;

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
		for (bit = 7; bit >= 0; bit--) {
			printf("%d", (p[i] & (1 << bit)) >> bit);
		}
		printf(" ");
	}
	printf("\n");
}

static uint32_t crc32_add(uint32_t crc, uint8_t byte)
{
	const uint32_t ether_crc32_poly = 0x04C11DB7;
	uint32_t byte32 = bit_reverse(byte, 32);
	int i;

	for (i = 0; i < 8; i++) {
		if ((crc ^ byte32) & (1 << 31)) {
			crc <<= 1;
			crc ^= ether_crc32_poly;
		} else {
			crc <<= 1;
		}
		byte32 <<= 1;
	}
	return crc;
}

uint32_t ether_crc32_le(void *buf, unsigned int len)
{
	unsigned int i;
	uint64_t chunk;
	uint32_t crc;

	/* seed */
	crc = 0xFFFFFFFF;
	for (i = 0; i < len; i += 4) {
		generic_table_field_get(
				buf + i,
				&chunk,
				31, 0,
				4);
		crc = crc32_add(crc, chunk & 0xFF);
		crc = crc32_add(crc, (chunk >> 8) & 0xFF);
		crc = crc32_add(crc, (chunk >> 16) & 0xFF);
		crc = crc32_add(crc, (chunk >> 24) & 0xFF);
	}
	return bit_reverse(~crc, 32);
}

