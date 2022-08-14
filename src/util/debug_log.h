/************************************************************//**
*
*	@file: debug_log.h
*	@author: Martin Fouilleul
*	@date: 05/04/2019
*	@revision:
*
*****************************************************************/
#ifndef __DEBUG_LOG_H_
#define __DEBUG_LOG_H_

#include<stdio.h>
#include"typedefs.h"
#include"macro_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif
//NOTE(martin): the default logging level can be adjusted by defining LOG_DEFAULT_LEVEL. As the name suggest, it is the default, but it
//              can be adjusted at runtime with LogLevel()
#ifndef LOG_DEFAULT_LEVEL
	#define LOG_DEFAULT_LEVEL LOG_LEVEL_WARNING
#endif

//NOTE(martin): the default output can be adjusted by defining LOG_DEFAULT_OUTPUT. It can be adjusted at runtime with LogOutput()
#ifndef LOG_DEFAULT_OUTPUT
	#define LOG_DEFAULT_OUTPUT stdout
#endif

//NOTE(martin): LOG_SUBSYSTEM can be defined in each compilation unit to associate it with a subsystem, like this:
//              #define LOG_SUBSYSTEM "name"

typedef enum { LOG_LEVEL_ERROR,
               LOG_LEVEL_WARNING,
	       LOG_LEVEL_MESSAGE,
	       LOG_LEVEL_DEBUG,
	       LOG_LEVEL_COUNT } log_level;

void LogGeneric(log_level level,
		const char* subsystem,
		const char* functionName,
                const char* fileName,
		u32 line,
		const char* msg,
		...);

void LogOutput(FILE* output);
void LogLevel(log_level level);
void LogFilter(const char* subsystem, log_level level);

#define LOG_GENERIC(level, func, file, line, msg, ...) LogGeneric(level, LOG_SUBSYSTEM, func, file, line, msg, ##__VA_ARGS__ )

#define LOG_ERROR(msg, ...) LOG_GENERIC(LOG_LEVEL_ERROR, __FUNCTION__, __FILE__, __LINE__, msg, ##__VA_ARGS__ )

//NOTE(martin): warnings, messages, and debug info can be enabled in debug mode by defining LOG_COMPILE_XXX, XXX being the max desired log level
//              error logging is always compiled
#if defined(LOG_COMPILE_WARNING) || defined(LOG_COMPILE_MESSAGE) || defined(LOG_COMPILE_DEBUG)
	#define LOG_WARNING(msg, ...) LOG_GENERIC(LOG_LEVEL_WARNING, __FUNCTION__, __FILE__, __LINE__, msg, ##__VA_ARGS__ )

	#if defined(LOG_COMPILE_MESSAGE) || defined(LOG_COMPILE_DEBUG)
		#define LOG_MESSAGE(msg, ...) LOG_GENERIC(LOG_LEVEL_MESSAGE, __FUNCTION__, __FILE__, __LINE__, msg, ##__VA_ARGS__ )

		#if defined(LOG_COMPILE_DEBUG)
			#define LOG_DEBUG(msg, ...) LOG_GENERIC(LOG_LEVEL_DEBUG, __FUNCTION__, __FILE__, __LINE__, msg, ##__VA_ARGS__ )
		#else
			#define LOG_DEBUG(msg, ...)
		#endif
	#else
		#define LOG_MESSAGE(msg, ...)
		#define LOG_DEBUG(msg, ...)
	#endif
#else
	#define LOG_WARNING(msg, ...)
	#define LOG_MESSAGE(msg, ...)
	#define LOG_DEBUG(msg, ...)
#endif

#ifndef NO_ASSERT
	#include<assert.h>
	#define _ASSERT_(x, msg) assert(x && msg)
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

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__DEBUG_LOG_H_
