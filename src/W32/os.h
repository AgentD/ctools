#ifndef OS_H
#define OS_H



#include "tl_string.h"
#include "tl_fs.h"

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>



#ifdef __cplusplus
extern "C" {
#endif

/** \brief Translate a GetLastError value to an TL_FS_* error code */
int errno_to_fs( int code );

#ifdef __cplusplus
}
#endif

#endif /* OS_H */

