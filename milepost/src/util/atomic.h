//*****************************************************************
//
//	$file: atomic.h $
//	$author: Martin Fouilleul $
//	$date: 22/12/2022 $
//	$revision: $
//	$note: (C) 2022 by Martin Fouilleul - all rights reserved $
//
//*****************************************************************
#ifndef __ATOMIC_H_
#define __ATOMIC_H_

#include"platform.h"

#if (defined(COMPILER_CL) || defined(COMPILER_CLANG_CL)) && defined(__STDC_NO_ATOMICS__)
	#define _Atomic(t) volatile t
	//TODO
#else
	#include<stdatomic.h>
#endif

#endif //__ATOMIC_H_
