/************************************************************//**
*
*	@file: platform_log.c
*	@author: Martin Fouilleul
*	@date: 18/04/2023
*
*****************************************************************/
#include"platform_debug.h"

typedef struct log_config
{
	log_output* output;
	log_level level;
} log_config;

//TODO: make default output a compile-time constant to avoid check in log_push()?
static log_config __logConfig = {0, LOG_LEVEL_INFO};

void log_set_output(log_output* output)
{
	__logConfig.output = output;
}

void log_set_level(log_level level)
{
	__logConfig.level = level;
}

void platform_log_push(log_output* output,
                       log_level level,
                       const char* file,
                       const char* function,
                       int line,
                       const char* fmt,
                       va_list ap);

void log_push(log_level level,
               const char* function,
               const char* file,
               int line,
               const char* fmt,
               ...)
{
	if(!__logConfig.output)
	{
		__logConfig.output = LOG_DEFAULT_OUTPUT;
	}

	if(level <= __logConfig.level)
	{
		va_list ap;
		va_start(ap, fmt);
		platform_log_push(__logConfig.output, level, file, function, line, fmt, ap);
		va_end(ap);
	}
}
