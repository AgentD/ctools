#include <stdlib.h>
#include <stdio.h>

#include "tl_process.h"

int main( int argc, char** argv )
{
    char buffer[ 128 ];
    int i;

    for( i=0; i<argc; ++i )
    {
        puts( argv[i] );
        fflush( stdout );
    }

    fgets( buffer, sizeof(buffer), stdin );
    fprintf( stdout, "STDOUT: %s", buffer );
    fflush( stdout );
    fprintf( stderr, "STDERR: %s", buffer );
    fflush( stderr );

    tl_sleep( 100 );
    return 100;
}

