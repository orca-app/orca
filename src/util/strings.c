/************************************************************//**
*
*	@file: strings.c
*	@author: Martin Fouilleul
*	@date: 29/05/2021
*	@revision:
*
*****************************************************************/
#include<string.h>	// strlen(), strcpy(), etc.
#include<stdarg.h>	// va_list() etc.
#include"debug_log.h"
#include"strings.h"

#define LOG_SUBSYSTEM "Strings"

//----------------------------------------------------------------------------------
// string slices as values
//----------------------------------------------------------------------------------

str8 str8_from_buffer(u64 len, char* buffer)
{
	return((str8){.len = len, .ptr = buffer});
}

str8 str8_from_cstring(char* str)
{
	if(!str)
	{
		return((str8){0});
	}
	else
	{
		return(str8_from_buffer(strlen(str), (char*)str));
	}
}

str8 str8_slice(str8 s, u64 start, u64 end)
{
	ASSERT(start <= end && start <= s.len && end <= s.len);
	return((str8){.len = end - start, .ptr = s.ptr + start});
}

str8 str8_push_buffer(mem_arena* arena, u64 len, char* buffer)
{
	str8 str = {};
	str.len = len;
	str.ptr = mem_arena_alloc_array(arena, char, len);
	memcpy(str.ptr, buffer, len);
	return(str);
}

str8 str8_push_cstring(mem_arena* arena, const char* str)
{
	if(!str)
	{
		return((str8){0});
	}
	else
	{
		return(str8_push_buffer(arena, strlen(str), (char*)str));
	}
}

str8 str8_push_copy(mem_arena* arena, str8 s)
{
	return(str8_push_buffer(arena, str8_unbox(s)));
}

str8 str8_push_slice(mem_arena* arena, str8 s, u64 start, u64 end)
{
	str8 slice = str8_slice(s, start, end);
	return(str8_push_copy(arena, slice));
}

str8 str8_pushfv(mem_arena* arena, const char* format, va_list args)
{
	//NOTE(martin):
	//	We first compute the number of characters to write passing a size of 0.
	//      then we allocate len+1 (since vsnprint always terminates with a '\0').
	//      We could call vsprintf since we know the size, but I'm not sure there's a hard
	//      guarantee that vsprintf and vsnprintf write the same number of characters in all
	//      and every case, and that would be a difficult bug to spot, so it seems better to
	//      waste one byte and be safe.
	char dummy;
	str8 str = {};
	va_list argCopy;
	va_copy(argCopy, args);
	str.len = vsnprintf(&dummy, 0, format, argCopy);
	va_end(argCopy);

	str.ptr = mem_arena_alloc_array(arena, char, str.len + 1);
	vsnprintf((char*)str.ptr, str.len + 1, format, args);
	return(str);
}

str8 str8_pushf(mem_arena* arena, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	str8 str = str8_pushfv(arena, format, args);
	va_end(args);
	return(str);
}

int str8_cmp(str8 s1, str8 s2)
{
	int res = strncmp(s1.ptr, s2.ptr, minimum(s1.len, s2.len));
	if(!res)
	{
		res = (s1.len < s2.len)? -1 : ((s1.len == s2.len)? 0 : 1);
	}
	return(res);
}

char* str8_to_cstring(mem_arena* arena, str8 string)
{
	char* cstr = mem_arena_alloc_array(arena, char, string.len+1);
	memcpy(cstr, string.ptr, string.len);
	cstr[string.len] = '\0';
	return(cstr);
}

//----------------------------------------------------------------------------------
// string lists
//----------------------------------------------------------------------------------

void str8_list_init(str8_list* list)
{
	ListInit(&list->list);
	list->eltCount = 0;
	list->len = 0;
}

void str8_list_push(mem_arena* arena, str8_list* list, str8 str)
{
	str8_elt* elt = mem_arena_alloc_type(arena, str8_elt);
	elt->string = str;
	ListAppend(&list->list, &elt->listElt);
	list->eltCount++;
	list->len += str.len;
}

void str8_list_pushf(mem_arena* arena, str8_list* list, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	str8 str = str8_pushfv(arena, format, args);
	va_end(args);
	str8_list_push(arena, list, str);
}

str8 str8_list_collate(mem_arena* arena, str8_list list, str8 prefix, str8 separator, str8 postfix)
{
	str8 str = {};
	str.len = prefix.len + list.len + list.eltCount*separator.len + postfix.len;
	str.ptr = mem_arena_alloc_array(arena, char, str.len);
	char* dst = str.ptr;
	memcpy(dst, prefix.ptr, prefix.len);
	dst += prefix.len;

	str8_elt* elt = ListFirstEntry(&list.list, str8_elt, listElt);
	if(elt)
	{
		memcpy(dst, elt->string.ptr, elt->string.len);
		dst += elt->string.len;
		elt = ListNextEntry(&list.list, elt, str8_elt, listElt);
	}

	for( ; elt != 0; elt = ListNextEntry(&list.list, elt, str8_elt, listElt))
	{
		memcpy(dst, separator.ptr, separator.len);
		dst += separator.len;
		memcpy(dst, elt->string.ptr, elt->string.len);
		dst += elt->string.len;
	}
	memcpy(dst, postfix.ptr, postfix.len);
	return(str);
}

