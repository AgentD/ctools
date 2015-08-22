#include <stdlib.h>
#include <string.h>

#include "tl_string.h"


int main( void )
{
    tl_string str;

    tl_string_init( &str );

    /* invalid base values -> base 10 */
    tl_string_append_uint( &str, 123456, -1 );
    if( strcmp( tl_string_cstr( &str ), "123456" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    tl_string_append_uint( &str, 123456, 0 );
    if( strcmp( tl_string_cstr( &str ), "123456" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    tl_string_append_uint( &str, 123456, 1 );
    if( strcmp( tl_string_cstr( &str ), "123456" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    /* base 2 */
    tl_string_append_uint( &str, 0xA5, 2 );
    if( strcmp( tl_string_cstr( &str ), "10100101" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    /* base 4 */
    tl_string_append_uint( &str, 12345, 4 );
    if( strcmp( tl_string_cstr( &str ), "3000321" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    /* base 8 */
    tl_string_append_uint( &str, 01234567, 8 );
    if( strcmp( tl_string_cstr( &str ), "1234567" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    /* base 10 */
    tl_string_append_uint( &str, 123456, 10 );
    if( strcmp( tl_string_cstr( &str ), "123456" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    /* base 16 */
    tl_string_append_uint( &str, 0x12ABCD, 16 );
    if( strcmp( tl_string_cstr( &str ), "12ABCD" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    /* base 32 */
    tl_string_append_uint( &str, 12345, 32 );
    if( strcmp( tl_string_cstr( &str ), "C1P" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    /* signed */
    tl_string_append_int( &str, -0xABCD, 16 );
    if( strcmp( tl_string_cstr( &str ), "-ABCD" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    tl_string_append_int( &str, 0xABCD, 16 );
    if( strcmp( tl_string_cstr( &str ), "ABCD" ) ) { return EXIT_FAILURE; }
    tl_string_clear( &str );

    tl_string_cleanup( &str );

    /**** ltrim ****/
    tl_string_init( &str );

    /* string of only white space */
    tl_string_append_utf8( &str, "  \t\t\r\n" );
    tl_string_trim_begin( &str );
    if( strcmp( tl_string_cstr( &str ), "" ) ) { return EXIT_FAILURE; }

    /* empty string */
    tl_string_append_utf8( &str, "" );
    tl_string_trim_begin( &str );
    if( strcmp( tl_string_cstr( &str ), "" ) ) { return EXIT_FAILURE; }

    /* actual non-space data */
    tl_string_append_utf8( &str, "  \t\t\r\nfoobar  \t\t\r\n" );
    tl_string_trim_begin( &str );
    if( strcmp( tl_string_cstr( &str ), "foobar  \t\t\r\n" ) )
        return EXIT_FAILURE;

    /* nothing to do */
    tl_string_clear( &str );
    tl_string_append_utf8( &str, "a b c" );
    tl_string_trim_begin( &str );
    if( strcmp( tl_string_cstr( &str ), "a b c" ) )
        return EXIT_FAILURE;

    tl_string_cleanup( &str );

    /**** rtrim ****/
    tl_string_init( &str );

    /* string of only white space */
    tl_string_append_utf8( &str, "  \t\t\r\n" );
    tl_string_trim_end( &str );
    if( strcmp( tl_string_cstr( &str ), "" ) ) { return EXIT_FAILURE; }

    /* empty string */
    tl_string_append_utf8( &str, "" );
    tl_string_trim_end( &str );
    if( strcmp( tl_string_cstr( &str ), "" ) ) { return EXIT_FAILURE; }

    /* actual non-space data */
    tl_string_append_utf8( &str, "  \t\t\r\nfoobar  \t\t\r\n" );
    tl_string_trim_end( &str );
    if( strcmp( tl_string_cstr( &str ), "  \t\t\r\nfoobar" ) )
        return EXIT_FAILURE;

    /* nothing to do */
    tl_string_clear( &str );
    tl_string_append_utf8( &str, "a b c" );
    tl_string_trim_end( &str );
    if( strcmp( tl_string_cstr( &str ), "a b c" ) )
        return EXIT_FAILURE;

    tl_string_cleanup( &str );
    return EXIT_SUCCESS;
}

