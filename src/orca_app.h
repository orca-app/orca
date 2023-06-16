/************************************************************//**
*
*	@file: orca_runtime.h
*	@author: Martin Fouilleul
*	@date: 17/04/2023
*
*****************************************************************/
#ifndef __ORCA_RUNTIME_H_
#define __ORCA_RUNTIME_H_

#include"wasm3.h"
#include"m3_env.h"
#include"m3_compile.h"

#define G_EVENTS(X) \
	X(G_EVENT_START, "OnInit", "", "") \
	X(G_EVENT_MOUSE_DOWN, "OnMouseDown", "", "i") \
	X(G_EVENT_MOUSE_UP, "OnMouseUp", "", "i") \
	X(G_EVENT_MOUSE_ENTER, "OnMouseEnter", "", "") \
	X(G_EVENT_MOUSE_LEAVE, "OnMouseLeave", "", "") \
	X(G_EVENT_MOUSE_MOVE, "OnMouseMove", "", "ffff") \
	X(G_EVENT_MOUSE_WHEEL, "OnMouseWheel", "", "ff") \
	X(G_EVENT_KEY_DOWN, "OnKeyDown", "", "i") \
	X(G_EVENT_KEY_UP, "OnKeyUp", "", "i") \
	X(G_EVENT_FRAME_REFRESH, "OnFrameRefresh", "", "") \
	X(G_EVENT_FRAME_RESIZE, "OnFrameResize", "", "ii")

typedef enum {
	#define G_EVENT_KIND(kind, ...) kind,
	G_EVENTS(G_EVENT_KIND)
	G_EVENT_COUNT
} guest_event_kind;


typedef struct g_event_handler_desc
{
	str8 name;
	str8 retTags;
	str8 argTags;
} g_event_handler_desc;

const g_event_handler_desc G_EVENT_HANDLER_DESC[] = {

	#define STR8LIT(s) {sizeof(s), s} //NOTE: msvc doesn't accept STR8(s) as compile-time constant...
	#define G_EVENT_HANDLER_DESC_ENTRY(kind, name, rets, args) {STR8LIT(name), STR8LIT(rets), STR8LIT(args)},

	G_EVENTS(G_EVENT_HANDLER_DESC_ENTRY)

	#undef G_EVENT_HANDLER_DESC_ENTRY
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
	IM3Function eventHandlers[G_EVENT_COUNT];

} orca_runtime;


orca_runtime* orca_runtime_get();


#endif //__ORCA_RUNTIME_H_
