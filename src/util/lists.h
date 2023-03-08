/************************************************************//**
*
*	@file: lists.h
*	@author: Martin Fouilleul
*	@date: 22/11/2017
*	@revision: 28/04/2019 : deleted containers which are not used by BLITz
*	@brief: Implements generic intrusive linked list and dynamic array
*
****************************************************************/
#ifndef __CONTAINERS_H_
#define __CONTAINERS_H_

#include"debug_log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OFFSET_OF_CONTAINER(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#ifdef __cplusplus
#define CONTAINER_OF(ptr, type, member) ({          \
		    const decltype( ((type *)0)->member ) *__mptr = (ptr);    \
		    (type *)( (char *)__mptr - OFFSET_OF_CONTAINER(type,member) );})
#else
/*
#define CONTAINER_OF(ptr, type, member) ({          \
		    const char *__mptr = (char*)(ptr);    \
		    (type *)(__mptr - OFFSET_OF_CONTAINER(type,member) );})
*/
#define CONTAINER_OF(ptr, type, member) (type *)((char*)(ptr) - OFFSET_OF_CONTAINER(type,member))

#endif

//-------------------------------------------------------------------------
// Intrusive linked lists
//-------------------------------------------------------------------------

#define list_entry(ptr, type, member) \
	CONTAINER_OF(ptr, type, member)

#define list_next(elt) (elt)->next
#define list_prev(elt) (elt)->prev

#define list_next_entry(list, elt, type, member) \
	((elt->member.next != list_end(list)) ? list_entry(elt->member.next, type, member) : 0)

#define list_prev_entry(list, elt, type, member) \
	((elt->member.prev != list_end(list)) ? list_entry(elt->member.prev, type, member) : 0)

#define list_checked_entry(list, type, member) \
	(((list) != 0) ? list_entry(list, type, member) : 0)

#define list_first_entry(list, type, member) \
	(list_checked_entry(list_begin(list), type, member))

#define list_last_entry(list, type, member) \
	(list_checked_entry(list_last(list), type, member))

#define for_list(list, elt, type, member) \
	for(type* elt = list_checked_entry(list_begin(list), type, member); \
	    elt != 0; \
	    elt = list_checked_entry(elt->member.next, type, member)) \

#define for_list_reverse(list, elt, type, member) \
	for(type* elt = list_checked_entry(list_last(list), type, member); \
	    elt != 0; \
	    elt = list_checked_entry(elt->member.prev, type, member)) \

#define for_list_safe(list, elt, type, member) \
	for(type* elt = list_checked_entry(list_begin(list), type, member), \
	    *__tmp = elt ? list_checked_entry(elt->member.next, type, member) : 0 ; \
	    elt != 0; \
	    elt = __tmp, \
	    __tmp = elt ? list_checked_entry(elt->member.next, type, member) : 0) \

#define list_pop_entry(list, type, member) (list_empty(list) ? 0 : list_entry(list_pop(list), type, member))

typedef struct list_elt list_elt;
struct list_elt
{
	list_elt* next;
	list_elt* prev;
};

typedef struct list_info
{
	list_elt* first;
	list_elt* last;
} list_info;

static inline void list_init(list_info* list)
{
	list->first = list->last = 0;
}

static inline list_elt* list_begin(list_info* list)
{
	return(list->first);
}
static inline list_elt* list_end(list_info* list)
{
	return(0);
}

static inline list_elt* list_last(list_info* list)
{
	return(list->last);
}

static inline void list_insert(list_info* list, list_elt* afterElt, list_elt* elt)
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

	ASSERT(elt->next != elt, "list_insert(): can't insert an element into itself");
}

static inline void list_insert_before(list_info* list, list_elt* beforeElt, list_elt* elt)
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

	ASSERT(elt->next != elt, "list_insert_before(): can't insert an element into itself");
}

static inline void list_remove(list_info* list, list_elt* elt)
{
	if(elt->prev)
	{
		elt->prev->next = elt->next;
	}
	else
	{
		DEBUG_ASSERT(list->first == elt);
		list->first = elt->next;
	}
	if(elt->next)
	{
		elt->next->prev = elt->prev;
	}
	else
	{
		DEBUG_ASSERT(list->last == elt);
		list->last = elt->prev;
	}
	elt->prev = elt->next = 0;
}

static inline void list_push(list_info* list, list_elt* elt)
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

static inline list_elt* list_pop(list_info* list)
{
	list_elt* elt = list_begin(list);
	if(elt != list_end(list))
	{
		list_remove(list, elt);
		return(elt);
	}
	else
	{
		return(0);
	}
}

