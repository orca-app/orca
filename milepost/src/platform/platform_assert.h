/************************************************************//**
*
*	@file: platform_assert.h
*	@author: Martin Fouilleul
*	@date: 18/04/2023
*
*****************************************************************/
#ifndef __PLATFORM_ASSERT_H_
#define __PLATFORM_ASSERT_H_

#include"platform.h"

//NOTE: assert macros
#ifndef NO_ASSERT
	#ifdef PLATFORM_ORCA
		int ORCA_IMPORT(orca_assert)(const char* file, const char* function, int line, const char* src, const char* msg);
		#define _ASSERT_(x, msg) ((x) || orca_assert(__FILE__, __FUNCTION__, __LINE__, #x, msg))
	#else
		#include<assert.h>
		#define _ASSERT_(x, msg) assert(x && msg)
	#endif

	#define ASSERT(x, ...) _ASSERT_(x, #__VA_ARGS__)

	#ifdef DEBUG
		#define DEBUG_ASSERT(x, ...) ASSERT(x, ##__VA_ARGS__ )
	#else
		#define DEBUG_ASSERT(x, ...)
	#endif
#else
	#define ASSERT(x, ...)
	#define DEBUG_ASSERT(x, ...)
#endif

#endif //__PLATFORM_ASSERT_H_
