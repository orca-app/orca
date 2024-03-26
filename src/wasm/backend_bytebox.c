/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "wasm.h"

#if OC_WASM_BACKEND_BYTEBOX

// clang-format off
#include "bytebox.h"

// clang-format on

typedef struct oc_wasm_binding_bytebox
{
    oc_wasm_host_proc proc;
    oc_wasm* wasm;
} oc_wasm_binding_bytebox;

typedef struct oc_wasm_binding_elt_bytebox
{
    oc_list_elt listElt;
    oc_wasm_binding_bytebox binding;
} oc_wasm_binding_elt_bytebox;

typedef struct oc_wasm
{
    oc_arena arena;
    oc_list bindings;
    oc_wasm_mem_callbacks memCallbacks;

    bb_import_package* imports;
    bb_module_definition* definition;
    bb_module_instance* instance;
} oc_wasm;

typedef union
{
    bb_func_handle bb;
    oc_wasm_function_handle* oc;
} oc_wasm_bytebox_func_handle_convert;

static_assert(sizeof(bb_func_handle) == sizeof(oc_wasm_function_handle*));

///////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers

// static char oc_wasm_valtype_to_bytebox_char(oc_wasm_valtype valtype)
// {
//     switch(valtype)
//     {
//         case OC_WASM_VALTYPE_I32:
//             return 'i';
//         case OC_WASM_VALTYPE_I64:
//             return 'I';
//         case OC_WASM_VALTYPE_F32:
//             return 'f';
//         case OC_WASM_VALTYPE_F64:
//             return 'F';
//     }

//     OC_ASSERT(false, "unhandled case %d", valtype);

//     return 'v';
// }

static bb_valtype oc_wasm_valtype_to_bytebox_valtype(oc_wasm_valtype valtype)
{
    switch(valtype)
    {
        case OC_WASM_VALTYPE_I32:
            return BB_VALTYPE_I32;
        case OC_WASM_VALTYPE_I64:
            return BB_VALTYPE_I64;
        case OC_WASM_VALTYPE_F32:
            return BB_VALTYPE_F32;
        case OC_WASM_VALTYPE_F64:
            return BB_VALTYPE_F64;
    }

    OC_ASSERT(false, "unhandled case %d", valtype);

    return BB_VALTYPE_I32;
}

static oc_wasm_valtype bytebox_valtype_to_oc_valtype(bb_valtype bbType)
{
    switch(bbType)
    {
        case BB_VALTYPE_I32:
            return OC_WASM_VALTYPE_I32;
        case BB_VALTYPE_I64:
            return OC_WASM_VALTYPE_I64;
        case BB_VALTYPE_F32:
            return OC_WASM_VALTYPE_F32;
        case BB_VALTYPE_F64:
            return OC_WASM_VALTYPE_F64;
        default:
            break;
    }

    OC_ASSERT(false, "Unexpected bytebox type here: %d", bbType);

    return OC_WASM_VALTYPE_I32;
}

static oc_wasm_val bytebox_val_to_oc_val(bb_val val)
{
    // TODO handle v128
    oc_wasm_val v;
    v.I64 = val.i64_val;
    return v;
}

static bb_val oc_wasm_val_to_bytebox_val(oc_wasm_val val)
{
    // TODO handle v128
    bb_val v;
    v.i64_val = val.I64;
    return v;
}

