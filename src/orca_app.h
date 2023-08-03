/************************************************************//**
*
*	@file: orca_runtime.h
*	@author: Martin Fouilleul
*	@date: 17/04/2023
*
*****************************************************************/
#ifndef __ORCA_RUNTIME_H_
#define __ORCA_RUNTIME_H_

#include"platform/platform_io_internal.h"

#include"wasm3.h"
#include"m3_env.h"
#include"m3_compile.h"

#define G_EXPORTS(X) \
	X(G_EXPORT_ON_INIT, "OnInit", "", "") \
	X(G_EXPORT_MOUSE_DOWN, "OnMouseDown", "", "i") \
	X(G_EXPORT_MOUSE_UP, "OnMouseUp", "", "i") \
	X(G_EXPORT_MOUSE_ENTER, "OnMouseEnter", "", "") \
	X(G_EXPORT_MOUSE_LEAVE, "OnMouseLeave", "", "") \
	X(G_EXPORT_MOUSE_MOVE, "OnMouseMove", "", "ffff") \
	X(G_EXPORT_MOUSE_WHEEL, "OnMouseWheel", "", "ff") \
	X(G_EXPORT_KEY_DOWN, "OnKeyDown", "", "i") \
	X(G_EXPORT_KEY_UP, "OnKeyUp", "", "i") \
	X(G_EXPORT_FRAME_REFRESH, "OnFrameRefresh", "", "") \
	X(G_EXPORT_FRAME_RESIZE, "OnFrameResize", "", "ii") \
	X(G_EXPORT_RAW_EVENT, "OnRawEvent", "", "i") \

typedef enum {
	#define G_EXPORT_KIND(kind, ...) kind,
	G_EXPORTS(G_EXPORT_KIND)
	G_EXPORT_COUNT
} guest_export_kind;


typedef struct g_export_desc
{
	str8 name;
	str8 retTags;
	str8 argTags;
} g_export_desc;

const g_export_desc G_EXPORT_DESC[] = {
	#define STR8LIT(s) {sizeof(s)-1, s} //NOTE: msvc doesn't accept STR8(s) as compile-time constant...
	#define G_EXPORT_DESC_ENTRY(kind, name, rets, args) {STR8LIT(name), STR8LIT(rets), STR8LIT(args)},

	G_EXPORTS(G_EXPORT_DESC_ENTRY)

	#undef G_EXPORT_DESC_ENTRY
	#undef STR8LIT
};

typedef struct wasm_memory
{
	char* ptr;
	u64 reserved;
	u64 committed;

} wasm_memory;

typedef struct orca_runtime
{
	str8 wasmBytecode;
	wasm_memory wasmMemory;

	// wasm3 data
	IM3Environment m3Env;
	IM3Runtime m3Runtime;
	IM3Module m3Module;
	IM3Function exports[G_EXPORT_COUNT];
	u32 rawEventOffset;

} orca_runtime;

typedef struct log_entry
{
	list_elt listElt;
	u64 cap;

	log_level level;
	str8 file;
	str8 function;
	int line;
	str8 msg;

	u64 recordIndex;

} log_entry;

typedef struct orca_debug_overlay
{
	bool show;
	mg_surface surface;
	mg_canvas canvas;
	mg_font fontReg;
	mg_font fontBold;
	ui_context ui;


	mem_arena logArena;
	list_info logEntries;
	list_info logFreeList;
	u32 entryCount;
	u32 maxEntries;
	u64 logEntryTotalCount;
	bool logScrollToLast;

} orca_debug_overlay;

typedef struct orca_app
{
	mp_window window;

	file_table fileTable;
	file_handle rootDir;

	orca_runtime runtime;

	orca_debug_overlay debugOverlay;

} orca_app;

orca_app* orca_app_get();
orca_runtime* orca_runtime_get();


int orca_assert(const char* file, const char* function, int line, const char* src, const char* note);
int orca_assert_fmt(const char* file, const char* function, int line, const char* src, const char* fmt, ...);
void orca_abort_fmt(const char* file, const char* function, int line, const char* fmt, ...);

#define _ORCA_ASSERT_(test, fmt, ...) ((test) || orca_assert_fmt(__FILE__, __FUNCTION__, __LINE__, #test, fmt, ##__VA_ARGS__))
#define ORCA_ASSERT(test, ...) _ORCA_ASSERT_(test, ORCA_VA_NOPT("", ##__VA_ARGS__) ORCA_ARG1(__VA_ARGS__) ORCA_VA_COMMA_TAIL(__VA_ARGS__))

#define ORCA_ABORT(fmt, ...) orca_abort_fmt(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#endif //__ORCA_RUNTIME_H_
