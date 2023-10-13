/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "strings.h"
#include "platform/platform_debug.h"

//----------------------------------------------------------------------------------
// string slices as values
//----------------------------------------------------------------------------------

oc_str8 oc_str8_from_buffer(u64 len, char* buffer)
{
    return ((oc_str8){ .ptr = buffer, .len = len });
}

oc_str8 oc_str8_slice(oc_str8 s, u64 start, u64 end)
{
    OC_ASSERT(start <= end && start <= s.len && end <= s.len);
    return ((oc_str8){ .ptr = s.ptr + start, .len = end - start });
}

oc_str8 oc_str8_push_buffer(oc_arena* arena, u64 len, char* buffer)
{
    oc_str8 str = { 0 };
    str.len = len;
    str.ptr = oc_arena_push_array(arena, char, len + 1);
    memcpy(str.ptr, buffer, len);
    str.ptr[str.len] = '\0';
    return (str);
}

oc_str8 oc_str8_push_cstring(oc_arena* arena, const char* str)
{
    int len = 0;
    if(str)
    {
        len = strlen(str);
    }
    return (oc_str8_push_buffer(arena, strlen(str), (char*)str));
}

oc_str8 oc_str8_push_copy(oc_arena* arena, oc_str8 s)
{
    return (oc_str8_push_buffer(arena, oc_str8_lp(s)));
}

char* oc_str8_to_cstring(oc_arena* arena, oc_str8 string)
{
    //NOTE: forward to push_copy, which null-terminates the copy
    string = oc_str8_push_copy(arena, string);
    return (string.ptr);
}

oc_str8 oc_str8_push_slice(oc_arena* arena, oc_str8 s, u64 start, u64 end)
{
    oc_str8 slice = oc_str8_slice(s, start, end);
    return (oc_str8_push_copy(arena, slice));
}

oc_str8 oc_str8_pushfv(oc_arena* arena, const char* format, va_list args)
{
    //NOTE(martin):
    //	We first compute the number of characters to write passing a size of 0.
    //  then we allocate len+1 (since vsnprint always terminates with a '\0').

    char dummy;
    oc_str8 str = { 0 };
    va_list argCopy;
    va_copy(argCopy, args);
    str.len = vsnprintf(&dummy, 0, format, argCopy);
    va_end(argCopy);

    str.ptr = oc_arena_push_array(arena, char, str.len + 1);
    vsnprintf((char*)str.ptr, str.len + 1, format, args);
    return (str);
}

oc_str8 oc_str8_pushf(oc_arena* arena, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    oc_str8 str = oc_str8_pushfv(arena, format, args);
    va_end(args);
    return (str);
}

int oc_str8_cmp(oc_str8 s1, oc_str8 s2)
{
    int res = strncmp(s1.ptr, s2.ptr, oc_min(s1.len, s2.len));
    if(!res)
    {
        res = (s1.len < s2.len) ? -1 : ((s1.len == s2.len) ? 0 : 1);
    }
    return (res);
}

//----------------------------------------------------------------------------------
// string lists
//----------------------------------------------------------------------------------

void oc_str8_list_init(oc_str8_list* list)
{
    oc_list_init(&list->list);
    list->eltCount = 0;
    list->len = 0;
}

void oc_str8_list_push(oc_arena* arena, oc_str8_list* list, oc_str8 str)
{
    oc_str8_elt* elt = oc_arena_push_type(arena, oc_str8_elt);
    elt->string = str;
    oc_list_append(&list->list, &elt->listElt);
    list->eltCount++;
    list->len += str.len;
}

void oc_str8_list_pushf(oc_arena* arena, oc_str8_list* list, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    oc_str8 str = oc_str8_pushfv(arena, format, args);
    va_end(args);
    oc_str8_list_push(arena, list, str);
}

oc_str8 oc_str8_list_collate(oc_arena* arena, oc_str8_list list, oc_str8 prefix, oc_str8 separator, oc_str8 postfix)
{
    oc_str8 str = { 0 };
    str.len = prefix.len + list.len + list.eltCount * separator.len + postfix.len;
    str.ptr = oc_arena_push_array(arena, char, str.len + 1);
    char* dst = str.ptr;
    memcpy(dst, prefix.ptr, prefix.len);
    dst += prefix.len;

    oc_str8_elt* elt = oc_list_first_entry(list.list, oc_str8_elt, listElt);
    if(elt)
    {
        memcpy(dst, elt->string.ptr, elt->string.len);
        dst += elt->string.len;
        elt = oc_list_next_entry(list.list, elt, oc_str8_elt, listElt);
    }

    for(; elt != 0; elt = oc_list_next_entry(list.list, elt, oc_str8_elt, listElt))
    {
        memcpy(dst, separator.ptr, separator.len);
        dst += separator.len;
        memcpy(dst, elt->string.ptr, elt->string.len);
        dst += elt->string.len;
    }
    memcpy(dst, postfix.ptr, postfix.len);
    str.ptr[str.len] = '\0';
    return (str);
}

