/************************************************************/ /**
*
*	@file: stdarg.h
*	@author: Martin Fouilleul
*	@date: 17/04/2023
*
*****************************************************************/
#ifndef __STDARG_H_
#define __STDARG_H_

#define va_list __builtin_va_list
#define va_start __builtin_va_start
#define va_arg __builtin_va_arg
#define va_copy __builtin_va_copy
#define va_end __builtin_va_end

#endif //__STDARG_H_
