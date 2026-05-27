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

typedef struct oc_list_links oc_list_links;

struct oc_list_links
{
    oc_list_links* prev;
    oc_list_links* next;
};

typedef struct oc_list
{
    oc_list_links* first;
    oc_list_links* last;
    u64 count;
} oc_list;

ORCA_API bool oc_list_empty(oc_list list);
ORCA_API void oc_list_insert(oc_list* list, oc_list_links* afterElt, oc_list_links* links);
ORCA_API void oc_list_insert_before(oc_list* list, oc_list_links* beforeElt, oc_list_links* links);
ORCA_API void oc_list_remove(oc_list* list, oc_list_links* links);
ORCA_API void oc_list_push_front(oc_list* list, oc_list_links* links);
ORCA_API oc_list_links* oc_list_pop_front(oc_list* list);
ORCA_API void oc_list_push_back(oc_list* list, oc_list_links* links);
ORCA_API oc_list_links* oc_list_pop_back(oc_list* list);

#define oc_list_count(list) ((list).count)
#define oc_list_begin(l) (l).first
#define oc_list_end(l) ((oc_list_links*)0)
#define oc_list_last(l) (l).last
#define oc_list_next(links) (links)->next
#define oc_list_prev(links) (links)->prev

#define oc_list_elt(elt, type, linksName) \
    oc_container_of(elt, type, linksName)

#define oc_list_next_elt(elt, type, linksName) \
    ((elt->linksName.next != 0) ? oc_list_elt(elt->linksName.next, type, linksName) : 0)

#define oc_list_prev_elt(elt, type, linksName) \
    ((elt->linksName.prev != 0) ? oc_list_elt(elt->linksName.prev, type, linksName) : 0)

#define oc_list_checked_elt(elt, type, linksName) \
    (((elt) != 0) ? oc_list_elt(elt, type, linksName) : 0)

#define oc_list_first_elt(list, type, linksName) \
    (oc_list_checked_elt(oc_list_begin(list), type, linksName))

#define oc_list_last_elt(list, type, linksName) \
    (oc_list_checked_elt(oc_list_last(list), type, linksName))

#define oc_list_pop_front_elt(list, type, linksName) (oc_list_empty(*list) ? 0 : oc_list_elt(oc_list_pop_front(list), type, linksName))
#define oc_list_pop_back_elt(list, type, linksName) (oc_list_empty(*list) ? 0 : oc_list_elt(oc_list_pop_back(list), type, linksName))

#define oc_list_for(list, elt, type, linksName)                                \
    for(type* elt = oc_list_checked_elt(oc_list_begin(list), type, linksName); \
        elt != 0;                                                              \
        elt = oc_list_checked_elt(elt->linksName.next, type, linksName))

#define oc_list_for_reverse(list, elt, type, linksName)                       \
    for(type* elt = oc_list_checked_elt(oc_list_last(list), type, linksName); \
        elt != 0;                                                             \
        elt = oc_list_checked_elt(elt->linksName.prev, type, linksName))

#define oc_list_for_indexed(list, it, type, linksName)                                    \
    for(                                                                                  \
        struct { size_t index; type* elt; } it = { 0, oc_list_checked_elt(oc_list_begin(list), type, linksName) }; \
        it.elt != 0;                                                                      \
        it.elt = oc_list_checked_elt(it.elt->linksName.next, type, linksName), it.index++)

#define oc_list_for_safe(list, elt, type, linksName)                                        \
    for(type* elt = oc_list_checked_elt(oc_list_begin(list), type, linksName),              \
              *__tmp = elt ? oc_list_checked_elt(elt->linksName.next, type, linksName) : 0; \
        elt != 0;                                                                           \
        elt = __tmp,                                                                        \
              __tmp = elt ? oc_list_checked_elt(elt->linksName.next, type, linksName) : 0)

//-------------------------------------------------------------------------
// Typed intrusive linked lists
//-------------------------------------------------------------------------

#define oc_typed_list(type, link)          \
    union                                  \
    {                                      \
        oc_list l;                         \
        type* t;                           \
        char (*ofs)[offsetof(type, link)]; \
    }

//NOTE: these macros typecheck that elt arguments are of the correct type for the list argument,
// and return elts of the correct type.
#define oc_typed_list_count(list) (list).l.count
#define oc_typed_list_empty(list) (oc_typed_list_count(list) == 0)
#define oc_typed_list_first(list) ((typeof((list).t))oc_typed_list_elt((list), (list).l.first))
#define oc_typed_list_last(list) oc_typed_list_elt((list), (list).l.last)

