/************************************************************//**
*
*	@file: debug.h
*	@author: Martin Fouilleul
*	@date: 13/08/2023
*
*****************************************************************/
#ifndef __DEBUG_H_
#define __DEBUG_H_

#include"platform/platform_debug.h"
#include"util/macro_helpers.h"

//----------------------------------------------------------------
// Logging
//----------------------------------------------------------------
#define log_generic(level, msg, ...) log_push(level, __FUNCTION__, __FILE__, __LINE__, msg, ##__VA_ARGS__)

#define log_error(msg, ...) log_generic(LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#ifndef LOG_COMPILE_WARNING
	#define LOG_COMPILE_WARNING 1
#endif

#ifndef LOG_COMPILE_INFO
	#define LOG_COMPILE_INFO 1
#endif

#if LOG_COMPILE_WARNING || LOG_COMPILE_INFO
	#define log_warning(msg, ...) log_generic(LOG_LEVEL_WARNING, msg, ##__VA_ARGS__)

	#if LOG_COMPILE_INFO
		#define log_info(msg, ...) log_generic(LOG_LEVEL_INFO, msg, ##__VA_ARGS__ )
	#else
		#define log_info(msg, ...)
	#endif
#else
	#define log_warning(msg, ...)
	#define log_info(msg, ...)
#endif

//----------------------------------------------------------------
// Abort/Assert
//----------------------------------------------------------------

#define ORCA_ABORT(fmt, ...) orca_abort(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define _ORCA_ASSERT_(test, fmt, ...) ((test) || (orca_assert_fail(__FILE__, __FUNCTION__, __LINE__, #test, fmt, ##__VA_ARGS__), 0))
#define ORCA_ASSERT(test, ...) _ORCA_ASSERT_(test, ORCA_VA_NOPT("", ##__VA_ARGS__) ORCA_ARG1(__VA_ARGS__) ORCA_VA_COMMA_TAIL(__VA_ARGS__))


#ifndef NO_ASSERT

	#define ASSERT(x, ...) ORCA_ASSERT(x, #__VA_ARGS__)

	#ifdef DEBUG
		#define DEBUG_ASSERT(x, ...) ASSERT(x, ##__VA_ARGS__ )
	#else
		#define DEBUG_ASSERT(x, ...)
	#endif
#else
	#define ASSERT(x, ...)
	#define DEBUG_ASSERT(x, ...)
#endif


#endif //__DEBUG_H_
