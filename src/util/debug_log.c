/************************************************************//**
*
*	@file: debug_log.c
*	@author: Martin Fouilleul
*	@date: 22/10/2020
*	@revision:
*
*****************************************************************/
#include"debug_log.h"
#include"platform_varg.h"

static const char* LOG_HEADINGS[LOG_LEVEL_COUNT] = {
	"Error",
	"Warning",
	"Info"};

static const char* LOG_FORMATS[LOG_LEVEL_COUNT] = {
	"\033[38;5;9m\033[1m",
	"\033[38;5;13m\033[1m",
	"\033[38;5;10m\033[1m"};

static const char* LOG_FORMAT_STOP = "\033[m";

typedef struct log_config
{
	log_output output;
	log_level level;
	bool enableVTColor;

} log_config;

#if PLATFORM_ORCA
	#define LOG_DEFAULT_OUTPUT (log_output){.kind = ORCA_LOG_OUTPUT_CONSOLE}
	#define LOG_DEFAULT_ENABLE_VT_COLOR true
#else
	#define LOG_DEFAULT_OUTPUT 0
	#define LOG_DEFAULT_ENABLE_VT_COLOR true
#endif

static log_config __logConfig = {.output = LOG_DEFAULT_OUTPUT,
                                  .level = LOG_DEFAULT_LEVEL,
                                  .enableVTColor = LOG_DEFAULT_ENABLE_VT_COLOR};

void log_set_output(log_output output)
{
	__logConfig.output = output;
}

void log_set_level(log_level level)
{
	__logConfig.level = level;
}

void log_enable_vt_color(bool enable)
{
	__logConfig.enableVTColor = enable;
}

#if PLATFORM_ORCA

#define STB_SPRINTF_IMPLEMENTATION
#include"ext/stb_sprintf.h"

typedef int orca_log_mode;
enum {ORCA_LOG_BEGIN, ORCA_LOG_APPEND};

extern void orca_log_entry(log_level level,
                           int fileLen,
                           const char* file,
                           int functionLen,
                           const char* function,
                           int line);

extern void orca_log_append(int msgLen, const char* msg);

char* log_stbsp_callback(char const* buf, void* user, int len)
{
	orca_log_append(len, buf);
	return((char*)buf);
}


//TODO: later, move this to orca_strings in milepost
size_t strlen(const char *s)
{
        size_t len = 0;
        while(s[len] != '\0')
        {
                len++;
        }
        return(len);
}

void log_generic(log_level level,
                 const char* functionName,
                 const char* fileName,
                 u32 line,
                 const char* msg,
                 ...)
{
	if(level <= __logConfig.level)
	{
		orca_log_entry(level, strlen(functionName), functionName, strlen(fileName), fileName, line);

		char buf[STB_SPRINTF_MIN];

		va_list ap;
		va_start(ap, msg);
		stbsp_vsprintfcb(log_stbsp_callback, 0, buf, msg, ap);
		va_end(ap);
	}
}

#else

void log_generic(log_level level,
                 const char* functionName,
                 const char* fileName,
                 u32 line,
                 const char* msg,
                 ...)
{
	if(__logConfig.output == 0)
	{
		__logConfig.output = stdout;
	}

	if(level <= __logConfig.level)
	{
		if(__logConfig.enableVTColor)
		{
			fprintf(__logConfig.output,
			        "%s%s:%s %s() in %s:%i: ",
			        LOG_FORMATS[level],
			        LOG_HEADINGS[level],
			        LOG_FORMAT_STOP,
			        functionName,
			        fileName,
			        line);
		}
		else
		{
			fprintf(__logConfig.output,
			        "%s: %s() in %s:%i: ",
			        LOG_HEADINGS[level],
			        functionName,
			        fileName,
			        line);
		}
		va_list ap;
		va_start(ap, msg);
		vfprintf(__logConfig.output, msg, ap);
		va_end(ap);
	}
}
#endif
