/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#ifndef __WASM_H_
#define __WASM_H_

// When more backends are added, make sure to check for them before automatically defaulting to wasm3
#ifndef OC_WASM_BACKEND_WASM3
    #define OC_WASM_BACKEND_WASM3 1
#endif

typedef enum oc_wasm_status
{
    OC_WASM_STATUS_SUCCESS,
    OC_WASM_STATUS_FAIL_UNKNOWN,
    OC_WASM_STATUS_FAIL_MEMALLOC,
    OC_WASM_STATUS_FAIL_DECODE,
    OC_WASM_STATUS_FAIL_INSTANTIATE,
    OC_WASM_STATUS_FAIL_TRAP_OUTOFBOUNDSMEMORYACCESS,
    OC_WASM_STATUS_FAIL_TRAP_DIVISIONBYZERO,
    OC_WASM_STATUS_FAIL_TRAP_INTEGEROVERFLOW,
    OC_WASM_STATUS_FAIL_TRAP_INTEGERCONVERSION,
    OC_WASM_STATUS_FAIL_TRAP_INDIRECTCALLTYPEMISMATCH,
    OC_WASM_STATUS_FAIL_TRAP_TABLEINDEXOUTOFRANGE,
    OC_WASM_STATUS_FAIL_TRAP_TABLEELEMENTISNULL,
    OC_WASM_STATUS_FAIL_TRAP_EXIT,
    OC_WASM_STATUS_FAIL_TRAP_ABORT,
    OC_WASM_STATUS_FAIL_TRAP_UNREACHABLE,
    OC_WASM_STATUS_FAIL_TRAP_STACKOVERFLOW,
} oc_wasm_status;

typedef enum oc_wasm_valtype
{
    OC_WASM_VALTYPE_I32,
    OC_WASM_VALTYPE_I64,
    OC_WASM_VALTYPE_F32,
    OC_WASM_VALTYPE_F64,
} oc_wasm_valtype;

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
    oc_wasm_valtype type;
} oc_wasm_val_tagged;

struct oc_wasm;
typedef struct oc_wasm oc_wasm;

typedef void (*oc_wasm_host_proc)(const i64* restrict params, i64* restrict returns, u8* memory, oc_wasm* wasm);

typedef struct oc_wasm_binding
{
    oc_str8 importName;
    oc_wasm_host_proc proc;
    oc_wasm_valtype* params;
    oc_wasm_valtype* returns;
    u32 countParams;
    u32 countReturns;
} oc_wasm_binding;

struct oc_wasm_function_handle;
typedef struct oc_wasm_function_handle oc_wasm_function_handle;

typedef struct oc_wasm_function_info
{
    oc_wasm_valtype* params;
    oc_wasm_valtype* returns;
    u32 countParams;
    u32 countReturns;
} oc_wasm_function_info;

enum
{
    OC_WASM_MEM_PAGE_SIZE = 1024 * 64, // 64KB
};

typedef void* (*oc_wasm_mem_resize_proc)(void* p, unsigned long newSize, void* userdata);
typedef void (*oc_wasm_memory_free_proc)(void* p, void* userdata);

typedef struct oc_wasm_mem_callbacks
{
    oc_wasm_mem_resize_proc resizeProc;
    oc_wasm_memory_free_proc freeProc;
    void* userdata;
} oc_wasm_mem_callbacks;

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

ORCA_API bool oc_wasm_status_is_fail(oc_wasm_status status);
ORCA_API oc_str8 oc_wasm_status_str8(oc_wasm_status status);

ORCA_API size_t oc_wasm_valtype_size(oc_wasm_valtype valtype);
ORCA_API oc_str8 oc_wasm_valtype_str8(oc_wasm_valtype valtype);

ORCA_API oc_wasm* oc_wasm_create(void);
ORCA_API void oc_wasm_destroy(oc_wasm* wasm);

ORCA_API oc_wasm_status oc_wasm_decode(oc_wasm* wasm, oc_str8 wasmBlob);
ORCA_API oc_wasm_status oc_wasm_add_binding(oc_wasm* wasm, oc_wasm_binding* binding);
ORCA_API oc_wasm_status oc_wasm_instantiate(oc_wasm* wasm, oc_str8 moduleDebugName, oc_wasm_mem_callbacks memCallbacks);

ORCA_API u64 oc_wasm_mem_size(oc_wasm* wasm);
ORCA_API oc_str8 oc_wasm_mem_get(oc_wasm* wasm);
ORCA_API oc_wasm_status oc_wasm_mem_resize(oc_wasm* wasm, u32 countPages);

ORCA_API oc_wasm_function_handle* oc_wasm_function_find(oc_wasm* wasm, oc_str8 exportName);
ORCA_API oc_wasm_function_info oc_wasm_function_get_info(oc_arena* scratch, oc_wasm* wasm, oc_wasm_function_handle* handle);
ORCA_API oc_wasm_status oc_wasm_function_call(oc_wasm* wasm, oc_wasm_function_handle* handle, oc_wasm_val* params, size_t countParams, oc_wasm_val* returns, size_t countReturns);

ORCA_API oc_wasm_global_handle* oc_wasm_global_find(oc_wasm* wasm, oc_str8 exportName, oc_wasm_valtype expectedType);
ORCA_API oc_wasm_val oc_wasm_global_get_value(oc_wasm_global_handle* global);
ORCA_API void oc_wasm_global_set_value(oc_wasm_global_handle* global, oc_wasm_val value);
ORCA_API oc_wasm_global_pointer oc_wasm_global_pointer_find(oc_wasm* wasm, oc_str8 exportName);

//////////////////////////////////////////////////////////////////
// Inline implementation

inline bool oc_wasm_status_is_fail(oc_wasm_status status)
{
    return status != OC_WASM_STATUS_SUCCESS;
}

#endif // __WASM_H_
