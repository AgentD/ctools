#include <stdlib.h>
#include <string.h>

#include "tl_string.h"
#include "tl_iterator.h"


int main( void )
{
    tl_iterator* it;
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

    /**** string tokenizing ****/
    tl_string_init( &str );

    /* empty string */
    tl_string_clear( &str );
    it = tl_string_tokenize( &str, " \t" );

    if( !it || it->has_data( it ) )
        return EXIT_FAILURE;

    it->destroy( it );

    /* string without seperators */
    tl_string_append_utf8( &str, "foobar" );
    it = tl_string_tokenize( &str, " \t" );

    if( !it || !it->has_data( it ) )
        return EXIT_FAILURE;

    if( strcmp( tl_string_cstr(it->get_value(it)), "foobar" )!=0 )
        return EXIT_FAILURE;

    it->next( it );
    if( it->has_data( it ) )
        return EXIT_FAILURE;

    it->destroy( it );

    /* string with seperators */
    tl_string_clear( &str );

    tl_string_append_utf8( &str, "foo bar\tbaz  \t\t  qux" );
    it = tl_string_tokenize( &str, " \t" );

    if( !it || !it->has_data( it ) )
        return EXIT_FAILURE;
    if( strcmp( tl_string_cstr(it->get_value(it)), "foo" )!=0 )
        return EXIT_FAILURE;

    it->next( it );
    if( !it->has_data( it ) )
        return EXIT_FAILURE;
    if( strcmp( tl_string_cstr(it->get_value(it)), "bar" )!=0 )
        return EXIT_FAILURE;

    it->next( it );
    if( !it->has_data( it ) )
        return EXIT_FAILURE;
    if( strcmp( tl_string_cstr(it->get_value(it)), "baz" )!=0 )
        return EXIT_FAILURE;

    it->next( it );
    if( !it->has_data( it ) )
        return EXIT_FAILURE;
    if( strcmp( tl_string_cstr(it->get_value(it)), "qux" )!=0 )
        return EXIT_FAILURE;

    it->next( it );
    if( it->has_data( it ) )
        return EXIT_FAILURE;

    it->destroy( it );

    /* seperators at the beginning and end */
    tl_string_clear( &str );

    tl_string_append_utf8( &str, "  foo   bar  " );
    it = tl_string_tokenize( &str, " \t" );

    if( !it || !it->has_data( it ) )
        return EXIT_FAILURE;
    if( strcmp( tl_string_cstr(it->get_value(it)), "foo" )!=0 )
        return EXIT_FAILURE;

    it->next( it );
    if( !it->has_data( it ) )
        return EXIT_FAILURE;
    if( strcmp( tl_string_cstr(it->get_value(it)), "bar" )!=0 )
        return EXIT_FAILURE;

    it->next( it );
    if( it->has_data( it ) )
        return EXIT_FAILURE;

    it->destroy( it );

    tl_string_cleanup( &str );

    /**** remove ****/
    /* after first mbseq */
    tl_string_init_cstr( &str, "aäöfüßs" );

    tl_string_remove( &str, 2, 3 );

    if( strcmp( tl_string_cstr(&str), "aäßs" )!=0 )
        return EXIT_FAILURE;
    if( str.data.used!=7 || str.charcount!=4 || str.mbseq!=1 )
        return EXIT_FAILURE;

    tl_string_cleanup( &str );

    /* across first mbseq */
    tl_string_init_cstr( &str, "abäö" );

    tl_string_remove( &str, 1, 15 );

    if( strcmp( tl_string_cstr(&str), "a" )!=0 )
        return EXIT_FAILURE;
    if( str.data.used!=2 || str.charcount!=1 || str.mbseq!=2 )
        return EXIT_FAILURE;

    tl_string_cleanup( &str );

    /* before first mbseq */
    tl_string_init_cstr( &str, "abcdäöü" );

    tl_string_remove( &str, 1, 2 );

    if( strcmp( tl_string_cstr(&str), "adäöü" )!=0 )
        return EXIT_FAILURE;
    if( str.data.used!=9 || str.charcount!=5 || str.mbseq!=2 )
        return EXIT_FAILURE;

    tl_string_cleanup( &str );
    return EXIT_SUCCESS;
}

