/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#ifndef __SYSTEM_H_
#define __SYSTEM_H_

#include "util/typedefs.h"
#include "util/strings.h"

typedef struct
{
    oc_str8 msg;
    i32 code;
} oc_sys_err_def;

extern oc_sys_err_def oc_sys_err;

oc_str8 oc_sys_getcwd(oc_arena* a);
bool oc_sys_path_exists(oc_str8 path);
bool oc_sys_mkdirs(oc_str8 path);
bool oc_sys_rmdir(oc_str8 path);
bool oc_sys_copy(oc_str8 src, oc_str8 dst);

#endif // __SYSTEM_H_
