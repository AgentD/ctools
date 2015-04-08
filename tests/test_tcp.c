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

static int run_test( tl_net_addr* peer, tl_net_addr* accept )
{
    tl_iostream *a_down, *b_down, *a, *b;
    tl_server* server;

    if( !(server = tl_network_create_server( accept, 10 )) )
        return 0;

    if( !(a = tl_network_create_client( peer )) )
        return 0;

    if( !(a_down = server->wait_for_client( server, 1000 )) )
        return 0;

    if( !(b = tl_network_create_client( peer )) )
        return 0;

    if( !(b_down = server->wait_for_client( server, 1000 )) )
        return 0;

    if( !test_transmission( a, a_down, "Hello From A" ) ) return 0;
    if( !test_transmission( b, b_down, "Hello From B" ) ) return 0;
    if( !test_transmission( a_down, a, "Greetings For A" ) ) return 0;
    if( !test_transmission( b_down, b, "Greetings For B" ) ) return 0;

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

