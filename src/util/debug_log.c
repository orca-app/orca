/************************************************************//**
*
*	@file: debug_log.c
*	@author: Martin Fouilleul
*	@date: 22/10/2020
*	@revision:
*
*****************************************************************/
#include<stdarg.h>
#include<string.h>
#include"debug_log.h"

static const char* LOG_HEADINGS[LOG_LEVEL_COUNT] = {
	"Error",
	"Warning",
	"Message",
	"Debug"};

static const char* LOG_FORMATS[LOG_LEVEL_COUNT] = {
	"\033[38;5;9m\033[1m",
	"\033[38;5;13m\033[1m",
	"\033[38;5;10m\033[1m",
	"\033[38;5;14m\033[1m" };

static const char* LOG_FORMAT_STOP = "\033[m";

enum {
	LOG_SUBSYSTEM_MAX_COUNT = 16 };

typedef struct log_config
{
	FILE* out;
	log_level level;
	const char* subsystemNames[LOG_SUBSYSTEM_MAX_COUNT];
	log_level subsystemLevels[LOG_SUBSYSTEM_MAX_COUNT];

} log_config;

static log_config __log_config = {.out = 0,
                                  .level = LOG_DEFAULT_LEVEL,
                                  .subsystemNames = {0},
                                  .subsystemLevels = {0}};

int LogFindSubsystem(const char* subsystem)
{
	for(int i=0; i<LOG_SUBSYSTEM_MAX_COUNT; i++)
	{
		if(__log_config.subsystemNames[i])
		{
			if(!strcmp(__log_config.subsystemNames[i], subsystem))
			{
				return(i);
			}
		}
	}
	return(-1);
}

void LogGeneric(log_level level,
		const char* subsystem,
		const char* functionName,
                const char* fileName,
		u32 line,
		const char* msg,
		...)
{
	int subsystemIndex = LogFindSubsystem(subsystem);
	int filterLevel = (subsystemIndex >= 0)? __log_config.subsystemLevels[subsystemIndex] : __log_config.level;

	if(level <= filterLevel)
	{
		if(!__log_config.out)
		{
			__log_config.out = LOG_DEFAULT_OUTPUT;
		}
		fprintf(__log_config.out,
			"%s%s:%s [%s] %s() in %s:%i: ",
			LOG_FORMATS[level],
			LOG_HEADINGS[level],
			LOG_FORMAT_STOP,
			subsystem,
			functionName,
			fileName,
			line);

		va_list ap;
		va_start(ap, msg);
		vfprintf(__log_config.out, msg, ap);
		va_end(ap);
	}
}

void LogOutput(FILE* output)
{
	__log_config.out = output;
}

void LogLevel(log_level level)
{
	__log_config.level = level;
}

void LogFilter(const char* subsystem, log_level level)
{
	int firstNull = -1;
	for(int i=0; i<LOG_SUBSYSTEM_MAX_COUNT; i++)
	{
		if(__log_config.subsystemNames[i])
		{
			if(!strcmp(__log_config.subsystemNames[i], subsystem))
			{
				__log_config.subsystemLevels[i] = level;
				return;
			}
		}
		else if(firstNull < 0)
		{
			firstNull = i;
		}
	}

	__log_config.subsystemNames[firstNull] = subsystem;
	__log_config.subsystemLevels[firstNull] = level;
}
