#include "tl_convert.h"
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
    unsigned int i;
    tl_blob b1;

    for( i=0; i<sizeof(strings)/sizeof(strings[0]); i+=2 )
    {
        if( !tl_base64_encode( &b1, strings[i], strlen(strings[i]), 0 ) )
            return EXIT_FAILURE;
        if( strncmp( b1.data, strings[i+1], strlen(strings[i+1]) ) )
            return EXIT_FAILURE;

        tl_blob_cleanup( &b1 );
        if( !tl_base64_decode( &b1, strings[i+1], strlen(strings[i+1]), 0 ) )
            return EXIT_FAILURE;
        if( b1.size!=strlen(strings[i]) )
            return EXIT_FAILURE;
        if( strncmp( b1.data, strings[i], strlen(strings[i]) ) )
            return EXIT_FAILURE;

        tl_blob_cleanup( &b1 );
    }

    return EXIT_SUCCESS;
}

