#include "tl_iostream.h"
#include "tl_process.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


const char* args[] =
{
    "argA",
    "argB",
    "argC",
    NULL
};


static int receive_message( tl_iostream* stream, const char* message )
{
    size_t value, len = strlen(message);
    char buffer[ 64 ];

    if( stream->read( stream, buffer, len, &value )!=0 || value!=len )
        return 0;
    if( strncmp( buffer, message, len )!=0 )
        return 0;
#ifdef _WIN32
    if( stream->read( stream, buffer, 2, &value )!=0 || value!=2 )
        return 0;
    return (buffer[0]=='\r' && buffer[1]=='\n');
#else
    if( stream->read( stream, buffer, 1, &value )!=0 || value!=1 )
        return 0;
    return buffer[0]=='\n';
#endif
}


static int test_process( const char* path, int flags )
{
    tl_process* proc = tl_process_create( path, args, NULL, flags );
    tl_iostream* err = tl_process_get_stderr( proc );
    tl_iostream* io = tl_process_get_stdio( proc );
    size_t value;
    int i;

    if( !proc || !io                                ) return 0;
    if(  (flags & TL_STDERR_TO_STDOUT) && err!=NULL ) return 0;
    if( !(flags & TL_STDERR_TO_STDOUT) && err==NULL ) return 0;

    io->set_timeout( io, 5000 );
    if( err )
        err->set_timeout( err, 5000 );

    io->write( io, "Hello, World!\n", 14, &value );
    if( value!=14 )
        return 0;

    for( i=0; args[i]; ++i )
    {
        if( !receive_message( io, args[i] ) )
            return 0;
    }

    if( !receive_message( io, "STDOUT: Hello, World!" ) )
        return 0;
    if( !receive_message( err ? err : io, "STDERR: Hello, World!" ) )
        return 0;

    tl_process_terminate( proc );
    tl_process_wait( proc, &i, 0 );
    tl_process_destroy( proc );
    return i==100;
}

int main( int argc, char** argv )
{
    int flags = TL_PIPE_STDIN|TL_PIPE_STDOUT|TL_PIPE_STDERR;

    if( argc<2 )
        return EXIT_FAILURE;
    if( !test_process( argv[1], flags ) )
        return EXIT_FAILURE;
    if( !test_process( argv[1], flags|TL_STDERR_TO_STDOUT ) )
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

