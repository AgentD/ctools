#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <signal.h>
#endif

#include "tl_process.h"

#ifndef _WIN32
static volatile int run = 1;

static void handle_sigterm( int signal )
{
    if( signal==SIGTERM )
        run = 0;
}
#endif

int main( int argc, char** argv )
{
#ifdef _WIN32
    MSG msg;
#endif
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

#ifdef _WIN32
    while( GetMessage( &msg, NULL, 0, 0 ) )
        tl_sleep( 10 );
#else
    signal( SIGTERM, handle_sigterm );
    while( run )
        tl_sleep( 10 );
#endif
    return 100;
}

