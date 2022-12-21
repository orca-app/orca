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

#define ListEntry(ptr, type, member) \
	CONTAINER_OF(ptr, type, member)

#define ListNext(elt) (elt)->next
#define ListPrev(elt) (elt)->prev

#define ListNextEntry(list, elt, type, member) \
	((elt->member.next != ListEnd(list)) ? ListEntry(elt->member.next, type, member) : 0)

#define ListPrevEntry(list, elt, type, member) \
	((elt->member.prev != ListEnd(list)) ? ListEntry(elt->member.prev, type, member) : 0)

#define ListCheckedEntry(list, type, member) \
	(((list) != 0) ? ListEntry(list, type, member) : 0)

#define ListFirstEntry(list, type, member) \
	(ListCheckedEntry(ListBegin(list), type, member))

#define ListLastEntry(list, type, member) \
	(ListCheckedEntry(ListLast(list), type, member))

#define for_each_in_list(list, elt, type, member) \
	for(type* elt = ListCheckedEntry(ListBegin(list), type, member); \
	    elt != 0; \
	    elt = ListCheckedEntry(elt->member.next, type, member)) \

#define for_each_in_list_reverse(list, elt, type, member) \
	for(type* elt = ListCheckedEntry(ListLast(list), type, member); \
	    elt != 0; \
	    elt = ListCheckedEntry(elt->member.prev, type, member)) \

#define for_each_in_list_safe(list, elt, type, member) \
	for(type* elt = ListCheckedEntry(ListBegin(list), type, member), \
	    *__tmp = elt ? ListCheckedEntry(elt->member.next, type, member) : 0 ; \
	    elt != 0; \
	    elt = __tmp, \
	    __tmp = elt ? ListCheckedEntry(elt->member.next, type, member) : 0) \

#define ListPopEntry(list, type, member) (ListEmpty(list) ? 0 : ListEntry(ListPop(list), type, member))

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

static inline void ListInit(list_info* list)
{
	list->first = list->last = 0;
}

static inline list_elt* ListBegin(list_info* list)
{
	return(list->first);
}
static inline list_elt* ListEnd(list_info* list)
{
	return(0);
}

static inline list_elt* ListLast(list_info* list)
{
	return(list->last);
}

static inline void ListInsert(list_info* list, list_elt* afterElt, list_elt* elt)
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

	ASSERT(elt->next != elt, "ListInsert(): can't insert an element into itself");
}

static inline void ListInsertBefore(list_info* list, list_elt* beforeElt, list_elt* elt)
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

	ASSERT(elt->next != elt, "ListInsertBefore(): can't insert an element into itself");
}

static inline void ListRemove(list_info* list, list_elt* elt)
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

static inline void ListPush(list_info* list, list_elt* elt)
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

static inline list_elt* ListPop(list_info* list)
{
	list_elt* elt = ListBegin(list);
	if(elt != ListEnd(list))
	{
		ListRemove(list, elt);
		return(elt);
	}
	else
	{
		return(0);
	}
}

static inline void ListPushBack(list_info* list, list_elt* elt)
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
#define ListAppend(a, b) ListPushBack(a, b)


static inline list_elt* ListPopBack(list_info* list)
{
	list_elt* elt = ListLast(list);
	if(elt != ListEnd(list))
	{
		ListRemove(list, elt);
		return(elt);
	}
	else
	{
		return(0);
	}
}

static inline bool ListEmpty(list_info* list)
{
	return(list->first == 0 || list->last == 0);
}


//-------------------------------------------------------------------------
// Circular Intrusive linked lists
//-------------------------------------------------------------------------

#define CListEntry(ptr, type, member) ListEntry(ptr, type, member)
#define CListNext(elt) ListNext(elt)
#define CListPrev(elt) ListPrev(elt)

#define CListNextEntry(head, elt, type, member) \
	((elt->member.next != CListEnd(head)) ? CListEntry(elt->member.next, type, member) : 0)

#define CListPrevEntry(head, elt, type, member) \
	((elt->member.prev != CListEnd(head)) ? CListEntry(elt->member.prev, type, member) : 0)

#define CListCheckedEntry(head, info, type, member) \
	((info != CListEnd(head)) ? CListEntry(info, type, member) : 0)

#define CListFirstEntry(head, type, member) \
	(CListCheckedEntry(head, CListBegin(head), type, member))

#define CListLastEntry(head, type, member) \
	(CListCheckedEntry(head, CListLast(head), type, member))

#define for_each_in_clist(list, elt, type, member)			\
	for(type* elt = CListEntry(CListBegin(list), type, member);	\
	    &elt->member != CListEnd(list);				\
	    elt = CListEntry(elt->member.next, type, member))		\


#define for_each_in_clist_reverse(list, elt, type, member)		\
	for(type* elt = CListEntry(CListLast(list), type, member);	\
	    &elt->member != CListEnd(list);				\
	    elt = CListEntry(elt->member.prev, type, member))		\


#define for_each_in_clist_safe(list, elt, type, member)			\
	for(type* elt = CListEntry(CListBegin(list), type, member),	\
	    *__tmp = CListEntry(elt->member.next, type, member);	\
	    &elt->member != CListEnd(list);				\
	    elt = CListEntry(&__tmp->member, type, member),		\
	    __tmp = CListEntry(elt->member.next, type, member))		\


#define CListPush(a, b) CListInsert(a, b)
#define CListInsertBefore(a, b) CListAppend(a, b)

#define CListPopEntry(list, type, member) (CListEmpty(list) ? 0 : CListEntry(CListPop(list), type, member))

static inline void CListInit(list_elt* info)
{
	info->next = info->prev = info;
}

static inline list_elt* CListBegin(list_elt* head)
{
	return(head->next ? head->next : head );
}
static inline list_elt* CListEnd(list_elt* head)
{
	return(head);
}

static inline list_elt* CListLast(list_elt* head)
{
	return(head->prev ? head->prev : head);
}

static inline void CListInsert(list_elt* head, list_elt* elt)
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

	ASSERT(elt->next != elt, "CListInsert(): can't insert an element into itself");
}

static inline void CListAppend(list_elt* head, list_elt* elt)
{
	CListInsert(head->prev, elt);
}

static inline void CListCat(list_elt* head, list_elt* list)
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
	CListInit(list);
}

static inline void CListRemove(list_elt* elt)
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

static inline list_elt* CListPop(list_elt* head)
{
	list_elt* it = CListBegin(head);
	if(it != CListEnd(head))
	{
		CListRemove(it);
		return(it);
	}
	else
	{
		return(0);
	}

}

static inline list_elt* CListPopBack(list_elt* head)
{
	list_elt* it = CListLast(head);
	if(it != CListEnd(head))
	{
		CListRemove(it);
		return(it);
	}
	else
	{
		return(0);
	}
}

static inline bool CListEmpty(list_elt* head)
{
	return(head->next == 0 || head->next == head);
}


#ifdef __cplusplus
} // extern "C"
#endif

#endif //__CONTAINERS_H_
