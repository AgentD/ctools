#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tl_packetserver.h"
#include "tl_network.h"



static int addrcmp( const tl_net_addr* a, const tl_net_addr* b )
{
    if( a->net!=b->net || a->transport!=b->transport || a->port!=b->port )
        return 0;
    if( a->net==TL_IPV4 && a->addr.ipv4!=b->addr.ipv4 )
        return 0;
    if( a->net==TL_IPV6 &&
        memcmp(a->addr.ipv6,b->addr.ipv6,sizeof(a->addr.ipv6))!=0 )
    {
        return 0;
    }
    return 1;
}

static int test_send( tl_packetserver* src, tl_packetserver* dst,
                      tl_net_addr* srcaddr, tl_net_addr* dstaddr,
                      const char* msg )
{
    size_t val, len = msg ? strlen(msg) : 0;
    tl_net_addr temp;
    char buffer[32];

    if( src->send( src, msg, dstaddr, len, &val )!=0 || val!=len )
        return 0;
    if( dst->receive( dst, buffer, &temp, sizeof(buffer), &val )!=0 )
        return 0;
    if( val!=len )
        return 0;
    if( msg && strncmp( buffer, msg, len )!=0 )
        return 0;
    if( !addrcmp( &temp, srcaddr ) )
        return 0;
    return 1;
}

static int run_test( int net, int transport, int aport, int bport )
{
    tl_net_addr a_addr, b_addr;
    tl_packetserver *a, *b;
    char buffer[ 32 ];
    int i;

    /* address where B can reach A */
    if( !tl_network_get_special_address( &a_addr, TL_LOOPBACK, net ) )
        return 0;
    a_addr.transport = transport;
    a_addr.port = aport;

    /* address where A can reach B */
    if( !tl_network_get_special_address( &b_addr, TL_LOOPBACK, net ) )
        return 0;
    b_addr.transport = transport;
    b_addr.port = bport;

    /* create servers */
    a = tl_network_create_packet_server( &a_addr, NULL, TL_DONT_FRAGMENT );
    if( !a )
        return 0;

    b = tl_network_create_packet_server( &b_addr, NULL, TL_DONT_FRAGMENT );
    if( !b )
        return 0;

    a->set_timeout( a, 1500 );
    b->set_timeout( b, 1500 );

    /* test transmissions */
    for( i=0; i<20; ++i )
    {
        sprintf( buffer, "Hello B %c", 'A'+i );

        if( !test_send( a, b, &a_addr, &b_addr, buffer ) )
            return 0;

        sprintf( buffer, "Hello B %c", 'a'+i );

        if( !test_send( b, a, &b_addr, &a_addr, buffer ) )
            return 0;

        if( !test_send( a, b, &a_addr, &b_addr, NULL ) )
            return 0;

        if( !test_send( b, a, &b_addr, &a_addr, NULL ) )
            return 0;
    }

    /* cleanup */
    a->destroy( a );
    b->destroy( b );
    return 1;
}



int main( void )
{
    if( !run_test( TL_IPV4, TL_UDP, 15000, 15010 ) ) return EXIT_FAILURE;
    if( !run_test( TL_IPV4, TL_UDP, 15010, 15000 ) ) return EXIT_FAILURE;
    if( !run_test( TL_IPV6, TL_UDP, 15000, 15010 ) ) return EXIT_FAILURE;
    if( !run_test( TL_IPV6, TL_UDP, 15010, 15000 ) ) return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