oc_str8 oc_str8_list_join(oc_arena* arena, oc_str8_list list)
{
    oc_str8 empty = { .ptr = 0, .len = 0 };
    return (oc_str8_list_collate(arena, list, empty, empty, empty));
}

oc_str8_list oc_str8_split(oc_arena* arena, oc_str8 str, oc_str8_list separators)
{
    oc_str8_list list = { 0 };
    oc_list_init(&list.list);

    char* ptr = str.ptr;
    char* end = str.ptr + str.len;
    char* subStart = ptr;
    for(; ptr < end; ptr++)
    {
        //NOTE(martin): search all separators and try to match them to the current ptr
        oc_str8* foundSep = 0;
        oc_list_for(separators.list, elt, oc_str8_elt, listElt)
        {
            oc_str8* separator = &elt->string;
            bool equal = true;
            for(u64 offset = 0;
                (offset < separator->len) && (ptr + offset < end);
                offset++)
            {
                if(separator->ptr[offset] != ptr[offset])
                {
                    equal = false;
                    break;
                }
            }
            if(equal)
            {
                foundSep = separator;
                break;
            }
        }
        if(foundSep)
        {
            oc_str8 sub = oc_str8_from_buffer(ptr - subStart, subStart);
            oc_str8_list_push(arena, &list, sub);
            ptr += foundSep->len - 1; //NOTE(martin): ptr is incremented at the end of the loop
            subStart = ptr + 1;
        }
    }
    //NOTE(martin): emit the last substring
    oc_str8 sub = oc_str8_from_buffer(ptr - subStart, subStart);
    oc_str8_list_push(arena, &list, sub);

    return (list);
}

//----------------------------------------------------------------------------------
// u16 strings
//----------------------------------------------------------------------------------
oc_str16 oc_str16_from_buffer(u64 len, u16* buffer)
{
    return ((oc_str16){ .ptr = buffer, .len = len });
}

oc_str16 oc_str16_slice(oc_str16 s, u64 start, u64 end)
{
    OC_ASSERT(start <= end && start <= s.len && end <= s.len);
    return ((oc_str16){ .ptr = s.ptr + start, .len = end - start });
}

oc_str16 oc_str16_push_buffer(oc_arena* arena, u64 len, u16* buffer)
{
    oc_str16 str = { 0 };
    str.len = len;
    str.ptr = oc_arena_push_array(arena, u16, len + 1);
    memcpy(str.ptr, buffer, len * sizeof(u16));
    str.ptr[str.len] = (u16)0;
    return (str);
}

oc_str16 oc_str16_push_copy(oc_arena* arena, oc_str16 s)
{
    return (oc_str16_push_buffer(arena, s.len, s.ptr));
}

oc_str16 oc_str16_push_slice(oc_arena* arena, oc_str16 s, u64 start, u64 end)
{
    oc_str16 slice = oc_str16_slice(s, start, end);
    return (oc_str16_push_copy(arena, slice));
}

void oc_str16_list_init(oc_str16_list* list)
{
    oc_list_init(&list->list);
    list->eltCount = 0;
    list->len = 0;
}

void oc_str16_list_push(oc_arena* arena, oc_str16_list* list, oc_str16 str)
{
    oc_str16_elt* elt = oc_arena_push_type(arena, oc_str16_elt);
    elt->string = str;
    oc_list_append(&list->list, &elt->listElt);
    list->eltCount++;
    list->len += str.len;
}

