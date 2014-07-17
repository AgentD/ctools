#include <stdlib.h>

#include "tl_string.h"



int match( const uint16_t* str1, const char* str2 )
{
    while( *str1 && *str2 )
    {
        if( *(str1++) != *(str2++) )
            return 0;
    }
    return ((*str1) == (*str2)) && ((*str1) == '\0');
}

int main( void )
{
    tl_string str;

    tl_string_init( &str );

    /* invalid base values -> base 10 */
    tl_string_append_uint( &str, 123456, -1 );
    if( !match( tl_string_cstr( &str ), "123456" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    tl_string_append_uint( &str, 123456, 0 );
    if( !match( tl_string_cstr( &str ), "123456" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    tl_string_append_uint( &str, 123456, 1 );
    if( !match( tl_string_cstr( &str ), "123456" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    /* base 2 */
    tl_string_append_uint( &str, 0xA5, 2 );
    if( !match( tl_string_cstr( &str ), "10100101" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    /* base 4 */
    tl_string_append_uint( &str, 12345, 4 );
    if( !match( tl_string_cstr( &str ), "3000321" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    /* base 8 */
    tl_string_append_uint( &str, 01234567, 8 );
    if( !match( tl_string_cstr( &str ), "1234567" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    /* base 10 */
    tl_string_append_uint( &str, 123456, 10 );
    if( !match( tl_string_cstr( &str ), "123456" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    /* base 16 */
    tl_string_append_uint( &str, 0x12ABCD, 16 );
    if( !match( tl_string_cstr( &str ), "12ABCD" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    /* base 32 */
    tl_string_append_uint( &str, 12345, 32 );
    if( !match( tl_string_cstr( &str ), "C1P" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    /* signed */
    tl_string_append_int( &str, -0xABCD, 16 );
    if( !match( tl_string_cstr( &str ), "-ABCD" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    tl_string_append_int( &str, 0xABCD, 16 );
    if( !match( tl_string_cstr( &str ), "ABCD" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    tl_string_cleanup( &str );
    return EXIT_SUCCESS;
}

