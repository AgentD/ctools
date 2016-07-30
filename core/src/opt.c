/* opt.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_EXPORT
#include "tl_opt.h"
#include <string.h>


#define SET_FLAG( field, flag, value )\
        if(value) (field) |= (flag); else (field) &= ~(flag)


static void remove_option( int* argc, char** argv, int idx, int count )
{
    int i;
    for( i=idx+count; i<(*argc); ++i )
        argv[i-count] = argv[i];
    *argc -= count;
}

static int escape_string( char* str )
{
    if( str[0]=='\\' && (str[1]=='-' || str[1]=='+') )
        goto escape;
    if( str[0]=='\\' && str[1]=='\\' && (str[2]=='-' || str[2]=='+') )
        goto escape;
    return 0;
escape:
    memmove( str, str+1, strlen(str) );
    return 1;
}

static int find_string( tl_option* options, size_t num_options,
                        const char* arg, int type, size_t* idx )
{
    size_t i;
    for( i=0; i<num_options; ++i )
    {
        if( options[i].type==type && !strcmp( arg, options[i].opt ) )
        {
            *idx = i;
            return 1;
        }
    }
    return 0;
}

static int find_flag( tl_option* options, size_t num_options,
                      char flag, int type, size_t* idx )
{
    size_t i;
    for( i=0; i<num_options; ++i )
    {
        if( options[i].type==type && options[i].opt[0]==flag )
        {
            *idx = i;
            return 1;
        }
    }
    return 0;
}

static int can_resolve_chars( tl_option* options, size_t num_options,
                              const char* arg, int type )
{
    size_t i;
    while( *arg && find_flag( options, num_options, *arg, type, &i ) )
        ++arg;
    return *arg == '\0';
}

static int handle_toggle(tl_option* opt, size_t num, const char* arg, int on)
{
    size_t i;

    if( find_string( opt, num, arg, TL_LONG_TOGGLE, &i ) )
    {
        SET_FLAG( *(opt[i].field), opt[i].value, on );
        return 1;
    }

    if( !can_resolve_chars( opt, num, arg, TL_SHORT_TOGGLE ) )
        return 0;

    while( *arg && find_flag( opt, num, *(arg++), TL_SHORT_TOGGLE, &i ) )
    {
        SET_FLAG( *(opt[i].field), opt[i].value, on );
    }
    return 1;
}

/****************************************************************************/

int tl_process_args( tl_option* options, size_t num_options,
                     int* argc, char** argv,
                     tl_option_error_handler handler )
{
    int i = 1, status;
    const char* arg;
    size_t j=0, len;

    assert( options && argc && argv );

    if( !num_options )
        return 0;

    while( i < (*argc) )
    {
        if( escape_string(argv[i]) || (argv[i][0]!='-' && argv[i][0]!='+') )
            goto skip;

        if( !strcmp( argv[i], "--" ) )
        {
            remove_option( argc, argv, i, 1 );
            break;
        }
        else if( argv[i][0]=='+' )
        {
            if( handle_toggle( options, num_options, argv[i]+1, 1 ) )
                goto removeone;
        }
        else if( !strncmp( argv[i], "--", 2 ) )
        {
            arg = argv[i]+2;
            if( find_string( options, num_options, arg, TL_LONG_FLAG, &j ) )
            {
                *(options[j].field) |= options[j].value;
                goto removeone;
            }
            if( find_string( options, num_options, arg, TL_LONG_OPTION, &j ) )
            {
                if( i==(*argc - 1) || argv[i+1][0]=='-' || argv[i+1][0]=='+' )
                    goto missingarg;
                escape_string( argv[i+1] );
                options[j].handle_option( options+j, argv[i+1] );
                goto removetwo;
            }

            for( j=0; j<num_options; ++j )
            {
                if( options[j].type!=TL_LONG_OPTION )
                    continue;
                len = strlen( options[j].opt );
                if( strncmp( arg, options[j].opt, len )!=0 )
                    continue;
                if( !arg[len] || (arg[len]=='=' && !arg[len+1]) )
                    goto missingarg;
                if( arg[len]=='=' )
                {
                    options[j].handle_option( options+j, arg+len+1 );
                    goto removeone;
                }
            }
        }
        else
        {
            arg = argv[i]+1;
            if( handle_toggle( options, num_options, arg, 0 ) )
                goto removeone;

            if( can_resolve_chars(options,num_options,arg,TL_SHORT_FLAG) )
            {
                while( *arg )
                {
                    find_flag(options,num_options,*(arg++),TL_SHORT_FLAG,&j);
                    *(options[j].field) |= options[j].value;
                }
                goto removeone;
            }

            if( find_string(options,num_options,arg,TL_SHORT_OPTION,&j) )
            {
                if( i==(*argc - 1) || argv[i+1][0]=='-' || argv[i+1][0]=='+' )
                    goto missingarg;
                escape_string( argv[i+1] );
                options[j].handle_option( options+j, argv[i+1] );
                goto removetwo;
            }
        }

        status = handler ? handler(NULL,argv[i],TL_OPT_UNKNOWN) : TL_OPT_FAIL;
    handleaction:
        if( status==TL_OPT_IGNORE ) goto skip;
        if( status==TL_OPT_REMOVE ) goto removeone;
        return 0;
    missingarg:
        status = handler ? handler(options+j,0,TL_OPT_MISSING_ARGUMENT) :
                           TL_OPT_FAIL;
        goto handleaction;
    removetwo: remove_option( argc, argv, i, 2 ); continue;
    removeone: remove_option( argc, argv, i, 1 ); continue;
    skip:      ++i;
    }

    return 1;
}

