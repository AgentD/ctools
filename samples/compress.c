/*
    Cumbersome to use command line tool that compresses or
    uncompresses an input file.
 */
#include "tl_compress.h"
#include "tl_iostream.h"
#include "tl_file.h"
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
    tl_iostream *infile, *outfile;
    const tl_blob* map;
    char line[128];
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
    if( tl_file_open( in, &infile, TL_READ ) != 0 )
    {
        fprintf( stderr, "error opening %s\n", in );
        return EXIT_FAILURE;
    }

    if( tl_file_open( out, &outfile, TL_WRITE|TL_CREATE|TL_OVERWRITE ) != 0 )
    {
        fprintf( stderr, "error opening %s\n", out );
        goto outinf;
    }

    /* map input file into memory */
    if( tl_fs_get_file_size( in, &size ) != 0 )
    {
        fprintf( stderr, "Cannot get size of %s\n", in );
        goto outfiles;
    }

    map = tl_file_map( infile, 0, size, TL_MAP_READ );

    if( !map )
    {
        fprintf( stderr, "Error mapping %s\n", in );
        goto outfiles;
    }

    /* process input */
    if( compress == COMP )
        ret = tl_compress_blob( &dst, map, algo, flags );
    else
        ret = tl_uncompress_blob( &dst, map, algo );

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
    if( tl_iostream_write_blob( outfile, &dst, NULL ) != 0 )
    {
        fprintf( stderr, "write: %s\n", out );
        goto outblob;
    }

    /* cleanup */
    status = EXIT_SUCCESS;
outblob:  tl_blob_cleanup( &dst );
outbuf:   tl_file_unmap( map );
outfiles: outfile->destroy( outfile );
outinf:   infile->destroy( infile );
    return status;
}

