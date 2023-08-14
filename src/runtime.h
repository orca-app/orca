/************************************************************//**
*
*	@file: runtime.h
*	@author: Martin Fouilleul
*	@date: 17/04/2023
*
*****************************************************************/
#ifndef __RUNTIME_H_
#define __RUNTIME_H_

#include"platform/platform_io_internal.h"

#include"wasm3.h"
#include"m3_env.h"
#include"m3_compile.h"

#define OC_EXPORTS(X) \
	X(OC_EXPORT_ON_INIT, "oc_on_init", "", "") \
	X(OC_EXPORT_MOUSE_DOWN, "oc_on_mouse_down", "", "i") \
	X(OC_EXPORT_MOUSE_UP, "oc_on_mouse_up", "", "i") \
	X(OC_EXPORT_MOUSE_ENTER, "oc_on_mouse_enter", "", "") \
	X(OC_EXPORT_MOUSE_LEAVE, "oc_on_mouse_leave", "", "") \
	X(OC_EXPORT_MOUSE_MOVE, "oc_on_mouse_move", "", "ffff") \
	X(OC_EXPORT_MOUSE_WHEEL, "oc_on_mouse_wheel", "", "ff") \
	X(OC_EXPORT_KEY_DOWN, "oc_on_key_down", "", "i") \
	X(OC_EXPORT_KEY_UP, "oc_on_key_up", "", "i") \
	X(OC_EXPORT_FRAME_REFRESH, "oc_on_frame_refresh", "", "") \
	X(OC_EXPORT_FRAME_RESIZE, "oc_on_resize", "", "ii") \
	X(OC_EXPORT_RAW_EVENT, "oc_on_raw_event", "", "i") \

typedef enum {
	#define OC_EXPORT_KIND(kind, ...) kind,
	OC_EXPORTS(OC_EXPORT_KIND)
	OC_EXPORT_COUNT
} guest_export_kind;


typedef struct oc_export_desc
{
	oc_str8 name;
	oc_str8 retTags;
	oc_str8 argTags;
} oc_export_desc;

const oc_export_desc OC_EXPORT_DESC[] = {
	#define OC_EXPORT_DESC_ENTRY(kind, name, rets, args) {OC_STR8_LIT(name), OC_STR8_LIT(rets), OC_STR8_LIT(args)},

	OC_EXPORTS(OC_EXPORT_DESC_ENTRY)

	#undef OC_EXPORT_DESC_ENTRY
	#undef OC_STR8_LIT
};

typedef struct wasm_memory
{
	char* ptr;
	u64 reserved;
	u64 committed;

} wasm_memory;

typedef struct oc_runtime_env
{
	oc_str8 wasmBytecode;
	wasm_memory wasmMemory;

	// wasm3 data
	IM3Environment m3Env;
	IM3Runtime m3Runtime;
	IM3Module m3Module;
	IM3Function exports[OC_EXPORT_COUNT];
	u32 rawEventOffset;

} oc_runtime_env;

typedef struct log_entry
{
	oc_list_elt listElt;
	u64 cap;

	oc_log_level level;
	oc_str8 file;
	oc_str8 function;
	int line;
	oc_str8 msg;

	u64 recordIndex;

} log_entry;

typedef struct oc_debug_overlay
{
	bool show;
	oc_surface surface;
	oc_canvas canvas;
	oc_font fontReg;
	oc_font fontBold;
	oc_ui_context ui;


	oc_arena logArena;
	oc_list logEntries;
	oc_list logFreeList;
	u32 entryCount;
	u32 maxEntries;
	u64 logEntryTotalCount;
	bool logScrollToLast;

} oc_debug_overlay;

typedef struct oc_runtime
{
	oc_window window;

	oc_file_table fileTable;
	oc_file rootDir;

	oc_runtime_env runtime;

	oc_debug_overlay debugOverlay;

} oc_runtime;

oc_runtime* oc_runtime_get();
oc_runtime_env* oc_runtime_env_get();

#endif //__RUNTIME_H_
