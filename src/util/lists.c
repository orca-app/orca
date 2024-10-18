/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*	@brief: Implements generic intrusive linked list and dynamic array
*
****************************************************************/

#include "lists.h"

void oc_list_init(oc_list* list)
{
    list->first = list->last = 0;
}

void oc_list_insert(oc_list* list, oc_list_elt* afterElt, oc_list_elt* elt)
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

void oc_list_insert_before(oc_list* list, oc_list_elt* beforeElt, oc_list_elt* elt)
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

void oc_list_remove(oc_list* list, oc_list_elt* elt)
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

void oc_list_push_front(oc_list* list, oc_list_elt* elt)
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

oc_list_elt* oc_list_pop_front(oc_list* list)
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

void oc_list_push_back(oc_list* list, oc_list_elt* elt)
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

oc_list_elt* oc_list_pop_back(oc_list* list)
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

bool oc_list_empty(oc_list list)
{
    return (list.first == 0 || list.last == 0);
}
