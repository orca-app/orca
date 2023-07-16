/************************************************************//**
*
*	@file: orca.h
*	@author: Martin Fouilleul
*	@date: 13/04/2023
*
*****************************************************************/
#ifndef __ORCA_H_
#define __ORCA_H_

#include"util/typedefs.h"
#include"util/lists.h"
#include"util/memory.h"
#include"util/strings.h"
#include"util/utf8.h"
#include"platform/platform.h"
#include"platform/platform_log.h"
#include"platform/platform_assert.h"
#include"platform/platform_clock.h"
#include"platform/platform_io.h"

#if COMPILER_CLANG
	#define ORCA_EXPORT __attribute__((visibility("default")))
#else
	#error "Orca apps can only be compiled with clang for now"
#endif

#endif //__ORCA_H_
