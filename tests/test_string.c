#include "tl_string.h"

#include <stdlib.h>



int main( void )
{
    tl_string str;
    size_t i, j;

    /* initialized string is supposed to be empty */
    tl_string_init( &str );
    if( str.charcount || str.surrogates ) return EXIT_FAILURE;
    if( str.vec.used!=1 ) return EXIT_FAILURE;
    if( *((uint16_t*)tl_array_at( &(str.vec), 0 ))!=0 ) return EXIT_FAILURE;
    if( *tl_string_cstr( &str ) != 0 ) return EXIT_FAILURE;

    if( tl_string_characters( &str ) ) return EXIT_FAILURE;
    if( tl_string_length( &str ) ) return EXIT_FAILURE;
    if( !tl_string_is_empty( &str ) ) return EXIT_FAILURE;
    if( tl_string_at( &str, 0 ) ) return EXIT_FAILURE;

    /* append codepoints <= 0xFFFF */
    tl_string_append_code_point( &str, '!' );
    if( str.charcount!=1 || str.surrogates!=1 ) return EXIT_FAILURE;
    if( str.vec.used!=2 ) return EXIT_FAILURE;
    if( *((uint16_t*)tl_array_at( &(str.vec), 0 ))!='!' ) return EXIT_FAILURE;
    if( *((uint16_t*)tl_array_at( &(str.vec), 1 ))!=0 ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[0] != '!' ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[1] !=  0  ) return EXIT_FAILURE;

    if( tl_string_characters( &str ) != 1 ) return EXIT_FAILURE;
    if( tl_string_length( &str ) != 1 ) return EXIT_FAILURE;
    if( tl_string_is_empty( &str ) ) return EXIT_FAILURE;
    if( tl_string_at( &str, 0 )!='!' ) return EXIT_FAILURE;

    for( i=0; i<90; ++i )
    {
        if( str.charcount!=(i+1)||str.surrogates!=(i+1) ) return EXIT_FAILURE;
        if( str.vec.used!=(i+2) ) return EXIT_FAILURE;
        if( tl_string_characters( &str ) != (i+1) ) return EXIT_FAILURE;
        if( tl_string_length( &str ) != (i+1) ) return EXIT_FAILURE;
        if( tl_string_is_empty( &str ) ) return EXIT_FAILURE;
        if( tl_string_at( &str, 0 )!='!' ) return EXIT_FAILURE;

        if( *((uint16_t*)tl_array_at( &(str.vec), 0 ))!='!' )
            return EXIT_FAILURE;
        if( tl_string_cstr( &str )[0] != '!' )
            return EXIT_FAILURE;

        for( j=1; j<(str.vec.used-1); ++j )
        {
            if( tl_string_at( &str, j )!=('!'+j-1) )
                return EXIT_FAILURE;
            if( *((uint16_t*)tl_array_at( &(str.vec), j ))!=('!'+j-1) )
                return EXIT_FAILURE;
            if( tl_string_cstr( &str )[j] != ('!'+j-1) )
                return EXIT_FAILURE;
        }

        if( *((uint16_t*)tl_array_at(&(str.vec), j)) ) return EXIT_FAILURE;
        if( tl_string_cstr(&str)[j]                  ) return EXIT_FAILURE;

        tl_string_append_code_point( &str, '!'+i );
    }

    /* clear string */
    tl_string_clear( &str );
    if( str.charcount || str.surrogates ) return EXIT_FAILURE;
    if( str.vec.used!=1 ) return EXIT_FAILURE;
    if( *((uint16_t*)tl_array_at( &(str.vec), 0 ))!=0 ) return EXIT_FAILURE;
    if( *tl_string_cstr( &str ) != 0 ) return EXIT_FAILURE;

    if( tl_string_characters( &str ) ) return EXIT_FAILURE;
    if( tl_string_length( &str ) ) return EXIT_FAILURE;
    if( !tl_string_is_empty( &str ) ) return EXIT_FAILURE;

    /* append invalid codepoints */
    tl_string_append_code_point( &str, 0xD8FF );
    tl_string_append_code_point( &str, 0x00110000 );

    if( str.charcount!=2 || str.surrogates!=2 ) return EXIT_FAILURE;
    if( str.vec.used!=3 ) return EXIT_FAILURE;

    if( tl_string_characters( &str )!=2 ) return EXIT_FAILURE;
    if( tl_string_length( &str )!=2 ) return EXIT_FAILURE;
    if( tl_string_is_empty( &str ) ) return EXIT_FAILURE;

    if( *((uint16_t*)tl_array_at(&(str.vec),0))!=0xFFFD ) return EXIT_FAILURE;
    if( *((uint16_t*)tl_array_at(&(str.vec),1))!=0xFFFD ) return EXIT_FAILURE;
    if( *((uint16_t*)tl_array_at(&(str.vec),2))!=0      ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[0] != 0xFFFD ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[1] != 0xFFFD ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[2] != 0      ) return EXIT_FAILURE;

    /* append codepoints that generate surrogates */
    tl_string_clear( &str );
    tl_string_append_code_point( &str, 'A' );
    tl_string_append_code_point( &str, 'B' );
    tl_string_append_code_point( &str, 0x0000FFFF );
    tl_string_append_code_point( &str, 0x00010000 );
    tl_string_append_code_point( &str, 0x0001F710 );
    tl_string_append_code_point( &str, 0x0001F61C );
    tl_string_append_code_point( &str, 'B' );
    tl_string_append_code_point( &str, 'A' );

    if( str.charcount!=8 || str.surrogates!=3 ) return EXIT_FAILURE;
    if( str.vec.used!=12 ) return EXIT_FAILURE;
    if( tl_string_length( &str )!=11 ) return EXIT_FAILURE;

    if( tl_string_cstr( &str )[ 0] != 'A'    ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 1] != 'B'    ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 2] != 0xFFFF ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 3] != 0xD800 ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 4] != 0xDC00 ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 5] != 0xD83D ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 6] != 0xDF10 ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 7] != 0xD83D ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 8] != 0xDE1C ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 9] != 'B'    ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[10] != 'A'    ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[11] != 0      ) return EXIT_FAILURE;

    if( tl_string_at( &str, 0 ) != 'A'      ) return EXIT_FAILURE;
    if( tl_string_at( &str, 1 ) != 'B'      ) return EXIT_FAILURE;
    if( tl_string_at( &str, 2 ) != 0x00FFFF ) return EXIT_FAILURE;
    if( tl_string_at( &str, 3 ) != 0x010000 ) return EXIT_FAILURE;
    if( tl_string_at( &str, 4 ) != 0x01F710 ) return EXIT_FAILURE;
    if( tl_string_at( &str, 5 ) != 0x01F61C ) return EXIT_FAILURE;
    if( tl_string_at( &str, 6 ) != 'B'      ) return EXIT_FAILURE;
    if( tl_string_at( &str, 7 ) != 'A'      ) return EXIT_FAILURE;
    if( tl_string_at( &str, 8 ) != 0        ) return EXIT_FAILURE;

    /* append UTF-8 */
    tl_string_clear( &str );
    tl_string_append_utf8( &str, "\x24\xC2\xA2\xE2\x82\xAC\xF0\xA4\xAD\xA2" );

    if( str.charcount!=4 || str.surrogates!=3 ) return EXIT_FAILURE;
    if( str.vec.used!=6 ) return EXIT_FAILURE;
    if( tl_string_length( &str )!=5 ) return EXIT_FAILURE;

    if( tl_string_cstr( &str )[ 0] != 0x0024 ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 1] != 0x00A2 ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 2] != 0x20AC ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 3] != 0xD852 ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 4] != 0xDF62 ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 5] != 0x0000 ) return EXIT_FAILURE;

    if( tl_string_at( &str, 0 ) != 0x0024  ) return EXIT_FAILURE;
    if( tl_string_at( &str, 1 ) != 0x00A2  ) return EXIT_FAILURE;
    if( tl_string_at( &str, 2 ) != 0x20AC  ) return EXIT_FAILURE;
    if( tl_string_at( &str, 3 ) != 0x24B62 ) return EXIT_FAILURE;
    if( tl_string_at( &str, 4 ) != 0       ) return EXIT_FAILURE;

    /* append UTF-8 substring */
    tl_string_clear( &str );
    tl_string_append_utf8_count( &str,
                                 "\xE2\x82\xAC\xF0\xA4\xAD\xA2\xC2\xA2", 2 );

    if( str.charcount!=2 || str.surrogates!=1 ) return EXIT_FAILURE;
    if( str.vec.used!=4 ) return EXIT_FAILURE;
    if( tl_string_length( &str )!=3 ) return EXIT_FAILURE;

    if( tl_string_cstr( &str )[ 0] != 0x20AC ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 1] != 0xD852 ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 2] != 0xDF62 ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[ 3] != 0x0000 ) return EXIT_FAILURE;

    if( tl_string_at( &str, 0 ) != 0x20AC  ) return EXIT_FAILURE;
    if( tl_string_at( &str, 1 ) != 0x24B62 ) return EXIT_FAILURE;
    if( tl_string_at( &str, 2 ) != 0       ) return EXIT_FAILURE;

    tl_string_cleanup( &str );
    return EXIT_SUCCESS;
}

