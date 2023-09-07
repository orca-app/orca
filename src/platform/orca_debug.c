/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <stdarg.h>

#include "platform_debug.c"
#include "util/strings.h"

//----------------------------------------------------------------
// stb sprintf callback and user struct
//----------------------------------------------------------------

typedef struct oc_stbsp_context
{
    oc_arena* arena;
    oc_str8_list list;
} oc_stbsp_context;

char* oc_stbsp_callback(char const* buf, void* user, int len)
{
    oc_stbsp_context* ctx = (oc_stbsp_context*)user;

    oc_str8 string = oc_str8_push_buffer(ctx->arena, len, (char*)buf);
    oc_str8_list_push(ctx->arena, &ctx->list, string);

    return ((char*)buf);
}

//----------------------------------------------------------------
// Logging
//----------------------------------------------------------------

typedef enum
{
    ORCA_LOG_OUTPUT_CONSOLE,
    ORCA_LOG_OUTPUT_FILE
} oc_log_output_kind;

typedef struct oc_log_output
{
    oc_log_output_kind kind;
    //TODO: file output
} oc_log_output;

static oc_log_output oc_logDefaultOutput = { .kind = ORCA_LOG_OUTPUT_CONSOLE };
oc_log_output* OC_LOG_DEFAULT_OUTPUT = &oc_logDefaultOutput;

void ORCA_IMPORT(oc_bridge_log)(oc_log_level level,
                                 int fileLen,
                                 const char* file,
                                 int functionLen,
                                 const char* function,
                                 int line,
                                 int msgLen,
                                 const char* msg);

void platform_log_push(oc_log_output* output,
                       oc_log_level level,
                       const char* file,
                       const char* function,
                       int line,
                       const char* fmt,
                       va_list ap)
{
    oc_arena* scratch = oc_scratch();
    oc_arena_scope tmp = oc_arena_scope_begin(scratch);

    oc_stbsp_context ctx = { .arena = scratch,
                             .list = { 0 } };

    char buf[STB_SPRINTF_MIN];
    stbsp_vsprintfcb(oc_stbsp_callback, &ctx, buf, fmt, ap);

    oc_str8 string = oc_str8_list_join(scratch, ctx.list);

    oc_bridge_log(level, strlen(file), file, strlen(function), function, line, oc_str8_ip(string));

    oc_arena_scope_end(tmp);
}

//----------------------------------------------------------------
// Assert/Abort
//----------------------------------------------------------------

_Noreturn void ORCA_IMPORT(oc_bridge_abort_ext)(const char* file, const char* function, int line, const char* msg);
_Noreturn void ORCA_IMPORT(oc_bridge_assert_fail)(const char* file, const char* function, int line, const char* src, const char* msg);

_Noreturn void oc_abort_ext(const char* file, const char* function, int line, const char* fmt, ...)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_stbsp_context ctx = {
        .arena = scratch.arena,
        .list = { 0 }
    };

    va_list ap;
    va_start(ap, fmt);

    char buf[STB_SPRINTF_MIN];
    stbsp_vsprintfcb(oc_stbsp_callback, &ctx, buf, fmt, ap);

    va_end(ap);

    oc_str8 msg = oc_str8_list_join(scratch.arena, ctx.list);

    oc_bridge_abort_ext(file, function, line, msg.ptr);

    oc_scratch_end(scratch);
}

_Noreturn void oc_assert_fail(const char* file, const char* function, int line, const char* src, const char* fmt, ...)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_stbsp_context ctx = {
        .arena = scratch.arena,
        .list = { 0 }
    };

    va_list ap;
    va_start(ap, fmt);

    char buf[STB_SPRINTF_MIN];
    stbsp_vsprintfcb(oc_stbsp_callback, &ctx, buf, fmt, ap);

    va_end(ap);

    oc_str8 msg = oc_str8_list_join(scratch.arena, ctx.list);

    oc_bridge_assert_fail(file, function, line, src, msg.ptr);

    oc_scratch_end(scratch);
}
