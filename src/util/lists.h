/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*	@brief: Implements generic intrusive linked list and dynamic array
*
****************************************************************/
#pragma once

#include "platform/platform_debug.h"
#include "util/macros.h"
#include "util/debug.h"

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------
// Intrusive linked lists
//-------------------------------------------------------------------------

#define oc_list_begin(l) (l).first
#define oc_list_end(l) ((oc_list_elt*)0)
#define oc_list_last(l) (l).last

#define oc_list_next(elt) (elt)->next
#define oc_list_prev(elt) (elt)->prev

#define oc_list_entry(elt, type, member) \
    oc_container_of(elt, type, member)

#define oc_list_next_entry(elt, type, member) \
    ((elt->member.next != 0) ? oc_list_entry(elt->member.next, type, member) : 0)

#define oc_list_prev_entry(elt, type, member) \
    ((elt->member.prev != 0) ? oc_list_entry(elt->member.prev, type, member) : 0)

#define oc_list_checked_entry(elt, type, member) \
    (((elt) != 0) ? oc_list_entry(elt, type, member) : 0)

#define oc_list_first_entry(list, type, member) \
    (oc_list_checked_entry(oc_list_begin(list), type, member))

#define oc_list_last_entry(list, type, member) \
    (oc_list_checked_entry(oc_list_last(list), type, member))

#define oc_list_for(list, elt, type, member)                                  \
    for(type* elt = oc_list_checked_entry(oc_list_begin(list), type, member); \
        elt != 0;                                                             \
        elt = oc_list_checked_entry(elt->member.next, type, member))

#define oc_list_for_reverse(list, elt, type, member)                         \
    for(type* elt = oc_list_checked_entry(oc_list_last(list), type, member); \
        elt != 0;                                                            \
        elt = oc_list_checked_entry(elt->member.prev, type, member))

#define oc_list_for_indexed(list, it, type, member)                                      \
    for(                                                                                 \
        struct { size_t index; type* elt; } it = { 0, oc_list_checked_entry(oc_list_begin(list), type, member) }; \
        it.elt != 0;                                                                     \
        it.elt = oc_list_checked_entry(it.elt->member.next, type, member), it.index++)

#define oc_list_for_safe(list, elt, type, member)                                       \
    for(type* elt = oc_list_checked_entry(oc_list_begin(list), type, member),           \
              *__tmp = elt ? oc_list_checked_entry(elt->member.next, type, member) : 0; \
        elt != 0;                                                                       \
        elt = __tmp,                                                                    \
              __tmp = elt ? oc_list_checked_entry(elt->member.next, type, member) : 0)

#define oc_list_pop_front_entry(list, type, member) (oc_list_empty(*list) ? 0 : oc_list_entry(oc_list_pop_front(list), type, member))
#define oc_list_pop_back_entry(list, type, member) (oc_list_empty(*list) ? 0 : oc_list_entry(oc_list_pop_back(list), type, member))

#define oc_list_count(list) ((list).count)

typedef struct oc_list_elt oc_list_elt;

struct oc_list_elt
{
    oc_list_elt* prev;
    oc_list_elt* next;
};

typedef struct oc_list
{
    oc_list_elt* first;
    oc_list_elt* last;
    u64 count;
} oc_list;

ORCA_API bool oc_list_empty(oc_list list);
ORCA_API void oc_list_insert(oc_list* list, oc_list_elt* afterElt, oc_list_elt* elt);
ORCA_API void oc_list_insert_before(oc_list* list, oc_list_elt* beforeElt, oc_list_elt* elt);
ORCA_API void oc_list_remove(oc_list* list, oc_list_elt* elt);
ORCA_API void oc_list_push_front(oc_list* list, oc_list_elt* elt);
ORCA_API oc_list_elt* oc_list_pop_front(oc_list* list);
ORCA_API void oc_list_push_back(oc_list* list, oc_list_elt* elt);
ORCA_API oc_list_elt* oc_list_pop_back(oc_list* list);

//-------------------------------------------------------------------------
// Typed intrusive linked lists
//-------------------------------------------------------------------------

typedef struct oc_typed_list_links oc_typed_list_links;

typedef struct oc_typed_list_links
{
    oc_typed_list_links* prev;
    oc_typed_list_links* next;
} oc_typed_list_links;

typedef struct oc_typed_list
{
    oc_typed_list_links* first;
    oc_typed_list_links* last;
    u64 count;
} oc_typed_list;

#define oc_typed_list(type, link)          \
    union                                  \
    {                                      \
        struct                             \
        {                                  \
            oc_typed_list_links* first;    \
            oc_typed_list_links* last;     \
            u64 count;                     \
        };                                 \
        type* t;                           \
        char (*ofs)[offsetof(type, link)]; \
    }

