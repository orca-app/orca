/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "wasm.h"

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

oc_str8 oc_wasm_status_str8(oc_wasm_status status)
{
    switch(status)
    {
        case OC_WASM_STATUS_SUCCESS:
            return OC_STR8("success");
        case OC_WASM_STATUS_FAIL_UNKNOWN:
            return OC_STR8("unknown");
        case OC_WASM_STATUS_FAIL_MEMALLOC:
            return OC_STR8("memalloc failure");
        case OC_WASM_STATUS_FAIL_DECODE:
            return OC_STR8("decode failure");
        case OC_WASM_STATUS_FAIL_INSTANTIATE:
            return OC_STR8("instantiate failure");
        case OC_WASM_STATUS_FAIL_TRAP_OUTOFBOUNDSMEMORYACCESS:
            return OC_STR8("trap: outofboundsmemoryaccess");
        case OC_WASM_STATUS_FAIL_TRAP_DIVISIONBYZERO:
            return OC_STR8("trap: division by zero");
        case OC_WASM_STATUS_FAIL_TRAP_INTEGEROVERFLOW:
            return OC_STR8("trap: integer overflow");
        case OC_WASM_STATUS_FAIL_TRAP_INTEGERCONVERSION:
            return OC_STR8("trap: integer conversion");
        case OC_WASM_STATUS_FAIL_TRAP_INDIRECTCALLTYPEMISMATCH:
            return OC_STR8("trap: indirect call type mismatch");
        case OC_WASM_STATUS_FAIL_TRAP_TABLEINDEXOUTOFRANGE:
            return OC_STR8("trap: table index out of range");
        case OC_WASM_STATUS_FAIL_TRAP_TABLEELEMENTISNULL:
            return OC_STR8("trap: table element is null");
        case OC_WASM_STATUS_FAIL_TRAP_EXIT:
            return OC_STR8("trap: exit");
        case OC_WASM_STATUS_FAIL_TRAP_ABORT:
            return OC_STR8("trap: abort");
        case OC_WASM_STATUS_FAIL_TRAP_UNREACHABLE:
            return OC_STR8("trap: unreachable");
        case OC_WASM_STATUS_FAIL_TRAP_STACKOVERFLOW:
            return OC_STR8("trap: stack overflow");
    }

    OC_ASSERT(false, "unhandled case %d", status);

    return OC_STR8("unknown");
}
