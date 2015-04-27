/*
 * opt.c
 * This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#define TL_EXPORT
#include "tl_opt.h"
#include <string.h>


#define SET_FLAG( field, flag, value )\
        if(value) (field) |= (flag); else (field) &= ~(flag)


static void remove_option( int* argc, char** argv, int index, int count )
{
    int i;
    for( i=index+count; i<(*argc); ++i )
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
                        const char* arg, int type, size_t* index )
{
    size_t i;
    for( i=0; i<num_options; ++i )
    {
        if( options[i].type==type && !strcmp( arg, options[i].opt ) )
        {
            *index = i;
            return 1;
        }
    }
    return 0;
}

static int find_flag( tl_option* options, size_t num_options,
                      char flag, int type, size_t* index )
{
    size_t i;
    for( i=0; i<num_options; ++i )
    {
        if( options[i].type==type && options[i].opt[0]==flag )
        {
            *index = i;
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

    if( !options || !num_options || !argc || !argv )
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

