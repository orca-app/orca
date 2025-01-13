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

wa_status oc_wasm_decode(oc_wasm* wasm, oc_str8 wasmBlob)
{
    wasm->module = wa_module_create(&wasm->arena, wasmBlob);

    if(wa_module_has_errors(wasm->module))
    {
        wa_module_print_errors(wasm->module);

        return (WA_PARSE_ERROR); ////TODO: this is only temporary, later change the API for module creation to allow retrieving more errors
    }
    return WA_OK;
}

wa_status oc_wasm_instantiate(oc_wasm* wasm, oc_str8 moduleDebugName, wa_import_package* package)
{
    oc_arena_scope scratch = oc_scratch_begin();

    wa_instance_options options = {
        .packageCount = 1,
        .importPackages = package,
    };

    wasm->instance = wa_instance_create(&wasm->arena, wasm->module, &options);

    oc_scratch_end(scratch);

    return (wasm->instance->status);
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

wa_status oc_wasm_mem_resize(oc_wasm* wasm, u32 n)
{
    wa_memory* mem = wasm->instance->memories[0];

    if(n <= mem->limits.max
       && (n >= mem->limits.min))
    {
        oc_base_allocator* allocator = oc_base_allocator_default();
        oc_base_commit(allocator, mem->ptr, n * WA_PAGE_SIZE);
        mem->limits.min = n;
        return WA_OK;
    }
    return WA_TRAP_MEMORY_OUT_OF_BOUNDS;
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

wa_func_type oc_wasm_function_get_info(oc_arena* scratch, oc_wasm* wasm, oc_wasm_function_handle* handle)
{
    wa_func_type res = { 0 };

    wa_func_type* type = wa_function_get_type((wa_func*)handle);
    if(type)
    {
        res = (wa_func_type){
            .params = oc_arena_push_array(scratch, wa_value_type, type->paramCount),
            .returns = oc_arena_push_array(scratch, wa_value_type, type->returnCount),
            .paramCount = type->paramCount,
            .returnCount = type->returnCount,
        };
        memcpy(res.params, type->params, type->paramCount * sizeof(wa_value_type));
        memcpy(res.returns, type->returns, type->returnCount * sizeof(wa_value_type));
    }
    return (res);
}

wa_status oc_wasm_function_call(oc_wasm* wasm,
                                oc_wasm_function_handle* handle,
                                wa_value* params,
                                size_t countParams,
                                wa_value* returns,
                                size_t countReturns)
{
    wa_status status = wa_instance_invoke(wasm->instance,
                                          (wa_func*)handle,
                                          countParams,
                                          params,
                                          countReturns,
                                          returns);
    return status;
}

oc_wasm_global_handle* oc_wasm_global_find(oc_wasm* wasm, oc_str8 exportName, wa_value_type expectedType);

wa_value oc_wasm_global_get_value(oc_wasm_global_handle* global);
void oc_wasm_global_set_value(oc_wasm_global_handle* global, wa_value value);
oc_wasm_global_pointer oc_wasm_global_pointer_find(oc_wasm* wasm, oc_str8 exportName);
