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

oc_str8 wa_value_type_str8(wa_value_type t)
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
