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

MP_API str8 path_slice_directory(str8 path);
MP_API str8 path_slice_filename(str8 path);

MP_API str8_list path_split(mem_arena* arena, str8 path);
MP_API str8 path_join(mem_arena* arena, str8_list elements);
MP_API str8 path_append_relative(mem_arena* arena, str8 parent, str8 relPath);

MP_API str8 path_find_executable(mem_arena* arena);
MP_API str8 path_find_resource(mem_arena* arena, str8 relPath);
MP_API str8 path_find_canonical(mem_arena* arena, str8 path);
MP_API str8 path_find_restricted(mem_arena* arena, str8 root, str8 relPath);

#endif //__PLATFORM_PATH_H_
