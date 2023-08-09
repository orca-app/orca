/************************************************************//**
*
*	@file: win32_string_helpers.h
*	@author: Martin Fouilleul
*	@date: 24/05/2023
*
*****************************************************************/
#ifndef __WIN32_STRING_HELPERS_H_
#define __WIN32_STRING_HELPERS_H_

#include"util/strings.h"

str16 win32_utf8_to_wide_null_terminated(mem_arena* arena, str8 s);
str8 win32_wide_to_utf8(mem_arena* arena, str16 s);


#endif // __WIN32_STRING_HELPERS_H_
