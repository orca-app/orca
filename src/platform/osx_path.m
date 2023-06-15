/************************************************************//**
*
*	@file: osx_path.m
*	@author: Martin Fouilleul
*	@date: 24/05/2023
*
*****************************************************************/

#import<Foundation/Foundation.h>
#include<libgen.h>
#include<mach-o/dyld.h>

#include"platform_path.c"

bool path_is_absolute(str8 path)
{
	return(path.len && (path.ptr[0] == '/'));
}

str8 path_executable(mem_arena* arena)
{@autoreleasepool{
	str8 result = {};
	u32 size = 0;
	_NSGetExecutablePath(0, &size);
	result.len = size;
	result.ptr = mem_arena_alloc_array(arena, char, result.len);
	_NSGetExecutablePath(result.ptr, &size);
	return(result);
}}

str8 path_canonical(mem_arena* arena, str8 path)
{
	mem_arena_scope scratch = mem_scratch_begin_next(arena);
	char* pathCString = str8_to_cstring(scratch.arena, path);

	char* real = realpath(pathCString, 0);
	str8 result = str8_push_cstring(arena, real);

	free(real);
	mem_scratch_end(scratch);

	return(result);
}
