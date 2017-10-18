/* addr_v6.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "../platform.h"
#include "bsdsock.h"

#if defined(_WIN32) && !defined(s6_addr)
#define s6_addr u.Byte
#endif

void convert_ipv6(const struct in6_addr *v6, tl_net_addr *addr)
{
	addr->addr.ipv6[7] = (v6->s6_addr[0] << 8) | v6->s6_addr[1];
	addr->addr.ipv6[6] = (v6->s6_addr[2] << 8) | v6->s6_addr[3];
	addr->addr.ipv6[5] = (v6->s6_addr[4] << 8) | v6->s6_addr[5];
	addr->addr.ipv6[4] = (v6->s6_addr[6] << 8) | v6->s6_addr[7];
	addr->addr.ipv6[3] = (v6->s6_addr[8] << 8) | v6->s6_addr[9];
	addr->addr.ipv6[2] = (v6->s6_addr[10] << 8) | v6->s6_addr[11];
	addr->addr.ipv6[1] = (v6->s6_addr[12] << 8) | v6->s6_addr[13];
	addr->addr.ipv6[0] = (v6->s6_addr[14] << 8) | v6->s6_addr[15];
}

void convert_in6addr(const tl_net_addr *addr, struct in6_addr *v6)
{
	v6->s6_addr[0] = (addr->addr.ipv6[7] >> 8) & 0xFF;
	v6->s6_addr[1] = addr->addr.ipv6[7] & 0xFF;
	v6->s6_addr[2] = (addr->addr.ipv6[6] >> 8) & 0xFF;
	v6->s6_addr[3] = addr->addr.ipv6[6] & 0xFF;
	v6->s6_addr[4] = (addr->addr.ipv6[5] >> 8) & 0xFF;
	v6->s6_addr[5] = addr->addr.ipv6[5] & 0xFF;
	v6->s6_addr[6] = (addr->addr.ipv6[4] >> 8) & 0xFF;
	v6->s6_addr[7] = addr->addr.ipv6[4] & 0xFF;
	v6->s6_addr[8] = (addr->addr.ipv6[3] >> 8) & 0xFF;
	v6->s6_addr[9] = addr->addr.ipv6[3] & 0xFF;
	v6->s6_addr[10] = (addr->addr.ipv6[2] >> 8) & 0xFF;
	v6->s6_addr[11] = addr->addr.ipv6[2] & 0xFF;
	v6->s6_addr[12] = (addr->addr.ipv6[1] >> 8) & 0xFF;
	v6->s6_addr[13] = addr->addr.ipv6[1] & 0xFF;
	v6->s6_addr[14] = (addr->addr.ipv6[0] >> 8) & 0xFF;
	v6->s6_addr[15] = addr->addr.ipv6[0] & 0xFF;
}
