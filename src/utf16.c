#include "tl_utf16.h"



#define IS_SURROGATE( x ) (((x) >= 0xD800) && ((x) <= 0xDFFF))

#define LEAD_OFFSET (0xD800 - (0x10000 >> 10))
#define SURROGATE_OFFSET (0x10000 - (0xD800 << 10) - 0xDC00)

#define IS_LEAD_SURROGATE( x ) (((x)>=0xD800) && ((x)<=0xDBFF))
#define IS_TRAIL_SURROGATE( x ) (((x)>=0xDC00) && ((x)<=0xDFFF))



size_t tl_utf16_charcount( const uint16_t* str )
{
    size_t count = 0;

    if( str )
    {
        while( *str )
        {
            if( IS_LEAD_SURROGATE( str[0] ) )
            {
                if( IS_TRAIL_SURROGATE( str[1] ) )
                {
                    ++str;
                }
            }

            ++count;
            ++str;
        }
    }

    return count;
}

size_t tl_utf16_strlen( const uint16_t* str, size_t chars )
{
    size_t i, count = 0;

    for( i=0; i<chars; ++i )
    {
        if( IS_LEAD_SURROGATE( str[0] ) )
        {
            if( IS_TRAIL_SURROGATE( str[1] ) )
            {
                ++count;
                ++str;
            }
        }
        ++count;
        ++str;
    }

    return count;
}

unsigned int tl_utf16_decode( const uint16_t* utf16, unsigned int* count )
{
    if( count )
        *count = 0;

    if( !utf16 )
        return 0;

    if( IS_SURROGATE(*utf16) )
    {
        if( count )
            *count = 2;

        return (utf16[0] << 10) + utf16[1] + SURROGATE_OFFSET;
    }

    if( count )
        *count = 1;

    return utf16[0];
}

unsigned int tl_utf16_encode( uint16_t* utf16, unsigned int cp )
{
    if( !utf16 )
        return 0;

    if( cp < 0x10000 )
    {
        utf16[0] = cp;
        return 1;
    }

    utf16[0] = LEAD_OFFSET + (cp >> 10);
    utf16[1] = 0xDC00 + (cp & 0x3FF);
    return 2;
}

size_t tl_utf16_estimate_utf8_length( const char* str, size_t chars )
{
    const unsigned char* ptr = (const unsigned char*)str;
    size_t i, count;

    for( i=0, count=0; *ptr && i<chars; ++ptr )
    {
        if( (*ptr & 0xC0) == 0x80 )
            continue;

        count += ((*ptr & 0xF8)==0xF0) ? 2 : 1;
        ++i;
    }

    return count;
}

