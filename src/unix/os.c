#include "os.h"



int errno_to_fs( int code )
{
    switch( code )
    {
    case EPERM:
    case EACCES:
    case EROFS:
        return TL_FS_ACCESS;
    case ENOENT:
        return TL_FS_NOT_EXIST;
    case ENOTDIR:
        return TL_FS_NOT_DIR;
    case ENOSPC:
    case EDQUOT:
        return TL_FS_NO_SPACE;
    case EEXIST:
        return TL_FS_EXISTS;
    case ENOTEMPTY:
        return TL_FS_NOT_EMPTY;
    }

    return code==0 ? 0 : TL_FS_SYS_ERROR;
}

char* to_utf8( const tl_string* in )
{
    size_t count = tl_string_utf8_len( in );
    char* str = malloc( count + 1 );

    if( str )
        tl_string_to_utf8( in, str, count+1 );

    return str;
}

