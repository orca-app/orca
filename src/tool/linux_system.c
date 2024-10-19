/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "orca.h"
#include "system.h"

oc_sys_err_def oc_sys_err;

oc_str8 oc_sys_getcwd(oc_arena* a)
{
    oc_unimplemented();
    return (oc_str8){0};
}

bool oc_sys_exists(oc_str8 path)
{
    oc_unimplemented();
    return false;
}

bool oc_sys_isdir(oc_str8 path)
{
    oc_unimplemented();
    return false;
}

bool oc_sys_mkdirs(oc_str8 path)
{
    oc_unimplemented();
    return false;
}

int remove_recursive(char* path)
{
    oc_unimplemented();
    return 0;
}

bool oc_sys_rmdir(oc_str8 path)
{
    oc_unimplemented();
    return false;
}

bool oc_sys_copy(oc_str8 src, oc_str8 dst)
{
    oc_unimplemented();
    return false;
}

bool oc_sys_copytree(oc_str8 src, oc_str8 dst)
{
    oc_unimplemented();
    return false;
}

bool oc_sys_move(oc_str8 src, oc_str8 dst)
{
    oc_unimplemented();
    return false;
}

oc_list oc_sys_read_dir(oc_arena *a, oc_str8 path)
{
    oc_unimplemented();
    return (oc_list){0};
}
