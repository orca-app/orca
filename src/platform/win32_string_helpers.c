/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "win32_string_helpers.h"

oc_str16 oc_win32_utf8_to_wide(oc_arena* arena, oc_str8 s)
{
    oc_str16 res = { 0 };
    res.len = 1 + MultiByteToWideChar(CP_UTF8, 0, s.ptr, s.len, NULL, 0);
    res.ptr = oc_arena_push_array(arena, u16, res.len);
    MultiByteToWideChar(CP_UTF8, 0, s.ptr, s.len, res.ptr, res.len);
    res.ptr[res.len - 1] = '\0';
    return (res);
}

oc_str8 oc_win32_wide_to_utf8(oc_arena* arena, oc_str16 s)
{
    oc_str8 res = { 0 };
    res.len = WideCharToMultiByte(CP_UTF8, 0, s.ptr, s.len, NULL, 0, NULL, NULL);
    res.ptr = oc_arena_push_array(arena, u8, res.len + 1);
    WideCharToMultiByte(CP_UTF8, 0, s.ptr, s.len, res.ptr, res.len, NULL, NULL);
    res.ptr[res.len] = '\0';
    return (res);
}

void oc_win32_path_normalize_slash_in_place(oc_str8 path)
{
    for(int i = 0; i < path.len; i++)
    {
        if(path.ptr[i] == '\\')
        {
            path.ptr[i] = '/';
        }
    }
}
