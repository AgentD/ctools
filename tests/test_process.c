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



int main( int argc, char** argv )
{
    char buffer[ 64 ];
    size_t value, len;
    tl_process* proc;
    tl_iostream* err;
    tl_iostream* io;
    int flags, i;

    if( argc<2 )
        return EXIT_FAILURE;

    /* */
    flags=TL_PIPE_STDIN|TL_PIPE_STDOUT|TL_PIPE_STDERR;
    proc = tl_process_create( argv[1], args, NULL, flags );
    io = tl_process_get_stdio( proc );
    err = tl_process_get_stderr( proc );

    io->set_timeout( io, 5000 );
    err->set_timeout( err, 5000 );

    io->write( io, "Hello, World!\n", 14, &value );

    for( i=0; args[i]; ++i )
    {
        if( io->read( io, buffer, 5, &value )!=0 )
            return EXIT_FAILURE;
        if( value!=5 )
            return EXIT_FAILURE;

        len = strlen( args[i] );
        if( strncmp( args[i], buffer, len )!=0 )
            return EXIT_FAILURE;
        if( buffer[len]!='\n' )
            return EXIT_FAILURE;
    }

    if( io->read( io, buffer, 22, &value )!=0 || value!=22 )
        return EXIT_FAILURE;
    if( strncmp( buffer, "STDOUT: Hello, World!\n", 22 )!=0 )
        return EXIT_FAILURE;

    if( err->read( err, buffer, 22, &value )!=0 || value!=22 )
        return EXIT_FAILURE;
    if( strncmp( buffer, "STDERR: Hello, World!\n", 22 )!=0 )
        return EXIT_FAILURE;

    tl_process_wait( proc, &i, 0 );
    if( i!=100 )
        return EXIT_FAILURE;
    tl_process_destroy( proc );

    /* */
    flags |= TL_STDERR_TO_STDOUT;
    proc = tl_process_create( argv[1], args, NULL, flags );
    io = tl_process_get_stdio( proc );
    err = tl_process_get_stderr( proc );
    if( err )
        return EXIT_FAILURE;

    io->set_timeout( io, 5000 );
    io->write( io, "Hello, World!\n", 14, &value );

    for( i=0; args[i]; ++i )
    {
        if( io->read( io, buffer, 5, &value )!=0 )
            return EXIT_FAILURE;
        if( value!=5 )
            return EXIT_FAILURE;

        len = strlen( args[i] );
        if( strncmp( args[i], buffer, len )!=0 )
            return EXIT_FAILURE;
        if( buffer[len]!='\n' )
            return EXIT_FAILURE;
    }

    if( io->read( io, buffer, 44, &value )!=0 || value!=44 )
        return EXIT_FAILURE;
    if( strncmp( buffer, "STDOUT: Hello, World!\n", 22 )!=0 )
        return EXIT_FAILURE;
    if( strncmp( buffer+22, "STDERR: Hello, World!\n", 22 )!=0 )
        return EXIT_FAILURE;

    tl_process_wait( proc, &i, 0 );
    if( i!=100 )
        return EXIT_FAILURE;
    tl_process_destroy( proc );
    return EXIT_SUCCESS;
}

