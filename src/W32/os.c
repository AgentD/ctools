#include "os.h"



int errno_to_fs( int code )
{
    switch( code )
    {
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    case ERROR_DIRECTORY:
        return TL_FS_NOT_EXIST;

    case ERROR_INVALID_ACCESS:
        return TL_FS_ACCESS;

    case ERROR_FILE_EXISTS:
    case ERROR_ALREADY_EXISTS:
        return TL_FS_EXISTS;


    case ERROR_DISK_FULL:
        return TL_FS_NO_SPACE;

        /*return TL_FS_NOT_DIR;
        return TL_FS_NOT_EMPTY;*/
    }

    return code==0 ? 0 : TL_FS_SYS_ERROR;
}