// static oc_wasm_status oc_bytebox_result_to_status(M3Result result)
// {
//     if(result == NULL)
//     {
//         return OC_WASM_STATUS_SUCCESS;
//     }
//     else if(result == m3Err_mallocFailed)
//     {
//         return OC_WASM_STATUS_FAIL_MEMALLOC;
//     }
//     else if(result == m3Err_incompatibleWasmVersion
//             || result == m3Err_wasmMalformed
//             || result == m3Err_misorderedWasmSection
//             || result == m3Err_wasmUnderrun
//             || result == m3Err_wasmOverrun
//             || result == m3Err_wasmMissingInitExpr
//             || result == m3Err_lebOverflow
//             || result == m3Err_missingUTF8
//             || result == m3Err_wasmSectionUnderrun
//             || result == m3Err_wasmSectionOverrun
//             || result == m3Err_invalidTypeId
//             || result == m3Err_tooManyMemorySections
//             || result == m3Err_tooManyArgsRets)
//     {
//         return OC_WASM_STATUS_FAIL_DECODE;
//     }
//     else if(result == m3Err_moduleNotLinked
//             || result == m3Err_moduleAlreadyLinked
//             || result == m3Err_functionLookupFailed
//             || result == m3Err_functionImportMissing
//             || result == m3Err_malformedFunctionSignature
//             || result == m3Err_noCompiler
//             || result == m3Err_unknownOpcode
//             || result == m3Err_restrictedOpcode
//             || result == m3Err_functionStackOverflow
//             || result == m3Err_functionStackUnderrun
//             || result == m3Err_mallocFailedCodePage
//             || result == m3Err_settingImmutableGlobal
//             || result == m3Err_typeMismatch
//             || result == m3Err_typeCountMismatch)
//     {
//         return OC_WASM_STATUS_FAIL_INSTANTIATE;
//     }
//     else if(result == m3Err_trapOutOfBoundsMemoryAccess)
//     {
//         return OC_WASM_STATUS_FAIL_TRAP_OUTOFBOUNDSMEMORYACCESS;
//     }
//     else if(result == m3Err_trapDivisionByZero)
//     {
//         return OC_WASM_STATUS_FAIL_TRAP_DIVISIONBYZERO;
//     }
//     else if(result == m3Err_trapIntegerOverflow)
//     {
//         return OC_WASM_STATUS_FAIL_TRAP_INTEGEROVERFLOW;
//     }
//     else if(result == m3Err_trapIntegerConversion)
//     {
//         return OC_WASM_STATUS_FAIL_TRAP_INTEGERCONVERSION;
//     }
//     else if(result == m3Err_trapIndirectCallTypeMismatch)
//     {
//         return OC_WASM_STATUS_FAIL_TRAP_INDIRECTCALLTYPEMISMATCH;
//     }
//     else if(result == m3Err_trapTableIndexOutOfRange)
//     {
//         return OC_WASM_STATUS_FAIL_TRAP_TABLEINDEXOUTOFRANGE;
//     }
//     else if(result == m3Err_trapTableElementIsNull)
//     {
//         return OC_WASM_STATUS_FAIL_TRAP_TABLEELEMENTISNULL;
//     }
//     else if(result == m3Err_trapExit)
//     {
//         return OC_WASM_STATUS_FAIL_TRAP_EXIT;
//     }
//     else if(result == m3Err_trapAbort)
//     {
//         return OC_WASM_STATUS_FAIL_TRAP_ABORT;
//     }
//     else if(result == m3Err_trapUnreachable)
//     {
//         return OC_WASM_STATUS_FAIL_TRAP_UNREACHABLE;
//     }
//     else if(result == m3Err_trapStackOverflow)
//     {
//         return OC_WASM_STATUS_FAIL_TRAP_STACKOVERFLOW;
//     }

//     // NOTE: intentionally not handling these since they are wasm3-specific
//     // missingCompiledCode
//     // wasmMemoryOverflow
//     // globalMemoryNotAllocated
//     // globaIndexOutOfBounds
//     // argumentCountMismatch
//     // argumentTypeMismatch
//     // globalLookupFailed
//     // globalTypeMismatch
//     // globalNotMutable

//     return OC_WASM_STATUS_FAIL_UNKNOWN;
// }

// static oc_wasm_status oc_wasm_handle_bytebox_result(oc_wasm* wasm, M3Result result, const char* msg)
// {
//     oc_wasm_status status = oc_bytebox_result_to_status(result);
//     if(oc_wasm_status_is_fail(status))
//     {
//         M3ErrorInfo errInfo = { 0 };
//         m3_GetErrorInfo(wasm->m3Runtime, &errInfo);
//         if(errInfo.message && result == errInfo.result)
//         {
//             oc_log_error("%s Underlying wasm3 error: %s", msg, errInfo.message);
//         }
//         else
//         {
//             oc_log_error("%s Wasm3 error.", msg);
//         }
//     }

//     return status;
// }

static const void oc_wasm_binding_bytebox_thunk(void* userdata, bb_module_instance* module, const bb_val* params, bb_val* returns)
{
    bb_slice mem = bb_module_instance_mem_all(module);

    oc_wasm_binding_bytebox* binding = userdata;

    binding->proc((const i64*)params, (i64*)returns, mem.data, binding->wasm);
}

