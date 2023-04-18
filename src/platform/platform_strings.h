/************************************************************//**
*
*	@file: platform_strings.h
*	@author: Martin Fouilleul
*	@date: 18/04/2023
*
*****************************************************************/
#ifndef __PLATFORM_STRINGS_H_
#define __PLATFORM_STRINGS_H_

#include"platform.h"
#include"platform_varg.h"

#if PLATFORM_ORCA
	#include"ext/stb_sprintf.h"

	size_t strlen(const char *s);
	int strcmp(const char *s1, const char *s2);
	char* strcpy(char *restrict s1, const char *restrict s2);

	#define vsnprintf stbsp_vsnprintf

#else
	#include<string.h>
	#include<stdio.h>
#endif

#endif //__PLATFORM_STRINGS_H_
