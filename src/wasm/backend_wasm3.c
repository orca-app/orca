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
    u32 paramCount;
    u32 returnCount;
    u32 paramsStackOffset;
    wa_host_proc proc;
    wa_instance* instance;
} oc_wasm_binding_wasm3;

typedef struct wa_module
{
    wa_status status;
    IM3Environment m3Env;
    IM3Module m3Module;
} wa_module;

typedef struct wa_instance
{
    wa_module* module;
    wa_status status;
    wa_memory memory;
    IM3Runtime m3Runtime;
} wa_instance;

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

static wa_status oc_wasm_handle_wasm3_result(M3Result result, const char* msg)
{
    wa_status status = oc_wasm3_result_to_status(result);
    if(wa_status_is_fail(status))
    {
        /*
        M3ErrorInfo errInfo = { 0 };
        m3_GetErrorInfo(wasm->m3Runtime, &errInfo);
        if(errInfo.message && result == errInfo.result)
        {
            oc_log_error("%s Underlying wasm3 error: %s", msg, errInfo.message);
        }
        else
        */
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

    oc_arena_scope scratch = oc_scratch_begin();
    wa_value* paramVals = oc_arena_push_array(scratch.arena, wa_value, binding->paramCount);
    wa_value* returnVals = oc_arena_push_array(scratch.arena, wa_value, binding->returnCount);

    for(u32 i = 0; i < binding->paramCount; i++)
    {
        paramVals[i].valI64 = ((i64*)params)[i];
    }
    binding->proc(binding->instance, paramVals, returnVals, 0);

    for(u32 i = 0; i < binding->returnCount; i++)
    {
        ((i64*)returns)[i] = returnVals[i].valI64;
    }

    oc_scratch_end(scratch);
    return (0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Wasm3 implementation of interface functions

wa_module* wa_module_create(oc_arena* arena, oc_str8 contents)
{
    wa_module* module = oc_arena_push_type(arena, wa_module);
    module->m3Env = m3_NewEnvironment();
    M3Result res = m3_ParseModule(module->m3Env, &module->m3Module, (u8*)contents.ptr, contents.len);

    module->status = oc_wasm_handle_wasm3_result(res, "The application couldn't parse its web assembly module");
    return module;
}

wa_instance* wa_instance_create(oc_arena* arena, wa_module* module, wa_instance_options* options)
{
    wa_instance* instance = oc_arena_push_type(arena, wa_instance);
    instance->module = module;
    u32 stackSize = 1 << 20;
    instance->m3Runtime = m3_NewRuntime(module->m3Env, stackSize, NULL);

    oc_base_allocator* allocator = oc_base_allocator_default();
    instance->memory.limits.min = 0;
    instance->memory.limits.max = (4ULL << 30) / WA_PAGE_SIZE;
    instance->memory.ptr = oc_base_reserve(allocator, (4ULL << 30));

    m3_RuntimeSetMemoryCallbacks(instance->m3Runtime,
                                 oc_runtime_wasm_memory_resize_callback,
                                 oc_runtime_wasm_memory_free_callback,
                                 (void*)&instance->memory);
    {
        M3Result res = m3_LoadModule(instance->m3Runtime, module->m3Module);
        if(res)
        {
            instance->status = oc_wasm_handle_wasm3_result(res, "The application couldn't load its web assembly module");
            return instance;
        }
        m3_SetModuleName(module->m3Module, "module");
    }

    bool wasLinkSuccessful = true;

    for(u32 i = 0; i < options->packageCount; i++)
    {
        wa_import_package* package = &options->importPackages[i];

        oc_list_for(package->bindings, elt, wa_import_package_elt, listElt)
        {
            wa_import_binding* binding = &elt->binding;

            const size_t totalTypes = binding->hostFunction.type.paramCount + binding->hostFunction.type.returnCount;

            char signature[128];
            {
                if((sizeof(signature) - 3) < totalTypes) // -3 accounts for null and open/close parens
                {
                    oc_log_error("Couldn't link function %s (too many params+returns, max %d but need %d)\n", binding->name.ptr, (int)sizeof(signature), (int)totalTypes);
                    wasLinkSuccessful = false;
                    continue;
                }

                size_t signature_index = 0;
                signature[signature_index] = 'v'; // default to void in case there are no returns
                ++signature_index;
                for(size_t i = 0; i < binding->hostFunction.type.returnCount; ++i, ++signature_index)
                {
                    char type = wa_value_type_to_wasm3_char(binding->hostFunction.type.returns[i]);
                    signature[signature_index] = type;
                }

                signature[signature_index] = '(';
                ++signature_index;
                signature[signature_index] = 'v';

                for(size_t i = 0; i < binding->hostFunction.type.paramCount; ++i, ++signature_index)
                {
                    char type = wa_value_type_to_wasm3_char(binding->hostFunction.type.params[i]);
                    signature[signature_index] = type;
                }

                signature[signature_index] = ')';
                ++signature_index;
                signature[signature_index] = '\0';
            }
            oc_wasm_binding_wasm3* user = oc_arena_push_type(arena, oc_wasm_binding_wasm3);
            user->proc = binding->hostFunction.proc;
            user->paramCount = binding->hostFunction.type.paramCount;
            user->returnCount = binding->hostFunction.type.returnCount;
            user->paramsStackOffset = sizeof(i64) * binding->hostFunction.type.returnCount;
            user->instance = instance;

            M3Result res = m3_LinkRawFunctionEx(module->m3Module, "*", binding->name.ptr, signature, oc_wasm_binding_wasm3_thunk, user);
            if(res != m3Err_none && res != m3Err_functionLookupFailed)
            {
                oc_wasm_handle_wasm3_result(res, "link failure");
                wasLinkSuccessful = false;
                continue;
            }
        }
    }

    instance->status = WA_OK;
    {
        M3Result res = m3_CompileModule(module->m3Module);
        if(res)
        {
            instance->status = oc_wasm_handle_wasm3_result(res, "The application couldn't compile its web assembly module");
        }
    }

    if(wasLinkSuccessful == false)
    {
        instance->status = WA_FAIL_INSTANTIATE;
    }

    return instance;
}

u64 oc_wasm_mem_size(wa_instance* instance)
{
    return m3_GetMemorySize(instance->m3Runtime);
}

oc_str8 oc_wasm_mem_get(wa_instance* instance)
{
    uint32_t memSize = 0;
    uint32_t memIndex = 0;
    u8* memory = (u8*)m3_GetMemory(instance->m3Runtime, &memSize, memIndex);

    return (oc_str8){ .ptr = (char*)memory, .len = memSize };
}

wa_status oc_wasm_mem_resize(wa_instance* instance, u32 countPages)
{
    M3Result res = ResizeMemory(instance->m3Runtime, countPages);
    if(res)
    {
        return oc_wasm_handle_wasm3_result(res, "Failed to resize wasm memory");
    }

    return WA_OK;
}

wa_func* wa_instance_find_function(wa_instance* instance, oc_str8 exportName)
{
    IM3Function m3Func = NULL;
    m3_FindFunction(&m3Func, instance->m3Runtime, exportName.ptr);

    return (wa_func*)m3Func;
}

wa_func_type wa_function_get_type(oc_arena* scratch, wa_instance* instance, wa_func* function)
{
    if(function == NULL)
    {
        return (wa_func_type){ 0 };
    }

    IM3Function m3Func = (IM3Function)function;

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

wa_status wa_instance_invoke(wa_instance* instance, wa_func* func, u32 argCount, wa_value* args, u32 retCount, wa_value* returns)
{
    if(func == NULL)
    {
        return WA_OK;
    }

    IM3Function m3Func = (IM3Function)func;

    const void* valuePtrs[128];
    OC_ASSERT(argCount < oc_array_size(valuePtrs), "Need more static storage for args");

    for(size_t i = 0; i < argCount; ++i)
    {
        valuePtrs[i] = &args[i];
    }

    M3Result res = m3_Call(m3Func, argCount, valuePtrs);
    if(res)
    {
        return oc_wasm_handle_wasm3_result(res, "Function call failed");
    }

    if(retCount > 0)
    {
        OC_ASSERT(retCount < oc_array_size(valuePtrs), "Need more static storage for returns");

        for(size_t i = 0; i < retCount; ++i)
        {
            valuePtrs[i] = &returns[i];
        }

        res = m3_GetResults(m3Func, (uint32_t)retCount, valuePtrs);
        if(res)
        {
            return oc_wasm_handle_wasm3_result(res, "Failed to get results from function call");
        }
    }

    return WA_OK;
}

wa_global* wa_instance_find_global(wa_instance* instance, oc_str8 exportName)
{
    IM3Global m3Global = m3_FindGlobal(instance->module->m3Module, exportName.ptr);
    /*
    M3ValueType m3Type = wa_value_type_to_wasm3_valtype(expectedType);
    if(m3Global && m3Global->type == m3Type)
    {
        oc_log_error("Found global %s, expected type %s but actual type was %s", exportName.ptr, wa_value_type_str8(expectedType).ptr, c_waTypes[m3Global->type]);
        return NULL;
    }
    */
    return (wa_global*)m3Global;
}

wa_value wa_global_get(wa_instance* instance, wa_global* global)
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

void wa_global_set(wa_instance* instance, wa_global* global, wa_value value)
{
    if(global)
    {
        IM3Global m3Global = (IM3Global)global;
        m3Global->intValue = value.valI64;
    }
}
