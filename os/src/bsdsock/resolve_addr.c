/* resolve_addr.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "../platform.h"
#include "bsdsock.h"

int tl_network_resolve_address( const tl_net_addr* addr, tl_string* out )
{
    struct sockaddr_storage addrbuf;
    char *host = NULL, *new;
    socklen_t size;
    size_t slen;
    int ret;

    assert( addr );

    if( !winsock_acquire( ) )
        return TL_ERR_INTERNAL;

    if( !encode_sockaddr( addr, &addrbuf, &size ) )
        return TL_ERR_NET_ADDR;

    slen = NI_MAXHOST;
#ifndef MACHINE_OS_WINDOWS
    for( ret=EAI_OVERFLOW; ret==EAI_OVERFLOW; slen*=2 )
#endif
    {
        if( !(new = realloc( host, slen )) )
        {
            ret = TL_ERR_ALLOC;
            goto out_free;
        }

        host = new;
        ret = getnameinfo( (const void*)&addrbuf, size,
                           host, slen, NULL, 0, NI_NAMEREQD );
    }

    switch( ret )
    {
    case 0:                                         break;
#ifndef MACHINE_OS_WINDOWS
    case EAI_SYSTEM:    ret = errno_to_fs( errno ); goto out_free;
#endif
    case EAI_AGAIN:     ret = TL_ERR_TIMEOUT;       goto out_free;
    case EAI_FAMILY:    ret = TL_ERR_NET_ADDR;      goto out_free;
    case EAI_NONAME:    ret = 0;                    goto out_free;
    case EAI_MEMORY:    ret = TL_ERR_ALLOC;         goto out_free;
    default:            ret = TL_ERR_INTERNAL;      goto out_free;
    }

    if( out )
    {
        memset(out, 0, sizeof(*out));
        out->data.reserved = slen;
        out->data.used = strlen(host) + 1;
        out->data.unitsize = 1;
        out->data.data = host;
        out->mbseq = out->charcount = out->data.used - 1;
    }

    winsock_release( );
    return 1;
out_free:
    free(host);
    winsock_release( );
    return ret;
}

