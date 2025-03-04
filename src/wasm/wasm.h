/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#ifndef OC_WASM_BACKEND_WASM3
    #define OC_WASM_BACKEND_WASM3 0
#endif

#ifndef OC_WASM_BACKEND_BYTEBOX
    #define OC_WASM_BACKEND_BYTEBOX 0
#endif

#ifndef OC_WASM_BACKEND_WARM
    #define OC_WASM_BACKEND_WARM 0
#endif

#define WA_STATUS(_)                                                              \
    _(WA_OK, "success")                                                           \
    /* parse/validation errors */                                                 \
    _(WA_PARSE_ERROR, "parsing error")                                            \
    _(WA_VALIDATION_TYPE_MISMATCH, "validation error: type mismatch")             \
    _(WA_VALIDATION_INVALID_TYPE, "validation error: invalid type index")         \
    _(WA_VALIDATION_INVALID_FUNCTION, "validation error: invalid function index") \
    _(WA_VALIDATION_INVALID_GLOBAL, "validation error: invalid global index")     \
    _(WA_VALIDATION_INVALID_LOCAL, "validation error: invalid local index")       \
    _(WA_VALIDATION_INVALID_TABLE, "validation error: invalid table index")       \
    _(WA_VALIDATION_INVALID_MEMORY, "validation error: invalid memory index")     \
    /* instantiation / invocation errors*/                                        \
    _(WA_FAIL_INVALID_ARGS, "invocation error: invalid arguments")                \
    _(WA_FAIL_MISSING_IMPORT, "instantiation error: missing import")              \
    _(WA_FAIL_IMPORT_TYPE_MISMATCH, "instantiation error: import type mismatch")  \
    _(WA_FAIL_MEMORY_ALLOC, "runtime error: memory allocation failed")            \
    _(WA_FAIL_INSTANTIATE, "instantiation error")                                 \
    /* runtime traps */                                                           \
    _(WA_TRAP_UNREACHABLE, "trap: unreachable")                                   \
    _(WA_TRAP_INVALID_OP, "trap: invalid opcode")                                 \
    _(WA_TRAP_DIVIDE_BY_ZERO, "trap: divide by zero")                             \
    _(WA_TRAP_INTEGER_OVERFLOW, "trap: integer overflow")                         \
    _(WA_TRAP_INVALID_INTEGER_CONVERSION, "trap: invalid integer conversion")     \
    _(WA_TRAP_STACK_OVERFLOW, "trap: stack overflow")                             \
    _(WA_TRAP_MEMORY_OUT_OF_BOUNDS, "trap: out of bounds memory access")          \
    _(WA_TRAP_TABLE_OUT_OF_BOUNDS, "trap: out of bounds table access")            \
    _(WA_TRAP_REF_NULL, "trap: ref null")                                         \
    _(WA_TRAP_INDIRECT_CALL_TYPE_MISMATCH, "trap: indirect call type mismatch")   \
    _(WA_TRAP_UNKNOWN, "trap: unknown")                                           \
    /* debug traps */                                                             \
    _(WA_TRAP_BREAKPOINT, "debug trap: breakpoint")                               \
    _(WA_TRAP_STEP, "debug trap: step")                                           \
    _(WA_TRAP_SUSPENDED, "debug trap: suspended")                                 \
    _(WA_TRAP_TERMINATED, "debug trap: terminated")                               \
    /* unknown*/                                                                  \
    _(WA_FAIL_UNKNOWN, "unknown error")

typedef enum wa_status
{

#define X(n, s) n,
    WA_STATUS(X)
#undef X
        WA_STATUS_COUNT,
} wa_status;

typedef enum wa_value_type
{
    WA_TYPE_I32 = 0x7f,
    WA_TYPE_I64 = 0x7e,
    WA_TYPE_F32 = 0x7d,
    WA_TYPE_F64 = 0x7c,
    WA_TYPE_V128 = 0x7b,
    WA_TYPE_FUNC_REF = 0x70,
    WA_TYPE_EXTERN_REF = 0x6f,

    WA_TYPE_UNKNOWN = 0x100,
    WA_TYPE_ANY = 0x101,
    WA_TYPE_REF = 0x102,
    WA_TYPE_NUM_OR_VEC = 0x103,
} wa_value_type;

typedef struct wa_module wa_module;
typedef struct wa_instance wa_instance;
typedef struct wa_interpreter wa_interpreter;
typedef struct wa_func wa_func;
typedef struct wa_global wa_global;

typedef union wa_value
{
    i32 valI32;
    i64 valI64;
    f32 valF32;
    f64 valF64;

    //TODO v128, funcref, externref...
    struct
    {
        wa_instance* refInstance;
        u32 refIndex;
    };
} wa_value;

typedef struct wa_func_type
{
    u32 paramCount;
    wa_value_type* params;

    u32 returnCount;
    wa_value_type* returns;

} wa_func_type;

typedef enum wa_limits_kind
{
    WA_LIMIT_MIN = 0,
    WA_LIMIT_MIN_MAX = 1,
} wa_limits_kind;

