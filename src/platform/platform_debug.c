/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "platform_debug.h"

typedef struct oc_log_config
{
    oc_log_output* output;
    oc_log_level level;
} oc_log_config;

//TODO: make default output a compile-time constant to avoid check in oc_log_ext()?
static oc_log_config __logConfig = { 0, OC_LOG_LEVEL_INFO };

void oc_log_set_output(oc_log_output* output)
{
    __logConfig.output = output;
}

void oc_log_set_level(oc_log_level level)
{
    __logConfig.level = level;
}

void platform_log_push(oc_log_output* output,
                       oc_log_level level,
                       const char* function,
                       const char* file,
                       int line,
                       const char* fmt,
                       va_list ap);

void oc_log_ext(oc_log_level level,
                const char* function,
                const char* file,
                int line,
                const char* fmt,
                ...)
{
    if(!__logConfig.output)
    {
        __logConfig.output = OC_LOG_DEFAULT_OUTPUT;
    }

    if(level <= __logConfig.level)
    {
        va_list ap;
        va_start(ap, fmt);
        platform_log_push(__logConfig.output, level, file, function, line, fmt, ap);
        va_end(ap);
    }
}
