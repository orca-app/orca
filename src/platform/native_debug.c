/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <stdio.h>

#include "app/app.h"
#include "platform_debug.c"
//----------------------------------------------------------------
// Logging
//----------------------------------------------------------------

#if OC_PLATFORM_WINDOWS
    #include <io.h>
    #define isatty _isatty
    #define fileno _fileno
#elif OC_PLATFORM_MACOS || PLATFORM_LINUX
    #include <unistd.h>
#endif

static const char* OC_LOG_HEADINGS[OC_LOG_LEVEL_COUNT] = {
    "Error",
    "Warning",
    "Info"
};

static const char* OC_LOG_FORMATS[OC_LOG_LEVEL_COUNT] = {
    "\033[38;5;9m\033[1m",
    "\033[38;5;13m\033[1m",
    "\033[38;5;10m\033[1m"
};

static const char* OC_LOG_FORMAT_STOP = "\033[m";

typedef struct oc_log_output
{
    FILE* f;
} oc_log_output;

static oc_log_output oc_logDefaultOutput = { 0 };
oc_log_output* OC_LOG_DEFAULT_OUTPUT = &oc_logDefaultOutput;

void platform_log_push(oc_log_output* output,
                       oc_log_level level,
                       const char* file,
                       const char* function,
                       int line,
                       const char* fmt,
                       va_list ap)
{
    if(output == OC_LOG_DEFAULT_OUTPUT && output->f == 0)
    {
        output->f = stdout;
    }

    int fd = fileno(output->f);
    if(isatty(fd))
    {
        fprintf(output->f,
                "%s%s:%s %s() in %s:%i: ",
                OC_LOG_FORMATS[level],
                OC_LOG_HEADINGS[level],
                OC_LOG_FORMAT_STOP,
                function,
                file,
                line);
    }
    else
    {
        fprintf(output->f,
                "%s: %s() in %s:%i: ",
                OC_LOG_HEADINGS[level],
                function,
                file,
                line);
    }
    vfprintf(output->f, fmt, ap);
}

//----------------------------------------------------------------
// Assert/Abort
//----------------------------------------------------------------

_Noreturn void oc_abort_ext(const char* file, const char* function, int line, const char* fmt, ...)
{
    oc_arena_scope scratch = oc_scratch_begin();

    va_list ap;
    va_start(ap, fmt);
    oc_str8 note = oc_str8_pushfv(scratch.arena, fmt, ap);
    va_end(ap);

    oc_str8 msg = oc_str8_pushf(scratch.arena,
                                "Fatal error in function %s() in file \"%s\", line %i:\n%.*s\n",
                                function,
                                file,
                                line,
                                (int)note.len,
                                note.ptr);

    oc_log_error(msg.ptr);

    oc_str8_list options = { 0 };
    oc_str8_list_push(scratch.arena, &options, OC_STR8("OK"));

    oc_alert_popup(OC_STR8("Fatal Error"), msg, options);

    //TODO: could terminate more gracefully?
    exit(-1);
}

_Noreturn void oc_assert_fail(const char* file, const char* function, int line, const char* src, const char* fmt, ...)
{
    oc_arena_scope scratch = oc_scratch_begin();

    va_list ap;
    va_start(ap, fmt);
    oc_str8 note = oc_str8_pushfv(scratch.arena, fmt, ap);
    va_end(ap);

    oc_str8 msg = oc_str8_pushf(scratch.arena,
                                "Assertion failed in function %s() in file \"%s\", line %i:\n%s\nNote: %.*s\n",
                                function,
                                file,
                                line,
                                src,
                                oc_str8_ip(note));

    oc_log_error(msg.ptr);

    oc_str8_list options = { 0 };
    oc_str8_list_push(scratch.arena, &options, OC_STR8("OK"));

    oc_alert_popup(OC_STR8("Assertion Failed"), msg, options);

    //TODO: could terminate more gracefully?
    exit(-1);
}
