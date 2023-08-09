/************************************************************//**
*
*	@file: win32_path.c
*	@author: Martin Fouilleul
*	@date: 24/05/2023
*
*****************************************************************/
#include<shlwapi.h> // PathIsRelative()

#include"win32_string_helpers.h"
#include"platform_path.c"

bool path_is_absolute(str8 path)
{
	mem_arena_scope scratch = mem_scratch_begin();
	str16 pathW = win32_utf8_to_wide_null_terminated(scratch.arena, path);
	bool result = !PathIsRelativeW(pathW.ptr);

	mem_scratch_end(scratch);
	return(result);
}

str8 path_executable(mem_arena* arena)
{
	///////////////////////////////////////////////////////////////////
	//TODO use wide chars
	///////////////////////////////////////////////////////////////////

	char* buffer = mem_arena_alloc_array(arena, char, MAX_PATH+2);
	int size = GetModuleFileName(NULL, buffer, MAX_PATH+1);
	//TODO: check for errors...
	for(int i=0; i<size; i++)
	{
		if(buffer[i] == '\\')
		{
			buffer[i] = '/';
		}
	}
	return(str8_from_buffer(size, buffer));
}

str8 path_canonical(mem_arena* arena, str8 path); //TODO
