/************************************************************//**
*
*	@file: platform_path.h
*	@author: Martin Fouilleul
*	@date: 24/05/2023
*
*****************************************************************/
#ifndef __PLATFORM_PATH_H_
#define __PLATFORM_PATH_H_

#include"util/strings.h"

/*NOTE:
	by convention, functions that take an arena and return a path
	allocated on that arena allocate 1 more character and null-terminate
	the string.
*/

MP_API str8 path_slice_directory(str8 path);
MP_API str8 path_slice_filename(str8 path);

MP_API str8_list path_split(mem_arena* arena, str8 path);
MP_API str8 path_join(mem_arena* arena, str8_list elements);
MP_API str8 path_append(mem_arena* arena, str8 parent, str8 relPath);

MP_API bool path_is_absolute(str8 path);
MP_API str8 path_executable(mem_arena* arena);
MP_API str8 path_canonical(mem_arena* arena, str8 path);

// helper: gets the path from path_executable() and appends relPath
MP_API str8 path_executable_relative(mem_arena* arena, str8 relPath);

#endif //__PLATFORM_PATH_H_
