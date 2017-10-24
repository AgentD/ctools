/*
    Renders a partial image of the Mandelbrot set and writes it
    out to a PNG file. tl_compress is used to deflate compress
    the image data for the PNG file.
 */
#include "tl_transform.h"
#include "tl_iostream.h"
#include "tl_blob.h"
#include "tl_hash.h"
#include "tl_file.h"

#define MAX_ITERATIONS 40

#define COLOR_RGB 2
#define COLOR_RGBA 6

static void mk_be32( unsigned char* temp, tl_u32 value )
{
    temp[0] = (value >> 24) & 0xFF;
    temp[1] = (value >> 16) & 0xFF;
    temp[2] = (value >>  8) & 0xFF;
    temp[3] =  value        & 0xFF;
}

static void write_chunk(tl_iostream* f, const char* id, const tl_blob* chunk)
{
    unsigned char temp[4];
    tl_u32 crc;

    mk_be32( temp, chunk ? chunk->size : 0 );
    f->write( f, temp, 4, NULL );
    f->write( f, id, 4, NULL );

    crc = tl_hash_crc32( 0, id, 4 );

    if( chunk )
    {
        tl_iostream_write_blob( f, chunk, NULL );
        crc = tl_hash_crc32( crc, chunk->data, chunk->size );
    }

    mk_be32( temp, crc );
    f->write( f, temp, 4, NULL );
}

static void write_header( tl_iostream* f, tl_u32 width, tl_u32 height,
                          tl_u8 type )
{
    tl_u8 buffer[13];
    tl_blob temp;

    mk_be32( buffer,   width );
    mk_be32( buffer+4, height );
    buffer[ 8] = 8;                 /* 8 bit per color */
    buffer[ 9] = type;              /* color type */
    buffer[10] = 0;                 /* deflate compression */
    buffer[11] = 0;                 /* default filtering */
    buffer[12] = 0;                 /* no interlace */

    temp.data = buffer;
    temp.size = sizeof(buffer);
    write_chunk( f, "IHDR", &temp );
}

static void write_image( tl_iostream* f, unsigned char* data,
                         tl_u32 width, tl_u32 height, tl_u8 colortype )
{
    unsigned int bpp = colortype == COLOR_RGBA ? 4 : 3, y, dx = width * bpp;
    unsigned char *inptr, *outptr;
    tl_blob img, compressed;

    tl_blob_init( &img, (width*bpp + 1) * height, NULL );

    inptr = data;
    outptr = img.data;

    for( y=0; y<height; ++y )
    {
        *(outptr++) = 0;                /* filter type: none */
        memcpy( outptr, inptr, dx );    /* scan line */
        outptr += dx;
        inptr += dx;
    }

    tl_transform_blob( &compressed, &img, TL_DEFLATE, TL_COMPRESS_GOOD );

    write_chunk( f, "IDAT", &compressed );
    tl_blob_cleanup( &compressed );
    tl_blob_cleanup( &img );
}

static void write_png_file( tl_iostream* f, unsigned char* image,
                            unsigned int width, unsigned int height,
                            int colortype )
{
    f->write( f, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8, NULL );
    write_header( f, width, height, colortype );
    write_image( f, image, width, height, colortype );
    write_chunk( f, "IEND", NULL );
}

static int is_in_mandelbrot( float X0, float Y0, float* r )
{
    float x = 0.0f, y = 0.0f, temp;
    size_t i;

    for( i=0; (x*x + y*y <= 4.0f) && i < MAX_ITERATIONS; ++i )
    {
        temp = x*x - y*y + X0;
        y = 2.0f*x*y + Y0;
        x = temp;
    }

    *r = ((float)i)/((float)MAX_ITERATIONS);
    return i == MAX_ITERATIONS;
}

static unsigned char image[ 800 * 600 * 3 ];

int main( void )
{
    unsigned char *ptr = image;
    unsigned int x, y;
    tl_file* f;
    float r;

    for( y=0; y<600; ++y )
    {
        for( x=0; x<800; ++x, ptr+=3 )
        {
            ptr[0] = ptr[1] = ptr[2] = 0x00;

            if( !is_in_mandelbrot( 3.0f*((float)x)/800.0f - 2.0f,
                                   2.0f*((float)y)/600.0f - 1.0f, &r ) )
            {
                ptr[0] = ( 9.0f*(1.0f-r)*r*r*r) * 255.0f;
                ptr[1] = (15.0f*(1.0f-r)*(1-r)*r*r) * 255.0f;
                ptr[2] = ( 8.5f*(1.0f-r)*(1-r)*(1-r)*r) * 255.0f;
            }
        }
    }


    tl_file_open( "test.png", &f, TL_WRITE|TL_CREATE|TL_OVERWRITE );
    write_png_file( (tl_iostream*)f, image, 800, 600, COLOR_RGB );
    ((tl_iostream*)f)->destroy( (tl_iostream*)f );
    return 0;
}

