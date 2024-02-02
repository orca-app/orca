/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "platform_path.c"
#include <sys/auxv.h>
#include <string.h>

bool oc_path_is_absolute(oc_str8 path)
{
    return (path.len && (path.ptr[0] == '/'));
}

oc_str8 oc_path_executable(oc_arena* arena)
{
    const char* pathname = (const char*)getauxval(AT_EXECFN);
    OC_ASSERT(pathname);
    return oc_str8_push_cstring(arena, pathname);
}

oc_str8 oc_path_canonical(oc_arena* arena, oc_str8 path)
{
    oc_arena_scope scratch = oc_scratch_begin_next(arena);
    char* pathCString = oc_str8_to_cstring(scratch.arena, path);

    char* real = realpath(pathCString, 0);
    oc_str8 result = oc_str8_push_cstring(arena, real);

    free(real);
    oc_scratch_end(scratch);

    return (result);
}
