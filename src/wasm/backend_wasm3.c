/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "wasm.h"

#include "m3_compile.h"
#include "m3_env.h"
#include "wasm3.h"

typedef struct oc_wasm_binding_wasm3
{
    oc_wasm_binding info;
    u32 paramsStackOffset;
    oc_wasm* wasm;
} oc_wasm_binding_wasm3;

typedef struct oc_wasm_binding_elt_wasm3
{
    oc_list_elt listElt;
    oc_wasm_binding_wasm3 binding;
} oc_wasm_binding_elt_wasm3;

typedef struct oc_wasm
{
    oc_arena arena;
    oc_list bindings;

    IM3Environment m3Env;
    IM3Runtime m3Runtime;
    IM3Module m3Module;
} oc_wasm;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers

static char wa_value_type_to_wasm3_char(wa_value_type valtype)
{
    switch(valtype)
    {
        case WA_TYPE_I32:
            return 'i';
        case WA_TYPE_I64:
            return 'I';
        case WA_TYPE_F32:
            return 'f';
        case WA_TYPE_F64:
            return 'F';
        default:
            break;
    }

    OC_ASSERT(false, "unhandled case %d", valtype);

    return 'v';
}

static char wa_value_type_to_wasm3_valtype(wa_value_type valtype)
{
    switch(valtype)
    {
        case WA_TYPE_I32:
            return 'i';
        case WA_TYPE_I64:
            return 'I';
        case WA_TYPE_F32:
            return 'f';
        case WA_TYPE_F64:
            return 'F';
        default:
            break;
    }

    OC_ASSERT(false, "unhandled case %d", valtype);

    return 'v';
}

static wa_value_type wasm3_valuetype_to_valtype(M3ValueType m3Type)
{
    switch(m3Type)
    {
        case c_m3Type_i32:
            return WA_TYPE_I32;
        case c_m3Type_i64:
            return WA_TYPE_I64;
        case c_m3Type_f32:
            return WA_TYPE_F32;
        case c_m3Type_f64:
            return WA_TYPE_F64;

        case c_m3Type_none:
        case c_m3Type_unknown:
        default:
            break;
    }

    OC_ASSERT(false, "Unexpected wasm3 type here: %d", m3Type);

    return WA_TYPE_I32;
}

static wa_status oc_wasm3_result_to_status(M3Result result)
{
    if(result == NULL)
    {
        return WA_OK;
    }
    else if(result == m3Err_mallocFailed)
    {
        return WA_FAIL_MEMORY_ALLOC;
    }
    else if(result == m3Err_incompatibleWasmVersion
            || result == m3Err_wasmMalformed
            || result == m3Err_misorderedWasmSection
            || result == m3Err_wasmUnderrun
            || result == m3Err_wasmOverrun
            || result == m3Err_wasmMissingInitExpr
            || result == m3Err_lebOverflow
            || result == m3Err_missingUTF8
            || result == m3Err_wasmSectionUnderrun
            || result == m3Err_wasmSectionOverrun
            || result == m3Err_invalidTypeId
            || result == m3Err_tooManyMemorySections
            || result == m3Err_tooManyArgsRets)
    {
        return WA_PARSE_ERROR;
    }
    else if(result == m3Err_moduleNotLinked
            || result == m3Err_moduleAlreadyLinked
            || result == m3Err_functionLookupFailed
            || result == m3Err_functionImportMissing
            || result == m3Err_malformedFunctionSignature
            || result == m3Err_noCompiler
            || result == m3Err_unknownOpcode
            || result == m3Err_restrictedOpcode
            || result == m3Err_functionStackOverflow
            || result == m3Err_functionStackUnderrun
            || result == m3Err_mallocFailedCodePage
            || result == m3Err_settingImmutableGlobal
            || result == m3Err_typeMismatch
            || result == m3Err_typeCountMismatch)
    {
        return WA_FAIL_INSTANTIATE;
    }
    else if(result == m3Err_trapOutOfBoundsMemoryAccess)
    {
        return WA_TRAP_MEMORY_OUT_OF_BOUNDS;
    }
    else if(result == m3Err_trapDivisionByZero)
    {
        return WA_TRAP_DIVIDE_BY_ZERO;
    }
    else if(result == m3Err_trapIntegerOverflow)
    {
        return WA_TRAP_INTEGER_OVERFLOW;
    }
    else if(result == m3Err_trapIntegerConversion)
    {
        return WA_TRAP_INVALID_INTEGER_CONVERSION;
    }
    else if(result == m3Err_trapIndirectCallTypeMismatch)
    {
        return WA_TRAP_INDIRECT_CALL_TYPE_MISMATCH;
    }
    else if(result == m3Err_trapTableIndexOutOfRange)
    {
        return WA_TRAP_TABLE_OUT_OF_BOUNDS;
    }
    else if(result == m3Err_trapTableElementIsNull)
    {
        return WA_TRAP_REF_NULL;
    }
    else if(result == m3Err_trapExit)
    {
        return WA_TRAP_UNKNOWN;
    }
    else if(result == m3Err_trapAbort)
    {
        return WA_TRAP_UNKNOWN;
    }
    else if(result == m3Err_trapUnreachable)
    {
        return WA_TRAP_UNREACHABLE;
    }
    else if(result == m3Err_trapStackOverflow)
    {
        return WA_TRAP_STACK_OVERFLOW;
    }

    // NOTE: intentionally not handling these since they are wasm3-specific
    // missingCompiledCode
    // wasmMemoryOverflow
    // globalMemoryNotAllocated
    // globaIndexOutOfBounds
    // argumentCountMismatch
    // argumentTypeMismatch
    // globalLookupFailed
    // globalTypeMismatch
    // globalNotMutable

    return WA_FAIL_UNKNOWN;
}

