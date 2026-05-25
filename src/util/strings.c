/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <stdio.h>
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
    if(!s.ptr)
    {
        OC_DEBUG_ASSERT(s.len == 0);
        return (oc_str8){ 0 };
    }
    end = oc_max(start, end);
    start = oc_min(start, s.len);
    end = oc_min(end, s.len);
    return ((oc_str8){ .ptr = s.ptr + start, .len = end - start });
}

oc_str8 oc_str8_push_buffer(oc_allocator* allocator, u64 len, char* buffer)
{
    oc_str8 str = { 0 };
    str.len = len;
    str.ptr = oc_allocator_push_array(allocator, char, len + 1);
    memcpy(str.ptr, buffer, len);
    str.ptr[str.len] = '\0';
    return (str);
}

oc_str8 oc_str8_push_cstring(oc_allocator* allocator, const char* str)
{
    int len = 0;
    if(str)
    {
        len = strlen(str);
    }
    return (oc_str8_push_buffer(allocator, strlen(str), (char*)str));
}

oc_str8 oc_str8_push_copy(oc_allocator* allocator, oc_str8 s)
{
    return (oc_str8_push_buffer(allocator, oc_str8_lp(s)));
}

char* oc_str8_to_cstring(oc_allocator* allocator, oc_str8 string)
{
    //NOTE: forward to push_copy, which null-terminates the copy
    string = oc_str8_push_copy(allocator, string);
    return (string.ptr);
}

oc_str8 oc_str8_push_slice(oc_allocator* allocator, oc_str8 s, u64 start, u64 end)
{
    oc_str8 slice = oc_str8_slice(s, start, end);
    return (oc_str8_push_copy(allocator, slice));
}

oc_str8 oc_str8_pushfv(oc_allocator* allocator, const char* format, va_list args)
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

    str.ptr = oc_allocator_push_array(allocator, char, str.len + 1);
    vsnprintf((char*)str.ptr, str.len + 1, format, args);
    return (str);
}

oc_str8 oc_str8_pushf(oc_allocator* allocator, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    oc_str8 str = oc_str8_pushfv(allocator, format, args);
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
    list->list = (typeof(list->list)){ 0 };
    list->len = 0;
}

void oc_str8_list_push(oc_allocator* allocator, oc_str8_list* list, oc_str8 str)
{
    oc_str8_elt* elt = oc_allocator_push_type(allocator, oc_str8_elt);
    elt->string = str;
    oc_typed_list_push_back(&list->list, elt);
    list->len += str.len;
}

oc_str8 oc_str8_list_pop_back(oc_str8_list* list)
{
    oc_str8 string = { 0 };

    oc_str8_elt* elt = oc_typed_list_pop_back(&list->list);
    if(elt)
    {
        OC_DEBUG_ASSERT(elt->string.len < list->len);
        list->len -= elt->string.len;
        string = elt->string;
    }
    return string;
}

void oc_str8_list_push_front(oc_allocator* allocator, oc_str8_list* list, oc_str8 str)
{
    oc_str8_elt* elt = oc_allocator_push_type(allocator, oc_str8_elt);
    elt->string = str;
    oc_typed_list_push_front(&list->list, elt);
    list->len += str.len;
}

oc_str8 oc_str8_list_pop_front(oc_str8_list* list)
{
    oc_str8 string = { 0 };

    oc_str8_elt* elt = oc_typed_list_pop_front(&list->list);
    if(elt)
    {
        OC_DEBUG_ASSERT(elt->string.len < list->len);
        list->len -= elt->string.len;
        string = elt->string;
    }
    return string;
}

void oc_str8_list_pushf(oc_allocator* allocator, oc_str8_list* list, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    oc_str8 str = oc_str8_pushfv(allocator, format, args);
    va_end(args);
    oc_str8_list_push(allocator, list, str);
}

oc_str8 oc_str8_list_collate(oc_allocator* allocator, oc_str8_list list, oc_str8 prefix, oc_str8 separator, oc_str8 postfix)
{
    oc_str8 str = { 0 };
    str.len = prefix.len + list.len + postfix.len;
    if(oc_typed_list_count(list.list) && separator.len)
    {
        str.len += oc_typed_list_count(list.list) * separator.len - 1;
    }

    str.ptr = oc_allocator_push_array(allocator, char, str.len + 1);
    char* dst = str.ptr;
    memcpy(dst, prefix.ptr, prefix.len);
    dst += prefix.len;

    oc_str8_elt* elt = oc_typed_list_first(list.list);
    if(elt)
    {
        memcpy(dst, elt->string.ptr, elt->string.len);
        dst += elt->string.len;
        elt = oc_typed_list_next(list.list, elt);
    }

    for(; elt != 0; elt = oc_typed_list_next(list.list, elt))
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

oc_str8 oc_str8_list_join(oc_allocator* allocator, oc_str8_list list)
{
    oc_str8 empty = { .ptr = 0, .len = 0 };
    return (oc_str8_list_collate(allocator, list, empty, empty, empty));
}

oc_str8_list oc_str8_split(oc_allocator* allocator, oc_str8 str, oc_str8_list separators)
{
    oc_str8_list list = { 0 };

    char* ptr = str.ptr;
    char* end = str.ptr + str.len;
    char* subStart = ptr;
    for(; ptr < end; ptr++)
    {
        //NOTE(martin): search all separators and try to match them to the current ptr
        oc_str8* foundSep = 0;
        oc_typed_list_for(separators.list, elt)
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
            oc_str8_list_push(allocator, &list, sub);
            ptr += foundSep->len - 1; //NOTE(martin): ptr is incremented at the end of the loop
            subStart = ptr + 1;
        }
    }
    //NOTE(martin): emit the last substring
    oc_str8 sub = oc_str8_from_buffer(ptr - subStart, subStart);
    oc_str8_list_push(allocator, &list, sub);

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
    end = oc_max(start, end);
    start = oc_min(start, s.len);
    end = oc_min(end, s.len);
    return ((oc_str16){ .ptr = s.ptr + start, .len = end - start });
}