static void* oc_wasm_mem_resize_bytebox(void* mem, size_t new_size_bytes, size_t old_size_bytes, void* userdata)
{
    oc_wasm_mem_callbacks* memCallbacks = (oc_wasm_mem_callbacks*)userdata;
    return memCallbacks->resizeProc(mem, new_size_bytes, memCallbacks->userdata);
}

static void oc_wasm_mem_free_bytebox(void* mem, size_t size_bytes, void* userdata)
{
    oc_wasm_mem_callbacks* memCallbacks = (oc_wasm_mem_callbacks*)userdata;
    memCallbacks->freeProc(mem, memCallbacks->userdata);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Wasm3 implementation of interface functions

oc_wasm_status oc_wasm_decode(oc_wasm* wasm, oc_str8 wasmBlob)
{
    bb_module_definition_init_opts opts = { 0 };
    wasm->definition = bb_module_definition_create(opts);
    bb_error err = bb_module_definition_decode(wasm->definition, wasmBlob.ptr, wasmBlob.len);
    if(err != BB_ERROR_OK)
    {
        oc_log_error("caught error decoding module: %s", bb_error_str(err));
        return OC_WASM_STATUS_FAIL_DECODE;
    }
    return OC_WASM_STATUS_SUCCESS;
}

oc_wasm_status oc_wasm_add_binding(oc_wasm* wasm, oc_wasm_binding* binding)
{
    if(wasm->imports == NULL)
    {
        wasm->imports = bb_import_package_init("orca");
    }

    oc_wasm_binding_elt_bytebox* elt = oc_arena_push_type(&wasm->arena, oc_wasm_binding_elt_bytebox);
    oc_list_push(&wasm->bindings, &elt->listElt);

    elt->binding.proc = binding->proc;
    elt->binding.wasm = wasm;

    bb_valtype param_types[64];
    OC_ASSERT(binding->countParams <= oc_array_size(param_types));
    for(int i = 0; i < binding->countParams; ++i)
    {
        param_types[i] = oc_wasm_valtype_to_bytebox_valtype(binding->params[i]);
    }

    bb_valtype return_types[64];
    OC_ASSERT(binding->countReturns <= oc_array_size(return_types));
    for(int i = 0; i < binding->countReturns; ++i)
    {
        return_types[i] = oc_wasm_valtype_to_bytebox_valtype(binding->returns[i]);
    }

    bb_error err = bb_import_package_add_function(wasm->imports,
                                                  &oc_wasm_binding_bytebox_thunk,
                                                  binding->importName.ptr,
                                                  param_types,
                                                  binding->countParams,
                                                  return_types,
                                                  binding->countReturns,
                                                  &elt->binding);
    if(err != BB_ERROR_OK)
    {
        oc_log_error("caught error adding function binding: %s", bb_error_str(err));
        return OC_WASM_STATUS_FAIL_UNKNOWN;
    }

    return OC_WASM_STATUS_SUCCESS;

    // size_t allocSize = 0;
    // allocSize += sizeof(oc_wasm_binding_elt_bytebox);
    // allocSize += binding->countParams * sizeof(oc_wasm_valtype);
    // allocSize += binding->countReturns * sizeof(oc_wasm_valtype);

    // oc_wasm_binding_elt_bytebox* elt = (oc_wasm_binding_elt_bytebox*)oc_arena_push_aligned(&wasm->arena, allocSize, _Alignof(oc_wasm_binding_elt_bytebox));
    // oc_list_push(&wasm->bindings, &elt->listElt);

    // elt->binding.info.importName = oc_str8_push_copy(&wasm->arena, binding->importName);
    // elt->binding.info.proc = binding->proc;
    // elt->binding.info.countParams = binding->countParams;
    // elt->binding.info.countReturns = binding->countReturns;
    // elt->binding.info.params = (oc_wasm_valtype*)(((char*)elt) + sizeof(oc_wasm_binding_elt_bytebox));
    // elt->binding.info.returns = elt->binding.info.params + elt->binding.info.countParams;
    // memcpy(elt->binding.info.params, binding->params, binding->countParams * sizeof(oc_wasm_valtype));
    // memcpy(elt->binding.info.returns, binding->returns, binding->countReturns * sizeof(oc_wasm_valtype));

    // elt->binding.paramsStackOffset = sizeof(i64) * binding->countReturns;
    // elt->binding.wasm = wasm;

    // return OC_WASM_STATUS_SUCCESS;
}

// TODO move moduleDebugName to oc_wasm_decode()
oc_wasm_status oc_wasm_instantiate(oc_wasm* wasm, oc_str8 moduleDebugName, oc_wasm_mem_callbacks memCallbacks)
{
    OC_ASSERT(memCallbacks.resizeProc);
    OC_ASSERT(memCallbacks.freeProc);

    wasm->instance = bb_module_instance_create(wasm->definition);
    wasm->memCallbacks = memCallbacks;

    bb_wasm_memory_config memconfig = {
        .resize_callback = oc_wasm_mem_resize_bytebox,
        .free_callback = oc_wasm_mem_free_bytebox,
        .userdata = &wasm->memCallbacks,
    };

    bb_module_instance_instantiate_opts opts = {
        .packages = &wasm->imports,
        .num_packages = 1,
        .wasm_memory_config = memconfig,
        .stack_size = 65536,
        .enable_debug = false,
    };

    bb_error err = bb_module_instance_instantiate(wasm->instance, opts);
    if(err != BB_ERROR_OK)
    {
        oc_log_error("caught error instaniating module: %s", bb_error_str(err));
        return OC_WASM_STATUS_FAIL_INSTANTIATE;
    }

    return OC_WASM_STATUS_SUCCESS;

    // m3_RuntimeSetMemoryCallbacks(wasm->m3Runtime, memCallbacks.resizeProc, memCallbacks.freeProc, memCallbacks.userdata);

    // {
    //     M3Result res = m3_LoadModule(wasm->m3Runtime, wasm->m3Module);
    //     if(res)
    //     {
    //         return oc_wasm_handle_bytebox_result(wasm, res, "The application couldn't load its web assembly module");
    //     }
    //     m3_SetModuleName(wasm->m3Module, moduleDebugName.ptr);
    // }

    // bool wasLinkSuccessful = true;
    // oc_list_for(wasm->bindings, elt, oc_wasm_binding_elt_bytebox, listElt)
    // {
    //     const oc_wasm_binding_bytebox* binding = &elt->binding;
    //     const size_t totalTypes = binding->info.countParams + binding->info.countReturns;

    //     char signature[128];
    //     {
    //         if((sizeof(signature) - 3) < totalTypes) // -3 accounts for null and open/close parens
    //         {
    //             oc_log_error("Couldn't link function %s (too many params+returns, max %d but need %d)\n", elt->binding.info.importName.ptr, (int)sizeof(signature), (int)totalTypes);
    //             wasLinkSuccessful = false;
    //             continue;
    //         }

    //         size_t signature_index = 0;
    //         signature[signature_index] = 'v'; // default to void in case there are no returns
    //         ++signature_index;
    //         for(size_t i = 0; i < binding->info.countReturns; ++i, ++signature_index)
    //         {
    //             char type = oc_wasm_valtype_to_bytebox_char(binding->info.returns[i]);
    //             signature[signature_index] = type;
    //         }

    //         signature[signature_index] = '(';
    //         ++signature_index;
    //         signature[signature_index] = 'v';

    //         for(size_t i = 0; i < binding->info.countParams; ++i, ++signature_index)
    //         {
    //             char type = oc_wasm_valtype_to_bytebox_char(binding->info.params[i]);
    //             signature[signature_index] = type;
    //         }

    //         signature[signature_index] = ')';
    //         ++signature_index;
    //         signature[signature_index] = '\0';
    //     }

    //     M3Result res = m3_LinkRawFunctionEx(wasm->m3Module, "*", binding->info.importName.ptr, signature, oc_wasm_binding_bytebox_thunk, binding);
    //     if(res != m3Err_none && res != m3Err_functionLookupFailed)
    //     {
    //         oc_wasm_handle_bytebox_result(wasm, res, "link failure");
    //         wasLinkSuccessful = false;
    //         continue;
    //     }
    // }

    // {
    //     M3Result res = m3_CompileModule(wasm->m3Module);
    //     if(res)
    //     {
    //         return oc_wasm_handle_bytebox_result(wasm, res, "The application couldn't compile its web assembly module");
    //     }
    // }

    // if(wasLinkSuccessful == false)
    // {
    //     return OC_WASM_STATUS_FAIL_INSTANTIATE;
    // }

    // return OC_WASM_STATUS_SUCCESS;
}

u64 oc_wasm_mem_size(oc_wasm* wasm)
{
    bb_slice mem = bb_module_instance_mem_all(wasm->instance);
    return mem.length;
}

oc_str8 oc_wasm_mem_get(oc_wasm* wasm)
{
    bb_slice mem = bb_module_instance_mem_all(wasm->instance);
    return (oc_str8){ .ptr = mem.data, .len = mem.length };
}

oc_wasm_status oc_wasm_mem_resize(oc_wasm* wasm, u32 countPages)
{
    bb_error err = bb_module_instance_mem_grow(wasm->instance, countPages);
    if(err != BB_ERROR_OK)
    {
        oc_log_error("caught error resizing wasm memory: %s", bb_error_str(err));
        return OC_WASM_STATUS_FAIL_UNKNOWN; // TODO rename this to OC_WASM_STATUS_FAIL
    }

    return OC_WASM_STATUS_SUCCESS;
}

oc_wasm_function_handle* oc_wasm_function_find(oc_wasm* wasm, oc_str8 exportName)
{
    bb_func_handle handle;
    bb_error err = bb_module_instance_find_func(wasm->instance, exportName.ptr, &handle);
    if(err != BB_ERROR_OK)
    {
        oc_log_error("caught error resizing wasm memory: %s", bb_error_str(err));
        return NULL;
    }

    oc_wasm_bytebox_func_handle_convert convert;
    convert.bb = handle;
    return convert.oc;
}

oc_wasm_function_info oc_wasm_function_get_info(oc_arena* scratch, oc_wasm* wasm, oc_wasm_function_handle* handle)
{
    oc_wasm_bytebox_func_handle_convert convert;
    convert.oc = handle;

    if(bb_func_handle_isvalid(convert.bb) == false)
    {
        return (oc_wasm_function_info){ 0 };
    }

    bb_func_info bbInfo = bb_module_instance_func_info(wasm->instance, convert.bb);

    u32 totalTypes = bbInfo.num_params + bbInfo.num_returns;
    oc_wasm_valtype* types = (totalTypes > 0) ? oc_arena_push_array(scratch, oc_wasm_valtype, totalTypes) : NULL;

    oc_wasm_function_info info;
    info.countParams = (u32)bbInfo.num_params;
    info.countReturns = (u32)bbInfo.num_returns;
    info.params = types;
    info.returns = types + info.countParams;

    for(u32 i = 0; i < info.countParams; ++i)
    {
        bb_valtype valtype = bbInfo.params[i];
        info.params[i] = bytebox_valtype_to_oc_valtype(valtype);
    }

    for(u32 i = 0; i < info.countReturns; ++i)
    {
        bb_valtype valtype = bbInfo.returns[i];
        info.returns[i] = bytebox_valtype_to_oc_valtype(valtype);
    }

    return info;
}

oc_wasm_status oc_wasm_function_call(oc_wasm* wasm, oc_wasm_function_handle* handle, oc_wasm_val* params, size_t countParams, oc_wasm_val* returns, size_t countReturns)
{
    oc_wasm_bytebox_func_handle_convert convert;
    convert.oc = handle;

    bb_val bbParams[64];
    OC_ASSERT(countParams < oc_array_size(bbParams));
    bb_val bbReturns[64];
    OC_ASSERT(countReturns < oc_array_size(bbReturns));

    for(size_t i = 0; i < countParams; ++i)
    {
        bbParams[i] = oc_wasm_val_to_bytebox_val(params[i]);
    }

    bb_error err = bb_module_instance_invoke(wasm->instance, convert.bb, bbParams, countParams, bbReturns, countReturns, (bb_module_instance_invoke_opts){ 0 });
    if(err != BB_ERROR_OK)
    {
        oc_log_error("caught error invoking function: %s", bb_error_str(err));
        return OC_WASM_STATUS_FAIL_UNKNOWN;
    }

    for(size_t i = 0; i < countReturns; ++i)
    {
        returns[i] = bytebox_val_to_oc_val(bbReturns[i]);
    }

    return OC_WASM_STATUS_SUCCESS;

    // if(handle == NULL)
    // {
    //     return OC_WASM_STATUS_SUCCESS;
    // }

    // IM3Function m3Func = (IM3Function)handle;

    // const void* valuePtrs[128];
    // if(oc_array_size(valuePtrs) < countParams)
    // {
    //     OC_ASSERT("Need more static storage for params");
    // }

    // for(size_t i = 0; i < countParams; ++i)
    // {
    //     valuePtrs[i] = &params[i];
    // }

    // M3Result res = m3_Call(m3Func, countParams, valuePtrs);
    // if(res)
    // {
    //     return oc_wasm_handle_bytebox_result(wasm, res, "Function call failed");
    // }

    // if(countReturns > 0)
    // {
    //     if(oc_array_size(valuePtrs) < countReturns)
    //     {
    //         OC_ASSERT("Need more static storage for returns");
    //     }

    //     for(size_t i = 0; i < countReturns; ++i)
    //     {
    //         valuePtrs[i] = &returns[i];
    //     }

    //     res = m3_GetResults(m3Func, (uint32_t)countReturns, valuePtrs);
    //     if(res)
    //     {
    //         return oc_wasm_handle_bytebox_result(wasm, res, "Failed to get results from function call");
    //     }
    // }

    // return OC_WASM_STATUS_SUCCESS;
}

oc_wasm_global_handle* oc_wasm_global_find(oc_wasm* wasm, oc_str8 exportName, oc_wasm_valtype expectedType)
{
    bb_global global = bb_module_instance_find_global(wasm->instance, exportName.ptr);
    if(global.value)
    {
        OC_ASSERT(expectedType == bytebox_valtype_to_oc_valtype(global.type));
    }

    return (oc_wasm_global_handle*)global.value;

    // IM3Global m3Global = m3_FindGlobal(wasm->m3Module, exportName.ptr);
    // M3ValueType m3Type = oc_wasm_valtype_to_bytebox_valtype(expectedType);
    // if(m3Global && m3Global->type == m3Type)
    // {
    //     oc_log_error("Found global %s, expected type %s but actual type was %s", exportName.ptr, oc_wasm_valtype_str(expectedType).ptr, c_waTypes[m3Global->type]);
    //     return NULL;
    // }

    // return (oc_wasm_global_handle*)m3Global;
}

oc_wasm_val oc_wasm_global_get_value(oc_wasm_global_handle* global)
{
    if(global)
    {
        bb_val* val = (bb_val*)global;
        return bytebox_val_to_oc_val(*val);
    }

    return (oc_wasm_val){ 0 };

    // if(global == NULL)
    // {
    //     return (oc_wasm_val){ 0 };
    // }

    // IM3Global m3Global = (IM3Global)global;
    // oc_wasm_val v;
    // v.I64 = m3Global->intValue; // doesn't really matter what the actual value is since we just need the bits
    // return v;
}

void oc_wasm_global_set_value(oc_wasm_global_handle* global, oc_wasm_val value)
{
    if(global)
    {
        bb_val* val = (bb_val*)global;
        *val = oc_wasm_val_to_bytebox_val(value);
    }
}

oc_wasm_global_pointer oc_wasm_global_pointer_find(oc_wasm* wasm, oc_str8 exportName)
{
    oc_wasm_global_handle* global = oc_wasm_global_find(wasm, exportName, OC_WASM_VALTYPE_I32);
    if(global)
    {
        return (oc_wasm_global_pointer){ .handle = global, .address = oc_wasm_global_get_value(global).I32 };
    }

    return (oc_wasm_global_pointer){ 0 };

    // oc_wasm_global_handle* global = oc_wasm_global_find(wasm, exportName, OC_WASM_VALTYPE_I32);
    // IM3Global m3Global = (IM3Global)global;

    // return (oc_wasm_global_pointer){ .handle = global, .address = (u32)m3Global->intValue };
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// implementation creation / destruction

void oc_wasm_destroy(oc_wasm* wasm)
{
    bb_module_instance_destroy(wasm->instance);
    bb_import_package_deinit(wasm->imports);
    bb_module_definition_destroy(wasm->definition);

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

    return wasm;
}

#endif // OC_WASM_BACKEND_BYTEBOX
