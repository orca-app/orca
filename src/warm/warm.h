/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#define OC_NO_APP_LAYER 1
#include "orca.h"
#include "wasm/wasm.h"

#include "instructions.h"

//------------------------------------------------------------------------
// wasm module structs
//------------------------------------------------------------------------
typedef struct wa_module_loc
{
    u64 start;
    u64 len;
} wa_module_loc;

typedef union wa_code
{
    u64 valU64;
    i64 valI64;
    i32 valU32;
    i32 valI32;

    f32 valF32;
    f64 valF64;

    wa_instr_op opcode;

    u32 index;
    wa_value_type valueType;

    struct
    {
        u32 align;
        u32 offset;
    } memArg;

    u8 laneIndex;

} wa_code;

typedef struct wa_instr wa_instr;

typedef struct wa_instr
{
    oc_list_elt listElt;

    wa_instr_op op;
    u32 immCount;
    wa_code* imm;

    wa_func_type* blockType;
    wa_instr* elseBranch;
    wa_instr* end;

    wa_module_loc loc;
    u32 codeIndex;
} wa_instr;

typedef struct wa_import wa_import;
typedef struct wa_instance wa_instance;

typedef struct wa_func
{
    wa_func_type* type;

    u32 localCount;
    wa_value_type* locals;

    oc_list instructions;

    u32 codeLen;
    wa_code* code;

    wa_import* import;
    wa_host_proc proc;
    void* user;

    wa_instance* extInstance;
    u32 extIndex;

    u32 maxRegCount;
} wa_func;

typedef struct wa_global_desc
{
    wa_value_type type;
    bool mut;
    oc_list init;

    u32 codeLen;
    wa_code* code;
} wa_global_desc;

typedef struct wa_table_type
{
    wa_value_type type;
    wa_limits limits;
} wa_table_type;

typedef enum wa_element_mode
{
    WA_ELEMENT_PASSIVE,
    WA_ELEMENT_ACTIVE,
    WA_ELEMENT_DECLARATIVE,
} wa_element_mode;

typedef struct wa_element
{
    wa_value_type type;
    wa_element_mode mode;
    u32 tableIndex;
    oc_list tableOffset;
    u64 initCount;
    oc_list* initInstr;

    wa_code* tableOffsetCode;
    wa_code** code;

    wa_value* refs;
} wa_element;

typedef enum wa_data_mode
{
    WA_DATA_PASSIVE,
    WA_DATA_ACTIVE,
} wa_data_mode;

typedef struct wa_data_segment
{
    wa_data_mode mode;
    u32 memoryIndex;
    oc_list memoryOffset;
    wa_code* memoryOffsetCode;
    oc_str8 init;

} wa_data_segment;

typedef enum wa_import_kind // this should be kept equal with export kinds
{
    WA_IMPORT_FUNCTION = 0x00,
    WA_IMPORT_TABLE = 0x01,
    WA_IMPORT_MEMORY = 0x02,
    WA_IMPORT_GLOBAL = 0x03,
} wa_import_kind;

typedef struct wa_import
{
    oc_str8 moduleName;
    oc_str8 importName;

    wa_import_kind kind;
    u32 index;
    wa_value_type type;
    wa_limits limits;
    bool mut;

} wa_import;

typedef enum wa_export_kind // this should be kept equal with import kinds
{
    WA_EXPORT_FUNCTION = 0x00,
    WA_EXPORT_TABLE = 0x01,
    WA_EXPORT_MEMORY = 0x02,
    WA_EXPORT_GLOBAL = 0x03,
} wa_export_kind;

typedef struct wa_export
{
    oc_str8 name;
    wa_export_kind kind;
    u32 index;

} wa_export;

typedef struct wa_section
{
    oc_list_elt listElt;
    u64 id;
    u64 len;
    u64 offset;
    oc_str8 name;

} wa_section;

typedef struct wa_name_entry
{
    u32 index;
    oc_str8 name;
} wa_name_entry;

typedef struct wa_module_toc
{
    wa_section types;
    wa_section imports;
    wa_section functions;
    wa_section tables;
    wa_section memory;
    wa_section globals;
    wa_section exports;
    wa_section start;
    wa_section elements;
    wa_section dataCount;
    wa_section code;
    wa_section data;

    wa_section names;
    oc_list customSections;
} wa_module_toc;

typedef struct wa_module_error
{
    oc_list_elt moduleElt;
    oc_list_elt astElt;

    bool blockEnd;

    wa_status status;
    oc_str8 string;

} wa_module_error;

typedef struct wa_debug_info wa_debug_info;

