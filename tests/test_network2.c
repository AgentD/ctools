#include <stdlib.h>
#include <string.h>
#include "tl_network.h"
#include "tl_iostream.h"


int main( void )
{
    const char* teststring = "Hello, world!";
    tl_iostream* stream;
    tl_net_addr peer;

    peer.transport = TL_UDP;
    peer.port = 15000;
    if( !tl_network_resolve_name( "127.0.0.1", TL_IPV4, &peer ) )
        return EXIT_FAILURE;

    stream = tl_network_create_client( &peer );
    if( !stream )
        return EXIT_FAILURE;

    stream->write_raw( stream, teststring, strlen( teststring ), NULL );
    stream->destroy( stream );
    return EXIT_SUCCESS;
}

