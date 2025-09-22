/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "wasm.h"
#include "warm.h"

//------------------------------------------------------------------------
// source info structs
//------------------------------------------------------------------------

typedef struct wa_source_file
{
    oc_str8 rootPath; // slice into fullPath
    oc_str8 fullPath;
    // timestamp, hash, etc
} wa_source_file;

typedef struct wa_source_info
{
    u64 fileCount;
    wa_source_file* files;

} wa_source_info;

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

typedef enum wa_type_kind
{
    WA_TYPE_NIL = 0,
    WA_TYPE_VOID,
    WA_TYPE_BASIC,
    WA_TYPE_POINTER,
    WA_TYPE_STRUCT,
    WA_TYPE_UNION,
    WA_TYPE_ENUM,
    WA_TYPE_ARRAY,
    WA_TYPE_NAMED,
    //...
} wa_type_kind;

typedef enum wa_type_encoding
{
    WA_TYPE_UNKNOWN_ENCODING,
    WA_TYPE_SIGNED,
    WA_TYPE_UNSIGNED,
    WA_TYPE_FLOAT,
    WA_TYPE_BOOL,
} wa_type_encoding;

typedef struct wa_type wa_type;

typedef struct wa_type_field
{
    oc_list_elt listElt;
    oc_str8 name;
    wa_type* type;
    u64 offset;
} wa_type_field;

typedef struct wa_type_enumerator
{
    oc_list_elt listElt;
    oc_str8 name;
    //TODO value
} wa_type_enumerator;

//NOTE: once built, type members are guaranteed to point to a valid (possibly 'unknown') type.
typedef struct wa_type
{
    oc_str8 name;
    wa_type_kind kind;
    u64 size;

    union
    {
        wa_type_encoding encoding;
        wa_type* type;
        oc_list fields;

        struct
        {
            wa_type* type;
            u64 count;
        } array;

        struct
        {
            wa_type* type;
            oc_list enumerators;
        } enumType;

        //TODO enum, etc...
    };
} wa_type;

//------------------------------------------------------------------------
// Variables debug info
//------------------------------------------------------------------------

typedef struct dw_info dw_info;
typedef struct dw_loc dw_loc;
typedef oc_ptr_option(dw_loc) dw_loc_ptr_option;

typedef struct wa_debug_variable
{
    oc_str8 name;
    u64 uid;
    dw_loc_ptr_option loc;
    wa_type* type;
} wa_debug_variable;

typedef struct wa_debug_range
{
    u64 low;
    u64 high;
} wa_debug_range;

typedef struct wa_debug_range_list
{
    u64 count;
    wa_debug_range* ranges;
} wa_debug_range_list;

typedef struct wa_debug_unit
{
    u64 globalCount;
    wa_debug_variable* globals;

    wa_debug_range_list rangeList;
} wa_debug_unit;

typedef struct wa_debug_scope wa_debug_scope;

typedef struct wa_debug_scope
{
    oc_list_elt listElt;
    oc_list children;
    wa_debug_scope* parent;

    wa_debug_range_list rangeList;

    u64 varCount;
    wa_debug_variable* vars;
} wa_debug_scope;

typedef oc_ptr_option(dw_loc) dw_loc_option;

typedef struct wa_debug_function
{
    wa_debug_unit* unit;
    dw_loc_option frameBase;

    u64 totalVarDecl;
    wa_debug_scope body;

} wa_debug_function;

typedef oc_ptr_option(wa_debug_function) wa_debug_function_ptr_option;

//------------------------------------------------------------------------
// Debug info struct
//------------------------------------------------------------------------

typedef struct wa_debug_info
{
    wa_source_info sourceInfo;

    u64 wasmToLineCount;
    wa_wasm_to_line_entry* wasmToLine;

    u32 warmToWasmMapLen;
    oc_list* warmToWasmMap;

    u32 wasmToWarmMapLen;
    oc_list* wasmToWarmMap;

    wa_register_map** registerMaps;

    u64 unitCount;
    wa_debug_unit* units;

    u64 functionCount;
    wa_debug_function_ptr_option* functions;
} wa_debug_info;

//------------------------------------------------------------------------
// helpers
//------------------------------------------------------------------------
wa_type* wa_type_strip(wa_type* t);

//------------------------------------------------------------------------
// building debug info
//------------------------------------------------------------------------
void wa_import_dwarf(wa_module* module, oc_str8 contents);
void wa_import_debug_locals(wa_module* module, dw_info* dwarf);
void wa_warm_to_wasm_loc_push(wa_module* module, u32 funcIndex, u32 codeIndex, wa_instr* instr);
void wa_wasm_to_warm_loc_push(wa_module* module, u32 funcIndex, u32 codeIndex, wa_instr* instr);

//------------------------------------------------------------------------
// using debug info
//------------------------------------------------------------------------
wa_line_loc wa_line_loc_from_warm_loc(wa_module* module, wa_warm_loc loc);
wa_warm_loc wa_warm_loc_from_line_loc(wa_module* module, wa_line_loc loc);
oc_str8 wa_debug_variable_get_value(oc_arena* arena, wa_interpreter* interpreter, wa_call_frame* frame, wa_debug_function* funcInfo, wa_debug_variable* var);

typedef oc_ptr_option(wa_debug_scope) wa_debug_scope_ptr_option;
wa_debug_scope_ptr_option wa_debug_get_scope_for_warm_loc(wa_interpreter* interpreter, wa_warm_loc warmLoc);

typedef oc_ptr_option(wa_debug_unit) wa_debug_unit_ptr_option;
wa_debug_unit_ptr_option wa_debug_get_unit_for_warm_loc(wa_interpreter* interpreter, wa_warm_loc warmLoc);
