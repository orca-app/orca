/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <shlwapi.h>                                           // PathIsRelative()

#include "platform_path.c"
#include "win32_string_helpers.h"

bool oc_path_is_absolute(oc_str8 path)
{
    oc_arena_scope scratch = oc_scratch_begin();
    oc_str16 pathW = oc_win32_utf8_to_wide(scratch.arena, path);
    bool result = !PathIsRelativeW(pathW.ptr);

    oc_scratch_end(scratch);
    return (result);
}

oc_str8 oc_path_executable(oc_arena* arena)
{
    ///////////////////////////////////////////////////////////////////
    //TODO use wide chars
    ///////////////////////////////////////////////////////////////////

    char* buffer = oc_arena_push_array(arena, char, MAX_PATH + 2);
    int size = GetModuleFileName(NULL, buffer, MAX_PATH + 1);
    //TODO: check for errors...
    for(int i = 0; i < size; i++)
    {
        if(buffer[i] == '\\')
        {
            buffer[i] = '/';
        }
    }
    return (oc_str8_from_buffer(size, buffer));
}

oc_str8 oc_path_canonical(oc_arena* arena, oc_str8 path); //TODO