oc_str16 oc_str16_list_collate(oc_arena* arena, oc_str16_list list, oc_str16 prefix, oc_str16 separator, oc_str16 postfix)
{
    oc_str16 str = { 0 };
    str.len = prefix.len + list.len + list.eltCount * separator.len + postfix.len;
    str.ptr = oc_arena_push_array(arena, u16, str.len + 1);
    char* dst = (char*)str.ptr;
    memcpy(dst, prefix.ptr, prefix.len * sizeof(u16));
    dst += prefix.len * sizeof(u16);

    oc_str16_elt* elt = oc_list_first_entry(list.list, oc_str16_elt, listElt);
    if(elt)
    {
        memcpy(dst, elt->string.ptr, elt->string.len * sizeof(u16));
        dst += elt->string.len * sizeof(u16);
        elt = oc_list_next_entry(list.list, elt, oc_str16_elt, listElt);
    }

    for(; elt != 0; elt = oc_list_next_entry(list.list, elt, oc_str16_elt, listElt))
    {
        memcpy(dst, separator.ptr, separator.len * sizeof(u16));
        dst += separator.len * sizeof(u16);
        memcpy(dst, elt->string.ptr, elt->string.len * sizeof(u16));
        dst += elt->string.len * sizeof(u16);
    }
    memcpy(dst, postfix.ptr, postfix.len * sizeof(u16));
    str.ptr[str.len] = (u16)0;
    return (str);
}

oc_str16 oc_str16_list_join(oc_arena* arena, oc_str16_list list)
{
    oc_str16 empty = { .ptr = 0, .len = 0 };
    return (oc_str16_list_collate(arena, list, empty, empty, empty));
}

//----------------------------------------------------------------------------------
// u32 strings
//----------------------------------------------------------------------------------
oc_str32 oc_str32_from_buffer(u64 len, u32* buffer)
{
    return ((oc_str32){ .ptr = buffer, .len = len });
}

oc_str32 oc_str32_slice(oc_str32 s, u64 start, u64 end)
{
    OC_ASSERT(start <= end && start <= s.len && end <= s.len);
    return ((oc_str32){ .ptr = s.ptr + start, .len = end - start });
}

oc_str32 oc_str32_push_buffer(oc_arena* arena, u64 len, u32* buffer)
{
    oc_str32 str = { 0 };
    str.len = len;
    str.ptr = oc_arena_push_array(arena, u32, len + 1);
    memcpy(str.ptr, buffer, len * sizeof(u32));
    str.ptr[str.len] = 0;
    return (str);
}

oc_str32 oc_str32_push_copy(oc_arena* arena, oc_str32 s)
{
    return (oc_str32_push_buffer(arena, s.len, s.ptr));
}

oc_str32 oc_str32_push_slice(oc_arena* arena, oc_str32 s, u64 start, u64 end)
{
    oc_str32 slice = oc_str32_slice(s, start, end);
    return (oc_str32_push_copy(arena, slice));
}

void oc_str32_list_init(oc_str32_list* list)
{
    oc_list_init(&list->list);
    list->eltCount = 0;
    list->len = 0;
}

void oc_str32_list_push(oc_arena* arena, oc_str32_list* list, oc_str32 str)
{
    oc_str32_elt* elt = oc_arena_push_type(arena, oc_str32_elt);
    elt->string = str;
    oc_list_append(&list->list, &elt->listElt);
    list->eltCount++;
    list->len += str.len;
}

oc_str32 oc_str32_list_collate(oc_arena* arena, oc_str32_list list, oc_str32 prefix, oc_str32 separator, oc_str32 postfix)
{
    oc_str32 str = { 0 };
    str.len = prefix.len + list.len + list.eltCount * separator.len + postfix.len;
    str.ptr = oc_arena_push_array(arena, u32, str.len + 1);
    char* dst = (char*)str.ptr;
    memcpy(dst, prefix.ptr, prefix.len * sizeof(u32));
    dst += prefix.len * sizeof(u32);

    oc_str32_elt* elt = oc_list_first_entry(list.list, oc_str32_elt, listElt);
    if(elt)
    {
        memcpy(dst, elt->string.ptr, elt->string.len * sizeof(u32));
        dst += elt->string.len * sizeof(u32);
        elt = oc_list_next_entry(list.list, elt, oc_str32_elt, listElt);
    }

    for(; elt != 0; elt = oc_list_next_entry(list.list, elt, oc_str32_elt, listElt))
    {
        memcpy(dst, separator.ptr, separator.len * sizeof(u32));
        dst += separator.len * sizeof(u32);
        memcpy(dst, elt->string.ptr, elt->string.len * sizeof(u32));
        dst += elt->string.len * sizeof(u32);
    }
    memcpy(dst, postfix.ptr, postfix.len * sizeof(u32));
    str.ptr[str.len] = 0;
    return (str);
}

oc_str32 oc_str32_list_join(oc_arena* arena, oc_str32_list list)
{
    oc_str32 empty = { .ptr = 0, .len = 0 };
    return (oc_str32_list_collate(arena, list, empty, empty, empty));
}