oc_str16 oc_str16_push_buffer(oc_allocator* allocator, u64 len, u16* buffer)
{
    oc_str16 str = { 0 };
    str.len = len;
    str.ptr = oc_allocator_push_array(allocator, u16, len + 1);
    memcpy(str.ptr, buffer, len * sizeof(u16));
    str.ptr[str.len] = (u16)0;
    return (str);
}

oc_str16 oc_str16_push_copy(oc_allocator* allocator, oc_str16 s)
{
    return (oc_str16_push_buffer(allocator, s.len, s.ptr));
}

oc_str16 oc_str16_push_slice(oc_allocator* allocator, oc_str16 s, u64 start, u64 end)
{
    oc_str16 slice = oc_str16_slice(s, start, end);
    return (oc_str16_push_copy(allocator, slice));
}

void oc_str16_list_init(oc_str16_list* list)
{
    list->list = (typeof(list->list)){ 0 };
    list->len = 0;
}

void oc_str16_list_push(oc_allocator* allocator, oc_str16_list* list, oc_str16 str)
{
    oc_str16_elt* elt = oc_allocator_push_type(allocator, oc_str16_elt);
    elt->string = str;
    oc_typed_list_push_back(&list->list, elt);
    list->len += str.len;
}

oc_str16 oc_str16_list_collate(oc_allocator* allocator, oc_str16_list list, oc_str16 prefix, oc_str16 separator, oc_str16 postfix)
{
    oc_str16 str = { 0 };
    str.len = prefix.len + list.len + oc_typed_list_count(list.list) * separator.len + postfix.len;
    str.ptr = oc_allocator_push_array(allocator, u16, str.len + 1);
    char* dst = (char*)str.ptr;
    memcpy(dst, prefix.ptr, prefix.len * sizeof(u16));
    dst += prefix.len * sizeof(u16);

    oc_str16_elt* elt = oc_typed_list_first(list.list);
    if(elt)
    {
        memcpy(dst, elt->string.ptr, elt->string.len * sizeof(u16));
        dst += elt->string.len * sizeof(u16);
        elt = oc_typed_list_next(list.list, elt);
    }

    for(; elt != 0; elt = oc_typed_list_next(list.list, elt))
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

oc_str16 oc_str16_list_join(oc_allocator* allocator, oc_str16_list list)
{
    oc_str16 empty = { .ptr = 0, .len = 0 };
    return (oc_str16_list_collate(allocator, list, empty, empty, empty));
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
    if(!s.ptr)
    {
        OC_DEBUG_ASSERT(s.len == 0);
        return (oc_str32){ 0 };
    }
    end = oc_max(start, end);
    start = oc_min(start, s.len);
    end = oc_min(end, s.len);
    return ((oc_str32){ .ptr = s.ptr + start, .len = end - start });
}

oc_str32 oc_str32_push_buffer(oc_allocator* allocator, u64 len, u32* buffer)
{
    oc_str32 str = { 0 };
    str.len = len;
    str.ptr = oc_allocator_push_array(allocator, u32, len + 1);
    memcpy(str.ptr, buffer, len * sizeof(u32));
    str.ptr[str.len] = 0;
    return (str);
}

oc_str32 oc_str32_push_copy(oc_allocator* allocator, oc_str32 s)
{
    return (oc_str32_push_buffer(allocator, s.len, s.ptr));
}

oc_str32 oc_str32_push_slice(oc_allocator* allocator, oc_str32 s, u64 start, u64 end)
{
    oc_str32 slice = oc_str32_slice(s, start, end);
    return (oc_str32_push_copy(allocator, slice));
}

void oc_str32_list_init(oc_str32_list* list)
{
    list->list = (typeof(list->list)){ 0 };
    list->len = 0;
}

void oc_str32_list_push(oc_allocator* allocator, oc_str32_list* list, oc_str32 str)
{
    oc_str32_elt* elt = oc_allocator_push_type(allocator, oc_str32_elt);
    elt->string = str;
    oc_typed_list_push_back(&list->list, elt);
    list->len += str.len;
}

oc_str32 oc_str32_list_collate(oc_allocator* allocator, oc_str32_list list, oc_str32 prefix, oc_str32 separator, oc_str32 postfix)
{
    oc_str32 str = { 0 };
    str.len = prefix.len + list.len + oc_typed_list_count(list.list) * separator.len + postfix.len;
    str.ptr = oc_allocator_push_array(allocator, u32, str.len + 1);
    char* dst = (char*)str.ptr;
    memcpy(dst, prefix.ptr, prefix.len * sizeof(u32));
    dst += prefix.len * sizeof(u32);

    oc_str32_elt* elt = oc_typed_list_first(list.list);
    if(elt)
    {
        memcpy(dst, elt->string.ptr, elt->string.len * sizeof(u32));
        dst += elt->string.len * sizeof(u32);
        elt = oc_typed_list_next(list.list, elt);
    }

    for(; elt != 0; elt = oc_typed_list_next(list.list, elt))
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

oc_str32 oc_str32_list_join(oc_allocator* allocator, oc_str32_list list)
{
    oc_str32 empty = { .ptr = 0, .len = 0 };
    return (oc_str32_list_collate(allocator, list, empty, empty, empty));
}
