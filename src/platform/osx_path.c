/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include <libgen.h>
#include <mach-o/dyld.h>

#include "platform_path.c"

bool oc_path_is_absolute(oc_str8 path)
{
    return (path.len && (path.ptr[0] == '/'));
}

oc_str8 oc_path_executable(oc_arena* arena)
{
    oc_str8 result = {};
    u32 size = 0;
    _NSGetExecutablePath(0, &size);
    result.len = size;
    result.ptr = oc_arena_push_array(arena, char, result.len + 1);
    _NSGetExecutablePath(result.ptr, &size);
    result.ptr[result.len] = '\0';
    return (result);
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
