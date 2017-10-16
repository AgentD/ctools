/* network.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_network.h"

#include <assert.h>
#include <string.h>

int tl_network_get_special_address(tl_net_addr *addr, int type, int net)
{
	assert(addr);

	switch (net) {
	case TL_IPV4:
		switch (type) {
		case TL_LOOPBACK:
			addr->addr.ipv4 = (127 << 24) | (1);
			break;
		case TL_BROADCAST:
			addr->addr.ipv4 = 0xFFFFFFFF;
			break;
		case TL_ALL:
			addr->addr.ipv4 = 0;
			break;
		default:
			return 0;
		}
		break;
	case TL_IPV6:
		switch (type) {
		case TL_LOOPBACK:
			memset(addr->addr.ipv6, 0, sizeof(addr->addr.ipv6));
			addr->addr.ipv6[0] = 0x0001;
			break;
		case TL_ALL:
			memset(addr->addr.ipv6, 0, sizeof(addr->addr.ipv6));
			break;
		default:
			return 0;
		}
		break;
	default:
		return 0;
	}

	addr->net = net;
	return 1;
}

static int is_actually_v4(const tl_net_addr *a)
{
	if (a->addr.ipv6[7] || a->addr.ipv6[6])
		return 0;
	if (a->addr.ipv6[5] || a->addr.ipv6[4])
		return 0;
	/* IPv4-mapped-IPv6 address */
	if (a->addr.ipv6[3] == 0x0000 && a->addr.ipv6[2] == 0xFFFF)
		return 1;
	/* SIIT address */
	if (a->addr.ipv6[3] == 0xFFFF && a->addr.ipv6[2] == 0x0000)
		return 1;
	return 0;
}

int tl_net_addr_convert(tl_net_addr *dst, const tl_net_addr *src, int target)
{
	if (src->net == target) {
		memcpy(dst, src, sizeof(*src));
		return 1;
	}

	switch (target) {
	case TL_IPV4:
		if (src->net != TL_IPV6)
			return 0;
		if (!is_actually_v4(src))
			return 0;
		dst->addr.ipv4 = ((tl_u32) src->addr.ipv6[1] << 16) |
				 ((tl_u32) src->addr.ipv6[0]);
		break;
	case TL_IPV6:
		if (src->net != TL_IPV4)
			return 0;
		memset(dst->addr.ipv6, 0, sizeof(dst->addr.ipv6));
		dst->addr.ipv6[2] = 0xFFFF;
		dst->addr.ipv6[1] = (src->addr.ipv4 >> 16) & 0xFFFF;
		dst->addr.ipv6[0] = src->addr.ipv4 & 0xFFFF;
		break;
	default:
		return 0;
	}

	dst->transport = src->transport;
	dst->net = target;
	dst->port = src->port;
	return 1;
}

int tl_net_addr_equal(const tl_net_addr *a, const tl_net_addr *b)
{
	tl_net_addr conv;

	assert(a && b);

	if ((a->transport != b->transport) || (a->port != b->port))
		return 0;

	switch (a->net) {
	case TL_IPV4:
		switch (b->net) {
		case TL_IPV4:
			return a->addr.ipv4 == b->addr.ipv4;
		case TL_IPV6:
			if (!tl_net_addr_convert(&conv, b, TL_IPV4))
				return 0;
			return a->addr.ipv4 == conv.addr.ipv4;
		}
		break;
	case TL_IPV6:
		switch (b->net) {
		case TL_IPV4:
			if (!tl_net_addr_convert(&conv, a, TL_IPV4))
				return 0;
			return b->addr.ipv4 == conv.addr.ipv4;
		case TL_IPV6:
			return memcmp(a->addr.ipv6, b->addr.ipv6,
				      sizeof(b->addr.ipv6)) == 0;
		}
		break;
	}
	return 0;
}
