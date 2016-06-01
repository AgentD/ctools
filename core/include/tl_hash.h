/*
 * tl_hash.h
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

/**
 * \file tl_hash.h
 *
 * \brief Contains hash functions
 */
#ifndef TL_HASH_H
#define TL_HASH_H

/**
 * \page hash Hash and Check Sum Algorithms
 *
 * There are several different non-cryptographic hash functions available.
 * Non-cryptographic means that it is comparatively easy for an attacker to
 * manipulate data in a way that it still produces the same hash value. The
 * hash functions are intended for use with hash maps/tables, the check sum
 * algorithms are intended for error detection in transmissions (but can also
 * be used as hash function) with the main goal of being fast to compute and
 * still provide good distribution and collision resistance.
 *
 * The following functions are available:
 * \li The function \ref tl_hash_murmur3_32 computes a 32 bit MurmurHash3 hash
 * \li The function \ref tl_hash_crc32 computes a 32 bit cyclic redundancy
 *     check sum
 */



#include "tl_predef.h"



#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Compute a 32 bit MurmurHash 3 hash value of a block of data
 *
 * The Murmur family of hash functions is designed for storing arbitrary data
 * in hash tables/maps. The MurmurHash 3 function has good distribution, good
 * avalanche behavior and good collision resistance. It strikes a good balance
 * between performance and hash-quality and is widely used in a number of
 * software projects (e.g. libstdc++, Perl, nginx, ...).
 *
 * \note The hash function accepts a "seed" value. This value can be used to
 *       randomize the hash function when using it for a hash table. If an
 *       attacker knows the hash-table implementation and hash function, they
 *       can produce input that degrades the map to a list in order to DOS an
 *       application. An unpredictable seed ensures that an attacker cannot
 *       guess the hash values even if they know the hash function that is
 *       used.
 *
 * \param data The block of data to process
 * \param len  The size fo the block in bytes
 * \param seed A seed value to start the hash computation with. A different
 *             value can be used per application run to randomize the hashes.
 *
 * \return The calculated hash value
 */
TLAPI tl_u32 tl_hash_murmur3_32( const void* data, size_t len, tl_u32 seed );

/**
 * \brief Compute the CRC-32 sum of a block of data
 *
 * CRC stands for cyclic redundancy check and is a simple check sum for error
 * detection typically used by many protocols and file formats, but can also
 * be used as a simple hash function. The CRC-32 produces a 32 bit check sum.
 * The simplest possible CRC would be a CRC-1 which is the equivalent of
 * a parity bit.
 *
 * \param crc  The result value of the previous block. Typically 0 or
 *             all bits 1 for the initial block.
 * \param data The block of data to process
 * \param len  The size of the block in bytes
 *
 * \return The calculated check sum
 */
TLAPI tl_u32 tl_hash_crc32( tl_u32 crc, const void* data, size_t len );

#ifdef __cplusplus
}
#endif

#endif /* TL_HASH_H */

