/*
 * hash.c
 * This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#define TL_EXPORT
#include "tl_hash.h"
#include <string.h>

/*
    Karl Malbrain's compact CRC-32. See "A compact CCITT crc16 and crc32 C
    implementation that balances processor cache usage against speed"
 */
static const tl_u32 s_crc32[16] =
{
    0x00000000, 0x1DB71064, 0x3B6E20C8, 0x26D930AC,
    0x76DC4190, 0x6B6B51F4, 0x4DB26158, 0x5005713C,
    0xEDB88320, 0xF00F9344, 0xD6D6A3E8, 0xCB61B38C,
    0x9B64C2B0, 0x86D3D2D4, 0xA00AE278, 0xBDBDF21C
};

tl_u32 tl_hash_crc32( tl_u32 crc, const void* data, size_t len )
{
    const tl_u8* ptr = data;
    tl_u8 b;
    crc = ~crc;
    while( len-- )
    {
        b = *ptr++;
        crc = (crc >> 4) ^ s_crc32[ (crc & 0x0F) ^ (b & 0x0F) ];
        crc = (crc >> 4) ^ s_crc32[ (crc & 0x0F) ^ (b >> 4)   ];
    }
    return ~crc;
}



#define MM3_C1 0xCC9E2D51
#define MM3_C2 0x1B873593
#define MM3_R1 15
#define MM3_R2 13
#define MM3_M 5
#define MM3_N 0xE6546B64

#ifdef TL_ALLIGN_MEMORY
static tl_u32 read32_unaligned( const void* buf )
{
    tl_u32 ret;
    memcpy( &ret, buf, 4 );
    return ret;
}
#endif

tl_u32 tl_hash_murmur3_32( const void* data, size_t len, tl_u32 seed )
{
    const tl_u32* blocks = (const tl_u32*)data;
    tl_u32 hash = seed, k1 = 0, k;
    size_t nblocks = len / 4, i;
    const unsigned char* tail;

    for( i=0; i<nblocks; ++i )
    {
    #ifdef TL_ALLIGN_MEMORY
        if( ((size_t)blocks) % sizeof(tl_u32) )
        {
            k = read32_unaligned( blocks+i );
        }
        else
    #endif
        {
            k = blocks[i];
        }

        k *= MM3_C1;
        k = (k << MM3_R1) | (k >> (32 - MM3_R1));
        k *= MM3_C2;

        hash ^= k;
        hash = ((hash << MM3_R2) | (hash >> (32 - MM3_R2)));
        hash = hash * MM3_M + MM3_N;
    }

    tail = (const unsigned char*)data + nblocks*4;

    switch( len & 3 )
    {
    case 3: k1 ^= tail[2] << 16;
    case 2: k1 ^= tail[1] << 8;
    case 1: k1 ^= tail[0];
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

