/*
    Cumbersome to use command line tool that compresses or
    uncompresses an input file.
 */
#include "tl_transform.h"
#include "tl_iostream.h"
#include "tl_splice.h"
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


static void opt_algo_callback( tl_option* opt, const char* value )
{
    (void)opt;
    if( !strcmp( value, "deflate" ) )
    {
        algo = TL_DEFLATE;
    }
    else if( !strcmp( value, "inflate" ) )
    {
        algo = TL_INFLATE;
    }
    else
    {
        fprintf( stderr, "Unknown compression algorithm '%s'\n", value );
        exit( EXIT_FAILURE );
    }
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
    puts( "usage: compress [ -fg ] [-a|--algorithm <algo>]\n"
          "                --in <file> --out <file> ...\n\n"

          "Compress or uncompress a file. If no options other than intput\n"
          "file and output file are given, the program defaults to deflate\n"
          "compressing the input file.\n\n"

          "Options:" );
    puts( "  -f, --fast\n"
          "    Prefere compression speed over file size.\n"
          "  -g, --good\n"
          "    Prefere small file size over compression speed.\n"
          "  -a, --algorithm <algo>\n"
          "    Specifies that the input file should be compressed with a\n"
          "    specific compression algorithm." );
    puts( "  --in <file>\n"
          "    Specifies the input file to process.\n"
          "  --out <file>\n"
          "    Specifies the output file to generate.\n" );

    puts( "The following algorithms are supported:\n"
          "  \"deflate\" (default)\n"
          "  \"inflate\"\n" );
}

static tl_option options[] =
{
    { TL_SHORT_OPTION, "a",          0,      NULL, opt_algo_callback },
    { TL_LONG_OPTION,  "algorithm",  0,      NULL, opt_algo_callback },
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
    tl_file *infile, *outfile;
    tl_transform *comp;
    char line[128];
    int i, ret;

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

    if( tl_fs_exists( out ) == 0 )
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
        goto out;
    }

    /* compress */
    comp = tl_create_transform( algo, flags );

    if( !comp )
    {
        fputs( "Compression algorithm unsupported\n", stderr );
        goto out;
    }

    /* convert in loop */
    while(infile != NULL || outfile != NULL)
    {
        if(infile)
        {
            ret = tl_iostream_splice( (tl_iostream*)comp, (tl_iostream*)infile,
                                      4096, NULL, 0 );

            if(ret == TL_EOF)
            {
                ((tl_iostream*)infile)->destroy( (tl_iostream*)infile );
                infile = NULL;
                comp->flush(comp, TL_TRANSFORM_FLUSH_EOF);
            }
            else if(ret < 0)
            {
                fputs("Error reading from input", stderr);
                goto out;
            }
        }

        ret = tl_iostream_splice( (tl_iostream*)outfile, (tl_iostream*)comp,
                                  4096, NULL, 0 );

        if(ret == TL_EOF)
        {
            ((tl_iostream*)outfile)->destroy( (tl_iostream*)outfile );
            outfile = NULL;
        }
        else if(ret < 0)
        {
            fputs("Error writing to output", stderr);
            goto out;
        }
    }

    /* cleanup */
    ((tl_iostream*)comp)->destroy( (tl_iostream*)comp );

out:
    if(outfile)
        ((tl_iostream*)outfile)->destroy( (tl_iostream*)outfile );
    if(infile)
        ((tl_iostream*)infile)->destroy( (tl_iostream*)infile );

    return outfile==NULL && infile==NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}