typedef struct wa_module
{
    oc_arena* arena;
    oc_list errors;

    wa_module_toc toc;

    u32 functionNameCount;
    wa_name_entry* functionNames;

    u32 typeCount;
    wa_func_type* types;

    u32 importCount;
    wa_import* imports;

    u32 functionImportCount;
    u32 functionCount;
    wa_func* functions;

    bool hasStart;
    u32 startIndex;

    u32 globalImportCount;
    u32 globalCount;
    wa_global_desc* globals;

    u32 tableImportCount;
    u32 tableCount;
    wa_table_type* tables;

    u32 elementCount;
    wa_element* elements;

    u32 exportCount;
    wa_export* exports;

    u32 memoryImportCount;
    u32 memoryCount;
    wa_limits* memories;

    u32 dataCount;
    wa_data_segment* data;

    wa_debug_info* debugInfo;

} wa_module;

enum
{
    //TODO put these in compiler, except max reg
    WA_TYPE_STACK_MAX_LEN = 512,
    WA_CONTROL_STACK_MAX_LEN = 512,
    WA_MAX_REG = 4096,
    WA_MAX_SLOT_COUNT = 4096,
};

oc_str8 wa_module_get_function_name(wa_module* module, u32 index);

extern const wa_func_type WA_BLOCK_VALUE_TYPES[];
bool wa_is_value_type(u64 t);
bool wa_is_value_type_numeric(u64 t);

//------------------------------------------------------------------------
// Instance
//------------------------------------------------------------------------
typedef struct wa_typed_value
{
    wa_value_type type;
    wa_value value;

} wa_typed_value;

typedef struct wa_global
{
    wa_value_type type;
    bool mut;
    wa_value value;
} wa_global;

typedef struct wa_instance
{
    wa_status status;

    oc_arena* arena;
    wa_module* module;

    wa_func* functions;
    wa_global** globals;
    wa_table** tables;
    wa_memory** memories;

    wa_data_segment* data;
    wa_element* elements;
} wa_instance;

//------------------------------------------------------------------------
// Interpreter
//------------------------------------------------------------------------

typedef struct wa_control
{
    wa_instance* instance;
    wa_func* func;
    wa_code* returnPC;
    wa_value* returnFrame;
} wa_control;

enum
{
    WA_CONTROL_STACK_SIZE = 256,
    WA_LOCALS_BUFFER_SIZE = WA_MAX_SLOT_COUNT * 256,
};

typedef struct wa_interpreter
{
    wa_instance* instance;
    wa_code* code;

    u32 argCount;
    wa_value* args;
    u32 retCount;
    wa_value* returns;

    wa_control controlStack[WA_CONTROL_STACK_SIZE];
    u32 controlStackTop;

    wa_value* localsBuffer;
    wa_value* locals;
    wa_code* pc;

    _Atomic(bool) suspend;
    bool terminated;

    oc_arena arena;
    oc_list breakpoints;
    oc_list breakpointFreeList;

    wa_code cachedRegs[WA_MAX_REG];

} wa_interpreter;

wa_breakpoint* wa_interpreter_find_breakpoint_any(wa_interpreter* interpreter, wa_warm_loc* loc);
wa_breakpoint* wa_interpreter_find_breakpoint(wa_interpreter* interpreter, wa_warm_loc* loc);
wa_breakpoint* wa_interpreter_add_breakpoint(wa_interpreter* interpreter, wa_warm_loc* loc);
wa_breakpoint* wa_interpreter_find_breakpoint_line(wa_interpreter* interpreter, wa_line_loc* loc);

wa_breakpoint* wa_interpreter_add_breakpoint_line(wa_interpreter* interpreter, wa_line_loc* loc);
void wa_interpreter_remove_breakpoint(wa_interpreter* interpreter, wa_breakpoint* bp);
wa_instr_op wa_breakpoint_saved_opcode(wa_breakpoint* bp);
void wa_interpreter_cache_registers(wa_interpreter* interpreter);
wa_status wa_interpreter_continue(wa_interpreter* interpreter);
wa_status wa_interpreter_step(wa_interpreter* interpreter);
wa_status wa_interpreter_step_line(wa_interpreter* interpreter);
void wa_interpreter_suspend(wa_interpreter* interpreter);

/*
enum
{
    WA_TYPE_STACK_MAX_LEN = 512,
    WA_CONTROL_STACK_MAX_LEN = 512,
    WA_MAX_REG = 4096,
    WA_MAX_SLOT_COUNT = 4096,
};




typedef struct wa_import wa_import;
typedef struct wa_module wa_module;
typedef struct wa_instance wa_instance;
typedef struct wa_func wa_func;



bool wa_module_has_errors(wa_module* module);
void wa_module_print_errors(wa_module* module);

wa_import_package wa_instance_exports(oc_arena* arena, wa_instance* instance, oc_str8 name);

//wip...
wa_status wa_instance_status(wa_instance* instance);
*/
