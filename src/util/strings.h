/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "debug.h"
#include "lists.h"
#include "memory.h"
#include "typedefs.h"
#include <stdarg.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*NOTE:
	By convention, functions that take an arena and return a string slice allocated on
	this arena, always allocate one more element and null-terminate the string. This is
	done so we can pass those strings directly to C APIs that requires C strings without
	having to do a copy with oc_str8_to_cstring().

	This does _not_ applies to the string returned by oc_str8_split(). Those are slices
	into the original string. Only the _list nodes_ are allocated on the arena.
*/

//----------------------------------------------------------------------------------
// u8 strings
//----------------------------------------------------------------------------------
typedef struct oc_str8
{
    char* ptr;
    size_t len;
} oc_str8;

OC_STATIC_ASSERT(__builtin_constant_p(""));
#define OC_STR8(s) ((oc_str8){ .ptr = (char*)s, .len = (s) ? (__builtin_constant_p(s) ? sizeof(s) - 1 : strlen(s)) : 0 })

//NOTE: this only works with string literals, but is sometimes necessary to generate compile-time constants
#define OC_STR8_LIT(s)                          \
    {                                           \
        .ptr = (char*)(s), .len = sizeof(s) - 1 \
    }

#define oc_str8_lp(s) ((s).len), ((s).ptr)
#define oc_str8_ip(s) (int)oc_str8_lp(s)

ORCA_API oc_str8 oc_str8_from_buffer(u64 len, char* buffer);
//FIXME(pld): audit calls to oc_str8_slice, some seem to assume a length
//instead of an end-point.
ORCA_API oc_str8 oc_str8_slice(oc_str8 s, u64 start, u64 end);
static inline oc_str8 oc_str8_slice_len(oc_str8 s, u64 start, u64 len)
{
    return oc_str8_slice(s, start, start + len);
}

ORCA_API oc_str8 oc_str8_push_buffer(oc_arena* arena, u64 len, char* buffer);
ORCA_API oc_str8 oc_str8_push_cstring(oc_arena* arena, const char* str);
ORCA_API oc_str8 oc_str8_push_copy(oc_arena* arena, oc_str8 s);
ORCA_API oc_str8 oc_str8_push_slice(oc_arena* arena, oc_str8 s, u64 start, u64 end);

ORCA_API oc_str8 oc_str8_pushfv(oc_arena* arena, const char* format, va_list args);
ORCA_API oc_str8 oc_str8_pushf(oc_arena* arena, const char* format, ...);

ORCA_API int oc_str8_cmp(oc_str8 s1, oc_str8 s2);

static inline bool oc_str8_eq(oc_str8 s1, oc_str8 s2)
{
    return !oc_str8_cmp(s1, s2);
}

ORCA_API char* oc_str8_to_cstring(oc_arena* arena, oc_str8 string);

//----------------------------------------------------------------------------------
// string lists
//----------------------------------------------------------------------------------
typedef struct oc_str8_elt
{
    oc_list_elt listElt;
    oc_str8 string;
} oc_str8_elt;

typedef struct oc_str8_list
{
    oc_list list;
    u64 eltCount;
    u64 len;
} oc_str8_list;

ORCA_API void oc_str8_list_push(oc_arena* arena, oc_str8_list* list, oc_str8 str);
ORCA_API void oc_str8_list_pushf(oc_arena* arena, oc_str8_list* list, const char* format, ...);
ORCA_API void oc_str8_list_push_front(oc_arena* arena, oc_str8_list* list, oc_str8 str);
ORCA_API void oc_str8_list_remove(oc_str8_list* list, oc_str8_elt* elt);

ORCA_API oc_str8 oc_str8_list_collate(oc_arena* arena, oc_str8_list list, oc_str8 prefix, oc_str8 separator, oc_str8 postfix);
ORCA_API oc_str8 oc_str8_list_join(oc_arena* arena, oc_str8_list list);
ORCA_API oc_str8_list oc_str8_split(oc_arena* arena, oc_str8 str, oc_str8_list separators);

