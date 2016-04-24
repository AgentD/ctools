#include "tl_network.h"

#include <stdlib.h>
#include <stdio.h>



int main( void )
{
    tl_net_addr addr;

    /* resolve IPv4 addresses */
    if( !tl_network_resolve_name( "127.0.0.1", TL_ANY, &addr, 1 ) )
        return EXIT_FAILURE;
    if( addr.net!=TL_IPV4 )
        return EXIT_FAILURE;
    if( addr.addr.ipv4!=0x7F000001 )
        return EXIT_FAILURE;

    if( !tl_network_resolve_name( "192.168.1.1", TL_ANY, &addr, 1 ) )
        return EXIT_FAILURE;
    if( addr.net!=TL_IPV4 )
        return EXIT_FAILURE;
    if( addr.addr.ipv4!=0xC0A80101 )
        return EXIT_FAILURE;

    /* resolve IPv6 addresses */
    if( !tl_network_resolve_name( "::1", TL_ANY, &addr, 1 ) )
        return EXIT_FAILURE;
    if( addr.net!=TL_IPV6 )
        return EXIT_FAILURE;
    if( addr.addr.ipv6[7]!=0x0000 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[6]!=0x0000 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[5]!=0x0000 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[4]!=0x0000 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[3]!=0x0000 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[2]!=0x0000 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[1]!=0x0000 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[0]!=0x0001 ) return EXIT_FAILURE;

    if( !tl_network_resolve_name("FE80::0202:B3FF:FE1E:8329",TL_ANY,&addr,1) )
        return EXIT_FAILURE;
    if( addr.net!=TL_IPV6 )
        return EXIT_FAILURE;
    if( addr.addr.ipv6[7]!=0xFE80 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[6]!=0x0000 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[5]!=0x0000 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[4]!=0x0000 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[3]!=0x0202 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[2]!=0xB3FF ) return EXIT_FAILURE;
    if( addr.addr.ipv6[1]!=0xFE1E ) return EXIT_FAILURE;
    if( addr.addr.ipv6[0]!=0x8329 ) return EXIT_FAILURE;


    if( !tl_network_resolve_name("::ffff:192.0.2.128",TL_ANY,&addr,1) )
        return EXIT_FAILURE;
    if( addr.net!=TL_IPV6 )
        return EXIT_FAILURE;
    if( addr.addr.ipv6[7]!=0x0000 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[6]!=0x0000 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[5]!=0x0000 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[4]!=0x0000 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[3]!=0x0000 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[2]!=0xFFFF ) return EXIT_FAILURE;
    if( addr.addr.ipv6[1]!=0xC000 ) return EXIT_FAILURE;
    if( addr.addr.ipv6[0]!=0x0280 ) return EXIT_FAILURE;

    /* resolve hostname */
    if( !tl_network_resolve_name( "localhost", TL_IPV4, &addr,1 ) )
        return EXIT_FAILURE;
    if( addr.net!=TL_IPV4 )
        return EXIT_FAILURE;

    if( !tl_network_resolve_name( "localhost", TL_IPV6, &addr, 1 ) )
        return EXIT_FAILURE;
    if( addr.net!=TL_IPV6 )
        return EXIT_FAILURE;

    /* resolve DNS name */
    if( !tl_network_resolve_name( "www.example.com", TL_IPV4, &addr, 1 ) )
        return EXIT_FAILURE;
    if( addr.net!=TL_IPV4 )
        return EXIT_FAILURE;

    if( !tl_network_resolve_name( "www.example.com", TL_IPV6, &addr,1 ) )
        return EXIT_FAILURE;
    if( addr.net!=TL_IPV6 )
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

