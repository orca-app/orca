/************************************************************//**
*
*	@file: platform_log.h
*	@author: Martin Fouilleul
*	@date: 18/04/2023
*
*****************************************************************/
#ifndef __PLATFORM_LOG_H_
#define __PLATFORM_LOG_H_

#include"platform.h"
#include"strings.h"

typedef enum { LOG_LEVEL_ERROR,
               LOG_LEVEL_WARNING,
               LOG_LEVEL_INFO,
               LOG_LEVEL_COUNT } log_level;

typedef struct log_output log_output;

extern log_output* LOG_DEFAULT_OUTPUT;

MP_API void log_set_level(log_level level);
MP_API void log_set_output(log_output* output);
MP_API void log_push(log_level level,
                     str8 function,
                     str8 file,
                     int line,
                     const char* fmt,
                     ...);

#define log_generic(level, msg, ...) log_push(level, STR8(__FUNCTION__), STR8(__FILE__), __LINE__, msg, ##__VA_ARGS__)

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


#endif //__PLATFORM_LOG_H_
