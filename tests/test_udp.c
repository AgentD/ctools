#include "tl_packetserver.h"
#include "tl_iostream.h"
#include "tl_network.h"

#include <stdlib.h>
#include <string.h>

int main( void )
{
    tl_packetserver* pserver;
    char buffer[ 16 ];
    tl_net_addr addr;
    tl_iostream* str;
    size_t val;

    /* create server */
    if( !tl_network_get_special_address( &addr, TL_ALL, TL_IPV4 ) )
        return EXIT_FAILURE;

    addr.transport = TL_UDP;
    addr.port = 15000;

    pserver = tl_network_create_packet_server( &addr, NULL, 0 );
    if( !pserver )
        return EXIT_FAILURE;

    pserver->set_timeout( pserver, 1500 );

    /* create client */
    if( !tl_network_resolve_name( "127.0.0.1", TL_IPV4, &addr, 1 ) )
        return EXIT_FAILURE;

    addr.transport = TL_UDP;
    addr.port = 15000;

    str = tl_network_create_client( &addr, NULL, 0 );
    if( !str )
        return EXIT_FAILURE;

    str->set_timeout( str, 1500 );

    /* send packet to server */
    if( str->write( str, "Hello", 5, &val )!=0 || val!=5 )
        return EXIT_FAILURE;
    if( pserver->receive( pserver, buffer, &addr, 5, &val )!=0 || val!=5 )
        return EXIT_FAILURE;
    if( strncmp( buffer, "Hello", 5 )!=0 )
        return EXIT_FAILURE;

    /* send response to client */
    if( pserver->send( pserver, "World", &addr, 5, &val )!=0 || val!=5 )
        return EXIT_FAILURE;
    if( str->read( str, buffer, 5, &val )!=0 || val!=5 )
        return EXIT_FAILURE;
    if( strncmp( buffer, "World", 5 )!=0 )
        return EXIT_FAILURE;

    /* cleanup */
    str->destroy( str );
    pserver->destroy( pserver );
    return EXIT_SUCCESS;
}

