/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*	@brief: Implements generic intrusive linked list and dynamic array
*
****************************************************************/
#ifndef __CONTAINERS_H_
#define __CONTAINERS_H_

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

#define oc_list_entry(ptr, type, member) \
    oc_container_of(ptr, type, member)

#define oc_list_next_entry(list, elt, type, member) \
    ((elt->member.next != oc_list_end(list)) ? oc_list_entry(elt->member.next, type, member) : 0)

#define oc_list_prev_entry(list, elt, type, member) \
    ((elt->member.prev != oc_list_end(list)) ? oc_list_entry(elt->member.prev, type, member) : 0)

#define oc_list_checked_entry(list, type, member) \
    (((list) != 0) ? oc_list_entry(list, type, member) : 0)

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

#define oc_list_for_safe(list, elt, type, member)                                       \
    for(type* elt = oc_list_checked_entry(oc_list_begin(list), type, member),           \
              *__tmp = elt ? oc_list_checked_entry(elt->member.next, type, member) : 0; \
        elt != 0;                                                                       \
        elt = __tmp,                                                                    \
              __tmp = elt ? oc_list_checked_entry(elt->member.next, type, member) : 0)

#define oc_list_pop_entry(list, type, member) (oc_list_empty(*list) ? 0 : oc_list_entry(oc_list_pop(list), type, member))

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

static inline void oc_list_init(oc_list* list)
{
    list->first = list->last = 0;
}

static inline void oc_list_insert(oc_list* list, oc_list_elt* afterElt, oc_list_elt* elt)
{
    elt->prev = afterElt;
    elt->next = afterElt->next;
    if(afterElt->next)
    {
        afterElt->next->prev = elt;
    }
    else
    {
        list->last = elt;
    }
    afterElt->next = elt;

    OC_DEBUG_ASSERT(elt->next != elt, "oc_list_insert(): can't insert an element into itself");
}

static inline void oc_list_insert_before(oc_list* list, oc_list_elt* beforeElt, oc_list_elt* elt)
{
    elt->next = beforeElt;
    elt->prev = beforeElt->prev;

    if(beforeElt->prev)
    {
        beforeElt->prev->next = elt;
    }
    else
    {
        list->first = elt;
    }
    beforeElt->prev = elt;

    OC_DEBUG_ASSERT(elt->next != elt, "oc_list_insert_before(): can't insert an element into itself");
}

static inline void oc_list_remove(oc_list* list, oc_list_elt* elt)
{
    if(elt->prev)
    {
        elt->prev->next = elt->next;
    }
    else
    {
        OC_DEBUG_ASSERT(list->first == elt);
        list->first = elt->next;
    }
    if(elt->next)
    {
        elt->next->prev = elt->prev;
    }
    else
    {
        OC_DEBUG_ASSERT(list->last == elt);
        list->last = elt->prev;
    }
    elt->prev = elt->next = 0;
}

static inline void oc_list_push(oc_list* list, oc_list_elt* elt)
{
    elt->next = list->first;
    elt->prev = 0;
    if(list->first)
    {
        list->first->prev = elt;
    }
    else
    {
        list->last = elt;
    }
    list->first = elt;
}

static inline oc_list_elt* oc_list_pop(oc_list* list)
{
    oc_list_elt* elt = oc_list_begin(*list);
    if(elt != oc_list_end(*list))
    {
        oc_list_remove(list, elt);
        return (elt);
    }
    else
    {
        return (0);
    }
}

static inline void oc_list_push_back(oc_list* list, oc_list_elt* elt)
{
    elt->prev = list->last;
    elt->next = 0;
    if(list->last)
    {
        list->last->next = elt;
    }
    else
    {
        list->first = elt;
    }
    list->last = elt;
}

#define oc_list_append(a, b) oc_list_push_back(a, b)

static inline oc_list_elt* oc_list_pop_back(oc_list* list)
{
    oc_list_elt* elt = oc_list_last(*list);
    if(elt != oc_list_end(*list))
    {
        oc_list_remove(list, elt);
        return (elt);
    }
    else
    {
        return (0);
    }
}

static inline bool oc_list_empty(oc_list list)
{
    return (list.first == 0 || list.last == 0);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__CONTAINERS_H_
