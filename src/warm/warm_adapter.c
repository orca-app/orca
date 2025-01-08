/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "orca.h"
#include "wasm/wasm.h"
#include "warm.c"

//TODO: implement wasm.h API in terms of warm.c

typedef struct oc_wasm
{
    oc_arena arena;
    wa_module* module;
    wa_instance* instance;

    u32 bindingCount;
    oc_list bindings;
} oc_wasm;

oc_wasm* oc_wasm_create(void)
{
    oc_wasm* wasm = (oc_wasm*)malloc(sizeof(oc_wasm));
    memset(wasm, 0, sizeof(oc_wasm));
    oc_arena_init(&wasm->arena);
    return (wasm);
}

void oc_wasm_destroy(oc_wasm* wasm)
{
    oc_arena_cleanup(&wasm->arena);
    free(wasm);
}

oc_wasm_status oc_wasm_decode(oc_wasm* wasm, oc_str8 wasmBlob)
{
    wasm->module = wa_module_create(&wasm->arena, wasmBlob);

    if(wa_module_has_errors(wasm->module))
    {
        wa_module_print_errors(wasm->module);
        return (OC_WASM_STATUS_FAIL_DECODE);
    }
    return OC_WASM_STATUS_SUCCESS;
}

typedef struct wa_import_binding_elt
{
    oc_list_elt listElt;
    wa_import_binding binding;
} wa_import_binding_elt;

typedef struct oc_wasm_binding_warm
{
    oc_wasm_host_proc proc;
    oc_wasm* wasm;
    i16 countParams;
    i16 countReturns;
} oc_wasm_binding_warm;

void oc_wasm_binding_warm_thunk(wa_instance* instance, wa_value* args, wa_value* returns, void* user)
{
    oc_wasm_binding_warm* binding = (oc_wasm_binding_warm*)user;
    oc_arena_scope scratch = oc_scratch_begin();

    //TODO: remove the need for this marshalling
    i64* args64 = oc_arena_push_array(scratch.arena, i64, binding->countParams);
    i64* returns64 = oc_arena_push_array(scratch.arena, i64, binding->countReturns);

    for(u32 i = 0; i < binding->countParams; i++)
    {
        args64[i] = args[i].valI64;
    }

    binding->proc(args64, returns64, (u8*)instance->memories[0]->ptr, binding->wasm);

    for(u32 i = 0; i < binding->countReturns; i++)
    {
        returns[i].valI64 = returns64[i];
    }

    oc_scratch_end(scratch);
}

wa_value_type oc_wasm_valtype_to_wa_value_type(oc_wasm_valtype type)
{
    switch(type)
    {
        case OC_WASM_VALTYPE_I32:
            return WA_TYPE_I32;
        case OC_WASM_VALTYPE_I64:
            return WA_TYPE_I64;
        case OC_WASM_VALTYPE_F32:
            return WA_TYPE_F32;
        case OC_WASM_VALTYPE_F64:
            return WA_TYPE_F64;
    }
}

oc_wasm_valtype oc_wa_value_type_to_wasm_valtype(wa_value_type type)
{
    switch(type)
    {
        case WA_TYPE_I32:
            return OC_WASM_VALTYPE_I32;
        case WA_TYPE_I64:
            return OC_WASM_VALTYPE_I64;
        case WA_TYPE_F32:
            return OC_WASM_VALTYPE_F32;
        case WA_TYPE_F64:
            return OC_WASM_VALTYPE_F64;

        default:
            return OC_WASM_VALTYPE_I32;
    }
}

oc_wasm_status oc_wasm_add_binding(oc_wasm* wasm, oc_wasm_binding* binding)
{
    wa_import_binding_elt* elt = oc_arena_push_type(&wasm->arena, wa_import_binding_elt);

    oc_wasm_binding_warm* userData = oc_arena_push_type(&wasm->arena, oc_wasm_binding_warm);
    userData->proc = binding->proc;
    userData->wasm = wasm;
    userData->countParams = binding->countParams;
    userData->countReturns = binding->countReturns;

    elt->binding = (wa_import_binding){
        .name = oc_str8_push_copy(&wasm->arena, binding->importName),
        .kind = WA_BINDING_HOST_FUNCTION,
        .hostFunction = {
            .type = {
                .paramCount = binding->countParams,
                .params = oc_arena_push_array(&wasm->arena, wa_value_type, binding->countParams),
                .returnCount = binding->countReturns,
                .returns = oc_arena_push_array(&wasm->arena, wa_value_type, binding->countReturns),
            },
            .proc = oc_wasm_binding_warm_thunk,
            .userData = (void*)userData,
        },
    };
    for(u32 paramIndex = 0; paramIndex < binding->countParams; paramIndex++)
    {
        wa_value_type type = oc_wasm_valtype_to_wa_value_type(binding->params[paramIndex]);
        elt->binding.hostFunction.type.params[paramIndex] = type;
    }
    for(u32 returnIndex = 0; returnIndex < binding->countReturns; returnIndex++)
    {
        wa_value_type type = oc_wasm_valtype_to_wa_value_type(binding->returns[returnIndex]);
        elt->binding.hostFunction.type.returns[returnIndex] = type;
    }

    oc_list_push_back(&wasm->bindings, &elt->listElt);
    wasm->bindingCount++;

    return (OC_WASM_STATUS_SUCCESS);
}

