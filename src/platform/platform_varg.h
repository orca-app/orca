/************************************************************//**
*
*	@file: platform_varg.h
*	@author: Martin Fouilleul
*	@date: 17/04/2023
*
*****************************************************************/
#ifndef __PLATFORM_VARG_H_
#define __PLATFORM_VARG_H_

#include"platform.h"

#if PLATFORM_ORCA
	#define va_list __builtin_va_list
	#define va_start __builtin_va_start
	#define va_arg __builtin_va_arg
	#define va_copy __builtin_va_copy
	#define va_end __builtin_va_end
#else
	#include<stdarg.h>
#endif

#endif //__PLATFORM_VARG_H_
