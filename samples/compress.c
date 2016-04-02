/*
    Cumbersome to use command line tool that compresses or
    uncompresses an input file.
 */
#include "tl_compress.h"
#include "tl_opt.h"
#include "tl_fs.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>


#define COMP 1
#define UNCOMP 2


static int algo = TL_DEFLATE;
static unsigned long flags = TL_COMPRESS_FAST;
static const char* in = NULL;
static const char* out = NULL;
static int compress = COMP;


static void opt_algo_callback( tl_option* opt, const char* value )
{
    if( !strcmp( value, "deflate" ) )
    {
        algo = TL_DEFLATE;
    }
    else
    {
        fprintf( stderr, "Unknown compression algorithm '%s'\n", value );
        exit( EXIT_FAILURE );
    }

    compress = opt->value;
}

static void set_in_file( tl_option* opt, const char* value )
{
    (void)opt;
    if( in )
    {
        fputs( "Input file specified more than once!\n", stderr );
        exit( EXIT_FAILURE );
    }
    in = value;
}

static void set_out_file( tl_option* opt, const char* value )
{
    (void)opt;
    if( out )
    {
        fputs( "Output file specified more than once!\n", stderr );
        exit( EXIT_FAILURE );
    }
    out = value;
}

static int opt_error_handler( tl_option* opt, const char* option, int issue )
{
    if( issue==TL_OPT_UNKNOWN )
        fprintf( stderr, "Unknown option '%s'\n", option );
    else if( issue==TL_OPT_MISSING_ARGUMENT )
        fprintf( stderr, "Option '%s' requires an argument\n", opt->opt );
    return TL_OPT_FAIL;
}

static void usage( void )
{
    puts( "usage: compress [ -fg ] [-c|--compress|-d|--uncompress <algo>]\n"
          "                --in <file> --out <file> ...\n\n"

          "Compress or uncompress a file. If no options other than intput\n"
          "file and output file are given, the program defaults to deflate\n"
          "compressing the input file.\n\n"

          "Options:" );
    puts( "  -f, --fast\n"
          "    Prefere compression speed over file size.\n"
          "  -g, --good\n"
          "    Prefere small file size over compression speed.\n"
          "  -c, --compress <algo>\n"
          "    Specifies that the input file should be compressed with a\n"
          "    specific compression algorithm." );
    puts( "  -d, --uncompress <algo>\n"
          "    Specifies that the input file is compressed with the given\n"
          "    compression algorithm and should be uncompressed.\n"
          "  --in <file>\n"
          "    Specifies the input file to process.\n"
          "  --out <file>\n"
          "    Specifies the output file to generate.\n" );

    puts( "The following compression algorithms are supported:\n"
          "  \"deflate\" (default)\n" );
}

static tl_option options[] =
{
    { TL_SHORT_OPTION, "c",          COMP,   NULL, opt_algo_callback },
    { TL_LONG_OPTION,  "compress",   COMP,   NULL, opt_algo_callback },
    { TL_SHORT_OPTION, "d",          UNCOMP, NULL, opt_algo_callback },
    { TL_LONG_OPTION,  "uncompress", UNCOMP, NULL, opt_algo_callback },
    { TL_LONG_OPTION,  "in",         0,      NULL, set_in_file       },
    { TL_LONG_OPTION,  "out",        0,      NULL, set_out_file      },

    { TL_SHORT_FLAG,   "f",          TL_COMPRESS_FAST, &flags, NULL },
    { TL_SHORT_FLAG,   "g",          TL_COMPRESS_GOOD, &flags, NULL },
    { TL_LONG_FLAG,    "fast",       TL_COMPRESS_FAST, &flags, NULL },
    { TL_LONG_FLAG,    "good",       TL_COMPRESS_GOOD, &flags, NULL },
};


int main( int argc, char** argv )
{
    size_t numoptions = sizeof(options)/sizeof(options[0]);
    int i, status = EXIT_FAILURE, ret;
    char line[128], *buffer;
    FILE *infile, *outfile;
    tl_blob dst;
    tl_u64 size;

    /* process command line */
    for( i=1; i<argc; ++i )
    {
        if( !strcmp( argv[i], "--help" ) || !strcmp( argv[i], "-h" ) )
        {
            usage( );
            return EXIT_SUCCESS;
        }
    }

    if( !tl_process_args(options,numoptions,&argc,argv,opt_error_handler) )
        return EXIT_FAILURE;

    /* sanity check input and output file paths */
    if( !in )
    {
        fputs( "No input file specified\n", stderr );
        return EXIT_FAILURE;
    }

    if( !out )
    {
        fputs( "No output file specified\n", stderr );
        return EXIT_FAILURE;
    }

    if( tl_fs_exists( out ) )
    {
        printf( "WARNING: Output file '%s' exists, overwrite [y|N]? ", out );
        fgets( line, sizeof(line), stdin );

        for( i=0; isspace(line[i]); ++i ) { }

        if( line[i] != 'y' && line[i] != 'Y' )
        {
            puts( "Aborting" );
            return EXIT_FAILURE;
        }
    }

    /* open files */
    infile = fopen( in, "rb" );
    if( !infile )
    {
        fprintf( stderr, "%s: %s\n", in, strerror(errno) );
        return EXIT_FAILURE;
    }

    outfile = fopen( out, "wb" );
    if( !outfile )
    {
        fprintf( stderr, "%s: %s\n", out, strerror(errno) );
        goto outinf;
    }

    /* read input file into buffer */
    size = tl_fs_get_file_size( in );
    buffer = malloc( size );

    if( !buffer )
    {
        fputs( "Out of memory\n", stderr );
        goto outfiles;
    }

    if( fread( buffer, 1, size, infile ) != size )
    {
        fprintf( stderr, "fread: %s\n", strerror(errno) );
        goto outbuf;
    }

    /* process input buffer */
    if( compress == COMP )
        ret = tl_compress( &dst, buffer, size, algo, flags );
    else
        ret = tl_uncompress( &dst, buffer, size, algo );

    switch( ret )
    {
    case 0:
        break;
    case TL_ERR_NOT_SUPPORTED:
        fputs( "Compression algorithm unsupported\n", stderr );
        goto outbuf;
    case TL_ERR_ALLOC:
        fputs( "Out of memory\n", stderr );
        goto outbuf;
    default:
        fputs( "Unknown error\n", stderr );
        goto outbuf;
    }

    /* write out result */
    if( fwrite( dst.data, 1, dst.size, outfile ) != dst.size )
    {
        fprintf( stderr, "fwrite: %s\n", strerror(errno) );
        goto outblob;
    }

    /* cleanup */
    status = EXIT_SUCCESS;
outblob:  tl_blob_cleanup( &dst );
outbuf:   free( buffer );
outfiles: fclose( outfile );
outinf:   fclose( infile );
    return status;
}

