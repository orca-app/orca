/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "util/macros.h"
#include "platform/platform_io_internal.h"
#include "runtime_memory.h"
#include "runtime_clipboard.h"
#include "warm/wasm.h"

//------------------------------------------------------------------------
// Debugger / overlay structs
//------------------------------------------------------------------------

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
    oc_canvas_renderer renderer;
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

typedef struct wa_type wa_type;

typedef struct oc_debugger_value
{
    oc_list_elt listElt;
    oc_list children;

    oc_str8 name;
    wa_type* type;
    oc_str8 data;

    bool expanded;

} oc_debugger_value;

typedef struct wa_source_node wa_source_node;

typedef struct wa_source_node
{
    oc_list_elt listElt;
    oc_list children;
    wa_source_node* parent;

    u64 id;
    u64 index;
    oc_str8 name; // slice into a wa_source_file path

    bool expanded;
    oc_str8 contents;
} wa_source_node;

typedef enum oc_debugger_code_tab_mode
{
    OC_DEBUGGER_CODE_TAB_SOURCE,
    OC_DEBUGGER_CODE_TAB_ASSEMBLY,
} oc_debugger_code_tab_mode;

typedef struct oc_debugger_code_tab
{
    oc_list_elt listElt;
    oc_debugger_code_tab_mode mode;
    wa_source_node* selectedFile;
    i64 selectedFunction;

    //TODO: cleanup scroll stuff
    bool freshScroll;
    bool autoScroll;

    u64 selectedLine;
    u64 selectedIndex;

    f32 scrollSpeed;
    f32 lastScroll;

} oc_debugger_code_tab;

typedef struct oc_debugger
{
    bool init;
    oc_window window;
    oc_canvas_renderer renderer;
    oc_surface surface;
    oc_canvas_context canvas;
    oc_ui_context* ui;

    wa_source_node sourceTree;
    i64 selectedFrame;
    bool browseSymbols;
    bool showRegisters;

    oc_arena tabsArena;
    oc_list tabsFreeList;
    oc_list tabs;
    oc_debugger_code_tab* selectedTab;

    oc_arena debugArena;
    oc_list* locals;
    oc_list globals;

} oc_debugger;

//------------------------------------------------------------------------
// Wasm environment
//------------------------------------------------------------------------

// Note oc_on_test() is a special handler only called for --test modules

#define OC_EXPORTS(X)                                                                                     \
    X(OC_EXPORT_ON_TEST, "oc_on_test", (WA_TYPE_I32), ())                                                 \
    X(OC_EXPORT_ON_INIT, "oc_on_init", (), ())                                                            \
    X(OC_EXPORT_MOUSE_DOWN, "oc_on_mouse_down", (WA_TYPE_I32), ())                                        \
    X(OC_EXPORT_MOUSE_UP, "oc_on_mouse_up", (WA_TYPE_I32), ())                                            \
    X(OC_EXPORT_MOUSE_ENTER, "oc_on_mouse_enter", (), ())                                                 \
    X(OC_EXPORT_MOUSE_LEAVE, "oc_on_mouse_leave", (), ())                                                 \
    X(OC_EXPORT_MOUSE_MOVE, "oc_on_mouse_move", (WA_TYPE_F32, WA_TYPE_F32, WA_TYPE_F32, WA_TYPE_F32), ()) \
    X(OC_EXPORT_MOUSE_WHEEL, "oc_on_mouse_wheel", (WA_TYPE_F32, WA_TYPE_F32), ())                         \
    X(OC_EXPORT_KEY_DOWN, "oc_on_key_down", (WA_TYPE_F32, WA_TYPE_F32), ())                               \
    X(OC_EXPORT_KEY_UP, "oc_on_key_up", (WA_TYPE_F32, WA_TYPE_F32), ())                                   \
    X(OC_EXPORT_FRAME_REFRESH, "oc_on_frame_refresh", (), ())                                             \
    X(OC_EXPORT_FRAME_RESIZE, "oc_on_resize", (WA_TYPE_I32, WA_TYPE_I32), ())                             \
    X(OC_EXPORT_RAW_EVENT, "oc_on_raw_event", (WA_TYPE_F32), ())                                          \
    X(OC_EXPORT_TERMINATE, "oc_on_terminate", (), ())                                                     \
    X(OC_EXPORT_ARENA_PUSH, "oc_arena_push_stub", (WA_TYPE_I32, WA_TYPE_I64), (WA_TYPE_I32))

