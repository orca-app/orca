/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "platform/platform.h"
#include "util/typedefs.h"

//----------------------------------------------------------------------------------------
//NOTE(martin): Helpers for result types
//----------------------------------------------------------------------------------------

#define oc_result(valueType, errorType) \
    struct                              \
    {                                   \
        bool ok;                        \
        union                           \
        {                               \
            valueType value;            \
            errorType error;            \
        };                              \
    }

void oc_set_last_catch_result(bool r);
bool oc_get_last_catch_result(void);

#define oc_wrap_value(type, val) ((type){ .ok = true, .value = val })
#define oc_wrap_error(type, err) ((type){ .ok = false, .error = err })

#define oc_unwrap(e) ({__typeof__(e) tmp = e; OC_ASSERT(tmp.ok); tmp.value; })
#define oc_unwrap_or(e, d) ({__typeof__(e) tmp = e; (tmp.ok ? tmp.value : (d)); })
#define oc_if_unwrap(e) \
    ({__typeof__(e) tmp = e; oc_set_last_catch_result(tmp.ok); tmp.value; });              \
    if(oc_get_last_catch_result())

#define oc_catch(e) \
    ({__typeof__(e) tmp = e; oc_set_last_catch_result(tmp.ok); tmp.value; });          \
    if(!oc_get_last_catch_result())

#define oc_check(e) ((e).ok)

//NOTE: when value is a pointer these helpers allow dereferencing the pointer
#define oc_ptr_deref(e) ({__typeof__(e) tmp = e; OC_ASSERT(tmp.ok); *tmp.value; })
#define oc_ptr_deref_or(e, d) ({__typeof__(e) tmp = e; (tmp.ok ? *tmp.value : d); })
#define oc_ptr_deref_catch(e) \
    ({__typeof__(e) tmp = e; oc_set_last_catch_result(tmp.ok != 0); (tmp.ok ? *tmp.value : (__typeof__(*tmp.value)){0}); });                    \
    if(!oc_get_last_catch_result())

//NOTE: when value is a struct or a pointer to a struct these helpers allow accessing the struct fields
#define oc_field_or(e, f, d) ({__typeof__(e) tmp = e; (tmp.ok ? tmp.value.f : d); })
#define oc_field_catch(e, f) \
    ({__typeof__(e) tmp = e; oc_set_last_catch_result(tmp.ok); tmp.value.f; });                   \
    if(!oc_get_last_catch_result())

#define oc_ptr_field_or(e, f, d) ({__typeof__(e) tmp = e; (tmp.ok ? tmp.value->f : d); })
#define oc_ptr_field_catch(e, f) \
    ({__typeof__(e) tmp = e; oc_set_last_catch_result(tmp.ok); tmp.value->f; });                       \
    if(!oc_get_last_catch_result())

//----------------------------------------------------------------------------------------
//NOTE(martin): Helpers for option types
//----------------------------------------------------------------------------------------

#define oc_option(valueType) \
    struct                   \
    {                        \
        bool ok;             \
        valueType value;     \
    }

#define oc_ptr_option(valueType) \
    union                        \
    {                            \
        valueType* ok;           \
        valueType* value;        \
    }

#define oc_wrap_nil(type) ((type){ 0 })
#define oc_wrap_ptr(type, p) ((type){ .value = p })

//NOTE: we can use normal
// - oc_wrap_value can be used with value options. For pointer options use oc_wrap_ptr.
// - Other result constructs like oc_unwrap/oc_catch etc can be used with either value or pointer options.
