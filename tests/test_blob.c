#include "tl_blob.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



const char* strings[] =
{
    "",            "",
    "A",           "QQ==",
    "AA",          "QUE=",
    "AAA",         "QUFB",
    "AAAA",        "QUFBQQ==",
    "AAAAA",       "QUFBQUE=",
    "AAAAAA",      "QUFBQUFB",
    "Foobar Test", "Rm9vYmFyIFRlc3Q=",
    "Foobartest",  "Rm9vYmFydGVzdA==",
    "Foobar",      "Rm9vYmFy",
};



int main( void )
{
    unsigned char buffer[ 256 ];
    unsigned int i;
    tl_blob b0, b1;

    /********** initialization **********/
    for( i=0; i<sizeof(buffer); ++i )
        buffer[ i ] = i;

    tl_blob_init( &b0, 0, NULL );
    if( b0.size || b0.data ) return EXIT_FAILURE;
    tl_blob_cleanup( &b0 );

    tl_blob_init( &b0, 0, buffer );
    if( b0.size || b0.data ) return EXIT_FAILURE;
    tl_blob_cleanup( &b0 );

    tl_blob_init( &b0, sizeof(buffer), NULL );
    if( b0.size!=sizeof(buffer) || !b0.data ) return EXIT_FAILURE;
    tl_blob_cleanup( &b0 );

    tl_blob_init( &b0, sizeof(buffer), buffer );
    if( b0.size!=sizeof(buffer) || !b0.data ) return EXIT_FAILURE;

    for( i=0; i<sizeof(buffer); ++i )
    {
        if( buffer[i] != ((unsigned char*)b0.data)[i] )
            return EXIT_FAILURE;
    }
    tl_blob_cleanup( &b0 );

    /********** split **********/
    tl_blob_init( &b0, sizeof(buffer), buffer );
    if( !tl_blob_split( &b1, &b0, sizeof(buffer)*10 ) ) return EXIT_FAILURE;
    if( b1.size || b1.data ) return EXIT_FAILURE;
    if( b0.size!=sizeof(buffer) || !b0.data ) return EXIT_FAILURE;
    for( i=0; i<sizeof(buffer); ++i )
    {
        if( buffer[i] != ((unsigned char*)b0.data)[i] )
            return EXIT_FAILURE;
    }

    if( !tl_blob_split( &b1, &b0, 0 ) ) return EXIT_FAILURE;
    if( b0.size || b0.data ) return EXIT_FAILURE;
    if( b1.size!=sizeof(buffer) || !b1.data ) return EXIT_FAILURE;
    for( i=0; i<sizeof(buffer); ++i )
    {
        if( buffer[i] != ((unsigned char*)b1.data)[i] )
            return EXIT_FAILURE;
    }

    if( !tl_blob_split( &b0, &b1, 100 ) ) return EXIT_FAILURE;
    if( b0.size!=(sizeof(buffer)-100) || !b0.data ) return EXIT_FAILURE;
    if( b1.size!=100 || !b1.data ) return EXIT_FAILURE;
    for( i=0; i<100; ++i )
    {
        if( buffer[i] != ((unsigned char*)b1.data)[i] )
            return EXIT_FAILURE;
    }
    for( ; i<sizeof(buffer); ++i )
    {
        if( buffer[i] != ((unsigned char*)b0.data)[i-100] )
            return EXIT_FAILURE;
    }
    tl_blob_cleanup( &b0 );
    tl_blob_cleanup( &b1 );

    /********** base64 **********/
    for( i=0; i<sizeof(strings)/sizeof(strings[0]); i+=2 )
    {
        tl_blob_init( &b0, strlen( strings[ i ] ), strings[ i ] );
        if( !tl_blob_encode_base64( &b1, &b0, 0 ) )
            return EXIT_FAILURE;
        if( strncmp( b1.data, strings[i+1], strlen( strings[i+1] ) ) )
            return EXIT_FAILURE;

        tl_blob_cleanup( &b0 );
        if( !tl_blob_decode_base64( &b0, &b1, 0 ) )
            return EXIT_FAILURE;
        if( b0.size!=strlen(strings[i]) )
            return EXIT_FAILURE;
        if( strncmp( b0.data, strings[ i ], strlen( strings[ i ] ) ) )
            return EXIT_FAILURE;

        tl_blob_cleanup( &b0 );
        tl_blob_cleanup( &b1 );
    }

    /********** append **********/
    for( i=0; i<sizeof(buffer); ++i )
        buffer[ i ] = i;

    tl_blob_init( &b0, sizeof(buffer), buffer );

    if( !tl_blob_append_raw( &b0, buffer, 0 ) ) return EXIT_FAILURE;
    if( b0.size!=sizeof(buffer) || !b0.data ) return EXIT_FAILURE;

    if( !tl_blob_append_raw( &b0, NULL, 0 ) ) return EXIT_FAILURE;
    if( b0.size!=sizeof(buffer) || !b0.data ) return EXIT_FAILURE;

    for( i=0; i<sizeof(buffer); ++i )
        buffer[ i ] = sizeof(buffer) - i;

    if( !tl_blob_append_raw( &b0, buffer, sizeof(buffer) ) )
        return EXIT_FAILURE;

    if( b0.size!=2*sizeof(buffer) || !b0.data )
        return EXIT_FAILURE;

    for( i=0; i<sizeof(buffer); ++i )
    {
        if( ((unsigned char*)b0.data)[i] != (i & 0xFF) )
            return EXIT_FAILURE;
    }

    for( i=0; i<sizeof(buffer); ++i )
    {
        if( ((unsigned char*)b0.data)[i+sizeof(buffer)] !=
            ((sizeof(buffer)-i) & 0xFF) )
            return EXIT_FAILURE;
    }

    if( !tl_blob_append_raw( &b0, NULL, sizeof(buffer) ) )
        return EXIT_FAILURE;

    if( b0.size!=3*sizeof(buffer) || !b0.data )
        return EXIT_FAILURE;

    /********** truncate **********/
    tl_blob_truncate( &b0, 15*sizeof(buffer) );
    if( b0.size>3*sizeof(buffer) || !b0.data )
        return EXIT_FAILURE;
    tl_blob_truncate( &b0, sizeof(buffer) );
    if( b0.size!=sizeof(buffer) || !b0.data )
        return EXIT_FAILURE;

    for( i=0; i<sizeof(buffer); ++i )
    {
        if( ((unsigned char*)b0.data)[i]!=i )
            return EXIT_FAILURE;
    }

    tl_blob_cleanup( &b0 );

    /********* remove **********/
    for( i=0; i<sizeof(buffer); ++i )
        buffer[ i ] = i;

    tl_blob_init( &b0, sizeof(buffer), buffer );

    tl_blob_remove( &b0, 0, 0 );
    if( b0.size!=sizeof(buffer) || !b0.data ) return EXIT_FAILURE;
    tl_blob_remove( &b0, 10000, 0 );
    if( b0.size!=sizeof(buffer) || !b0.data ) return EXIT_FAILURE;

    tl_blob_remove( &b0, sizeof(buffer)-10, 10000 );
    if( b0.size!=(sizeof(buffer)-10) || !b0.data ) return EXIT_FAILURE;
    for( i=0; i<(sizeof(buffer)-10); ++i )
    {
        if( ((unsigned char*)b0.data)[i]!=i )
            return EXIT_FAILURE;
    }

    tl_blob_remove( &b0, 0, 10 );
    if( b0.size!=(sizeof(buffer)-20) || !b0.data ) return EXIT_FAILURE;
    for( i=0; i<(sizeof(buffer)-20); ++i )
    {
        if( ((unsigned char*)b0.data)[i]!=(i+10) )
            return EXIT_FAILURE;
    }

    tl_blob_cleanup( &b0 );

    tl_blob_init( &b0, sizeof(buffer), buffer );

    tl_blob_remove( &b0, 10, 10 );
    if( b0.size!=(sizeof(buffer)-10) || !b0.data ) return EXIT_FAILURE;
    for( i=0; i<10; ++i )
    {
        if( ((unsigned char*)b0.data)[i]!=i )
            return EXIT_FAILURE;
    }

    for( ; i<sizeof(buffer)-10; ++i )
    {
        if( ((unsigned char*)b0.data)[i]!=(i+10) )
            return EXIT_FAILURE;
    }

    tl_blob_cleanup( &b0 );

    /********** insert **********/
    tl_blob_init( &b0, sizeof(buffer), buffer );

    if( !tl_blob_insert_raw( &b0, buffer, 20, 20 ) )
        return EXIT_FAILURE;

    if( b0.size!=(sizeof(buffer)+20) )
        return EXIT_FAILURE;

    for( i=0; i<20; ++i )
    {
        if( ((unsigned char*)b0.data)[i]!=i )
            return EXIT_FAILURE;
    }

    for( ; i<40; ++i )
    {
        if( ((unsigned char*)b0.data)[i]!=(i-20) )
            return EXIT_FAILURE;
    }

    for( ; i<(sizeof(buffer)+20); ++i )
    {
        if( ((unsigned char*)b0.data)[i]!=(i-20) )
            return EXIT_FAILURE;
    }

    tl_blob_cleanup( &b0 );

    tl_blob_init( &b0, sizeof(buffer), buffer );

    if( !tl_blob_insert_raw( &b0, buffer, 0, 20 ) )
        return EXIT_FAILURE;

    if( b0.size!=(sizeof(buffer)+20) )
        return EXIT_FAILURE;

    for( i=0; i<20; ++i )
    {
        if( ((unsigned char*)b0.data)[i]!=i )
            return EXIT_FAILURE;
    }

    for( ; i<(sizeof(buffer)+20); ++i )
    {
        if( ((unsigned char*)b0.data)[i]!=(i-20) )
            return EXIT_FAILURE;
    }

    tl_blob_cleanup( &b0 );

    tl_blob_init( &b0, sizeof(buffer), buffer );

    if( !tl_blob_insert_raw( &b0, buffer, 10000, 20 ) )
        return EXIT_FAILURE;

    if( b0.size!=(sizeof(buffer)+20) )
        return EXIT_FAILURE;

    for( i=0; i<sizeof(buffer); ++i )
    {
        if( ((unsigned char*)b0.data)[i]!=i )
            return EXIT_FAILURE;
    }

    for( ; i<b0.size; ++i )
    {
        if( ((unsigned char*)b0.data)[i]!=(i-sizeof(buffer)) )
            return EXIT_FAILURE;
    }

    if( !tl_blob_insert_raw( &b0, NULL, 10000, 20 ) )
        return EXIT_FAILURE;

    if( b0.size!=(sizeof(buffer)+40) )
        return EXIT_FAILURE;

    if( !tl_blob_insert_raw( &b0, buffer, 10000, 0 ) )
        return EXIT_FAILURE;

    if( b0.size!=(sizeof(buffer)+40) )
        return EXIT_FAILURE;

    tl_blob_cleanup( &b0 );

    return EXIT_SUCCESS;
}