//NOTE: these macros typecheck that item arguments are of the correct type for the list argument,
// and return items of the correct type.
#define oc_typed_list_count(list) (list).count
#define oc_typed_list_empty(list) (oc_typed_list_count(list) == 0)

#define oc_typed_list_first(list) ((typeof((list).t))oc_typed_list_entry((list), (list).first))
#define oc_typed_list_last(list) oc_typed_list_entry((list), (list).last)

#define oc_typed_list_next(list, item) \
    ((typeof(list.t)(*)(typeof(list.t), u64))oc_typed_list_next_generic)(item, oc_typed_list_links_offset(list))

#define oc_typed_list_prev(list, item) \
    ((typeof(list.t)(*)(typeof(list.t), u64))oc_typed_list_prev_generic)(item, oc_typed_list_links_offset(list))

#define oc_typed_list_push_front(list, item) \
    ((void (*)(typeof(list), typeof((list)->t), oc_typed_list_links*))oc_typed_list_push_front_generic)(list, item, oc_typed_list_links(*(list), item))

#define oc_typed_list_push_back(list, item) \
    ((void (*)(typeof(list), typeof((list)->t), oc_typed_list_links*))oc_typed_list_push_back_generic)(list, item, oc_typed_list_links(*(list), item))

#define oc_typed_list_remove(list, item) \
    ((void (*)(typeof(list), typeof((list)->t), oc_typed_list_links*))oc_typed_list_remove_generic)(list, item, oc_typed_list_links(*(list), item))

#define oc_typed_list_pop_front(list) \
    ((typeof((list)->t)(*)(typeof(list), u64))oc_typed_list_pop_front_generic)(list, oc_typed_list_links_offset(*(list)))

#define oc_typed_list_pop_back(list) \
    ((typeof((list)->t)(*)(typeof(list), u64))oc_typed_list_pop_back_generic)(list, oc_typed_list_links_offset(*(list)))

#define oc_typed_list_insert_before(list, before, item)                                                                           \
    ((void (*)(typeof(list), typeof((list)->t), oc_typed_list_links*, oc_typed_list_links*))oc_typed_list_insert_before_generic)( \
        list,                                                                                                                     \
        item,                                                                                                                     \
        oc_typed_list_links(*(list), before),                                                                                     \
        oc_typed_list_links(*(list), item))

#define oc_typed_list_insert_after(list, after, item)                                                                            \
    ((void (*)(typeof(list), typeof((list)->t), oc_typed_list_links*, oc_typed_list_links*))oc_typed_list_insert_after_generic)( \
        list,                                                                                                                    \
        item,                                                                                                                    \
        oc_typed_list_links(*(list), after),                                                                                     \
        oc_typed_list_links(*(list), item))

//NOTE: list loops
#define oc_typed_list_for(list, it)                      \
    for(typeof((list).t) it = oc_typed_list_first(list); \
        it != 0;                                         \
        it = oc_typed_list_next(list, it))

#define oc_typed_list_for_reverse(list, it)             \
    for(typeof((list).t) it = oc_typed_list_last(list); \
        it != 0;                                        \
        it = oc_typed_list_prev(list, it))

//NOTE: (internal) generic typed list functions and macros

#define oc_typed_list_links_offset(list) sizeof(*((list).ofs))
#define oc_typed_list_links(list, item) ((oc_typed_list_links*)(((char*)item) + oc_typed_list_links_offset(list)))
#define oc_typed_list_entry(list, links) ((typeof((list).t))(links - oc_typed_list_links_offset(list)))

ORCA_API void* oc_typed_list_next_generic(void* item, u64 linksOffset);
ORCA_API void* oc_typed_list_prev_generic(void* item, u64 linksOffset);
ORCA_API void oc_typed_list_push_front_generic(oc_typed_list* list, void* item, oc_typed_list_links* elt);
ORCA_API void oc_typed_list_push_back_generic(oc_typed_list* list, void* item, oc_typed_list_links* elt);
ORCA_API void oc_typed_list_remove_generic(oc_typed_list* list, void* item, oc_typed_list_links* links);
ORCA_API void* oc_typed_list_pop_front_generic(oc_typed_list* list, u64 linksOffset);
ORCA_API void* oc_typed_list_pop_back_generic(oc_typed_list* list, u64 linksOffset);
ORCA_API void oc_typed_list_insert_before_generic(oc_typed_list* list, void* item, oc_typed_list_links* before, oc_typed_list_links* links);
ORCA_API void oc_typed_list_insert_after_generic(oc_typed_list* list, void* item, oc_typed_list_links* after, oc_typed_list_links* links);

#ifdef __cplusplus
} // extern "C"
#endif