oc_wasm_status oc_wasm_instantiate(oc_wasm* wasm, oc_str8 moduleDebugName, oc_wasm_mem_callbacks memCallbacks)
{
    oc_arena_scope scratch = oc_scratch_begin();

    wa_import_package package = {
        .name = OC_STR8("env"),
        .bindingCount = wasm->bindingCount,
        .bindings = oc_arena_push_array(scratch.arena, wa_import_binding, wasm->bindingCount),
    };
    u32 i = 0;
    oc_list_for(wasm->bindings, elt, wa_import_binding_elt, listElt)
    {
        wa_import_binding* binding = &package.bindings[i];
        *binding = elt->binding;
        i++;
    }
    wa_instance_options options = {
        .packageCount = 1,
        .importPackages = &package,
    };

    wasm->instance = wa_instance_create(&wasm->arena, wasm->module, &options);

    oc_scratch_end(scratch);

    if(wasm->instance->status == WA_OK)
    {
        return OC_WASM_STATUS_SUCCESS;
    }
    else
    {
        return OC_WASM_STATUS_FAIL_INSTANTIATE;
    }
}

u64 oc_wasm_mem_size(oc_wasm* wasm)
{
    return (wasm->instance->memories[0]->limits.min * WA_PAGE_SIZE);
}

oc_str8 oc_wasm_mem_get(oc_wasm* wasm)
{
    return (oc_str8){
        .ptr = wasm->instance->memories[0]->ptr,
        .len = wasm->instance->memories[0]->limits.min * WA_PAGE_SIZE,
    };
}

oc_wasm_status oc_wasm_mem_resize(oc_wasm* wasm, u32 n)
{
    wa_memory* mem = wasm->instance->memories[0];

    if(n <= mem->limits.max
       && (n >= mem->limits.min))
    {
        oc_base_allocator* allocator = oc_base_allocator_default();
        oc_base_commit(allocator, mem->ptr, n * WA_PAGE_SIZE);
        mem->limits.min = n;
        return OC_WASM_STATUS_SUCCESS;
    }
    return OC_WASM_STATUS_FAIL_MEMALLOC;
}

oc_wasm_function_handle* oc_wasm_function_find(oc_wasm* wasm, oc_str8 exportName)
{
    wa_func* func = wa_instance_find_function(wasm->instance, exportName);
    return (oc_wasm_function_handle*)func;
}

oc_wasm_global_pointer oc_wasm_global_pointer_find(oc_wasm* wasm, oc_str8 exportName)
{
    oc_wasm_global_pointer res = { 0 };
    wa_global* global = wa_instance_find_global(wasm->instance, exportName);
    if(global && global->type == WA_TYPE_I32)
    {
        res.handle = (oc_wasm_global_handle*)global;
        res.address = global->value.valI32;
    }
    return (res);
}

oc_wasm_function_info oc_wasm_function_get_info(oc_arena* scratch, oc_wasm* wasm, oc_wasm_function_handle* handle)
{
    oc_wasm_function_info res = { 0 };

    wa_func_type* type = wa_function_get_type((wa_func*)handle);
    if(type)
    {
        res = (oc_wasm_function_info){
            .params = oc_arena_push_array(scratch, oc_wasm_valtype, type->paramCount),
            .returns = oc_arena_push_array(scratch, oc_wasm_valtype, type->returnCount),
            .countParams = type->paramCount,
            .countReturns = type->returnCount,
        };
        for(u32 i = 0; i < type->paramCount; i++)
        {
            res.params[i] = oc_wa_value_type_to_wasm_valtype(type->params[i]);
        }
        for(u32 i = 0; i < type->returnCount; i++)
        {
            res.returns[i] = oc_wa_value_type_to_wasm_valtype(type->returns[i]);
        }
    }
    return (res);
}

oc_wasm_status oc_wasm_function_call(oc_wasm* wasm,
                                     oc_wasm_function_handle* handle,
                                     oc_wasm_val* params,
                                     size_t countParams,
                                     oc_wasm_val* returns,
                                     size_t countReturns)
{
    //////////////////////////////////////////////////////
    //TODO: avoid having to do this marshalling
    //////////////////////////////////////////////////////
    oc_arena_scope scratch = oc_scratch_begin();
    wa_value* waParams = oc_arena_push_array(scratch.arena, wa_value, countParams);
    wa_value* waReturns = oc_arena_push_array(scratch.arena, wa_value, countReturns);

    for(u32 i = 0; i < countParams; i++)
    {
        waParams[i].valI64 = params[i].I64;
    }

    wa_status status = wa_instance_invoke(wasm->instance,
                                          (wa_func*)handle,
                                          countParams,
                                          waParams,
                                          countReturns,
                                          waReturns);

    for(u32 i = 0; i < countReturns; i++)
    {
        params[i].I64 = waReturns[i].valI64;
    }

    oc_scratch_end(scratch);

    if(status == WA_OK)
    {
        return OC_WASM_STATUS_SUCCESS;
    }
    else
    {
        //TODO: translate errors
        return OC_WASM_STATUS_FAIL_UNKNOWN;
    }
}

oc_wasm_global_handle* oc_wasm_global_find(oc_wasm* wasm, oc_str8 exportName, oc_wasm_valtype expectedType);

oc_wasm_val oc_wasm_global_get_value(oc_wasm_global_handle* global);
void oc_wasm_global_set_value(oc_wasm_global_handle* global, oc_wasm_val value);
oc_wasm_global_pointer oc_wasm_global_pointer_find(oc_wasm* wasm, oc_str8 exportName);
