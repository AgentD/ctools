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
        len = strlen( args[i] );
#ifdef _WIN32
        ++len;
#endif
        if( io->read( io, buffer, (len+1), &value )!=0 )
            return EXIT_FAILURE;
        if( value!=(len+1) )
            return EXIT_FAILURE;
#ifdef _WIN32
        --len;
#endif
        if( strncmp( args[i], buffer, len )!=0 )
            return EXIT_FAILURE;
#ifdef _WIN32
        if( buffer[len]!='\r' || buffer[len+1]!='\n' )
            return EXIT_FAILURE;
#else
        if( buffer[len]!='\n' )
            return EXIT_FAILURE;
#endif
    }

#ifdef _WIN32
    if( io->read( io, buffer, 23, &value )!=0 || value!=23 )
        return EXIT_FAILURE;
    if( strncmp( buffer, "STDOUT: Hello, World!\r\n", 23 )!=0 )
        return EXIT_FAILURE;

    if( err->read( err, buffer, 23, &value )!=0 || value!=23 )
        return EXIT_FAILURE;
    if( strncmp( buffer, "STDERR: Hello, World!\r\n", 22 )!=0 )
        return EXIT_FAILURE;
#else
    if( io->read( io, buffer, 22, &value )!=0 || value!=22 )
        return EXIT_FAILURE;
    if( strncmp( buffer, "STDOUT: Hello, World!\n", 22 )!=0 )
        return EXIT_FAILURE;

    if( err->read( err, buffer, 22, &value )!=0 || value!=22 )
        return EXIT_FAILURE;
    if( strncmp( buffer, "STDERR: Hello, World!\n", 22 )!=0 )
        return EXIT_FAILURE;
#endif

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
        len = strlen( args[i] );
#ifdef _WIN32
        ++len;
#endif
        if( io->read( io, buffer, (len+1), &value )!=0 )
            return EXIT_FAILURE;
        if( value!=(len+1) )
            return EXIT_FAILURE;
#ifdef _WIN32
        --len;
#endif
        if( strncmp( args[i], buffer, len )!=0 )
            return EXIT_FAILURE;
#ifdef _WIN32
        if( buffer[len]!='\r' || buffer[len+1]!='\n' )
            return EXIT_FAILURE;
#else
        if( buffer[len]!='\n' )
            return EXIT_FAILURE;
#endif
    }
#ifdef _WIN32
    if( io->read( io, buffer, 23, &value )!=0 || value!=23 )
        return EXIT_FAILURE;
printf("%s\n",buffer);
    if( strncmp( buffer, "STDOUT: Hello, World!\r\n", 23 )!=0 )
        return EXIT_FAILURE;
    if( strncmp( buffer+23, "STDERR: Hello, World!\r\n", 23 )!=0 )
        return EXIT_FAILURE;
#else
    if( io->read( io, buffer, 44, &value )!=0 || value!=44 )
        return EXIT_FAILURE;
    if( strncmp( buffer, "STDOUT: Hello, World!\n", 22 )!=0 )
        return EXIT_FAILURE;
    if( strncmp( buffer+22, "STDERR: Hello, World!\n", 22 )!=0 )
        return EXIT_FAILURE;
#endif

    tl_process_wait( proc, &i, 0 );
    if( i!=100 )
        return EXIT_FAILURE;
    tl_process_destroy( proc );
    return EXIT_SUCCESS;
}