static wa_status oc_wasm_handle_wasm3_result(oc_wasm* wasm, M3Result result, const char* msg)
{
    wa_status status = oc_wasm3_result_to_status(result);
    if(wa_status_is_fail(status))
    {
        M3ErrorInfo errInfo = { 0 };
        m3_GetErrorInfo(wasm->m3Runtime, &errInfo);
        if(errInfo.message && result == errInfo.result)
        {
            oc_log_error("%s Underlying wasm3 error: %s", msg, errInfo.message);
        }
        else
        {
            oc_log_error("%s Wasm3 error.", msg);
        }
    }

    return status;
}

static const void* oc_wasm_binding_wasm3_thunk(IM3Runtime runtime, IM3ImportContext ctx, uint64_t* stack, void* mem)
{
    const oc_wasm_binding_wasm3* binding = (oc_wasm_binding_wasm3*)ctx->userdata;

    void* params = ((char*)stack) + binding->paramsStackOffset;
    void* returns = stack;

    binding->info.proc((const i64*)params, (i64*)returns, mem, binding->wasm);
    return (0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Wasm3 implementation of interface functions

wa_status oc_wasm_decode(oc_wasm* wasm, oc_str8 wasmBlob)
{
    M3Result res = m3_ParseModule(wasm->m3Env, &wasm->m3Module, (u8*)wasmBlob.ptr, wasmBlob.len);
    return oc_wasm_handle_wasm3_result(wasm, res, "The application couldn't parse its web assembly module");
}

wa_status oc_wasm_add_binding(oc_wasm* wasm, oc_wasm_binding* binding)
{
    size_t allocSize = 0;
    allocSize += sizeof(oc_wasm_binding_elt_wasm3);
    allocSize += binding->countParams * sizeof(wa_value_type);
    allocSize += binding->countReturns * sizeof(wa_value_type);

    oc_wasm_binding_elt_wasm3* elt = (oc_wasm_binding_elt_wasm3*)oc_arena_push_aligned(&wasm->arena, allocSize, _Alignof(oc_wasm_binding_elt_wasm3));
    oc_list_push_front(&wasm->bindings, &elt->listElt);

    elt->binding.info.importName = oc_str8_push_copy(&wasm->arena, binding->importName);
    elt->binding.info.proc = binding->proc;
    elt->binding.info.countParams = binding->countParams;
    elt->binding.info.countReturns = binding->countReturns;
    elt->binding.info.params = (wa_value_type*)(((char*)elt) + sizeof(oc_wasm_binding_elt_wasm3));
    elt->binding.info.returns = elt->binding.info.params + elt->binding.info.countParams;
    memcpy(elt->binding.info.params, binding->params, binding->countParams * sizeof(wa_value_type));
    memcpy(elt->binding.info.returns, binding->returns, binding->countReturns * sizeof(wa_value_type));

    elt->binding.paramsStackOffset = sizeof(i64) * binding->countReturns;
    elt->binding.wasm = wasm;

    return WA_OK;
}

wa_status oc_wasm_instantiate(oc_wasm* wasm, oc_str8 moduleDebugName, oc_wasm_memory* memory)
{
    m3_RuntimeSetMemoryCallbacks(wasm->m3Runtime,
                                 oc_runtime_wasm_memory_resize_callback,
                                 oc_runtime_wasm_memory_free_callback,
                                 (void*)memory);

    {
        M3Result res = m3_LoadModule(wasm->m3Runtime, wasm->m3Module);
        if(res)
        {
            return oc_wasm_handle_wasm3_result(wasm, res, "The application couldn't load its web assembly module");
        }
        m3_SetModuleName(wasm->m3Module, moduleDebugName.ptr);
    }

    bool wasLinkSuccessful = true;
    oc_list_for(wasm->bindings, elt, oc_wasm_binding_elt_wasm3, listElt)
    {
        const oc_wasm_binding_wasm3* binding = &elt->binding;
        const size_t totalTypes = binding->info.countParams + binding->info.countReturns;

        char signature[128];
        {
            if((sizeof(signature) - 3) < totalTypes) // -3 accounts for null and open/close parens
            {
                oc_log_error("Couldn't link function %s (too many params+returns, max %d but need %d)\n", elt->binding.info.importName.ptr, (int)sizeof(signature), (int)totalTypes);
                wasLinkSuccessful = false;
                continue;
            }

            size_t signature_index = 0;
            signature[signature_index] = 'v'; // default to void in case there are no returns
            ++signature_index;
            for(size_t i = 0; i < binding->info.countReturns; ++i, ++signature_index)
            {
                char type = wa_value_type_to_wasm3_char(binding->info.returns[i]);
                signature[signature_index] = type;
            }

            signature[signature_index] = '(';
            ++signature_index;
            signature[signature_index] = 'v';

            for(size_t i = 0; i < binding->info.countParams; ++i, ++signature_index)
            {
                char type = wa_value_type_to_wasm3_char(binding->info.params[i]);
                signature[signature_index] = type;
            }

            signature[signature_index] = ')';
            ++signature_index;
            signature[signature_index] = '\0';
        }

        M3Result res = m3_LinkRawFunctionEx(wasm->m3Module, "*", binding->info.importName.ptr, signature, oc_wasm_binding_wasm3_thunk, binding);
        if(res != m3Err_none && res != m3Err_functionLookupFailed)
        {
            oc_wasm_handle_wasm3_result(wasm, res, "link failure");
            wasLinkSuccessful = false;
            continue;
        }
    }

    {
        M3Result res = m3_CompileModule(wasm->m3Module);
        if(res)
        {
            return oc_wasm_handle_wasm3_result(wasm, res, "The application couldn't compile its web assembly module");
        }
    }

    if(wasLinkSuccessful == false)
    {
        return WA_FAIL_INSTANTIATE;
    }

    return WA_OK;
}

u64 oc_wasm_mem_size(oc_wasm* wasm)
{
    return m3_GetMemorySize(wasm->m3Runtime);
}

oc_str8 oc_wasm_mem_get(oc_wasm* wasm)
{
    uint32_t memSize = 0;
    uint32_t memIndex = 0;
    u8* memory = (u8*)m3_GetMemory(wasm->m3Runtime, &memSize, memIndex);

    return (oc_str8){ .ptr = (char*)memory, .len = memSize };
}

wa_status oc_wasm_mem_resize(oc_wasm* wasm, u32 countPages)
{
    M3Result res = ResizeMemory(wasm->m3Runtime, countPages);
    if(res)
    {
        return oc_wasm_handle_wasm3_result(wasm, res, "Failed to resize wasm memory");
    }

    return WA_OK;
}

oc_wasm_function_handle* oc_wasm_function_find(oc_wasm* wasm, oc_str8 exportName)
{
    IM3Function m3Func = NULL;
    m3_FindFunction(&m3Func, wasm->m3Runtime, exportName.ptr);

    return (oc_wasm_function_handle*)m3Func;
}

wa_func_type oc_wasm_function_get_info(oc_arena* scratch, oc_wasm* wasm, oc_wasm_function_handle* handle)
{
    if(handle == NULL)
    {
        return (wa_func_type){ 0 };
    }

    IM3Function m3Func = (IM3Function)handle;

    u32 paramCount = m3_GetArgCount(m3Func);
    u32 returnCount = m3_GetRetCount(m3Func);
    u32 totalTypes = paramCount + returnCount;

    wa_value_type* types = (totalTypes > 0) ? oc_arena_push_array(scratch, wa_value_type, totalTypes) : NULL;

    wa_func_type info;
    info.paramCount = paramCount;
    info.returnCount = returnCount;
    info.params = types;
    info.returns = types + info.paramCount;

    for(u32 i = 0; i < info.paramCount; ++i)
    {
        M3ValueType m3Type = m3_GetArgType(m3Func, i);
        info.params[i] = wasm3_valuetype_to_valtype(m3Type);
    }

    for(u32 i = 0; i < info.returnCount; ++i)
    {
        M3ValueType m3Type = m3_GetRetType(m3Func, i);
        info.returns[i] = wasm3_valuetype_to_valtype(m3Type);
    }

    return info;
}

wa_status oc_wasm_function_call(oc_wasm* wasm, oc_wasm_function_handle* handle, wa_value* params, size_t paramCount, wa_value* returns, size_t returnCount)
{
    if(handle == NULL)
    {
        return WA_OK;
    }

    IM3Function m3Func = (IM3Function)handle;

    const void* valuePtrs[128];
    OC_ASSERT(paramCount < oc_array_size(valuePtrs), "Need more static storage for params");

    for(size_t i = 0; i < paramCount; ++i)
    {
        valuePtrs[i] = &params[i];
    }

    M3Result res = m3_Call(m3Func, paramCount, valuePtrs);
    if(res)
    {
        return oc_wasm_handle_wasm3_result(wasm, res, "Function call failed");
    }

    if(returnCount > 0)
    {
        OC_ASSERT(returnCount < oc_array_size(valuePtrs), "Need more static storage for returns");

        for(size_t i = 0; i < returnCount; ++i)
        {
            valuePtrs[i] = &returns[i];
        }

        res = m3_GetResults(m3Func, (uint32_t)returnCount, valuePtrs);
        if(res)
        {
            return oc_wasm_handle_wasm3_result(wasm, res, "Failed to get results from function call");
        }
    }

    return WA_OK;
}

oc_wasm_global_handle* oc_wasm_global_find(oc_wasm* wasm, oc_str8 exportName, wa_value_type expectedType)
{
    IM3Global m3Global = m3_FindGlobal(wasm->m3Module, exportName.ptr);
    M3ValueType m3Type = wa_value_type_to_wasm3_valtype(expectedType);
    if(m3Global && m3Global->type == m3Type)
    {
        oc_log_error("Found global %s, expected type %s but actual type was %s", exportName.ptr, wa_value_type_str8(expectedType).ptr, c_waTypes[m3Global->type]);
        return NULL;
    }

    return (oc_wasm_global_handle*)m3Global;
}

wa_value oc_wasm_global_get_value(oc_wasm_global_handle* global)
{
    if(global == NULL)
    {
        return (wa_value){ 0 };
    }

    IM3Global m3Global = (IM3Global)global;
    wa_value v;
    v.valI64 = m3Global->intValue; // doesn't really matter what the actual value is since we just need the bits
    return v;
}

void oc_wasm_global_set_value(oc_wasm_global_handle* global, wa_value value)
{
    if(global)
    {
        IM3Global m3Global = (IM3Global)global;
        m3Global->intValue = value.valI64;
    }
}

oc_wasm_global_pointer oc_wasm_global_pointer_find(oc_wasm* wasm, oc_str8 exportName)
{
    oc_wasm_global_handle* global = oc_wasm_global_find(wasm, exportName, WA_TYPE_I32);
    IM3Global m3Global = (IM3Global)global;

    return (oc_wasm_global_pointer){ .handle = global, .address = (u32)m3Global->intValue };
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// implementation creation / destruction

void oc_wasm_destroy(oc_wasm* wasm)
{
    oc_arena arena = wasm->arena;
    oc_arena_cleanup(&arena);
}

oc_wasm* oc_wasm_create(void)
{
    oc_arena arena;
    oc_arena_init(&arena);

    oc_wasm* wasm = oc_arena_push_type(&arena, oc_wasm);
    memset(wasm, 0, sizeof(oc_wasm));

    wasm->arena = arena;
    oc_list_init(&wasm->bindings);

    u32 stackSize = 1 << 20;
    wasm->m3Env = m3_NewEnvironment();
    wasm->m3Runtime = m3_NewRuntime(wasm->m3Env, stackSize, NULL);

    return wasm;
}