typedef struct wa_limits
{
    wa_limits_kind kind;
    u32 min;
    u32 max;

} wa_limits;

typedef struct wa_memory
{
    wa_limits limits;
    char* ptr;
} wa_memory;

enum
{
    WA_PAGE_SIZE = 64 * 1 << 10,
};

typedef struct wa_table
{
    wa_value_type type;
    wa_limits limits;
    wa_value* contents;
} wa_table;

typedef enum wa_binding_kind
{
    WA_BINDING_WASM_GLOBAL,
    WA_BINDING_WASM_FUNCTION,
    WA_BINDING_WASM_MEMORY,
    WA_BINDING_WASM_TABLE,

    WA_BINDING_HOST_GLOBAL,
    WA_BINDING_HOST_FUNCTION,
    WA_BINDING_HOST_MEMORY,
    WA_BINDING_HOST_TABLE,

} wa_binding_kind;

typedef void (*wa_host_proc)(wa_interpreter* interpreter, wa_value* args, wa_value* returns, void* user); //TODO: complete with memory, return status / etc

typedef struct wa_host_function
{
    wa_func_type type;
    wa_host_proc proc;
    void* userData;
} wa_host_function;

typedef struct wa_import_binding
{
    oc_str8 name;
    wa_binding_kind kind;
    wa_instance* instance;

    union
    {
        u32 wasmGlobal;
        u32 wasmMemory;
        u32 wasmTable;
        u32 wasmFunction;

        wa_global* hostGlobal;
        wa_memory* hostMemory;
        wa_table* hostTable;
        wa_host_function hostFunction;
    };
} wa_import_binding;

typedef struct wa_import_package_elt
{
    oc_list_elt listElt;
    wa_import_binding binding;
} wa_import_package_elt;

typedef struct wa_import_package
{
    oc_str8 name;
    u32 bindingCount;
    oc_list bindings;
} wa_import_package;

typedef struct wa_instance_options
{
    u32 packageCount;
    wa_import_package* importPackages;

    //...
} wa_instance_options;

void wa_import_package_push_binding(oc_arena* arena, wa_import_package* package, wa_import_binding* binding);

enum
{
    OC_WASM_MEM_PAGE_SIZE = 1024 * 64, // 64KB
};

typedef u32 oc_wasm_addr;
typedef u32 oc_wasm_size;

bool wa_status_is_fail(wa_status status);
oc_str8 wa_status_string(wa_status status);
oc_str8 wa_value_type_string(wa_value_type type);

wa_module* wa_module_create(oc_arena* arena, oc_str8 contents);
void wa_module_destroy(wa_module* module);
wa_status wa_module_status(wa_module* module);

wa_instance* wa_instance_create(oc_arena* arena, wa_module* module, wa_instance_options* options);
void wa_instance_destroy(wa_instance* instance);
wa_status wa_instance_status(wa_instance* instance);

wa_func* wa_instance_find_function(wa_instance* instance, oc_str8 name);
wa_func_type wa_func_get_type(oc_arena* arena, wa_instance* instance, wa_func* func);

wa_global* wa_instance_find_global(wa_instance* instance, oc_str8 name);
wa_value wa_global_get(wa_instance* instance, wa_global* global);
void wa_global_set(wa_instance* instance, wa_global* global, wa_value value);

wa_memory wa_instance_get_memory(wa_instance* instance);
oc_str8 wa_instance_get_memory_str8(wa_instance* instance);
wa_status wa_instance_resize_memory(wa_instance* instance, u32 countPages);

wa_interpreter* wa_interpreter_create(oc_arena* arena);
void wa_interpreter_destroy(wa_interpreter* interpreter);

wa_status wa_interpreter_invoke(wa_interpreter* interpreter,
                                wa_instance* instance,
                                wa_func* function,
                                u32 argCount,
                                wa_value* args,
                                u32 retCount,
                                wa_value* returns);

wa_status wa_interpreter_continue(wa_interpreter* interpreter);
wa_status wa_interpreter_step(wa_interpreter* interpreter);

wa_instance* wa_interpreter_current_instance(wa_interpreter* interpreter);

//////////////////////////////////////////////////////////////////
// Inline implementation

inline bool wa_status_is_fail(wa_status status)
{
    return status != WA_OK;
}

/////////////////////////////////
// Debugger

typedef struct wa_breakpoint wa_breakpoint;

typedef struct wa_bytecode_loc
{
    wa_instance* instance;
    wa_func* func;
    u32 index;
} wa_bytecode_loc;

wa_breakpoint* wa_interpreter_find_breakpoint(wa_interpreter* interpreter, wa_bytecode_loc* loc);
wa_breakpoint* wa_interpreter_add_breakpoint(wa_interpreter* interpreter, wa_bytecode_loc* loc);
void wa_interpreter_remove_breakpoint(wa_interpreter* interpreter, wa_breakpoint* bp);

typedef struct wa_source_node
{
    oc_list_elt listElt;
    oc_list children;
    oc_str8 name;
    oc_str8 path;

    oc_str8 contents;
} wa_source_node;

wa_source_node* wa_module_get_source_tree(wa_module* module);
