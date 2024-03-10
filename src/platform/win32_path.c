/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <shlwapi.h> // PathIsRelative()

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
    oc_str8 path = oc_str8_from_buffer(size, buffer);

    oc_win32_path_normalize_slash_in_place(path);
    return (path);
}

oc_str8 oc_path_canonical(oc_arena* arena, oc_str8 path)
{
	oc_arena_scope scratch = oc_scratch_begin_next(arena);
	oc_str16 pathW = oc_win32_utf8_to_wide(scratch.arena, path);

	DWORD required_size = GetFullPathNameW(pathW.ptr, 0, NULL, NULL);
	if(required_size == 0)
	{
		oc_log_error("oc_path_canonical: GetFullPathNameW failed to get required buffer size for full path of %.*s",
			oc_str8_ip(path));
		oc_scratch_end(scratch);
		return path;
	}

	WCHAR *path_buf = oc_arena_push(scratch.arena, required_size);
	DWORD retval = GetFullPathNameW(pathW.ptr, required_size, path_buf, NULL);
	if(retval == 0)
	{
		oc_log_error("oc_path_canonical: GetFullPathNameW failed to get the full path of %.*s",
			oc_str8_ip(path));
		oc_scratch_end(scratch);
		return path;
	}

	OC_ASSERT(retval < required_size);
	oc_str16 full_path_str16 = {.ptr = path_buf, .len = required_size - 1};
	oc_str8 result = oc_win32_wide_to_utf8(arena, full_path_str16);

	oc_scratch_end(scratch);

	return (result);
}
