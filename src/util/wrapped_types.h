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

#define oc_result_type(valueType, errorType) \
    struct                                   \
    {                                        \
        errorType error;                     \
        valueType value;                     \
    }

void oc_set_last_error(i32 e);
i32 oc_last_error(void);

#define oc_result_value(type, val) ((type){ .value = val })
#define oc_result_error(type, err) ((type){ .error = err })
#define oc_result_check(r) ((r).error == 0)
#define oc_result_unwrap(r) ({__typeof__(r) tmp = r; OC_ASSERT(!tmp.error); tmp.value; })
#define oc_result_if(r) \
    ({__typeof__(r) tmp = r; oc_set_last_error(tmp.error); tmp.value; });              \
    if(oc_last_error() == 0)

#define oc_catch(r) \
    ({__typeof__(r) tmp = r; oc_set_last_error(tmp.error); (tmp.error == 0) ? tmp.value : (__typeof__(tmp.value)){0}; });          \
    if(oc_last_error() != 0)

#define oc_try(r)                                               \
    ({__typeof__(r) tmp = r; \
    oc_set_last_error(tmp.error); \
    (tmp.error == 0) ? tmp.value : (__typeof__(tmp.value)){0}; });                                                      \
    if(oc_last_error() != 0)                                    \
    {                                                           \
        return oc_result_error(__typeof__(r), oc_last_error()); \
    }

//----------------------------------------------------------------------------------------
//NOTE(martin): Helpers for option types
//----------------------------------------------------------------------------------------

#define oc_option_type(valueType) \
    struct                        \
    {                             \
        bool ok;                  \
        valueType value;          \
    }

#define oc_ptr_option_type(valueType) \
    union                             \
    {                                 \
        valueType* ok;                \
        valueType* value;             \
    }

void oc_option_set_last_result(bool b);
bool oc_option_get_last_result(void);

#define oc_option_value(type, v) ((type){ .ok = true, .value = v })
#define oc_option_ptr(type, p) ((type){ .value = p })
#define oc_option_nil(type) ((type){ .value = 0 })
#define oc_option_check(opt) ((opt).ok)
#define oc_option_unwrap(opt) ({__typeof__(opt) tmp = opt; OC_DEBUG_ASSERT(tmp.ok); tmp.value; })
#define oc_option_if(opt) \
    ({__typeof__(opt) tmp = opt; oc_option_set_last_result(tmp.ok); tmp.ok ? tmp.value : (__typeof__(tmp.value)){0}; });                \
    if(oc_option_get_last_result())

#define oc_option_orelse(opt)       \
    ({__typeof__(opt) tmp = opt; oc_option_set_last_result(tmp.ok); tmp.ok ? tmp.value : (__typeof__(tmp.value)){0}; });                          \
    if(oc_option_get_last_result()) \
    {                               \
    }                               \
    else
