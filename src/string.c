#include "tl_string.h"

#include <stdarg.h>



#define LEAD_OFFSET (0xD800 - (0x10000 >> 10))
#define SURROGATE_OFFSET (0x10000 - (0xD800 << 10) - 0xDC00)

#define IS_SURROGATE( x ) (((x) >= 0xD800) && ((x) <= 0xDFFF))
#define UNICODE_MAX 0x0010FFFF
#define REPLACEMENT_CHAR 0xFFFD
#define BOM 0xFEFF
#define BOM2 0xFFFE




void tl_string_init( tl_string* this )
{
    uint16_t null = 0;

    if( this )
    {
        tl_array_init( &(this->vec), sizeof(uint16_t) );
        tl_array_append( &(this->vec), &null );

        this->charcount = 0;
        this->surrogates = 0;
    }
}

void tl_string_cleanup( tl_string* this )
{
    if( this )
    {
        tl_array_cleanup( &(this->vec) );

        this->charcount = 0;
        this->surrogates = 0;
    }
}

int tl_string_copy( tl_string* this, const tl_string* src )
{
    if( tl_array_copy( &this->vec, &src->vec ) )
        return 0;

    this->charcount  = src->charcount;
    this->surrogates = src->surrogates;
    return 1;
}

size_t tl_string_characters( tl_string* this )
{
    return this ? this->charcount : 0;
}

size_t tl_string_length( tl_string* this )
{
    return this ? (this->vec.used - 1) : 0;
}

void tl_string_clear( tl_string* this )
{
    if( this )
    {
        tl_array_resize( &(this->vec), 1 );
        *((uint16_t*)this->vec.data) = 0;

        this->charcount = 0;
        this->surrogates = 0;
    }
}

int tl_string_is_empty( tl_string* this )
{
    return (!this) || (this->charcount==0);
}

unsigned int tl_string_at( tl_string* this, size_t index )
{
    uint16_t* ptr;
    size_t i;

    if( this && (index < this->charcount) )
    {
        /* direct mapping of character index to array index */
        if( index < this->surrogates )
            return ((uint16_t*)this->vec.data)[ index ];

        /* linearly search to target character index */
        ptr = ((uint16_t*)this->vec.data) + this->surrogates;
        i = this->surrogates;

        while( i<index )
        {
            ptr += IS_SURROGATE(*ptr) ? 2 : 1;
            ++i;
        }

        /* decode and return target index value */
        if( IS_SURROGATE(*ptr) )
            return (ptr[0] << 10) + ptr[1] + SURROGATE_OFFSET;

        return *ptr;
    }

    return 0;
}

uint16_t* tl_string_cstr( tl_string* this )
{
    return this ? this->vec.data : NULL;
}

void tl_string_append_code_point( tl_string* this, unsigned int cp )
{
    uint16_t val[2];

    if( this )
    {
        if( IS_SURROGATE(cp) || cp>UNICODE_MAX || cp==BOM || cp==BOM2 )
            cp = REPLACEMENT_CHAR;

        if( cp <= 0xFFFF )
        {
            val[0] = cp;

            tl_array_insert( &(this->vec), this->vec.used - 1, val, 1 );

            /* no surrogates yet? */
            if( this->surrogates == this->charcount )
                ++this->surrogates;
        }
        else
        {
            val[0] = LEAD_OFFSET + (cp >> 10);
            val[1] = 0xDC00 + (cp & 0x3FF);

            tl_array_insert( &(this->vec), this->vec.used - 1, val, 2 );
        }

        ++this->charcount;
    }
}

void tl_string_append_utf8( tl_string* this, const char* utf8 )
{
    unsigned char* str = (unsigned char*)utf8;
    unsigned int cp, i, len;

    if( !this || !str )
        return;

    while( *str )
    {
        len = 1;
        cp = *(str++);

             if( (cp & 0xFE) == 0xFC ) { len = 6; cp &= 0x01; }
        else if( (cp & 0xFC) == 0xF8 ) { len = 5; cp &= 0x03; }
        else if( (cp & 0xF8) == 0xF0 ) { len = 4; cp &= 0x07; }
        else if( (cp & 0xF0) == 0xE0 ) { len = 3; cp &= 0x0F; }
        else if( (cp & 0xE0) == 0xC0 ) { len = 2; cp &= 0x1F; }
        else if( cp >= 0x80          ) { len = 1; cp = REPLACEMENT_CHAR; }

        for( i=1; i<len; ++i )
        {
            if( ((*str) & 0xC0)!=0x80 )
            {
                ++str;
                cp = REPLACEMENT_CHAR;
                break;
            }

            cp <<= 6;
            cp |= (*(str++)) & 0x3F;
        }

        tl_string_append_code_point( this, cp );
    }
}

void tl_string_append_latin1( tl_string* this, const char* latin1 )
{
    unsigned char* str = (unsigned char*)latin1;

    if( !this || !str )
        return;

    while( *str )
        tl_string_append_code_point( this, *(str++) );
}

void tl_string_append_utf8_count( tl_string* this, const char* utf8,
                                  size_t count )
{
    unsigned char* str = (unsigned char*)utf8;
    unsigned int cp, i, len;
    size_t j;

    if( !this || !str )
        return;

    for( j=0; j<count; ++j )
    {
        len = 1;
        cp = *(str++);

             if( (cp & 0xFE) == 0xFC ) { len = 6; cp &= 0x01; }
        else if( (cp & 0xFC) == 0xF8 ) { len = 5; cp &= 0x03; }
        else if( (cp & 0xF8) == 0xF0 ) { len = 4; cp &= 0x07; }
        else if( (cp & 0xF0) == 0xE0 ) { len = 3; cp &= 0x0F; }
        else if( (cp & 0xE0) == 0xC0 ) { len = 2; cp &= 0x1F; }
        else if( cp >= 0x80          ) { len = 1; cp = REPLACEMENT_CHAR; }

        for( i=1; i<len; ++i )
        {
            if( ((*str) & 0xC0)!=0x80 )
            {
                ++str;
                cp = REPLACEMENT_CHAR;
                break;
            }

            cp <<= 6;
            cp |= (*(str++)) & 0x3F;
        }

        tl_string_append_code_point( this, cp );
    }
}

void tl_string_append_latin1_count( tl_string* this, const char* latin1,
                                    size_t count )
{
    unsigned char* str = (unsigned char*)latin1;
    size_t i;

    if( !this || !str )
        return;

    for( i=0; i<count; ++i )
        tl_string_append_code_point( this, *(str++) );
}

void tl_string_append_uint( tl_string* this, unsigned long value, int base )
{
    char buffer[ 128 ];     /* enough for a 128 bit number in base 2 */
    int digit, i = sizeof(buffer)-1;

    if( !this )
        return;

    if( !value )
    {
        buffer[ i-- ] = 0x30;
    }
    else
    {
        base = base<2 ? 10 : (base>36 ? 36 : base);

        while( value!=0 )
        {
            digit = value % base;
            value /= base;

            buffer[ i-- ] = (digit<10) ? (0x30|digit) : (0x41 + digit - 10);
        }
    }

    tl_string_append_latin1_count( this, buffer+i+1, sizeof(buffer)-i-1 );
}

void tl_string_append_int( tl_string* this, long value, int base )
{
    if( value < 0 )
    {
        tl_string_append_code_point( this, '-' );
        value = -value;
    }

    tl_string_append_uint( this, value, base );
}

