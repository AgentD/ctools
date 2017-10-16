/* murmur3.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_hash.h"
#include <string.h>

#define MM3_C1 0xCC9E2D51
#define MM3_C2 0x1B873593
#define MM3_R1 15
#define MM3_R2 13
#define MM3_M 5
#define MM3_N 0xE6546B64

#ifdef TL_ALLIGN_MEMORY
static tl_u32 read32_unaligned(const void *buf)
{
	tl_u32 ret;
	memcpy(&ret, buf, 4);
	return ret;
}
#endif

tl_u32 tl_hash_murmur3_32(const void *data, size_t len, tl_u32 seed)
{
	const tl_u32 *blocks = (const tl_u32 *)data;
	tl_u32 hash = seed, k1 = 0, k;
	size_t nblocks = len / 4, i;
	const unsigned char *tail;

	for (i = 0; i < nblocks; ++i) {
#ifdef TL_ALLIGN_MEMORY
		if (((size_t)blocks) % sizeof(tl_u32))
			k = read32_unaligned(blocks + i);
		else
#endif
			k = blocks[i];

		k *= MM3_C1;
		k = (k << MM3_R1) | (k >> (32 - MM3_R1));
		k *= MM3_C2;

		hash ^= k;
		hash = ((hash << MM3_R2) | (hash >> (32 - MM3_R2)));
		hash = hash * MM3_M + MM3_N;
	}

	tail = (const unsigned char *)data + nblocks * 4;

	switch (len & 3) {
	case 3:
		k1 ^= tail[2] << 16; /* fall-through */
	case 2:
		k1 ^= tail[1] << 8;  /* fall-through */
	case 1:
		k1 ^= tail[0];       /* fall-through */
		k1 *= MM3_C1;
		k1 = (k1 << MM3_R1) | (k1 >> (32 - MM3_R1));
		k1 *= MM3_C2;
		hash ^= k1;
	}

	hash ^= len;
	hash ^= (hash >> 16);
	hash *= 0x85EBCA6B;
	hash ^= (hash >> 13);
	hash *= 0xC2B2AE35;
	hash ^= (hash >> 16);
	return hash;
}
