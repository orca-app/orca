/************************************************************/ /**
*
*	@file: platform_path.h
*	@author: Martin Fouilleul
*	@date: 24/05/2023
*
*****************************************************************/
#ifndef __PLATFORM_PATH_H_
#define __PLATFORM_PATH_H_

#include"platform.h"
#include "util/strings.h"

/*NOTE:
	by convention, functions that take an arena and return a path
	allocated on that arena allocate 1 more character and null-terminate
	the string.
*/

ORCA_API oc_str8 oc_path_slice_directory(oc_str8 path);
ORCA_API oc_str8 oc_path_slice_filename(oc_str8 path);

ORCA_API oc_str8_list oc_path_split(oc_arena* arena, oc_str8 path);
ORCA_API oc_str8 oc_path_join(oc_arena* arena, oc_str8_list elements);
ORCA_API oc_str8 oc_path_append(oc_arena* arena, oc_str8 parent, oc_str8 relPath);

ORCA_API bool oc_path_is_absolute(oc_str8 path);

#if !defined(OC_PLATFORM_ORCA) || !OC_PLATFORM_ORCA

ORCA_API oc_str8 oc_path_executable(oc_arena* arena);
ORCA_API oc_str8 oc_path_canonical(oc_arena* arena, oc_str8 path);

// helper: gets the path from oc_path_executable() and appends relPath
ORCA_API oc_str8 oc_path_executable_relative(oc_arena* arena, oc_str8 relPath);
#endif

#endif //__PLATFORM_PATH_H_
