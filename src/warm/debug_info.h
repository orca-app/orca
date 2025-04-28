/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "wasm/wasm.h"
#include "warm.h"

//------------------------------------------------------------------------
// line info structs
//------------------------------------------------------------------------

typedef struct wa_wasm_to_line_entry
{
    u64 wasmOffset;
    wa_line_loc loc;
} wa_wasm_to_line_entry;

typedef struct wa_bytecode_mapping
{
    oc_list_elt listElt;

    u32 funcIndex;
    u32 codeIndex;
    wa_instr* instr;

} wa_bytecode_mapping;

//------------------------------------------------------------------------
// wasm stack slot to register mappings
//------------------------------------------------------------------------

typedef struct wa_register_range
{
    u64 start;
    u64 end;
    wa_value_type type;
} wa_register_range;

typedef struct wa_register_map
{
    u32 count;
    wa_register_range* ranges;
} wa_register_map;

//------------------------------------------------------------------------
// Debug type info
//------------------------------------------------------------------------

typedef enum wa_debug_type_kind
{
    WA_DEBUG_TYPE_VOID,
    WA_DEBUG_TYPE_BASIC,
    WA_DEBUG_TYPE_POINTER,
    WA_DEBUG_TYPE_STRUCT,
    WA_DEBUG_TYPE_UNION,
    WA_DEBUG_TYPE_ENUM,
    WA_DEBUG_TYPE_ARRAY,
    WA_DEBUG_TYPE_NAMED,
    //...
} wa_debug_type_kind;

typedef enum wa_debug_type_encoding
{
    WA_DEBUG_TYPE_SIGNED,
    WA_DEBUG_TYPE_UNSIGNED,
    WA_DEBUG_TYPE_FLOAT,
    WA_DEBUG_TYPE_BOOL,
} wa_debug_type_encoding;

typedef struct wa_debug_type wa_debug_type;

typedef struct wa_debug_type_field
{
    oc_list_elt listElt;
    oc_str8 name;
    wa_debug_type* type;
    u64 offset;
} wa_debug_type_field;

typedef struct wa_debug_type
{
    oc_list_elt listElt;
    u64 dwarfRef;
    oc_str8 name;
    wa_debug_type_kind kind;
    u64 size;

    union
    {
        wa_debug_type_encoding encoding;
        wa_debug_type* type;
        oc_list fields;

        struct
        {
            wa_debug_type* type;
            u64 count;
        } array;

        //TODO enum, etc...
    };
} wa_debug_type;

//------------------------------------------------------------------------
// Variables debug info
//------------------------------------------------------------------------

typedef struct dw_info dw_info;
typedef struct dw_loc dw_loc;

typedef struct wa_debug_variable
{
    oc_str8 name;
    dw_loc* loc;
    wa_debug_type* type;
} wa_debug_variable;

typedef struct wa_debug_function
{
    dw_loc* frameBase;
    u32 count;
    wa_debug_variable* vars;
} wa_debug_function;

//------------------------------------------------------------------------
// Debug info struct
//------------------------------------------------------------------------

typedef struct wa_debug_info
{
    u32 warmToWasmMapLen;
    oc_list* warmToWasmMap;

    u32 wasmToWarmMapLen;
    oc_list* wasmToWarmMap;

    dw_info* dwarf;

    wa_source_info sourceInfo;

    u64 wasmToLineCount;
    wa_wasm_to_line_entry* wasmToLine;

    wa_register_map** registerMaps;

    wa_debug_function* functionLocals;

} wa_debug_info;

//------------------------------------------------------------------------
// helpers
//------------------------------------------------------------------------
wa_debug_type* wa_debug_type_strip(wa_debug_type* t);

//------------------------------------------------------------------------
// building debug info
//------------------------------------------------------------------------
void wa_parse_dwarf(wa_module* module, oc_str8 contents);
void wa_import_debug_locals(wa_module* module);
void wa_warm_to_wasm_loc_push(wa_module* module, u32 funcIndex, u32 codeIndex, wa_instr* instr);
void wa_wasm_to_warm_loc_push(wa_module* module, u32 funcIndex, u32 codeIndex, wa_instr* instr);

//------------------------------------------------------------------------
// using debug info
//------------------------------------------------------------------------
wa_line_loc wa_line_loc_from_warm_loc(wa_module* module, wa_warm_loc loc);
wa_warm_loc wa_warm_loc_from_line_loc(wa_module* module, wa_line_loc loc);
oc_str8 wa_debug_variable_get_value(oc_arena* arena, wa_interpreter* interpreter, wa_debug_function* funcInfo, wa_debug_variable* var);
