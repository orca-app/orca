/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "wasm.h"

#include "bytebox.h"

typedef struct oc_wasm_binding_bytebox
{
    oc_wasm_host_proc proc;
    oc_wasm* wasm;
    i16 countParams;
    i16 countReturns;
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

static_assert(sizeof(bb_func_handle) == sizeof(oc_wasm_function_handle*), "function handle struct size mismatch");

///////////////////////////////////////////////////////////////////////////////////////////////////
// Helpers

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

static void oc_wasm_binding_bytebox_thunk(void* userdata, bb_module_instance* module, const bb_val* params, bb_val* returns)
{
    bb_slice mem = bb_module_instance_mem_all(module);

    oc_wasm_binding_bytebox* binding = userdata;

    // we have to do a temp translation due to bb_val being 128 bits in size, but the orca wasm layer expects 64-bit sized values
    i64 orca_params[255];
    i64 orca_returns[255];

    OC_ASSERT(binding->countParams <= oc_array_size(orca_params));
    OC_ASSERT(binding->countReturns <= oc_array_size(orca_returns));

    for(int i = 0; i < binding->countParams; ++i)
    {
        orca_params[i] = params[i].i64_val;
    }

    binding->proc((const i64*)orca_params, (i64*)orca_returns, (u8*)mem.data, binding->wasm);

    for(int i = 0; i < binding->countReturns; ++i)
    {
        returns[i].i64_val = orca_returns[i];
    }
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
        oc_log_error("caught error decoding module: %s\n", bb_error_str(err));
        return OC_WASM_STATUS_FAIL_DECODE;
    }
    return OC_WASM_STATUS_SUCCESS;
}

oc_wasm_status oc_wasm_add_binding(oc_wasm* wasm, oc_wasm_binding* binding)
{
    if(wasm->imports == NULL)
    {
        wasm->imports = bb_import_package_init("env");
    }

    oc_wasm_binding_elt_bytebox* elt = oc_arena_push_type(&wasm->arena, oc_wasm_binding_elt_bytebox);
    oc_list_push(&wasm->bindings, &elt->listElt);

    elt->binding.proc = binding->proc;
    elt->binding.countParams = binding->countParams;
    elt->binding.countReturns = binding->countReturns;
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
        oc_log_error("caught error adding function binding: %s\n", bb_error_str(err));
        return OC_WASM_STATUS_FAIL_UNKNOWN;
    }

    return OC_WASM_STATUS_SUCCESS;
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
        oc_log_error("caught error instaniating module: %s\n", bb_error_str(err));
        return OC_WASM_STATUS_FAIL_INSTANTIATE;
    }

    return OC_WASM_STATUS_SUCCESS;
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
    bb_error err = bb_module_instance_mem_grow_absolute(wasm->instance, countPages);
    if(err != BB_ERROR_OK)
    {
        oc_log_error("caught error resizing wasm memory: %s\n", bb_error_str(err));
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
        // NOTE: we don't log an error in this case because orca speculatively looks for exports -
        //       it's ok if we don't find them.
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
        oc_log_error("caught error invoking function: %s\n", bb_error_str(err));
        return OC_WASM_STATUS_FAIL_UNKNOWN;
    }

    for(size_t i = 0; i < countReturns; ++i)
    {
        returns[i] = bytebox_val_to_oc_val(bbReturns[i]);
    }

    return OC_WASM_STATUS_SUCCESS;
}

oc_wasm_global_handle* oc_wasm_global_find(oc_wasm* wasm, oc_str8 exportName, oc_wasm_valtype expectedType)
{
    bb_global global = bb_module_instance_find_global(wasm->instance, exportName.ptr);
    if(global.value)
    {
        OC_ASSERT(expectedType == bytebox_valtype_to_oc_valtype(global.type));
    }

    return (oc_wasm_global_handle*)global.value;
}

oc_wasm_val oc_wasm_global_get_value(oc_wasm_global_handle* global)
{
    if(global)
    {
        bb_val* val = (bb_val*)global;
        return bytebox_val_to_oc_val(*val);
    }

    return (oc_wasm_val){ 0 };
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

    wasm->arena = arena;
    oc_list_init(&wasm->bindings);

    return wasm;
}
