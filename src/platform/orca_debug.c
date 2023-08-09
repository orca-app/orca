/************************************************************//**
*
*	@file: orca_debug.c
*	@author: Martin Fouilleul
*	@date: 13/08/2023
*
*****************************************************************/
#include<stdarg.h>

#include"platform_debug.c"
#include"util/strings.h"


//----------------------------------------------------------------
// stb sprintf callback and user struct
//----------------------------------------------------------------

typedef struct orca_stbsp_context
{
	mem_arena* arena;
	str8_list list;
} orca_stbsp_context;

char* orca_stbsp_callback(char const* buf, void* user, int len)
{
	orca_stbsp_context* ctx = (orca_stbsp_context*)user;

	str8 string = str8_push_buffer(ctx->arena, len, (char*)buf);
	str8_list_push(ctx->arena, &ctx->list, string);

	return((char*)buf);
}

//----------------------------------------------------------------
// Logging
//----------------------------------------------------------------

typedef enum
{
	ORCA_LOG_OUTPUT_CONSOLE,
	ORCA_LOG_OUTPUT_FILE
} orca_log_output_kind;

typedef struct log_output
{
	orca_log_output_kind kind;
	//TODO: file output
} log_output;

static log_output _logDefaultOutput = {.kind = ORCA_LOG_OUTPUT_CONSOLE};
log_output* LOG_DEFAULT_OUTPUT = &_logDefaultOutput;

void ORCA_IMPORT(orca_log)(log_level level,
                           int fileLen,
                           const char* file,
                           int functionLen,
                           const char* function,
                           int line,
                           int msgLen,
                           const char* msg);

void platform_log_push(log_output* output,
                       log_level level,
                       const char* file,
                       const char* function,
                       int line,
                       const char* fmt,
                       va_list ap)
{
	mem_arena* scratch = mem_scratch();
	mem_arena_scope tmp = mem_arena_scope_begin(scratch);

	orca_stbsp_context ctx = {.arena = scratch,
		                       .list = {0}};

	char buf[STB_SPRINTF_MIN];
	stbsp_vsprintfcb(orca_stbsp_callback, &ctx, buf, fmt, ap);

	str8 string = str8_list_join(scratch, ctx.list);

	orca_log(level, strlen(file), file, strlen(function), function, line, str8_ip(string));

	mem_arena_scope_end(tmp);
}

//----------------------------------------------------------------
// Assert/Abort
//----------------------------------------------------------------

_Noreturn void ORCA_IMPORT(orca_runtime_abort)(const char* file, const char* function, int line, const char* msg);
_Noreturn void ORCA_IMPORT(orca_runtime_assert_fail)(const char* file, const char* function, int line, const char* src, const char* msg);

_Noreturn void orca_abort(const char* file, const char* function, int line, const char* fmt, ...)
{
	mem_arena_scope scratch = mem_scratch_begin();

	orca_stbsp_context ctx = {
		.arena = scratch.arena,
		.list = {0}
	};

	va_list ap;
	va_start(ap, fmt);

	char buf[STB_SPRINTF_MIN];
	stbsp_vsprintfcb(orca_stbsp_callback, &ctx, buf, fmt, ap);

	va_end(ap);

	str8 msg = str8_list_join(scratch.arena, ctx.list);

	orca_runtime_abort(file, function, line, msg.ptr);

	mem_scratch_end(scratch);
}

_Noreturn void orca_assert_fail(const char* file, const char* function, int line, const char* src, const char* fmt, ...)
{
	mem_arena_scope scratch = mem_scratch_begin();

	orca_stbsp_context ctx = {
		.arena = scratch.arena,
		.list = {0}
	};

	va_list ap;
	va_start(ap, fmt);

	char buf[STB_SPRINTF_MIN];
	stbsp_vsprintfcb(orca_stbsp_callback, &ctx, buf, fmt, ap);

	va_end(ap);

	str8 msg = str8_list_join(scratch.arena, ctx.list);

	orca_runtime_assert_fail(file, function, line, src, msg.ptr);

	mem_scratch_end(scratch);
}
