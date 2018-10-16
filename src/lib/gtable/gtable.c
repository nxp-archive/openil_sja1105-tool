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
#include <lib/include/gtable.h>
#include <common.h>

/* these are *inclusive* */
#define ONES_TO_RIGHT_OF(x) ((1ull << ((x) + 1)) - 1)
#define ONES_TO_LEFT_OF(x) (~((1ull << (x)) - 1))

#define exchg(type, x, y) \
	{ \
		type z; \
		z = x; \
		x = y; \
		y = z; \
	}

int g_quirks = QUIRK_LSW32_IS_FIRST;

enum gtable_operation {
	GTABLE_PACK,
	GTABLE_UNPACK,
};

static inline int
get_le_offset(int offset)
{
	int closest_multiple_of_4;

	closest_multiple_of_4 = (offset / 4) * 4;
	offset -= closest_multiple_of_4;
	return closest_multiple_of_4 + (3 - offset);
}

static inline int
get_reverse_lsw32_offset(int offset, int len)
{
	int closest_multiple_of_4;
	int word_index;

	word_index = offset / 4;
	closest_multiple_of_4 = word_index * 4;
	offset -= closest_multiple_of_4;
	word_index = (len / 4) - word_index - 1;
	return word_index * 4 + offset;
}

static inline uint64_t
bit_reverse(uint64_t val, unsigned int width)
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

static inline void
correct_for_msb_right_quirk(
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
	exchg(int, *box_bit_start, *box_bit_end);
	*box_bit_mask  = ONES_TO_RIGHT_OF(*box_bit_start) &
			 ONES_TO_LEFT_OF(*box_bit_end);
}

static inline int
gtable_field_access(
		void     *table,
		uint64_t *value,
		int       tbl_bit_start,
		int       tbl_bit_end,
		int       tbl_len_bytes,
		enum      gtable_operation op,
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
		loge("gtable_access: invalid function call");
		return -EINVAL;
	}

	value_width = tbl_bit_start - tbl_bit_end + 1;
	if (value_width > 64) {
		loge("gtable_access: field %d-%d too large for 64 bits!",
		     tbl_bit_start, tbl_bit_end);
		return -ERANGE;
	}

	/* Check if "value" fits in "value_width" bits.
	 * If value_width is 64, the check will fail, but any
	 * 64-bit value will surely fit. */
	if ((op == GTABLE_PACK) &&
	    (value_width < 64) &&
	    (*value >= (1ull << value_width))) {
		loge("gtable_access: Warning, cannot store %" PRIX64
		     " inside %" PRIu64 " bits!", *value, value_width);
		*value &= (1ull << value_width) - 1;
		loge("Truncated value to %" PRIX64 ", this may not be "
		     "what you want.", *value);
	}
	/* Initialize parameter */
	if (op == GTABLE_UNPACK) {
		*value = 0;
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
		if (op == GTABLE_UNPACK) {
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

inline int
gtable_unpack(void *buf, uint64_t *value, int start, int end,
              int len_bytes)
{
	return gtable_field_access(buf, value, start, end,
	                           len_bytes, GTABLE_UNPACK, g_quirks);
}

inline int
gtable_pack(void *buf, uint64_t *value, int start, int end,
            int len_bytes)
{
	return gtable_field_access(buf, value, start, end,
	                           len_bytes, GTABLE_PACK, g_quirks);
}

void gtable_hexdump(void *table, int len)
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

void gtable_bitdump(void *table, int len)
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

int gtable_configure(int quirks)
{
	g_quirks = quirks;
	/* Check that no other one-hot bits are set */
	quirks &= ~(1 << QUIRK_LSW32_IS_FIRST);
	quirks &= ~(1 << QUIRK_LITTLE_ENDIAN);
	quirks &= ~(1 << QUIRK_MSB_ON_THE_RIGHT);
	return (quirks == 0) ? 0 : -EINVAL;
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
		gtable_unpack(buf + i, &chunk, 31, 0, 4);
		crc = crc32_add(crc, chunk & 0xFF);
		crc = crc32_add(crc, (chunk >> 8) & 0xFF);
		crc = crc32_add(crc, (chunk >> 16) & 0xFF);
		crc = crc32_add(crc, (chunk >> 24) & 0xFF);
	}
	return bit_reverse(~crc, 32);
}