#define oc_str8_list_first(sl) (oc_list_empty(sl.list) ? (oc_str8){ 0 } : (oc_list_first_entry(sl.list, oc_str8_elt, listElt)->string))
#define oc_str8_list_last(sl) (oc_list_empty(sl.list) ? (oc_str8){ 0 } : (oc_list_last_entry(sl.list, oc_str8_elt, listElt)->string))
#define oc_str8_list_for(sl, elt) oc_list_for(sl.list, elt, oc_str8_elt, listElt)
#define oc_str8_list_empty(sl) (oc_list_empty(sl.list))

// Allows use of oc_str8 with standard printf via the %.*s specifier
#define oc_str8_printf(str) (int)str.len, str.ptr

//----------------------------------------------------------------------------------
// u16 strings
//----------------------------------------------------------------------------------
typedef struct oc_str16
{
    u16* ptr;
    size_t len;
} oc_str16;

ORCA_API oc_str16 oc_str16_from_buffer(u64 len, u16* buffer);
ORCA_API oc_str16 oc_str16_slice(oc_str16 s, u64 start, u64 end);

ORCA_API oc_str16 oc_str16_push_buffer(oc_arena* arena, u64 len, u16* buffer);
ORCA_API oc_str16 oc_str16_push_copy(oc_arena* arena, oc_str16 s);
ORCA_API oc_str16 oc_str16_push_slice(oc_arena* arena, oc_str16 s, u64 start, u64 end);

typedef struct oc_str16_elt
{
    oc_list_elt listElt;
    oc_str16 string;
} oc_str16_elt;

typedef struct oc_str16_list
{
    oc_list list;
    u64 eltCount;
    u64 len;
} oc_str16_list;

ORCA_API void oc_str16_list_push(oc_arena* arena, oc_str16_list* list, oc_str16 str);
ORCA_API oc_str16 oc_str16_list_join(oc_arena* arena, oc_str16_list list);
ORCA_API oc_str16_list oc_str16_split(oc_arena* arena, oc_str16 str, oc_str16_list separators);

#define oc_str16_list_first(sl) (oc_list_empty(sl.list) ? (oc_str16){ 0 } : (oc_list_first_entry(sl.list, oc_str16_elt, listElt)->string))
#define oc_str16_list_last(sl) (oc_list_empty(sl.list) ? (oc_str16){ 0 } : (oc_list_last_entry(sl.list, oc_str16_elt, listElt)->string))
#define oc_str16_list_for(sl, elt) oc_list_for(sl.list, elt, oc_str16_elt, listElt)
#define oc_str16_list_empty(sl) (oc_list_empty(sl.list))

//----------------------------------------------------------------------------------
// u32 strings
//----------------------------------------------------------------------------------
typedef struct oc_str32
{
    u32* ptr;
    size_t len;
} oc_str32;

ORCA_API oc_str32 oc_str32_from_buffer(u64 len, u32* buffer);
ORCA_API oc_str32 oc_str32_slice(oc_str32 s, u64 start, u64 end);

ORCA_API oc_str32 oc_str32_push_buffer(oc_arena* arena, u64 len, u32* buffer);
ORCA_API oc_str32 oc_str32_push_copy(oc_arena* arena, oc_str32 s);
ORCA_API oc_str32 oc_str32_push_slice(oc_arena* arena, oc_str32 s, u64 start, u64 end);

typedef struct oc_str32_elt
{
    oc_list_elt listElt;
    oc_str32 string;
} oc_str32_elt;

typedef struct oc_str32_list
{
    oc_list list;
    u64 eltCount;
    u64 len;
} oc_str32_list;

ORCA_API void oc_str32_list_push(oc_arena* arena, oc_str32_list* list, oc_str32 str);
ORCA_API oc_str32 oc_str32_list_join(oc_arena* arena, oc_str32_list list);
ORCA_API oc_str32_list oc_str32_split(oc_arena* arena, oc_str32 str, oc_str32_list separators);

#define oc_str32_list_first(sl) (oc_list_empty(sl.list) ? (oc_str32){ 0 } : (oc_list_first_entry(sl.list, oc_str32_elt, listElt)->string))
#define oc_str32_list_last(sl) (oc_list_empty(sl.list) ? (oc_str32){ 0 } : (oc_list_last_entry(sl.list, oc_str32_elt, listElt)->string))
#define oc_str32_list_for(sl, elt) oc_list_for(sl.list, elt, oc_str32_elt, listElt)
#define oc_str32_list_empty(sl) (oc_list_empty(sl.list))

#ifdef __cplusplus
} // extern "C"
#endif
