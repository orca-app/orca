/************************************************************//**
*
*	@file: native_debug.c
*	@author: Martin Fouilleul
*	@date: 13/08/2023
*
*****************************************************************/
#include<stdio.h>

#include"app/mp_app.h"
#include"platform_debug.c"
//----------------------------------------------------------------
// Logging
//----------------------------------------------------------------

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
                       const char* file,
                       const char* function,
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
		        "%s%s:%s %s() in %s:%i: ",
		        LOG_FORMATS[level],
		        LOG_HEADINGS[level],
		        LOG_FORMAT_STOP,
		        function,
		        file,
		        line);
	}
	else
	{
		fprintf(output->f,
		        "%s: %s() in %s:%i: ",
		        LOG_HEADINGS[level],
		        function,
		        file,
		        line);
	}
	vfprintf(output->f, fmt, ap);
}

//----------------------------------------------------------------
// Assert/Abort
//----------------------------------------------------------------

_Noreturn void orca_abort(const char* file, const char* function, int line, const char* fmt, ...)
{
	mem_arena* scratch = mem_scratch();

	va_list ap;
	va_start(ap, fmt);
	str8 note = str8_pushfv(scratch, fmt, ap);
	va_end(ap);

	str8 msg = str8_pushf(scratch,
	                      "Fatal error in function %s() in file \"%s\", line %i:\n%.*s\n",
	                      function,
	                      file,
	                      line,
	                      (int)note.len,
	                      note.ptr);

	const char* msgCStr = str8_to_cstring(scratch, msg);
	log_error(msgCStr);

	const char* options[] = {"OK"};
	mp_alert_popup("Fatal Error", msgCStr, 1, options);

	//TODO: could terminate more gracefully?
	exit(-1);
}

_Noreturn void orca_assert_fail(const char* file, const char* function, int line, const char* src, const char* fmt, ...)
{
	mem_arena* scratch = mem_scratch();

	va_list ap;
	va_start(ap, fmt);
	str8 note = str8_pushfv(scratch, fmt, ap);
	va_end(ap);

	str8 msg = str8_pushf(scratch,
	                      "Assertion failed in function %s() in file \"%s\", line %i:\n%s\nNote: %.*s\n",
	                      function,
	                      file,
	                      line,
	                      src,
	                      str8_ip(note));

	const char* msgCStr = str8_to_cstring(scratch, msg);
	log_error(msgCStr);

	const char* options[] = {"OK"};
	mp_alert_popup("Assertion Failed", msgCStr, 1, options);

	//TODO: could terminate more gracefully?
	exit(-1);
}
