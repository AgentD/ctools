/* resolve_name.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "../platform.h"
#include "bsdsock.h"

static int have_duplicate(const tl_net_addr *addr, size_t count,
			  const tl_net_addr *find)
{
	size_t i;
	int ret;

	for (i = 0; i < count; ++i) {
		if (addr[i].net != find->net)
			continue;

		if (find->net == TL_IPV4
		    && find->addr.ipv4 == addr[i].addr.ipv4)
			return 1;

		if (find->net == TL_IPV6) {
			ret = memcmp(find->addr.ipv6, addr[i].addr.ipv6,
				     sizeof(find->addr.ipv6));
			if (ret == 0)
				return 1;
		}
	}

	return 0;
}

int resolve_name(const char *hostname, int proto,
		 tl_net_addr *addr, size_t count)
{
	struct addrinfo hints, *info, *p;
	struct in6_addr addr6;
	struct in_addr addr4;
	tl_net_addr conv;
	size_t i = 0;

	proto = (proto == TL_IPV6) ? AF_INET6 :
		((proto == TL_IPV4) ? AF_INET : AF_UNSPEC);

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = proto;

	if (!winsock_acquire())
		return 0;

	if (getaddrinfo(hostname, NULL, &hints, &info) != 0)
		goto out;

	for (p = info; p != NULL; p = p->ai_next) {
		if (p->ai_family != AF_INET && p->ai_family != AF_INET6)
			continue;

		if (proto != AF_UNSPEC && p->ai_family != proto)
			continue;

		if (addr) {
			if (i >= count)
				break;

			memset(&conv, 0, sizeof(conv));

			if (p->ai_family == AF_INET6) {
				addr6 =
				    ((struct sockaddr_in6 *)p->ai_addr)->
				    sin6_addr;
				convert_ipv6(&addr6, &conv);
			} else {
				addr4 =
				    ((struct sockaddr_in *)p->ai_addr)->
				    sin_addr;
				conv.addr.ipv4 = ntohl(addr4.s_addr);
			}

			conv.net = p->ai_family == AF_INET6 ?
				   TL_IPV6 : TL_IPV4;

			if (have_duplicate(addr, i, &conv))
				continue;

			addr[i] = conv;
		}

		++i;
	}

	freeaddrinfo(info);
out:
	winsock_release();
	return i;
}
