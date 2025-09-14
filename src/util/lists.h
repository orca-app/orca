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
} oc_list;

ORCA_API bool oc_list_empty(oc_list list);
ORCA_API void oc_list_init(oc_list* list);
ORCA_API void oc_list_insert(oc_list* list, oc_list_elt* afterElt, oc_list_elt* elt);
ORCA_API void oc_list_insert_before(oc_list* list, oc_list_elt* beforeElt, oc_list_elt* elt);
ORCA_API void oc_list_remove(oc_list* list, oc_list_elt* elt);
ORCA_API void oc_list_push_front(oc_list* list, oc_list_elt* elt);
ORCA_API oc_list_elt* oc_list_pop_front(oc_list* list);
ORCA_API void oc_list_push_back(oc_list* list, oc_list_elt* elt);
ORCA_API oc_list_elt* oc_list_pop_back(oc_list* list);

#ifdef __cplusplus
} // extern "C"
#endif
