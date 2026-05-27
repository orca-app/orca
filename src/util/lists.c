/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*	@brief: Implements generic intrusive linked list and dynamic array
*
****************************************************************/

#include "lists.h"

void oc_list_insert(oc_list* list, oc_list_links* after, oc_list_links* links)
{
    links->prev = after;
    links->next = after->next;
    if(after->next)
    {
        after->next->prev = links;
    }
    else
    {
        list->last = links;
    }
    after->next = links;
    list->count++;
    OC_DEBUG_ASSERT(links->next != links, "oc_list_insert(): can't insert an element into itself");
}

void oc_list_insert_before(oc_list* list, oc_list_links* before, oc_list_links* links)
{
    links->next = before;
    links->prev = before->prev;

    if(before->prev)
    {
        before->prev->next = links;
    }
    else
    {
        list->first = links;
    }
    before->prev = links;
    list->count++;
    OC_DEBUG_ASSERT(links->next != links, "oc_list_insert_before(): can't insert an element into itself");
}

void oc_list_remove(oc_list* list, oc_list_links* links)
{
    if(links->prev)
    {
        links->prev->next = links->next;
    }
    else
    {
        OC_DEBUG_ASSERT(list->first == links);
        list->first = links->next;
    }
    if(links->next)
    {
        links->next->prev = links->prev;
    }
    else
    {
        OC_DEBUG_ASSERT(list->last == links);
        list->last = links->prev;
    }
    links->prev = links->next = 0;

    OC_DEBUG_ASSERT(list->count);
    list->count--;
}

void oc_list_push_front(oc_list* list, oc_list_links* links)
{
    links->next = list->first;
    links->prev = 0;
    if(list->first)
    {
        list->first->prev = links;
    }
    else
    {
        list->last = links;
    }
    list->first = links;
    list->count++;
}

oc_list_links* oc_list_pop_front(oc_list* list)
{
    oc_list_links* links = oc_list_begin(*list);
    if(links != oc_list_end(*list))
    {
        oc_list_remove(list, links);
        return (links);
    }
    else
    {
        return (0);
    }
}

void oc_list_push_back(oc_list* list, oc_list_links* links)
{
    links->prev = list->last;
    links->next = 0;
    if(list->last)
    {
        list->last->next = links;
    }
    else
    {
        list->first = links;
    }
    list->last = links;
    list->count++;
}

oc_list_links* oc_list_pop_back(oc_list* list)
{
    oc_list_links* links = oc_list_last(*list);
    if(links != oc_list_end(*list))
    {
        oc_list_remove(list, links);
        return (links);
    }
    else
    {
        return (0);
    }
}

bool oc_list_empty(oc_list list)
{
    return (list.first == 0 || list.last == 0);
}

//-------------------------------------------------------------------------
// Typed intrusive linked lists
//-------------------------------------------------------------------------
#define oc_typed_list_entry_generic(links, linksOffset) \
    (links ? ((char*)links - linksOffset) : 0)

#define oc_list_links_generic(item, linksOffset) \
    ((oc_list_links*)((char*)item + linksOffset))

void* oc_typed_list_next_generic(void* item, u64 linksOffset)
{
    oc_list_links* links = oc_list_links_generic(item, linksOffset);
    return oc_typed_list_entry_generic(links->next, linksOffset);
}

void* oc_typed_list_prev_generic(void* item, u64 linksOffset)
{
    oc_list_links* links = oc_list_links_generic(item, linksOffset);
    return oc_typed_list_entry_generic(links->prev, linksOffset);
}

void oc_typed_list_push_front_generic(oc_list* list, void* item, oc_list_links* links)
{
    (void)item;

    links->next = list->first;
    links->prev = 0;
    if(list->first)
    {
        list->first->prev = links;
    }
    else
    {
        list->last = links;
    }
    list->first = links;
    list->count++;
}

void oc_typed_list_push_back_generic(oc_list* list, void* item, oc_list_links* links)
{
    (void)item;

    links->prev = list->last;
    links->next = 0;
    if(list->last)
    {
        list->last->next = links;
    }
    else
    {
        list->first = links;
    }
    list->last = links;
    list->count++;
}

void oc_typed_list_remove_generic(oc_list* list, void* item, oc_list_links* links)
{
    (void)item;

    if(links->prev)
    {
        links->prev->next = links->next;
    }
    else
    {
        OC_DEBUG_ASSERT(list->first == links);
        list->first = links->next;
    }
    if(links->next)
    {
        links->next->prev = links->prev;
    }
    else
    {
        OC_DEBUG_ASSERT(list->last == links);
        list->last = links->prev;
    }
    links->prev = links->next = 0;

    OC_DEBUG_ASSERT(list->count);
    list->count--;
}

void* oc_typed_list_pop_front_generic(oc_list* list, u64 linksOffset)
{
    oc_list_links* links = list->first;
    if(links)
    {
        void* item = oc_typed_list_entry_generic(links, linksOffset);
        oc_typed_list_remove_generic(list, item, links);
        return item;
    }
    else
    {
        return (0);
    }
}

void* oc_typed_list_pop_back_generic(oc_list* list, u64 linksOffset)
{
    oc_list_links* links = list->last;
    if(links)
    {
        void* item = oc_typed_list_entry_generic(links, linksOffset);
        oc_typed_list_remove_generic(list, item, links);
        return item;
    }
    else
    {
        return (0);
    }
}

void oc_typed_list_insert_before_generic(oc_list* list, void* item, oc_list_links* before, oc_list_links* links)
{
    (void)item;

    links->next = before;
    links->prev = before->prev;

    if(before->prev)
    {
        before->prev->next = links;
    }
    else
    {
        list->first = links;
    }
    before->prev = links;
    list->count++;
    OC_DEBUG_ASSERT(links->next != links, "oc_typed_list_insert_before(): can't insert an element into itself");
}

void oc_typed_list_insert_after_generic(oc_list* list, void* item, oc_list_links* after, oc_list_links* links)
{
    (void)item;

    links->prev = after;
    links->next = after->next;
    if(after->next)
    {
        after->next->prev = links;
    }
    else
    {
        list->last = links;
    }
    after->next = links;
    list->count++;
    OC_DEBUG_ASSERT(links->next != links, "oc_typed_list_insert(): can't insert an element into itself");
}
