/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "platform/platform_io_internal.h"
#include "runtime_memory.h"
#include "runtime_clipboard.h"
#include "wasm/wasm.h"

// Note oc_on_test() is a special handler only called for --test modules
#define OC_EXPORTS(X)                                         \
    X(OC_EXPORT_ON_TEST, "oc_on_test", "i", "")               \
    X(OC_EXPORT_ON_INIT, "oc_on_init", "", "")                \
    X(OC_EXPORT_MOUSE_DOWN, "oc_on_mouse_down", "", "i")      \
    X(OC_EXPORT_MOUSE_UP, "oc_on_mouse_up", "", "i")          \
    X(OC_EXPORT_MOUSE_ENTER, "oc_on_mouse_enter", "", "")     \
    X(OC_EXPORT_MOUSE_LEAVE, "oc_on_mouse_leave", "", "")     \
    X(OC_EXPORT_MOUSE_MOVE, "oc_on_mouse_move", "", "ffff")   \
    X(OC_EXPORT_MOUSE_WHEEL, "oc_on_mouse_wheel", "", "ff")   \
    X(OC_EXPORT_KEY_DOWN, "oc_on_key_down", "", "ii")         \
    X(OC_EXPORT_KEY_UP, "oc_on_key_up", "", "ii")             \
    X(OC_EXPORT_FRAME_REFRESH, "oc_on_frame_refresh", "", "") \
    X(OC_EXPORT_FRAME_RESIZE, "oc_on_resize", "", "ii")       \
    X(OC_EXPORT_RAW_EVENT, "oc_on_raw_event", "", "i")        \
    X(OC_EXPORT_TERMINATE, "oc_on_terminate", "", "")         \
    X(OC_EXPORT_ARENA_PUSH, "oc_arena_push_stub", "i", "iI")

typedef enum
{

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
#define OC_EXPORT_DESC_ENTRY(kind, name, rets, args) { OC_STR8_LIT(name), OC_STR8_LIT(rets), OC_STR8_LIT(args) },

    OC_EXPORTS(OC_EXPORT_DESC_ENTRY)

#undef OC_EXPORT_DESC_ENTRY
#undef OC_STR8_LIT
};

typedef struct oc_wasm_memory
{
    char* ptr;
    u64 reserved;
    u64 committed;

} oc_wasm_memory;

typedef struct oc_wasm_env
{
    oc_str8 wasmBytecode;
    oc_wasm_memory wasmMemory;

    oc_wasm* wasm;
    oc_wasm_function_handle* exports[OC_EXPORT_COUNT];
    u32 rawEventOffset;
} oc_wasm_env;

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
    oc_canvas_context context;

    oc_font fontReg;
    oc_font fontBold;
    oc_ui_context* ui;

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
    bool quit;
    oc_window window;
    oc_canvas_renderer canvasRenderer;
    oc_debug_overlay debugOverlay;

    oc_file_table fileTable;
    oc_file rootDir;

    oc_wasm_env env;

    oc_runtime_clipboard clipboard;
} oc_runtime;

oc_runtime* oc_runtime_get(void);
oc_wasm_env* oc_runtime_get_env(void);
oc_str8 oc_runtime_get_wasm_memory(void);

void oc_abort_ext_dialog(const char* file, const char* function, int line, const char* fmt, ...);
void oc_assert_fail_dialog(const char* file, const char* function, int line, const char* test, const char* fmt, ...);

#define _OC_ASSERT_DIALOG_(test, fmt, ...) \
    ((test) || (oc_assert_fail_dialog(__FILE__, __FUNCTION__, __LINE__, #test, fmt, ##__VA_ARGS__), 0))
#define OC_ASSERT_DIALOG(test, ...) \
    _OC_ASSERT_DIALOG_(test, OC_VA_NOPT("", ##__VA_ARGS__) OC_ARG1(__VA_ARGS__) OC_VA_COMMA_TAIL(__VA_ARGS__))

#define OC_WASM_TRAP(status)                                                                                        \
    do                                                                                                              \
    {                                                                                                               \
        if(oc_wasm_status_is_fail(status))                                                                          \
        {                                                                                                           \
            oc_abort_ext_dialog(__FILE__, __FUNCTION__, __LINE__, "%.*s", oc_str8_ip(oc_wasm_status_str8(status))); \
        }                                                                                                           \
    }                                                                                                               \
    while(0)
