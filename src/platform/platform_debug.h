/************************************************************//**
*
*	@file: platform_debug.h
*	@author: Martin Fouilleul
*	@date: 13/08/2023
*
*****************************************************************/
#ifndef __PLATFORM_DEBUG_H_
#define __PLATFORM_DEBUG_H_

#include"platform.h"

//----------------------------------------------------------------
// Assert / Abort
//----------------------------------------------------------------

MP_API _Noreturn void orca_abort(const char* file, const char* function, int line, const char* fmt, ...);
MP_API _Noreturn void orca_assert_fail(const char* file, const char* function, int line, const char* src, const char* fmt, ...);

//----------------------------------------------------------------
// Logging
//----------------------------------------------------------------

typedef enum { LOG_LEVEL_ERROR,
               LOG_LEVEL_WARNING,
               LOG_LEVEL_INFO,
               LOG_LEVEL_COUNT } log_level;

typedef struct log_output log_output;

extern log_output* LOG_DEFAULT_OUTPUT;

MP_API void log_set_level(log_level level);
MP_API void log_set_output(log_output* output);
MP_API void log_push(log_level level,
                     const char* function,
                     const char* file,
                     int line,
                     const char* fmt,
                     ...);


#endif //__PLATFORM_DEBUG_H_
