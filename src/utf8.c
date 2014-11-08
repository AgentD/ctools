#include "tl_utf8.h"



#define IS_SURROGATE( x ) (((x) >= 0xD800) && ((x) <= 0xDFFF))



size_t tl_utf8_charcount( const char* utf8 )
{
    const unsigned char* str = (const unsigned char*)utf8;
    size_t count = 0;

    while( *str )
    {
        if( (*str & 0xC0) != 0x80 )
            ++count;
        ++str;
    }

    return count;
}

size_t tl_utf8_strlen( const char* utf8, size_t chars )
{
    const unsigned char* str = (const unsigned char*)utf8;
    size_t i=0, count = 0;

    while( i < chars )
    {
        if( ((*str) & 0xC0) != 0x80 )
            ++i;

        ++count;
        ++str;
    }

    return count;
}

unsigned int tl_utf8_decode( const char* utf8, unsigned int* bytecount )
{
    unsigned char* data = (unsigned char*)utf8;

    if( bytecount )
        *bytecount = 0;

    if( !data )
        return 0;

    if( data[0] < 0x80 )
    {
        if( bytecount )
           *bytecount = 1;

        return data[0];
    }

    if( (data[0] & 0xE0)==0xC0 )
    {
        if( (data[1] & 0xC0)!=0x80 )
            return 0;

        if( bytecount )
           *bytecount = 2;

        return ((data[0] & 0x1F)<<6) | (data[1] & 0x3F);
    }

    if( (data[0] & 0xF0) == 0xE0 )
    {
        if( (data[1] & 0xC0)!=0x80 || (data[2] & 0xC0)!=0x80 )
            return 0;

        if( bytecount )
           *bytecount = 3;

        return ((data[0] & 0x0F)<<12) | ((data[1] & 0x3F)<<6) |
                (data[2] & 0x3F);
    }

    if( (data[0] & 0xF8) == 0xF0 )
    {
        if( (data[1] & 0xC0)!=0x80 || (data[2] & 0xC0)!=0x80 ||
            (data[3] & 0xC0)!=0x80 )
        {
            return 0;
        }

        if( bytecount )
           *bytecount = 4;

        return ((data[0] & 0x07)<<18) | ((data[1] & 0x3F)<<12) |
               ((data[2] & 0x3F)<< 6) |  (data[3] & 0x3F);
    }

    return 0;
}

unsigned int tl_utf8_encode( char* utf8, unsigned int cp )
{
    unsigned char* data = (unsigned char*)utf8;

    if( !data || cp>0x10FFFF )
        return 0;

    if( cp < 0x80 )
    {
        data[0] = cp;
        return 1;
    }

    if( cp < 0x800 )
    {
        data[0] = 0xC0 | ((cp>>6) & 0x1F);
        data[1] = 0x80 | ( cp     & 0x3F);
        return 2;
    }

    if( cp < 0x10000 )
    {
        data[0] = 0xE0 | ((cp>>12) & 0x0F);
        data[1] = 0x80 | ((cp>> 6) & 0x3F);
        data[2] = 0x80 | ( cp      & 0x3F);
        return 3;
    }

    data[0] = 0xF0 | ((cp>>18) & 0x07);
    data[1] = 0x80 | ((cp>>12) & 0x3F);
    data[2] = 0x80 | ((cp>> 6) & 0x3F);
    data[3] = 0x80 | ( cp      & 0x3F);
    return 4;
}

unsigned int tl_utf8_estimate_utf16_length( const uint16_t* in,
                                            size_t charcount )
{
    size_t i, count;

    if( !in || !charcount )
        return 0;

    for( count=0, i=0; i<charcount; ++i )
    {
        if( IS_SURROGATE(*in) )
        {
            count += 4;
            in += 2;
        }
        else
        {
                 if( (*in)<0x0080 ) count += 1;
            else if( (*in)<0x0800 ) count += 2;
            else                    count += 3;

            ++in;
        }
    }
    return count;
}
