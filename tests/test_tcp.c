#include <stdlib.h>
#include <string.h>
#include "tl_server.h"
#include "tl_network.h"
#include "tl_iostream.h"


static int test_transmission( tl_iostream* up, tl_iostream* down,
                              const char* message )
{
    size_t actual, len = strlen(message)+1;
    char buffer[ 128 ];

    if( up->write( up, message, len, &actual )!=0 || actual!=len )
        return 0;
    if( down->read( down, buffer, sizeof(buffer), &actual )!=0 )
        return 0;
    if( actual!=len || strcmp( buffer, message )!=0 )
        return 0;

    return 1;
}

static int check_addr_match( tl_iostream* a, tl_iostream* b )
{
    tl_net_addr temp, temp2;

    if( !tl_network_get_local_address( a, &temp ) ) return 0;
    if( !tl_network_get_peer_address( b, &temp2 ) ) return 0;
    if( temp.net!=temp2.net || temp.transport!=temp2.transport ) return 0;
    if( temp.addr.ipv4!=temp2.addr.ipv4 ) return 0;
    if( temp.port!=temp2.port ) return 0;
    return 1;
}

static int run_test( tl_net_addr* peer, tl_net_addr* accept )
{
    tl_iostream *a_down, *b_down, *a, *b;
    tl_server* server;
    tl_net_addr temp;

    if( !(server = tl_network_create_server( accept, 10, 0 )) )
        return 0;

    if( !(a = tl_network_create_client( peer, NULL, 0 )) )
        return 0;

    if( !(a_down = server->wait_for_client( server, 1000 )) )
        return 0;

    if( !(b = tl_network_create_client( peer, NULL, 0 )) )
        return 0;

    if( !(b_down = server->wait_for_client( server, 1000 )) )
        return 0;

    a->set_timeout( a, 1500 );
    b->set_timeout( b, 1500 );
    a_down->set_timeout( a_down, 1500 );
    b_down->set_timeout( b_down, 1500 );

    if( !test_transmission( a, a_down, "Hello From A" ) ) return 0;
    if( !test_transmission( b, b_down, "Hello From B" ) ) return 0;
    if( !test_transmission( a_down, a, "Greetings For A" ) ) return 0;
    if( !test_transmission( b_down, b, "Greetings For B" ) ) return 0;

    if( !tl_network_get_peer_address( a, &temp ) ) return 0;
    if( temp.net!=peer->net || temp.transport!=peer->transport ) return 0;
    if( temp.port!=peer->port ) return 0;
    if( temp.addr.ipv4!=peer->addr.ipv4 ) return 0;

    if( !tl_network_get_peer_address( b, &temp ) ) return 0;
    if( temp.net!=peer->net || temp.transport!=peer->transport ) return 0;
    if( temp.port!=peer->port ) return 0;
    if( temp.addr.ipv4!=peer->addr.ipv4 ) return 0;

    if( !check_addr_match( a, a_down ) ) return 0;
    if( !check_addr_match( b, b_down ) ) return 0;
    if( !check_addr_match( a_down, a ) ) return 0;
    if( !check_addr_match( b_down, b ) ) return 0;

    a->destroy( a );
    b->destroy( b );
    a_down->destroy( a_down );
    b_down->destroy( b_down );
    server->destroy( server );
    return 1;
}

int main( void )
{
    tl_net_addr accept;
    tl_net_addr peer;

    accept.transport = peer.transport = TL_TCP;
    accept.port = peer.port = 15000;

    /* test with IPv4 */
    if( !tl_network_get_special_address( &accept, TL_ANY, TL_IPV4 ) )
        return EXIT_FAILURE;
    if( !tl_network_get_special_address( &peer, TL_LOOPBACK, TL_IPV4 ) )
        return EXIT_FAILURE;

    if( !run_test( &peer, &accept ) )
        return EXIT_FAILURE;

    /* test with IPv6 */
    if( !tl_network_get_special_address( &accept, TL_LOOPBACK, TL_IPV6 ) )
        return EXIT_FAILURE;
    if( !tl_network_get_special_address( &peer, TL_LOOPBACK, TL_IPV6 ) )
        return EXIT_FAILURE;

    if( !run_test( &peer, &accept ) )
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

