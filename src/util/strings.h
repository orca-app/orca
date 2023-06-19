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
#include"platform/platform_strings.h"
#include"platform/platform_varg.h"

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

#define STR8(s) ((str8){.len = (s) ? strlen(s) : 0, .ptr = (char*)s})

#define str8_lp(s) ((s).len), ((s).ptr)
#define str8_ip(s) (int)str8_lp(s)

MP_API str8 str8_from_buffer(u64 len, char* buffer);
MP_API str8 str8_slice(str8 s, u64 start, u64 end);

MP_API str8 str8_push_buffer(mem_arena* arena, u64 len, char* buffer);
MP_API str8 str8_push_cstring(mem_arena* arena, const char* str);
MP_API str8 str8_push_copy(mem_arena* arena, str8 s);
MP_API str8 str8_push_slice(mem_arena* arena, str8 s, u64 start, u64 end);

MP_API str8 str8_pushfv(mem_arena* arena, const char* format, va_list args);
MP_API str8 str8_pushf(mem_arena* arena, const char* format, ...);

MP_API int str8_cmp(str8 s1, str8 s2);

MP_API char* str8_to_cstring(mem_arena* arena, str8 string);
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

MP_API void str8_list_push(mem_arena* arena, str8_list* list, str8 str);
MP_API void str8_list_pushf(mem_arena* arena, str8_list* list, const char* format, ...);

MP_API str8 str8_list_collate(mem_arena* arena, str8_list list, str8 prefix, str8 separator, str8 postfix);
MP_API str8 str8_list_join(mem_arena* arena, str8_list list);
MP_API str8_list str8_split(mem_arena* arena, str8 str, str8_list separators);

//----------------------------------------------------------------------------------
// u16 strings
//----------------------------------------------------------------------------------
typedef struct str16
{
	u64 len;
	u16* ptr;
} str16;

MP_API str16 str16_from_buffer(u64 len, u16* buffer);
MP_API str16 str16_slice(str16 s, u64 start, u64 end);

MP_API str16 str16_push_buffer(mem_arena* arena, u64 len, u16* buffer);
MP_API str16 str16_push_copy(mem_arena* arena, str16 s);
MP_API str16 str16_push_slice(mem_arena* arena, str16 s, u64 start, u64 end);

typedef struct str16_elt
{
	list_elt listElt;
	str16 string;
} str16_elt;

typedef struct str16_list
{
	list_info list;
	u64 eltCount;
	u64 len;
} str16_list;

MP_API void str16_list_push(mem_arena* arena, str16_list* list, str16 str);
MP_API str16 str16_list_join(mem_arena* arena, str16_list list);
MP_API str16_list str16_split(mem_arena* arena, str16 str, str16_list separators);

//----------------------------------------------------------------------------------
// u32 strings
//----------------------------------------------------------------------------------
typedef struct str32
{
	u64 len;
	u32* ptr;
} str32;

MP_API str32 str32_from_buffer(u64 len, u32* buffer);
MP_API str32 str32_slice(str32 s, u64 start, u64 end);

MP_API str32 str32_push_buffer(mem_arena* arena, u64 len, u32* buffer);
MP_API str32 str32_push_copy(mem_arena* arena, str32 s);
MP_API str32 str32_push_slice(mem_arena* arena, str32 s, u64 start, u64 end);

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

MP_API void str32_list_push(mem_arena* arena, str32_list* list, str32 str);
MP_API str32 str32_list_join(mem_arena* arena, str32_list list);
MP_API str32_list str32_split(mem_arena* arena, str32 str, str32_list separators);

#ifdef __cplusplus
} // extern "C"
#endif


#endif //__STRINGS_H_