str8 str8_list_join(mem_arena* arena, str8_list list)
{
	str8 empty = {.len = 0, .ptr = 0};
	return(str8_list_collate(arena, list, empty, empty, empty));
}

str8_list str8_split(mem_arena* arena, str8 str, str8_list separators)
{
	str8_list list = {};
	ListInit(&list.list);

	char* ptr = str.ptr;
	char* end = str.ptr + str.len;
	char* subStart = ptr;
	for(; ptr < end; ptr++)
	{
		//NOTE(martin): search all separators and try to match them to the current ptr
		str8* foundSep = 0;
		for_each_in_list(&separators.list, elt, str8_elt, listElt)
		{
			str8* separator = &elt->string;
			bool equal = true;
			for(u64 offset = 0;
			    (offset < separator->len) && (ptr+offset < end);
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
			//NOTE(martin): we found a separator. If the start of the current substring is != ptr,
			//              the current substring is not empty and we emit the substring
			if(ptr != subStart)
			{
				str8 sub = str8_from_buffer(ptr-subStart, subStart);
				str8_list_push(arena, &list, sub);
			}
			ptr += foundSep->len - 1; //NOTE(martin): ptr is incremented at the end of the loop
			subStart = ptr+1;
		}
	}
	//NOTE(martin): emit the last substring
	if(ptr != subStart)
	{
		str8 sub = str8_from_buffer(ptr-subStart, subStart);
		str8_list_push(arena, &list, sub);
	}
	return(list);
}

//----------------------------------------------------------------------------------
// u32 strings
//----------------------------------------------------------------------------------
str32 str32_from_buffer(u64 len, u32* buffer)
{
	return((str32){.len = len, .ptr = buffer});
}

str32 str32_slice(str32 s, u64 start, u64 end)
{
	ASSERT(start <= end && start <= s.len && end <= s.len);
	return((str32){.len = end - start, .ptr = s.ptr + start});
}

str32 str32_push_buffer(mem_arena* arena, u64 len, u32* buffer)
{
	str32 str = {};
	str.len = len;
	str.ptr = mem_arena_alloc_array(arena, u32, len);
	memcpy(str.ptr, buffer, len*sizeof(u32));
	return(str);
}

str32 str32_push_copy(mem_arena* arena, str32 s)
{
	return(str32_push_buffer(arena, s.len, s.ptr));
}

str32 str32_push_slice(mem_arena* arena, str32 s, u64 start, u64 end)
{
	str32 slice = str32_slice(s, start, end);
	return(str32_push_copy(arena, slice));
}

void str32_list_init(str32_list* list)
{
	ListInit(&list->list);
	list->eltCount = 0;
	list->len = 0;
}

void str32_list_push(mem_arena* arena, str32_list* list, str32 str)
{
	str32_elt* elt = mem_arena_alloc_type(arena, str32_elt);
	elt->string = str;
	ListAppend(&list->list, &elt->listElt);
	list->eltCount++;
	list->len += str.len;
}

str32 str32_list_collate(mem_arena* arena, str32_list list, str32 prefix, str32 separator, str32 postfix)
{
	str32 str = {};
	str.len = prefix.len + list.len + list.eltCount*separator.len + postfix.len;
	str.ptr = mem_arena_alloc_array(arena, u32, str.len);
	char* dst = (char*)str.ptr;
	memcpy(dst, prefix.ptr, prefix.len*sizeof(u32));
	dst += prefix.len*sizeof(u32);

	str32_elt* elt = ListFirstEntry(&list.list, str32_elt, listElt);
	if(elt)
	{
		memcpy(dst, elt->string.ptr, elt->string.len*sizeof(u32));
		dst += elt->string.len*sizeof(u32);
		elt = ListNextEntry(&list.list, elt, str32_elt, listElt);
	}

	for( ; elt != 0; elt = ListNextEntry(&list.list, elt, str32_elt, listElt))
	{
		memcpy(dst, separator.ptr, separator.len*sizeof(u32));
		dst += separator.len*sizeof(u32);
		memcpy(dst, elt->string.ptr, elt->string.len*sizeof(u32));
		dst += elt->string.len*sizeof(u32);
	}
	memcpy(dst, postfix.ptr, postfix.len*sizeof(u32));
	return(str);
}

str32 str32_list_join(mem_arena* arena, str32_list list)
{
	str32 empty = {.len = 0, .ptr = 0};
	return(str32_list_collate(arena, list, empty, empty, empty));
}


//----------------------------------------------------------------------------------
// Paths helpers
//----------------------------------------------------------------------------------
str8 mp_path_directory(str8 fullPath)
{
	i64 lastSlashIndex = -1;

	for(i64 i = fullPath.len-1; i >= 0; i--)
	{
		if(fullPath.ptr[i] == '/')
		{
			lastSlashIndex = i;
			break;
		}
	}
	str8 directory = str8_slice(fullPath, 0, lastSlashIndex+1);
	return(directory);
}

str8 mp_path_base_name(str8 fullPath)
{
	i64 lastSlashIndex = -1;

	for(i64 i = fullPath.len-1; i >= 0; i--)
	{
		if(fullPath.ptr[i] == '/')
		{
			lastSlashIndex = i;
			break;
		}
	}

	str8 basename = str8_slice(fullPath, lastSlashIndex+1, fullPath.len);
	return(basename);
}



#undef LOG_SUBSYSTEM
