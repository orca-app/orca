/************************************************************//**
*
*	@file: std_log.c
*	@author: Martin Fouilleul
*	@date: 18/04/2023
*
*****************************************************************/
#include<stdio.h>
#include"platform_log.c"

#if PLATFORM_WINDOWS
	#include<io.h>
	#define isatty _isatty
	#define fileno _fileno
#elif PLATFORM_MACOS || PLATFORM_LINUX
	#include<unistd.h>
#endif

static const char* LOG_HEADINGS[LOG_LEVEL_COUNT] = {
	"Error",
	"Warning",
	"Info"};

static const char* LOG_FORMATS[LOG_LEVEL_COUNT] = {
	"\033[38;5;9m\033[1m",
	"\033[38;5;13m\033[1m",
	"\033[38;5;10m\033[1m"};

static const char* LOG_FORMAT_STOP = "\033[m";

typedef struct log_output
{
	FILE* f;
} log_output;

static log_output _logDefaultOutput = {0};
log_output* LOG_DEFAULT_OUTPUT = &_logDefaultOutput;

void platform_log_push(log_output* output,
                       log_level level,
                       str8 function,
                       str8 file,
                       int line,
                       const char* fmt,
                       va_list ap)
{
	if(output == LOG_DEFAULT_OUTPUT && output->f == 0)
	{
		output->f = stdout;
	}

	int fd = fileno(output->f);
	if(isatty(fd))
	{
		fprintf(output->f,
		        "%s%s:%s %.*s() in %.*s:%i: ",
		        LOG_FORMATS[level],
		        LOG_HEADINGS[level],
		        LOG_FORMAT_STOP,
		        str8_ip(function),
		        str8_ip(file),
		        line);
	}
	else
	{
		fprintf(output->f,
		        "%s: %.*s() in %.*s:%i: ",
		        LOG_HEADINGS[level],
		        str8_ip(function),
		        str8_ip(file),
		        line);
	}
	vfprintf(output->f, fmt, ap);
}