#define oc_typed_list_next(list, elt) \
    ((typeof(list.t)(*)(typeof(list.t), u64))oc_typed_list_next_generic)(elt, oc_typed_list_get_links_offset(list))

#define oc_typed_list_prev(list, elt) \
    ((typeof(list.t)(*)(typeof(list.t), u64))oc_typed_list_prev_generic)(elt, oc_typed_list_get_links_offset(list))

#define oc_typed_list_push_front(list, elt) \
    ((void (*)(oc_list*, typeof((list)->t), oc_list_links*))oc_typed_list_push_front_generic)(&(list)->l, elt, oc_typed_list_get_links(*(list), elt))

#define oc_typed_list_push_back(list, elt) \
    ((void (*)(oc_list*, typeof((list)->t), oc_list_links*))oc_typed_list_push_back_generic)(&(list)->l, elt, oc_typed_list_get_links(*(list), elt))

#define oc_typed_list_remove(list, elt) \
    ((void (*)(oc_list*, typeof((list)->t), oc_list_links*))oc_typed_list_remove_generic)(&(list)->l, elt, oc_typed_list_get_links(*(list), elt))

#define oc_typed_list_pop_front(list) \
    ((typeof((list)->t)(*)(oc_list*, u64))oc_typed_list_pop_front_generic)(&(list)->l, oc_typed_list_get_links_offset(*(list)))

#define oc_typed_list_pop_back(list) \
    ((typeof((list)->t)(*)(oc_list*, u64))oc_typed_list_pop_back_generic)(&(list)->l, oc_typed_list_get_links_offset(*(list)))

#define oc_typed_list_insert_before(list, before, elt)                                                            \
    ((void (*)(oc_list*, typeof((list)->t), oc_list_links*, oc_list_links*))oc_typed_list_insert_before_generic)( \
        list,                                                                                                     \
        elt,                                                                                                      \
        oc_typed_list_get_links(*(list), before),                                                                 \
        oc_typed_list_get_links(*(list), elt))

#define oc_typed_list_insert_after(list, after, elt)                                                             \
    ((void (*)(oc_list*, typeof((list)->t), oc_list_links*, oc_list_links*))oc_typed_list_insert_after_generic)( \
        list,                                                                                                    \
        elt,                                                                                                     \
        oc_typed_list_get_links(*(list), after),                                                                 \
        oc_typed_list_get_links(*(list), elt))

//NOTE: list loops
#define oc_typed_list_for(list, it)                      \
    for(typeof((list).t) it = oc_typed_list_first(list); \
        it != 0;                                         \
        it = oc_typed_list_next(list, it))

#define oc_typed_list_for_reverse(list, it)             \
    for(typeof((list).t) it = oc_typed_list_last(list); \
        it != 0;                                        \
        it = oc_typed_list_prev(list, it))

#define oc_typed_list_for_safe(list, it)                                                                \
    for(typeof((list).t) it = oc_typed_list_first(list), __tmp = it ? oc_typed_list_next(list, it) : 0; \
        it != 0;                                                                                        \
        it = __tmp, __tmp = it ? oc_typed_list_next(list, it) : 0)

//NOTE: (internal) generic typed list functions and macros

#define oc_typed_list_get_links_offset(list) sizeof(*((list).ofs))
#define oc_typed_list_get_links(list, elt) ((oc_list_links*)(((char*)elt) + oc_typed_list_get_links_offset(list)))
#define oc_typed_list_elt(list, links) ((typeof((list).t))(links - oc_typed_list_get_links_offset(list)))

ORCA_API void* oc_typed_list_next_generic(void* elt, u64 linksOffset);
ORCA_API void* oc_typed_list_prev_generic(void* elt, u64 linksOffset);
ORCA_API void oc_typed_list_push_front_generic(oc_list* list, void* elt, oc_list_links* links);
ORCA_API void oc_typed_list_push_back_generic(oc_list* list, void* elt, oc_list_links* links);
ORCA_API void oc_typed_list_remove_generic(oc_list* list, void* elt, oc_list_links* links);
ORCA_API void* oc_typed_list_pop_front_generic(oc_list* list, u64 linksOffset);
ORCA_API void* oc_typed_list_pop_back_generic(oc_list* list, u64 linksOffset);
ORCA_API void oc_typed_list_insert_before_generic(oc_list* list, void* elt, oc_list_links* before, oc_list_links* links);
ORCA_API void oc_typed_list_insert_after_generic(oc_list* list, void* elt, oc_list_links* after, oc_list_links* links);

#ifdef __cplusplus
} // extern "C"
#endif
