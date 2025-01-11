/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "wasm.h"
#include "util/strings.h"

size_t oc_wasm_valtype_size(oc_wasm_valtype valtype)
{
    switch(valtype)
    {
        case OC_WASM_VALTYPE_I32:
            return sizeof(i32);
        case OC_WASM_VALTYPE_I64:
            return sizeof(i64);
        case OC_WASM_VALTYPE_F32:
            return sizeof(f32);
        case OC_WASM_VALTYPE_F64:
            return sizeof(f64);
    }

    OC_ASSERT(false, "unhandled case %d", valtype);

    return sizeof(i64);
}

oc_str8 oc_wasm_valtype_str(oc_wasm_valtype valtype)
{
    switch(valtype)
    {
        case OC_WASM_VALTYPE_I32:
            return OC_STR8("i32");
        case OC_WASM_VALTYPE_I64:
            return OC_STR8("i64");
        case OC_WASM_VALTYPE_F32:
            return OC_STR8("f32");
        case OC_WASM_VALTYPE_F64:
            return OC_STR8("f64");
    }

    OC_ASSERT(false, "unhandled case %d", valtype);

    return OC_STR8("unknown");
}

static oc_str8 WA_STATUS_STRINGS[] = {
#define X(n, s) OC_STR8_LIT(s),
    WA_STATUS(X)
#undef X
};

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
