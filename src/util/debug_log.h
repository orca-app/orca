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

#include"platform.h"
#include"typedefs.h"
#include"macro_helpers.h"

#ifdef __cplusplus
extern "C" {
#endif
//NOTE(martin): the default logging level can be adjusted by defining LOG_DEFAULT_LEVEL.
//              It can be adjusted at runtime with log_set_level()
#ifndef LOG_DEFAULT_LEVEL
	#define LOG_DEFAULT_LEVEL LOG_LEVEL_WARNING
#endif

typedef enum { LOG_LEVEL_ERROR,
               LOG_LEVEL_WARNING,
               LOG_LEVEL_INFO,
               LOG_LEVEL_COUNT } log_level;

#ifdef PLATFORM_ORCA
	typedef enum
	{
		ORCA_LOG_OUTPUT_CONSOLE,
		ORCA_LOG_OUTPUT_FILE
	} orca_log_output_kind;

	typedef struct log_output
	{
		orca_log_output_kind kind;
		//TODO: file
	} log_output;

#else
	#include<stdio.h>
	typedef FILE* log_output;
#endif

MP_API void log_set_level(log_level level);
MP_API void log_set_output(log_output output);
MP_API void log_enable_vt_color(bool enable);

MP_API void log_generic(log_level level,
                        const char* functionName,
                        const char* fileName,
                        u32 line,
                        const char* msg,
                        ...);

//NOTE(martin): warnings, messages, and debug info can be enabled in debug mode by defining LOG_COMPILE_XXX, XXX being the max desired log level
//              error logging is always compiled
#define LOG_ERROR(msg, ...) log_generic(LOG_LEVEL_ERROR, __FUNCTION__, __FILE__, __LINE__, msg, ##__VA_ARGS__ )

#if defined(LOG_COMPILE_WARNING) || defined(LOG_COMPILE_INFO)
	#define LOG_WARNING(msg, ...) log_generic(LOG_LEVEL_WARNING, __FUNCTION__, __FILE__, __LINE__, msg, ##__VA_ARGS__ )

	#if defined(LOG_COMPILE_INFO)
		#define LOG_INFO(msg, ...) log_generic(LOG_LEVEL_INFO, __FUNCTION__, __FILE__, __LINE__, msg, ##__VA_ARGS__ )
	#else
		#define LOG_INFO(msg, ...)
	#endif
#else
	#define LOG_WARNING(msg, ...)
	#define LOG_INFO(msg, ...)
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__DEBUG_LOG_H_