typedef enum oc_export_kind
{

#define OC_EXPORT_KIND(kind, ...) kind,
    OC_EXPORTS(OC_EXPORT_KIND)
        OC_EXPORT_COUNT
#undef OC_EXPORT_KIND
} oc_export_kind;

enum
{
    OC_EXPORT_DESC_MAX_PARAMS = 6,
    OC_EXPORT_DESC_MAX_RETURNS = 1,
};

typedef struct oc_export_desc
{
    oc_str8 name;
    u32 paramCount;
    wa_value_type params[OC_EXPORT_DESC_MAX_PARAMS];
    u32 returnCount;
    wa_value_type returns[OC_EXPORT_DESC_MAX_RETURNS];
} oc_export_desc;

const oc_export_desc OC_EXPORT_DESC[] = {
#define OC_EXPORT_DESC_ENTRY(kind, n, p, r) \
    {                                       \
        .name = OC_STR8_LIT(n),             \
        .paramCount = OC_COUNT10 p,         \
        .params = { OC_EXPAND p },          \
        .returnCount = OC_COUNT10 r,        \
        .returns = { OC_EXPAND r },         \
    },

    OC_EXPORTS(OC_EXPORT_DESC_ENTRY)

#undef OC_EXPORT_DESC_ENTRY
};

typedef enum oc_debugger_command
{
    OC_DEBUGGER_CONTINUE,
    OC_DEBUGGER_INSTR_STEP_OVER,
    OC_DEBUGGER_LINE_STEP_OVER,
    OC_DEBUGGER_INSTR_STEP_IN,
    OC_DEBUGGER_LINE_STEP_IN,
    OC_DEBUGGER_STEP_OUT,
    OC_DEBUGGER_QUIT,
} oc_debugger_command;

typedef struct oc_wasm_env
{
    oc_str8 wasmBytecode;

    oc_arena arena;
    wa_module* module;
    wa_instance* instance;
    wa_interpreter* interpreter;

    wa_func* exports[OC_EXPORT_COUNT];
    u32 rawEventOffset;

    oc_condition* suspendCond;
    oc_mutex* suspendMutex;
    _Atomic(bool) paused;
    bool prevPaused;

    oc_debugger_command debuggerCommand;

} oc_wasm_env;

//------------------------------------------------------------------------
// Orca runtime
//------------------------------------------------------------------------

typedef struct oc_runtime
{
    bool quit;
    oc_window window;
    oc_canvas_renderer canvasRenderer;

    oc_debugger debugger;
    oc_debug_overlay debugOverlay;

    oc_file_table fileTable;
    oc_file rootDir;

    oc_wasm_env env;

    oc_runtime_clipboard clipboard;

    oc_ringbuffer eventBuffer;
} oc_runtime;

extern oc_runtime __orcaApp;

oc_runtime* oc_runtime_get(void);
oc_wasm_env* oc_runtime_get_env(void);
oc_str8 oc_runtime_get_wasm_memory(void);

void oc_abort_ext_dialog(const char* file, const char* function, int line, const char* fmt, ...);
void oc_assert_fail_dialog(const char* file, const char* function, int line, const char* test, const char* fmt, ...);

#define _OC_ASSERT_DIALOG_(test, fmt, ...) \
    ((test) || (oc_assert_fail_dialog(__FILE__, __FUNCTION__, __LINE__, #test, fmt, ##__VA_ARGS__), 0))
#define OC_ASSERT_DIALOG(test, ...) \
    _OC_ASSERT_DIALOG_(test, OC_VA_NOPT("", ##__VA_ARGS__) OC_ARG1(__VA_ARGS__) OC_VA_COMMA_TAIL(__VA_ARGS__))

#define OC_WASM_TRAP(status)                                                                                     \
    do                                                                                                           \
    {                                                                                                            \
        if(wa_status_is_fail(status))                                                                            \
        {                                                                                                        \
            oc_abort_ext_dialog(__FILE__, __FUNCTION__, __LINE__, "%.*s", oc_str8_ip(wa_status_string(status))); \
        }                                                                                                        \
    }                                                                                                            \
    while(0)
