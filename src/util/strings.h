/************************************************************//**
*
*	@file: strings.h
*	@author: Martin Fouilleul
*	@date: 29/05/2021
*	@revision:
*
*****************************************************************/
#ifndef __STRINGS_H_
#define __STRINGS_H_

#include"typedefs.h"
#include"lists.h"
#include"memory.h"

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------------
// string slices as values
//----------------------------------------------------------------------------------
typedef struct str8
{
	u64 len;
	char* ptr;
} str8;

#define str8_lit(s) ((str8){.len = sizeof(s)-1, .ptr = (char*)(s)})
#define str8_unbox(s) (int)((s).len), ((s).ptr)

str8 str8_from_buffer(u64 len, char* buffer);
str8 str8_from_cstring(char* str);
str8 str8_slice(str8 s, u64 start, u64 end);

str8 str8_push_buffer(mem_arena* arena, u64 len, char* buffer);
str8 str8_push_cstring(mem_arena* arena, const char* str);
str8 str8_push_copy(mem_arena* arena, str8 s);
str8 str8_push_slice(mem_arena* arena, str8 s, u64 start, u64 end);

str8 str8_pushfv(mem_arena* arena, const char* format, va_list args);
str8 str8_pushf(mem_arena* arena, const char* format, ...);

int str8_cmp(str8 s1, str8 s2);

char* str8_to_cstring(mem_arena* arena, str8 string);
//----------------------------------------------------------------------------------
// string lists
//----------------------------------------------------------------------------------
typedef struct str8_elt
{
	list_elt listElt;
	str8 string;
} str8_elt;

typedef struct str8_list
{
	list_info list;
	u64 eltCount;
	u64 len;
} str8_list;

void str8_list_push(mem_arena* arena, str8_list* list, str8 str);
void str8_list_pushf(mem_arena* arena, str8_list* list, const char* format, ...);

str8 str8_list_join(mem_arena* arena, str8_list list);
str8_list str8_split(mem_arena* arena, str8 str, str8_list separators);

//----------------------------------------------------------------------------------
// u32 strings
//----------------------------------------------------------------------------------
typedef struct str32
{
	u64 len;
	u32* ptr;
} str32;

str32 str32_from_buffer(u64 len, u32* buffer);
str32 str32_slice(str32 s, u64 start, u64 end);

str32 str32_push_buffer(mem_arena* arena, u64 len, u32* buffer);
str32 str32_push_copy(mem_arena* arena, str32 s);
str32 str32_push_slice(mem_arena* arena, str32 s, u64 start, u64 end);

typedef struct str32_elt
{
	list_elt listElt;
	str32 string;
} str32_elt;

typedef struct str32_list
{
	list_info list;
	u64 eltCount;
	u64 len;
} str32_list;

void str32_list_push(mem_arena* arena, str32_list* list, str32 str);
str32 str32_list_join(mem_arena* arena, str32_list list);
str32_list str32_split(mem_arena* arena, str32 str, str32_list separators);

//----------------------------------------------------------------------------------
// Paths helpers
//----------------------------------------------------------------------------------
str8 mp_path_directory(str8 fullPath);
str8 mp_path_base_name(str8 fullPath);

#ifdef __cplusplus
} // extern "C"
#endif


#endif //__STRINGS_H_
