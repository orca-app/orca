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
	X(G_EXPORT_ON_MOUSE_DOWN, "OnMouseDown", "", "i") \
	X(G_EXPORT_ON_MOUSE_UP, "OnMouseUp", "", "i") \
	X(G_EXPORT_ON_MOUSE_ENTER, "OnMouseEnter", "", "") \
	X(G_EXPORT_ON_MOUSE_LEAVE, "OnMouseLeave", "", "") \
	X(G_EXPORT_ON_MOUSE_MOVE, "OnMouseMove", "", "ffff") \
	X(G_EXPORT_ON_MOUSE_WHEEL, "OnMouseWheel", "", "ff") \
	X(G_EXPORT_ON_KEY_DOWN, "OnKeyDown", "", "i") \
	X(G_EXPORT_ON_KEY_UP, "OnKeyUp", "", "i") \
	X(G_EXPORT_ON_FRAME_REFRESH, "OnFrameRefresh", "", "") \
	X(G_EXPORT_ON_FRAME_RESIZE, "OnFrameResize", "", "ii") \
	X(G_EXPORT_ON_RAW_EVENT, "OnRawEvent", "", "i") \
	X(G_EXPORT_GET_RAW_EVENT_PTR, "_OrcaGetRawEventPtr", "i", "") \
	X(G_EXPORT_CLOCK_INIT, "_OrcaClockInit", "", "")

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
	mg_surface surface;
	mg_canvas canvas;

	file_table fileTable;
	file_handle rootDir;

	orca_runtime runtime;

	orca_debug_overlay debugOverlay;

} orca_app;

orca_app* orca_app_get();
orca_runtime* orca_runtime_get();


#endif //__ORCA_RUNTIME_H_
