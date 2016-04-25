#include "tl_packetserver.h"
#include "tl_iostream.h"
#include "tl_network.h"
#include "tl_server.h"

#include <stdlib.h>
#include <string.h>



int main( void )
{
    tl_packetserver* pserver;
    char buffer[ 16 ];
    tl_server* server;
    tl_net_addr addr;
    tl_iostream* str;
    size_t val;

    /* create server */
    if( !tl_network_get_special_address( &addr, TL_ALL, TL_IPV4 ) )
        return EXIT_FAILURE;
    addr.transport = TL_UDP;
    addr.port = 15000;

    server = tl_network_create_server( &addr, 10, 0 );
    if( !server )
        return EXIT_FAILURE;

    addr.port = 15010;
    pserver = tl_network_create_packet_server( &addr, TL_ALLOW_BROADCAST );
    if( !pserver )
        return EXIT_FAILURE;

    /* send broadcast */
    if( !tl_network_get_special_address( &addr, TL_BROADCAST, TL_IPV4 ) )
        return EXIT_FAILURE;
    addr.transport = TL_UDP;
    addr.port = 15000;
    if( pserver->send( pserver, "Test", &addr, 4, &val )!=0 || val!=4 )
        return EXIT_FAILURE;

    /* receive broadcast and send response */
    str = server->wait_for_client( server, 5000 );
    if( !str )
        return EXIT_FAILURE;
    if( str->read( str, buffer, sizeof(buffer), &val )!=0 || val!=4 )
        return EXIT_FAILURE;
    if( strncmp( buffer, "Test", 4 )!=0 )
        return EXIT_FAILURE;
    if( str->write( str, "Hello", 5, &val )!=0 || val!=5 )
        return EXIT_FAILURE;

    /* receive respones */
    if( pserver->receive( pserver, buffer, &addr, 5, &val )!=0 || val!=5 )
        return EXIT_FAILURE;
    if( strncmp( buffer, "Hello", 5 )!=0 )
        return EXIT_FAILURE;

    /* cleanup */
    str->destroy( str );
    pserver->destroy( pserver );
    server->destroy( server );
    return EXIT_SUCCESS;
}

