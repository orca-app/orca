/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "wasm.h"
#include "util/strings.h"

static oc_str8 WA_STATUS_STRINGS[] = {
#define X(n, s) OC_STR8_LIT(s),
    WA_STATUS(X)
#undef X
};

oc_str8 wa_value_type_string(wa_value_type t)
{
    switch(t)
    {
        case WA_TYPE_I32:
            return OC_STR8("i32");
        case WA_TYPE_I64:
            return OC_STR8("i64");
        case WA_TYPE_F32:
            return OC_STR8("f32");
        case WA_TYPE_F64:
            return OC_STR8("f64");
        case WA_TYPE_V128:
            return OC_STR8("vec128");
        case WA_TYPE_FUNC_REF:
            return OC_STR8("funcref");
        case WA_TYPE_EXTERN_REF:
            return OC_STR8("externref");
        default:
            return OC_STR8("invalid type");
    }
}

oc_str8 wa_status_str8(wa_status status)
{
    if(status < WA_STATUS_COUNT)
    {
        return (WA_STATUS_STRINGS[status]);
    }
    else
    {
        OC_ASSERT(0, "unhandled wasm status %d", status);
        return (OC_STR8("error: unknown"));
    }
}

void wa_import_package_push_binding(oc_arena* arena, wa_import_package* package, wa_import_binding* binding)
{
    wa_import_package_elt* elt = oc_arena_push_type(arena, wa_import_package_elt);
    elt->binding = *binding;
    elt->binding.name = oc_str8_push_copy(arena, binding->name);

    if(binding->kind == WA_BINDING_HOST_FUNCTION)
    {
        wa_func_type* type = &binding->hostFunction.type;
        elt->binding.hostFunction.type.params = oc_arena_push_array(arena, wa_value_type, type->paramCount);
        elt->binding.hostFunction.type.returns = oc_arena_push_array(arena, wa_value_type, type->returnCount);

        memcpy(elt->binding.hostFunction.type.params, type->params, type->paramCount * sizeof(wa_value_type));
        memcpy(elt->binding.hostFunction.type.returns, type->returns, type->returnCount * sizeof(wa_value_type));
    }

    oc_list_push_back(&package->bindings, &elt->listElt);
    package->bindingCount++;
}

oc_str8 wa_instance_get_memory_str8(wa_instance* instance)
{
    wa_memory mem = wa_instance_get_memory(instance);
    return ((oc_str8){
        .ptr = mem.ptr,
        .len = mem.limits.min * WA_PAGE_SIZE,
    });
}
