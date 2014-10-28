#include "tl_string.h"

#include <string.h>



#define LEAD_OFFSET (0xD800 - (0x10000 >> 10))
#define SURROGATE_OFFSET (0x10000 - (0xD800 << 10) - 0xDC00)

#define IS_LEAD_SURROGATE( x ) (((x)>=0xD800) && ((x)<=0xDBFF))
#define IS_TRAIL_SURROGATE( x ) (((x)>=0xDC00) && ((x)<=0xDFFF))

#define IS_SURROGATE( x ) (((x) >= 0xD800) && ((x) <= 0xDFFF))
#define UNICODE_MAX 0x0010FFFF
#define REPLACEMENT_CHAR 0xFFFD
#define BOM 0xFEFF
#define BOM2 0xFFFE



/*
    determine how many UTF-16 code units are required to
    encode a given UTF-8 string
 */
static size_t utf8_count( const unsigned char* str, size_t chars )
{
    size_t i=0, count = 0;

    while( *str && i<chars )
    {
        if( (*str & 0xC0) != 0x80 )
        {
            count += ((*str & 0xF8)==0xF0) ? 2 : 1;
            ++i;
        }

        ++str;
    }

    return count;
}

static size_t utf8_strlen( const char* utf8 )
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

static size_t utf16_strlen( const uint16_t* str )
{
    size_t count = 0;

    while( *str )
    {
        if( IS_LEAD_SURROGATE( *str ) )
        {
            if( IS_TRAIL_SURROGATE( str[1] ) )
            {
                str += 2;
                ++count;
                continue;
            }
        }
        ++count;
        ++str;
    }

    return count;
}

static size_t utf16_code_count( const uint16_t* str, size_t chars )
{
    size_t i, count = 0;

    for( i=0; i<chars; ++i )
    {
        if( IS_LEAD_SURROGATE( *str ) )
        {
            if( IS_TRAIL_SURROGATE( str[1] ) )
            {
                count += 2;
                str += 2;
                continue;
            }
        }
        count += 1;
        str += 1;
    }

    return count;
}

/****************************************************************************/

