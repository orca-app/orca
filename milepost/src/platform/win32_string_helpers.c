/************************************************************//**
*
*	@file: win32_string_helpers.c
*	@author: Martin Fouilleul
*	@date: 24/05/2023
*
*****************************************************************/

#define WIN32_LEAN_AND_MEAN
#include<windows.h>

#include"win32_string_helpers.h"

str16 win32_utf8_to_wide_null_terminated(mem_arena* arena, str8 s)
{
	str16 res = {0};
	res.len = 1 + MultiByteToWideChar(CP_UTF8, 0, s.ptr, s.len, NULL, 0);
	res.ptr = mem_arena_alloc_array(arena, u16, res.len);
	MultiByteToWideChar(CP_UTF8, 0, s.ptr, s.len, res.ptr, res.len);
	res.ptr[res.len-1] = '\0';
	return(res);
}

str8 win32_wide_to_utf8(mem_arena* arena, str16 s)
{
	str8 res = {0};
	res.len = WideCharToMultiByte(CP_UTF8, 0, s.ptr, s.len, NULL, 0, NULL, NULL);
	res.ptr = mem_arena_alloc_array(arena, u8, res.len);
	WideCharToMultiByte(CP_UTF8, 0, s.ptr, s.len, res.ptr, res.len, NULL, NULL);
	return(res);
}
