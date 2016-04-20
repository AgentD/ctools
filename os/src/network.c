/*
 * network.c
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
#define TL_OS_EXPORT
#include "tl_network.h"

#include <assert.h>
#include <string.h>

int tl_network_get_special_address( tl_net_addr* addr, int type, int net )
{
    assert( addr );

    switch( net )
    {
    case TL_IPV4:
        switch( type )
        {
        case TL_LOOPBACK:   addr->addr.ipv4 = (127<<24)|(1); break;
        case TL_BROADCAST:  addr->addr.ipv4 = 0xFFFFFFFF;    break;
        case TL_ALL:        addr->addr.ipv4 = 0;             break;
        default:            return 0;
        }
        break;
    case TL_IPV6:
        switch( type )
        {
        case TL_LOOPBACK:
            memset( addr->addr.ipv6, 0, sizeof(addr->addr.ipv6) );
            addr->addr.ipv6[0] = 0x0001;
            break;
        case TL_ALL:
            memset( addr->addr.ipv6, 0, sizeof(addr->addr.ipv6) );
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

