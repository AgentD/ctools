#include "tl_string.h"

#include <stdlib.h>
#include <string.h>



const tl_u16 utf16[12] =
{
    'A',
    'B',
    0xFFFF,
    0xD800, 0xDC00,
    0xD83D, 0xDF10,
    0xD83D, 0xDE1C,
    'B',
    'A',
    0
};

const unsigned int utf32[9] =
{
    'A',
    'B',
    0x0000FFFF,
    0x00010000,
    0x0001F710,
    0x0001F61C,
    'B',
    'A',
    0
};

const unsigned char utf8str[] =
{
    'A',
    'B',
    0xEF, 0xBF, 0xBF,
    0xF0, 0x90, 0x80, 0x80,
    0xF0, 0x9F, 0x9C, 0x90,
    0xF0, 0x9F, 0x98, 0x9C,
    'B',
    'A',
    0
};




int main( void )
{
    tl_u16 buffer[ 16 ];
    unsigned char* ptr;
    size_t i, j, len;
    tl_string str;

    /* initialized string is supposed to be empty */
    tl_string_init( &str );
    if( str.charcount || str.mbseq ) return EXIT_FAILURE;
    if( str.data.used!=1 ) return EXIT_FAILURE;
    if( ((char*)str.data.data)[0]!=0 ) return EXIT_FAILURE;
    if( *tl_string_cstr( &str ) != 0 ) return EXIT_FAILURE;

    if( tl_string_characters( &str ) ) return EXIT_FAILURE;
    if( tl_string_length( &str ) ) return EXIT_FAILURE;
    if( !tl_string_is_empty( &str ) ) return EXIT_FAILURE;
    if( tl_string_at( &str, 0 ) ) return EXIT_FAILURE;
    if( tl_string_utf16_len( &str )!=0 ) return EXIT_FAILURE;

    /* append codepoints <= 0x7F */
    tl_string_append_code_point( &str, '\n' );
    if( str.charcount!=1 || str.mbseq!=1 ) return EXIT_FAILURE;
    if( str.data.used!=2 ) return EXIT_FAILURE;
    if( ((char*)str.data.data)[0]!='\n' ) return EXIT_FAILURE;
    if( ((char*)str.data.data)[1]!=0 ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[0] != '\n' ) return EXIT_FAILURE;
    if( tl_string_cstr( &str )[1] !=  0  ) return EXIT_FAILURE;
    if( tl_string_utf16_len( &str ) != 1  ) return EXIT_FAILURE;

    if( tl_string_characters( &str ) != 1 ) return EXIT_FAILURE;
    if( tl_string_length( &str ) != 1 ) return EXIT_FAILURE;
    if( tl_string_is_empty( &str ) ) return EXIT_FAILURE;
    if( tl_string_at( &str, 0 )!='\n' ) return EXIT_FAILURE;

    for( i=0; i<100; ++i )
    {
        if( str.charcount!=(i+1)||str.mbseq!=(i+1) ) return EXIT_FAILURE;
        if( str.data.used!=(i+2) ) return EXIT_FAILURE;
        if( tl_string_characters( &str ) != (i+1) ) return EXIT_FAILURE;
        if( tl_string_length( &str ) != (i+1) ) return EXIT_FAILURE;
        if( tl_string_is_empty( &str ) ) return EXIT_FAILURE;
        if( tl_string_at( &str, 0 )!='\n' ) return EXIT_FAILURE;
        if( tl_string_utf16_len( &str )!=(i+1) ) return EXIT_FAILURE;

        if( ((char*)str.data.data)[0]!='\n' )
            return EXIT_FAILURE;
        if( tl_string_cstr( &str )[0] != '\n' )
            return EXIT_FAILURE;

        for( j=1; j<(str.data.used-1); ++j )
        {
            if( tl_string_at( &str, j )!=('\n'+j-1) )
                return EXIT_FAILURE;
            if( ((unsigned char*)str.data.data)[j]!=('\n'+j-1) )
                return EXIT_FAILURE;
            if( (unsigned int)tl_string_cstr( &str )[j] != ('\n'+j-1) )
                return EXIT_FAILURE;
        }

        if( ((char*)str.data.data)[j] ) return EXIT_FAILURE;
        if( tl_string_cstr(&str)[j]   ) return EXIT_FAILURE;

        tl_string_append_code_point( &str, '\n'+i );
    }

    /* clear string */
    tl_string_clear( &str );
    if( str.charcount || str.mbseq ) return EXIT_FAILURE;
    if( str.data.used!=1 ) return EXIT_FAILURE;
    if( ((char*)str.data.data)[0]!=0 ) return EXIT_FAILURE;
    if( *tl_string_cstr( &str ) != 0 ) return EXIT_FAILURE;

    if( tl_string_characters( &str ) ) return EXIT_FAILURE;
    if( tl_string_length( &str ) ) return EXIT_FAILURE;
    if( !tl_string_is_empty( &str ) ) return EXIT_FAILURE;
    if( tl_string_utf16_len( &str )!=0 ) return EXIT_FAILURE;

    /* append invalid codepoints */
    tl_string_append_code_point( &str, 0xD8FF );
    tl_string_append_code_point( &str, 0x00110000 );

    if( str.charcount || str.mbseq ) return EXIT_FAILURE;
    if( str.data.used!=1 ) return EXIT_FAILURE;

    if( tl_string_characters( &str ) ) return EXIT_FAILURE;
    if( tl_string_length( &str ) ) return EXIT_FAILURE;
    if( !tl_string_is_empty( &str ) ) return EXIT_FAILURE;
    if( tl_string_utf16_len( &str ) ) return EXIT_FAILURE;

    /* append codepoints that generate multi-byte sequences
       and UTF-16 surrogate pairs */
    tl_string_clear( &str );

    for( i=0, j=0xFFFFFFFF; utf32[i]; ++i )
    {
        if( utf32[i]>0x7F && i<j )
            j = i;
        tl_string_append_code_point( &str, utf32[i] );
    }

    len = strlen((const char*)utf8str);
    if( str.charcount!=i || str.mbseq!=j ) return EXIT_FAILURE;
    if( str.data.used!=(len+1) ) return EXIT_FAILURE;
    if( tl_string_length( &str )!=len ) return EXIT_FAILURE;
    if( tl_string_utf16_len(&str)!=(sizeof(utf16)/2-1) ) return EXIT_FAILURE;

    ptr = (unsigned char*)tl_string_cstr( &str );
    if( memcmp( ptr, utf8str, len+1 )!=0 )
        return EXIT_FAILURE;

    for( i=0; i<=str.charcount; ++i )
        if( tl_string_at( &str, i ) != utf32[i] )
            return EXIT_FAILURE;

    /* convert to UTF-16 */
    if( tl_string_to_utf16( &str, buffer, sizeof(buffer)/2 )!=
        (sizeof(utf16)/2-1) )
        return EXIT_FAILURE;

    if( memcmp( buffer, utf16, sizeof(utf16) )!=0 )
        return EXIT_FAILURE;

    if( tl_string_to_utf16( &str, buffer, 5 )!=3 )
        return EXIT_FAILURE;

    if( tl_string_to_utf16( &str, buffer, 1 )!=0 || buffer[0]!='\0' )
        return EXIT_FAILURE;

    /* append UTF-8 */
    tl_string_clear( &str );
    tl_string_append_utf8( &str, (const char*)utf8str );

    if( str.charcount!=8 || str.mbseq!=2 ) return EXIT_FAILURE;
    if( str.data.used!=20 ) return EXIT_FAILURE;
    if( tl_string_length( &str )!=19 ) return EXIT_FAILURE;
    if( tl_string_utf16_len( &str )!=11 ) return EXIT_FAILURE;

    len = strlen( (const char*)utf8str );
    if( memcmp( tl_string_cstr(&str), utf8str, len+1 )!=0 )
        return EXIT_FAILURE;

    for( i=0; i<=str.charcount; ++i )
    {
        if( tl_string_at( &str, i ) != utf32[i] )
            return EXIT_FAILURE;
    }

    /* append UTF-8 substring */
    tl_string_clear( &str );
    tl_string_append_utf8_count( &str,
                                 "\xE2\x82\xAC\xF0\xA4\xAD\xA2\xC2\xA2", 2 );

    if( str.charcount!=2 || str.mbseq!=0 ) return EXIT_FAILURE;
    if( str.data.used!=8 ) return EXIT_FAILURE;
    if( tl_string_length( &str )!=7 ) return EXIT_FAILURE;
    if( tl_string_utf16_len( &str )!=3 ) return EXIT_FAILURE;

    if( tl_string_at( &str, 0 ) != 0x20AC  ) return EXIT_FAILURE;
    if( tl_string_at( &str, 1 ) != 0x24B62 ) return EXIT_FAILURE;
    if( tl_string_at( &str, 2 ) != 0       ) return EXIT_FAILURE;

    tl_string_cleanup( &str );

    /* append UTF-16 */
    tl_string_init( &str );

    tl_string_append_utf16( &str, utf16 );
    if( str.charcount!=(sizeof(utf32)/sizeof(utf32[0])-1) || str.mbseq!=2 )
        return EXIT_FAILURE;
    if( str.data.used!=sizeof(utf8str) )
        return EXIT_FAILURE;
    if( strcmp( tl_string_cstr( &str ), (const char*)utf8str )!=0 )
        return EXIT_FAILURE;

    for( i=0; i<=str.charcount; ++i )
    {
        if( tl_string_at( &str, i ) != utf32[i] )
            return EXIT_FAILURE;
    }

    tl_string_cleanup( &str );

    /* append UTF-16 substring */
    tl_string_init( &str );

    tl_string_append_utf16_count( &str, utf16, 4 );
    if( str.charcount!=4 || str.mbseq!=2 ) return EXIT_FAILURE;
    if( str.data.used!=10 ) return EXIT_FAILURE;
    if( tl_string_utf16_len( &str )!=5 ) return EXIT_FAILURE;

    if( memcmp( str.data.data, utf8str, 9 )!=0 )
        return EXIT_FAILURE;
    tl_string_cleanup( &str );

    /* retrieve and remove end */
    tl_string_init( &str );

    if( tl_string_last( &str )!=0 ) return EXIT_FAILURE;
    tl_string_drop_last( &str );
    if( tl_string_last( &str )!=0 ) return EXIT_FAILURE;

    tl_string_append_code_point( &str, 'A' );
    if( tl_string_last( &str )!='A' ) return EXIT_FAILURE;
    tl_string_append_code_point( &str, 'B' );
    if( tl_string_last( &str )!='B' ) return EXIT_FAILURE;
    tl_string_append_code_point( &str, 0x0000FFFF );
    if( tl_string_last( &str )!=0xFFFF ) return EXIT_FAILURE;
    tl_string_append_code_point( &str, 0x00010000 );
    if( tl_string_last( &str )!=0x10000 ) return EXIT_FAILURE;
    tl_string_append_code_point( &str, 0x0001F710 );
    if( tl_string_last( &str )!=0x1F710 ) return EXIT_FAILURE;
    tl_string_append_code_point( &str, 0x0001F61C );
    if( tl_string_last( &str )!=0x1F61C ) return EXIT_FAILURE;
    tl_string_append_code_point( &str, 'B' );
    if( tl_string_last( &str )!='B' ) return EXIT_FAILURE;
    tl_string_append_code_point( &str, 'A' );
    if( tl_string_last( &str )!='A' ) return EXIT_FAILURE;

    if( str.charcount!=8 ) return EXIT_FAILURE;
    tl_string_drop_last( &str );
    if( str.charcount!=7 ) return EXIT_FAILURE;
    if( tl_string_last( &str )!='B' ) return EXIT_FAILURE;
    tl_string_drop_last( &str );
    if( str.charcount!=6 ) return EXIT_FAILURE;
    if( tl_string_last( &str )!=0x1F61C ) return EXIT_FAILURE;
    tl_string_drop_last( &str );
    if( str.charcount!=5 ) return EXIT_FAILURE;
    if( tl_string_last( &str )!=0x1F710 ) return EXIT_FAILURE;
    tl_string_drop_last( &str );
    if( str.charcount!=4 ) return EXIT_FAILURE;
    if( tl_string_last( &str )!=0x10000 ) return EXIT_FAILURE;
    tl_string_drop_last( &str );
    if( str.charcount!=3 ) return EXIT_FAILURE;
    if( tl_string_last( &str )!=0xFFFF ) return EXIT_FAILURE;
    tl_string_drop_last( &str );
    if( str.charcount!=2 ) return EXIT_FAILURE;
    if( tl_string_last( &str )!='B' ) return EXIT_FAILURE;
    tl_string_drop_last( &str );
    if( str.charcount!=1 ) return EXIT_FAILURE;
    if( tl_string_last( &str )!='A' ) return EXIT_FAILURE;
    tl_string_drop_last( &str );
    if( str.charcount!=0 ) return EXIT_FAILURE;
    if( tl_string_last( &str )!=0 ) return EXIT_FAILURE;
    tl_string_drop_last( &str );
    if( str.charcount!=0 ) return EXIT_FAILURE;
    if( tl_string_last( &str )!=0 ) return EXIT_FAILURE;

    tl_string_cleanup( &str );

    return EXIT_SUCCESS;
}

