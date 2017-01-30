/* splice.c -- This file is part of ctools
 *
 * Copyright (C) 2015 - David Oberhollenzer
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#define TL_OS_EXPORT
#include "tl_splice.h"
#include "platform.h"

static int splice_copy( tl_iostream* out, tl_iostream* in,
                        size_t count, size_t* actual )
{
    size_t indiff, outdiff, outcount = 0;
    int res_r = 0, res_w = 0, res = 0;
    char buffer[ 1024 ];

    while( count )
    {
        indiff = count > sizeof(buffer) ? sizeof(buffer) : count;
        res_r = in->read( in, buffer, indiff, &indiff );

        if( (res_r==0 && !indiff) || (res_r!=0 && res_r!=TL_EOF) )
        {
            res = res_r;
            break;
        }

        while( indiff )
        {
            res_w = out->write( out, buffer, indiff, &outdiff );

            if( res_w!=0 )
            {
                res = res_w;
                goto out;
            }

            indiff -= outdiff;
            count -= outdiff;
            outcount += outdiff;
        }

        if( res_r==TL_EOF )
        {
            res = res_r;
            break;
        }
    }
out:
    if( actual )
        *actual = outcount;

    return res;
}

int tl_iostream_splice( tl_iostream* out, tl_iostream* in,
                        size_t count, size_t* actual, int flags )
{
    int res;

    assert( out && in );

    if( actual )
        *actual = 0;

    if( !count )
        return 0;

    if( flags & (~TL_SPLICE_ALL_FLAGS) )
        return TL_ERR_ARG;

    res = __tl_os_splice( out, in, count, actual );

    if( (res == TL_ERR_NOT_SUPPORTED) && !(flags & TL_SPLICE_NO_FALLBACK) )
        res = splice_copy( out, in, count, actual );

    return res;
}

