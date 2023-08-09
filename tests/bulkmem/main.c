/************************************************************//**
*
*	@file: main.c
*	@author: Martin Fouilleul
*	@date: 25/06/2023
*
*****************************************************************/

#include<stddef.h>
#include<stdint.h>

void* memset(void* b, int c, size_t n)
{
	return(__builtin_memset(b, c, n));
}

void* memcpy(void* dst, const void* src, size_t n)
{
	return(__builtin_memcpy(dst, src, n));
}

int _start()
{
	uint8_t buffer[1024];
	uint8_t dst[1024];
	memset(buffer, 0xff, 1024);
	if(buffer[32] != 0xff)
	{
		__builtin_unreachable();
	}

	memcpy(dst, buffer, 1024);
	if(dst[32] != 0xff)
	{
		__builtin_unreachable();
	}
	return(0);
}
