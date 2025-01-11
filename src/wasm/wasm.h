/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#ifndef __WASM_H_
#define __WASM_H_

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

typedef union oc_wasm_val
{
    i32 I32;
    i64 I64;
    f32 F32;
    f64 F64;
} oc_wasm_val;

typedef struct oc_wasm_val_tagged
{
    oc_wasm_val value;
    wa_value_type type;
} oc_wasm_val_tagged;

struct oc_wasm;
typedef struct oc_wasm oc_wasm;

typedef void (*oc_wasm_host_proc)(const i64* restrict params, i64* restrict returns, u8* memory, oc_wasm* wasm);

typedef struct oc_wasm_binding
{
    oc_str8 importName;
    oc_wasm_host_proc proc;
    wa_value_type* params;
    wa_value_type* returns;
    u32 countParams;
    u32 countReturns;
} oc_wasm_binding;

struct oc_wasm_function_handle;
typedef struct oc_wasm_function_handle oc_wasm_function_handle;

typedef struct oc_wasm_function_info
{
    wa_value_type* params;
    wa_value_type* returns;
    u32 countParams;
    u32 countReturns;
} oc_wasm_function_info;

enum
{
    OC_WASM_MEM_PAGE_SIZE = 1024 * 64, // 64KB
};

typedef struct oc_wasm_memory
{
    char* ptr;
    u64 reserved;
    u64 committed;

} oc_wasm_memory;

typedef u32 oc_wasm_addr;
typedef u32 oc_wasm_size;

struct oc_wasm_global_handle;
typedef struct oc_wasm_global_handle oc_wasm_global_handle;

// Convenience for wasm globals that are known to be pointers to static memory
typedef struct oc_wasm_global_pointer
{
    oc_wasm_global_handle* handle;
    oc_wasm_addr address;
} oc_wasm_global_pointer;

typedef struct oc_wasm oc_wasm;

bool wa_status_is_fail(wa_status status);
oc_str8 wa_status_str8(wa_status status);

oc_wasm* oc_wasm_create(void);
void oc_wasm_destroy(oc_wasm* wasm);

wa_status oc_wasm_decode(oc_wasm* wasm, oc_str8 wasmBlob);
wa_status oc_wasm_add_binding(oc_wasm* wasm, oc_wasm_binding* binding);
wa_status oc_wasm_instantiate(oc_wasm* wasm, oc_str8 moduleDebugName, oc_wasm_memory* memory);

u64 oc_wasm_mem_size(oc_wasm* wasm);
oc_str8 oc_wasm_mem_get(oc_wasm* wasm);
wa_status oc_wasm_mem_resize(oc_wasm* wasm, u32 countPages);

oc_wasm_function_handle* oc_wasm_function_find(oc_wasm* wasm, oc_str8 exportName);
oc_wasm_function_info oc_wasm_function_get_info(oc_arena* scratch, oc_wasm* wasm, oc_wasm_function_handle* handle);
wa_status oc_wasm_function_call(oc_wasm* wasm, oc_wasm_function_handle* handle, oc_wasm_val* params, size_t countParams, oc_wasm_val* returns, size_t countReturns);

oc_wasm_global_handle* oc_wasm_global_find(oc_wasm* wasm, oc_str8 exportName, wa_value_type expectedType);
oc_wasm_val oc_wasm_global_get_value(oc_wasm_global_handle* global);
void oc_wasm_global_set_value(oc_wasm_global_handle* global, oc_wasm_val value);
oc_wasm_global_pointer oc_wasm_global_pointer_find(oc_wasm* wasm, oc_str8 exportName);

oc_str8 wa_value_type_str8(wa_value_type type);

//////////////////////////////////////////////////////////////////
// Inline implementation

inline bool wa_status_is_fail(wa_status status)
{
    return status != WA_OK;
}

#endif // __WASM_H_