static inline void list_push_back(list_info* list, list_elt* elt)
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
#define list_append(a, b) list_push_back(a, b)


static inline list_elt* list_pop_back(list_info* list)
{
	list_elt* elt = list_last(list);
	if(elt != list_end(list))
	{
		list_remove(list, elt);
		return(elt);
	}
	else
	{
		return(0);
	}
}

static inline bool list_empty(list_info* list)
{
	return(list->first == 0 || list->last == 0);
}


//-------------------------------------------------------------------------
// Circular Intrusive linked lists
//-------------------------------------------------------------------------

#define clist_entry(ptr, type, member) list_entry(ptr, type, member)
#define clist_next(elt) list_next(elt)
#define clist_prev(elt) list_prev(elt)

#define clist_next_entry(head, elt, type, member) \
	((elt->member.next != clist_end(head)) ? clist_entry(elt->member.next, type, member) : 0)

#define clist_prev_entry(head, elt, type, member) \
	((elt->member.prev != clist_end(head)) ? clist_entry(elt->member.prev, type, member) : 0)

#define clist_checked_entry(head, info, type, member) \
	((info != clist_end(head)) ? clist_entry(info, type, member) : 0)

#define clist_first_entry(head, type, member) \
	(clist_checked_entry(head, clist_begin(head), type, member))

#define clist_last_entry(head, type, member) \
	(clist_checked_entry(head, clist_last(head), type, member))

#define for_clist(list, elt, type, member)			\
	for(type* elt = clist_entry(clist_begin(list), type, member);	\
	    &elt->member != clist_end(list);				\
	    elt = clist_entry(elt->member.next, type, member))		\


#define for_clist_reverse(list, elt, type, member)		\
	for(type* elt = clist_entry(clist_last(list), type, member);	\
	    &elt->member != clist_end(list);				\
	    elt = clist_entry(elt->member.prev, type, member))		\


#define for_clist_safe(list, elt, type, member)			\
	for(type* elt = clist_entry(clist_begin(list), type, member),	\
	    *__tmp = clist_entry(elt->member.next, type, member);	\
	    &elt->member != clist_end(list);				\
	    elt = clist_entry(&__tmp->member, type, member),		\
	    __tmp = clist_entry(elt->member.next, type, member))		\


#define clist_push(a, b) clist_insert(a, b)
#define clist_insert_before(a, b) clist_append(a, b)

#define clist_pop_entry(list, type, member) (clist_empty(list) ? 0 : clist_entry(clist_pop(list), type, member))

static inline void clist_init(list_elt* info)
{
	info->next = info->prev = info;
}

static inline list_elt* clist_begin(list_elt* head)
{
	return(head->next ? head->next : head );
}
static inline list_elt* clist_end(list_elt* head)
{
	return(head);
}

static inline list_elt* clist_last(list_elt* head)
{
	return(head->prev ? head->prev : head);
}

static inline void clist_insert(list_elt* head, list_elt* elt)
{
	elt->prev = head;
	elt->next = head->next;
	if(head->next)
	{
		head->next->prev = elt;
	}
	else
	{
		head->prev = elt;
	}
	head->next = elt;

	ASSERT(elt->next != elt, "clist_insert(): can't insert an element into itself");
}

static inline void clist_append(list_elt* head, list_elt* elt)
{
	clist_insert(head->prev, elt);
}

static inline void clist_cat(list_elt* head, list_elt* list)
{
	if(head->prev)
	{
		head->prev->next = list->next;
	}
	if(head->prev && head->prev->next)
	{
		head->prev->next->prev = head->prev;
	}
	head->prev = list->prev;
	if(head->prev)
	{
		head->prev->next = head;
	}
	clist_init(list);
}

static inline void clist_remove(list_elt* elt)
{
	if(elt->prev)
	{
		elt->prev->next = elt->next;
	}
	if(elt->next)
	{
		elt->next->prev = elt->prev;
	}
	elt->prev = elt->next = 0;
}

static inline list_elt* clist_pop(list_elt* head)
{
	list_elt* it = clist_begin(head);
	if(it != clist_end(head))
	{
		clist_remove(it);
		return(it);
	}
	else
	{
		return(0);
	}

}

static inline list_elt* clist_pop_back(list_elt* head)
{
	list_elt* it = clist_last(head);
	if(it != clist_end(head))
	{
		clist_remove(it);
		return(it);
	}
	else
	{
		return(0);
	}
}

static inline bool clist_empty(list_elt* head)
{
	return(head->next == 0 || head->next == head);
}


#ifdef __cplusplus
} // extern "C"
#endif

#endif //__CONTAINERS_H_
