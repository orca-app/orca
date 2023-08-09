/************************************************************//**
*
*	@file: orca_log.c
*	@author: Martin Fouilleul
*	@date: 18/04/2023
*
*****************************************************************/
#include"platform_log.c"
#include"util/memory.h"
#include"util/strings.h"

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

typedef struct orca_log_context
{
	mem_arena* arena;
	str8_list list;
} orca_log_context;

char* log_stbsp_callback(char const* buf, void* user, int len)
{
	orca_log_context* ctx = (orca_log_context*)user;

	str8 string = str8_push_buffer(ctx->arena, len, (char*)buf);
	str8_list_push(ctx->arena, &ctx->list, string);

	return((char*)buf);
}

void platform_log_push(log_output* output,
                       log_level level,
                       str8 function,
                       str8 file,
                       int line,
                       const char* fmt,
                       va_list ap)
{
	mem_arena* scratch = mem_scratch();
	mem_arena_scope tmp = mem_arena_scope_begin(scratch);

	orca_log_context ctx = {.arena = scratch,
		                       .list = {0}};

	char buf[STB_SPRINTF_MIN];
	stbsp_vsprintfcb(log_stbsp_callback, &ctx, buf, fmt, ap);

	str8 string = str8_list_join(scratch, ctx.list);

	orca_log(level, str8_ip(function), str8_ip(file), line, str8_ip(string));

	mem_arena_scope_end(tmp);
}
