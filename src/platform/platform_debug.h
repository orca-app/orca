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

ORCA_API _Noreturn void oc_abort_ext(const char* file, const char* function, int line, const char* fmt, ...);
ORCA_API _Noreturn void oc_assert_fail(const char* file, const char* function, int line, const char* src, const char* fmt, ...);

//----------------------------------------------------------------
// Logging
//----------------------------------------------------------------

typedef enum { OC_LOG_LEVEL_ERROR,
               OC_LOG_LEVEL_WARNING,
               OC_LOG_LEVEL_INFO,
               OC_LOG_LEVEL_COUNT } oc_log_level;

typedef struct oc_log_output oc_log_output;

extern oc_log_output* OC_LOG_DEFAULT_OUTPUT;

ORCA_API void oc_log_set_level(oc_log_level level);
ORCA_API void oc_log_set_output(oc_log_output* output);
ORCA_API void oc_log_ext(oc_log_level level,
                        const char* function,
                        const char* file,
                        int line,
                        const char* fmt,
                        ...);


#endif //__PLATFORM_DEBUG_H_
