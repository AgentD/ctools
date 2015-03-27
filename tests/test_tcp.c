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

    if( up->write_raw( up, message, len, &actual )!=0 || actual!=len )
        return 0;
    if( down->read_raw( down, buffer, sizeof(buffer), &actual )!=0 )
        return 0;
    if( actual!=len || strcmp( buffer, message )!=0 )
        return 0;

    return 1;
}

int main( void )
{
    tl_iostream *a_down, *b_down, *a, *b;
    tl_server* server;
    tl_net_addr peer;

    /* create server */
    server = tl_network_create_server( TL_IPV4, TL_TCP, 15000, 10 );

    if( !server )
        return EXIT_FAILURE;

    /* setup client connection data */
    peer.transport = TL_TCP;
    peer.port = 15000;
    if( !tl_network_resolve_name( "127.0.0.1", TL_IPV4, &peer ) )
        return EXIT_FAILURE;

    /* first client connection */
    a = tl_network_create_client( &peer );
    if( !a )
        return EXIT_FAILURE;

    a_down = server->wait_for_client( server, 1000 );
    if( !a_down )
        return EXIT_FAILURE;

    /* connect second client */
    b = tl_network_create_client( &peer );
    if( !b )
        return EXIT_FAILURE;

    b_down = server->wait_for_client( server, 1000 );
    if( !b_down )
        return EXIT_FAILURE;

    /* test transmission */
    if( !test_transmission( a, a_down, "Hello From A" ) )
        return EXIT_FAILURE;
    if( !test_transmission( b, b_down, "Hello From B" ) )
        return EXIT_FAILURE;
    if( !test_transmission( a_down, a, "Greetings For A" ) )
        return EXIT_FAILURE;
    if( !test_transmission( b_down, b, "Greetings For B" ) )
        return EXIT_FAILURE;

    /* cleanup */
    a->destroy( a );
    b->destroy( b );
    a_down->destroy( a_down );
    b_down->destroy( b_down );
    server->destroy( server );
    return EXIT_SUCCESS;
}

