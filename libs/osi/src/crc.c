/*
 * Copyright 2011-2017 Ayla Networks, Inc.  All rights reserved.
 *
 * Use of the accompanying software is permitted only in accordance
 * with and subject to the terms of the Software License Agreement
 * with Ayla Networks, Inc., a copy of which can be obtained from
 * Ayla Networks, Inc.
 */
#include <stdio.h>
#include <stddef.h>
#include "common.h"
#include "crc.h"


/*
 * 4-bit CRC table.
 * Each entry in the table is the change to the CRC for each
 * of the 4 bit values.
 */
static const u8 crc8_table[16] = {
	0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15,
	0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d
};


/*
 * Compute CRC-8 with polynomial 7 (x^8 + x^2 + x + 1).
 * MSB-first.  Use 4-bit table.
 */
u8 crc8(const void *buf, size_t len, u8 crc)
{
	const u8 *bp = buf;

	while (len-- > 0) {
		crc ^= *bp++;
		crc = (crc << 4) ^ crc8_table[crc >> 4];
		crc = (crc << 4) ^ crc8_table[crc >> 4];
	}
	return crc;
}


static const u16 crc16_table[16] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef
};

/*
 * Compute CRC-16 with IEEE polynomial
 * LSB-first.
 */
u16 crc16(const void *buf, size_t len, u16 crc)
{
	const u8 *bp = buf;

	while (len-- > 0) {
		crc ^= *bp++ << 8;
		crc = (crc << 4) ^ crc16_table[crc >> 12];
		crc = (crc << 4) ^ crc16_table[crc >> 12];
	}
	return crc;
}

static const u32 crc32_table[16] = {
	0, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
	0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
	0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
	0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c,
};

/*
 * Compute CRC-32 with IEEE polynomial
 * LSB-first.
 */
u32 crc32(const void *buf, size_t len, u32 crc)
{
	const u8 *bp = buf;

	while (len-- > 0) {
		crc ^= *bp++;
		crc = (crc >> 4) ^ crc32_table[crc & 0xf];
		crc = (crc >> 4) ^ crc32_table[crc & 0xf];
	}
	return crc;
}
