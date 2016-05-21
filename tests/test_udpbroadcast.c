#include "tl_packetserver.h"
#include "tl_network.h"

#include <stdlib.h>
#include <string.h>



int main( void )
{
    tl_packetserver* pserver1;
    tl_packetserver* pserver2;
    char buffer[ 16 ];
    tl_net_addr addr;
    size_t val;

    /* create servers */
    if( !tl_network_get_special_address( &addr, TL_ALL, TL_IPV4 ) )
        return EXIT_FAILURE;

    addr.transport = TL_UDP;
    addr.port = 15000;

    pserver1 = tl_network_create_packet_server( &addr, TL_ALLOW_BROADCAST );
    if( !pserver1 )
        return EXIT_FAILURE;

    addr.port = 16000;

    pserver2 = tl_network_create_packet_server( &addr, 0 );
    if( !pserver2 )
        return EXIT_FAILURE;

    /* send broadcast from first server */
    if( !tl_network_get_special_address( &addr, TL_BROADCAST, TL_IPV4 ) )
        return EXIT_FAILURE;

    addr.transport = TL_UDP;
    addr.port = 16000;

    if( pserver1->send( pserver1, "Test", &addr, 4, &val )!=0 || val!=4 )
        return EXIT_FAILURE;

    /* receive broadcast and send response */
    if( pserver2->receive(pserver2, buffer, &addr, sizeof(buffer), &val)!=0 )
        return EXIT_FAILURE;
    if( val!=4 || strncmp( buffer, "Test", 4 )!=0 )
        return EXIT_FAILURE;
    if( addr.transport != TL_UDP || addr.port != 15000 )
        return EXIT_FAILURE;

    if( pserver2->send( pserver2, "Hello", &addr, 5, &val )!=0 || val!=5 )
        return EXIT_FAILURE;

    /* receive respone */
    if( pserver1->receive( pserver1, buffer, &addr, sizeof(buffer), &val )!=0 )
        return EXIT_FAILURE;
    if( val!=5 || strncmp( buffer, "Hello", 5 )!=0 )
        return EXIT_FAILURE;
    if( addr.transport != TL_UDP || addr.port != 16000 )
        return EXIT_FAILURE;

    /* cleanup */
    pserver1->destroy( pserver1 );
    pserver2->destroy( pserver2 );
    return EXIT_SUCCESS;
}

