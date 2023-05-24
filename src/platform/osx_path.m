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

str8 path_find_executable(mem_arena* arena)
{@autoreleasepool{
	str8 result = {};
	u32 size = 0;
	_NSGetExecutablePath(0, &size);
	result.len = size;
	result.ptr = mem_arena_alloc_array(arena, char, result.len);
	_NSGetExecutablePath(result.ptr, &size);
	return(result);
}}

str8 path_find_resource(mem_arena* arena, str8 relPath)
{
	str8_list list = {};
	mem_arena* scratch = mem_scratch();

	str8 executablePath = path_find_executable(scratch);
	str8 dirPath = path_slice_directory(executablePath);

	str8_list_push(scratch, &list, dirPath);
	str8_list_push(scratch, &list, STR8("/"));
	str8_list_push(scratch, &list, relPath);
	str8 path = str8_list_join(scratch, list);
	char* pathCString = str8_to_cstring(scratch, path);
	char* buffer = mem_arena_alloc_array(scratch, char, path.len+1);
	char* real = realpath(pathCString, buffer);

	str8 result = str8_push_cstring(arena, real);
	return(result);
}

str8 path_find_canonical(mem_arena* arena, str8 path)
{
	mem_arena* scratch = mem_scratch();
	char* pathCString = str8_to_cstring(scratch, path);

	char* real = realpath(pathCString, 0);
	str8 result = str8_push_cstring(arena, real);

	free(real);

	return(result);
}