int tl_string_init( tl_string* this )
{
    uint16_t null = 0;

    if( this )
    {
        tl_array_init( &(this->vec), sizeof(uint16_t) );
        this->charcount = 0;
        this->surrogates = 0;

        if( tl_array_append( &(this->vec), &null ) )
            return 1;
    }

    return 0;
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

size_t tl_string_characters( const tl_string* this )
{
    return this ? this->charcount : 0;
}

size_t tl_string_length( const tl_string* this )
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

int tl_string_is_empty( const tl_string* this )
{
    return (!this) || (this->charcount==0);
}

unsigned int tl_string_at( const tl_string* this, size_t index )
{
    const uint16_t* ptr;
    size_t i;

    if( this && (index < this->charcount) )
    {
        /* direct mapping of character index to array index */
        if( index < this->surrogates )
            return ((const uint16_t*)this->vec.data)[ index ];

        /* linearly search to target character index */
        ptr = ((const uint16_t*)this->vec.data) + this->surrogates;
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

int tl_string_append_code_point( tl_string* this, unsigned int cp )
{
    uint16_t val[2];

    if( !this )
        return 0;

    if( IS_SURROGATE(cp) || cp>UNICODE_MAX || cp==BOM || cp==BOM2 )
        cp = REPLACEMENT_CHAR;

    if( cp <= 0xFFFF )
    {
        val[0] = cp;

        if( !tl_array_insert( &(this->vec), this->vec.used - 1, val, 1 ) )
            return 0;

        /* no surrogates yet? */
        if( this->surrogates == this->charcount )
            ++this->surrogates;
    }
    else
    {
        val[0] = LEAD_OFFSET + (cp >> 10);
        val[1] = 0xDC00 + (cp & 0x3FF);

        if( !tl_array_insert( &(this->vec), this->vec.used - 1, val, 2 ) )
            return 0;
    }

    ++this->charcount;
    return 1;
}

int tl_string_append_utf8( tl_string* this, const char* utf8 )
{
    if( !this ) return 0;
    if( !utf8 ) return 1;

    return tl_string_append_utf8_count( this, utf8, utf8_strlen( utf8 ) );
}

int tl_string_append_latin1( tl_string* this, const char* latin1 )
{
    if( !this   ) return 0;
    if( !latin1 ) return 1;

    return tl_string_append_latin1_count( this, latin1, strlen( latin1 ) );
}

int tl_string_append_utf16( tl_string* this, const uint16_t* str )
{
    if( !this ) return 0;
    if( !str  ) return 1;

    return tl_string_append_utf16_count( this, str, utf16_strlen( str ) );
}

int tl_string_append_utf8_count( tl_string* this, const char* utf8,
                                 size_t count )
{
    unsigned char* str = (unsigned char*)utf8;
    unsigned int cp, i, len;
    size_t u8len, j;
    uint16_t* dst;

    if( !this           ) return 0;
    if( !utf8 || !count ) return 1;

    /* compute number of bytes to add */
    if( !(u8len = utf8_count( str, count )) )
        return 1;

    /* resize array */
    dst = (uint16_t*)this->vec.data + this->vec.used - 1;

    if( !tl_array_resize( &this->vec, this->vec.used + u8len ) )
        return 0;

    for( j=0; j<count; ++j, ++this->charcount )
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

        if( IS_SURROGATE(cp) || cp>UNICODE_MAX || cp==BOM || cp==BOM2 )
            cp = REPLACEMENT_CHAR;

        if( cp <= 0xFFFF )
        {
            *dst++ = cp;

            /* no surrogates yet? */
            if( this->surrogates == this->charcount )
                ++this->surrogates;
        }
        else
        {
            *dst++ = LEAD_OFFSET + (cp >> 10);
            *dst++ = 0xDC00 + (cp & 0x3FF);
        }
    }

    *dst = 0;
    return 1;
}

int tl_string_append_latin1_count( tl_string* this, const char* latin1,
                                   size_t count )
{
    unsigned char* str = (unsigned char*)latin1;
    uint16_t* dst;
    size_t i;

    if( !this ) return 0;
    if( !str  ) return 1;

    dst = (uint16_t*)this->vec.data + this->vec.used - 1;

    if( !tl_array_resize( &this->vec, this->vec.used + count ) )
        return 0;

    for( i=0; i<count; ++i )
        *(dst++) = *(str++);

    *dst = 0;

    if( this->surrogates == this->charcount )
        this->surrogates += count;

    this->charcount += count;
    return 1;
}

int tl_string_append_utf16_count( tl_string* this, const uint16_t* str,
                                  size_t count )
{
    size_t i, total;
    uint16_t* dst;

    if( !this          ) return 0;
    if( !str || !count ) return 1;

    total = utf16_code_count( str, count );

    if( !total )
        return 1;

    dst = (uint16_t*)this->vec.data + this->vec.used - 1;

    if( !tl_array_resize( &this->vec, this->vec.used + total ) )
        return 0;

    for( i=0; i<count; ++i )
    {
        if( IS_LEAD_SURROGATE( *str ) )
        {
            if( IS_TRAIL_SURROGATE( str[1] ) )
            {
                *(dst++) = *(str++);
                *(dst++) = *(str++);
                ++this->charcount;
                continue;
            }

            *(dst++) = REPLACEMENT_CHAR;
            ++str;
        }
        else if( IS_TRAIL_SURROGATE( *str ) )
        {
            *(dst++) = REPLACEMENT_CHAR;
            ++str;
        }
        else
        {
            *(dst++) = *(str++);
        }

        if( this->surrogates == (this->charcount++) )
            ++this->surrogates;
    }

    *dst = 0;
    return 1;
}

int tl_string_append_uint( tl_string* this, unsigned long value, int base )
{
    char buffer[ 128 ];     /* enough for a 128 bit number in base 2 */
    int digit, i = sizeof(buffer)-1;

    if( !this )
        return 0;

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

    return tl_string_append_latin1_count(this,buffer+i+1,sizeof(buffer)-i-1);
}

int tl_string_append_int( tl_string* this, long value, int base )
{
    char buffer[ 129 ];     /* enough for a 128 bit number in base 2 + sign */
    int digit, i = sizeof(buffer)-1, sign = 0;

    if( !this )
        return 0;

    if( !value )
    {
        buffer[ i-- ] = 0x30;
    }
    else
    {
        if( value < 0 )
        {
            sign = 1;
            value = -value;
        }

        base = base<2 ? 10 : (base>36 ? 36 : base);

        while( value!=0 )
        {
            digit = value % base;
            value /= base;

            buffer[ i-- ] = (digit<10) ? (0x30|digit) : (0x41 + digit - 10);
        }

        if( sign )
            buffer[ i-- ] = '-';
    }

    return tl_string_append_latin1_count(this,buffer+i+1,sizeof(buffer)-i-1);
}

size_t tl_string_utf8_len( const tl_string* this )
{
    const uint16_t* in;
    size_t i, count=0;
    unsigned int cp;

    if( this )
    {
        for( in=this->vec.data, i=0; i<this->charcount; ++i )
        {
            if( IS_SURROGATE(*in) )
            {
                cp  = ((*(in++)) << 10) + SURROGATE_OFFSET;
                cp +=   *(in++);
            }
            else
            {
                cp = *(in++);
            }

                 if( cp<=0x007F ) count += 1;
            else if( cp<=0x07FF ) count += 2;
            else if( cp<=0xFFFF ) count += 3;
            else                  count += 4;
        }
    }
    return count;
}

size_t tl_string_to_utf8( const tl_string* this, char* buffer, size_t size )
{
    unsigned char* chr;
    const uint16_t* in;
    unsigned int cp;
    size_t i, j;

    /* sanity check */
    if( !buffer || !size )
        return 0;

    if( !this )
    {
        buffer[0] = '\0';
        return 0;
    }

    chr = (unsigned char*)buffer;
    in = this->vec.data;

    for( j=0, i=0; i<this->charcount; ++i )
    {
        if( IS_SURROGATE(*in) )
        {
            cp  = ((*(in++)) << 10) + SURROGATE_OFFSET;
            cp +=   *(in++);
        }
        else
        {
            cp = *(in++);
        }
    
        if( cp<=0x007F )
        {
            ++j;
            if( j>=(size-1) )
                break;

            *(chr++) = cp;
        }
        else if( cp<=0x07FF )
        {
            j += 2;
            if( j>=(size-1) )
                break;

            *(chr++) = ((cp>>6) & 0x1F) | 0xC0;
            *(chr++) = ( cp     & 0x3F) | 0x80;
        }
        else if( cp<=0xFFFF )
        {
            j += 3;
            if( j>=(size-1) )
                break;

            *(chr++) = ((cp>>12) & 0x0F) | 0xE0;
            *(chr++) = ((cp>> 6) & 0x3F) | 0x80;
            *(chr++) = ( cp      & 0x3F) | 0x80;
        }
        else
        {
            j += 4;
            if( j>=(size-1) )
                break;

            *(chr++) = ((cp>>18) & 0x07) | 0xF0;
            *(chr++) = ((cp>>12) & 0x3F) | 0x80;
            *(chr++) = ((cp>> 6) & 0x3F) | 0x80;
            *(chr++) = ( cp      & 0x3F) | 0x80;
        }
    }

    *chr = '\0';
    return i;
}

unsigned int tl_string_last( const tl_string* this )
{
    unsigned int cp = 0;
    uint16_t* ptr;

    if( this && this->charcount )
    {
        ptr = (uint16_t*)this->vec.data + this->vec.used - 2;
        cp = *ptr;
        return IS_SURROGATE(cp) ? (cp+(ptr[-1]<<10)+SURROGATE_OFFSET) : cp;
    }

    return cp;
}

void tl_string_drop_last( tl_string* this )
{
    unsigned int cp;

    if( this && this->charcount )
    {
        cp = ((uint16_t*)this->vec.data)[this->vec.used - 2];

        tl_array_resize( &this->vec,
                         this->vec.used - (IS_SURROGATE(cp) ? 2 : 1) );

        ((uint16_t*)this->vec.data)[ this->vec.used - 1 ] = '\0';

        --this->charcount;

        if( this->surrogates > this->charcount )
            this->surrogates = this->charcount;
    }
}

