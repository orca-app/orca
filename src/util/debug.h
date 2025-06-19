/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "platform/platform_debug.h"
#include "util/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------
// Logging
//----------------------------------------------------------------
#define oc_log_generic(level, msg, ...) oc_log_ext(level, __FUNCTION__, __FILE__, __LINE__, msg, ##__VA_ARGS__)

#define oc_log_error(msg, ...) oc_log_generic(OC_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#ifndef OC_LOG_COMPILE_WARNING
    #define OC_LOG_COMPILE_WARNING 1
#endif

#ifndef OC_LOG_COMPILE_INFO
    #define OC_LOG_COMPILE_INFO 1
#endif

#if OC_LOG_COMPILE_WARNING || OC_LOG_COMPILE_INFO
    #define oc_log_warning(msg, ...) oc_log_generic(OC_LOG_LEVEL_WARNING, msg, ##__VA_ARGS__)

    #if OC_LOG_COMPILE_INFO
        #define oc_log_info(msg, ...) oc_log_generic(OC_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)
    #else
        #define oc_log_info(msg, ...)
    #endif
#else
    #define oc_log_warning(msg, ...)
    #define oc_log_info(msg, ...)
#endif

//----------------------------------------------------------------
// Abort/Assert
//----------------------------------------------------------------

#define _OC_ABORT_(fmt, ...) oc_abort_ext(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)
#define OC_ABORT(...) _OC_ABORT_(OC_VA_NOPT("", ##__VA_ARGS__) OC_ARG1(__VA_ARGS__) OC_VA_COMMA_TAIL(__VA_ARGS__))

#ifdef OC_NO_ASSERT
    #define OC_ASSERT(x, ...)
    #define OC_DEBUG_ASSERT(x, ...)
#else
    #define _OC_ASSERT_(test, fmt, ...) ((test) || (oc_assert_fail(__FILE__, __FUNCTION__, __LINE__, #test, fmt, ##__VA_ARGS__), 0))
    #define OC_ASSERT(test, ...) _OC_ASSERT_(test, OC_VA_NOPT("", ##__VA_ARGS__) OC_ARG1(__VA_ARGS__) OC_VA_COMMA_TAIL(__VA_ARGS__))

    #ifdef OC_DEBUG
        #define OC_DEBUG_ASSERT(x, ...) OC_ASSERT(x, ##__VA_ARGS__)
    #else
        #define OC_DEBUG_ASSERT(x, ...)
    #endif
#endif

#define oc_notpossible() OC_ABORT("Not possible")
#define oc_unimplemented() OC_ABORT("Unimplemented")

#ifdef __cplusplus
}
#endif
