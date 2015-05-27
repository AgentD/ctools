#include "tl_hash.h"
#include <stdlib.h>

int main( void )
{
    tl_u32 v;

    /* CRC-32 */
    v = tl_hash_crc32( 0, "", 0 );

    if( v!=0 )
        return EXIT_FAILURE;

    v = tl_hash_crc32( 0, "Hello, World!", 13 );

    if( v!=0xEC4AC3D0 )
        return EXIT_FAILURE;

    v = tl_hash_crc32( 0, "The quick brown fox jumps over the lazy dog", 43 );

    if( v!=0x414FA339 )
        return EXIT_FAILURE;

    v = tl_hash_crc32( 0, "Test vector from febooti.com", 28 );

    if( v!=0x0C877F61 )
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

