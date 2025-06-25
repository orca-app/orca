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

#define oc_result_type_def(name, valueType, errorType) \
    typedef struct name                                \
    {                                                  \
        bool ok;                                       \
        union                                          \
        {                                              \
            valueType value;                           \
            errorType error;                           \
        };                                             \
    } name

extern oc_thread_local bool oc_lastCatchResult;

#define oc_wrap_value(type, val) ((type){ .ok = true, .value = val })
#define oc_wrap_error(type, err) ((type){ .ok = false, .error = err })

#define oc_unwrap(e) ({__typeof__(e) tmp = e; OC_ASSERT(tmp.ok); tmp.value; })
#define oc_unwrap_or(e, d) ({__typeof__(e) tmp = e; (tmp.ok ? tmp.value : (d)); })
#define oc_catch(e) \
    ({__typeof__(e) tmp = e; oc_lastCatchResult = tmp.ok; tmp.value; });          \
    if(!oc_lastCatchResult)

//----------------------------------------------------------------------------------------
//NOTE(martin): Helpers for option types
//----------------------------------------------------------------------------------------

#define oc_option_type_def(name, valueType) \
    typedef struct name                     \
    {                                       \
        bool ok;                            \
        valueType value;                    \
    } name

#define oc_option_ptr_type_def(name, valueType) \
    typedef struct name                         \
    {                                           \
        valueType* p;                           \
    } name

#define oc_wrap_nil(type) ((type){ 0 })

//NOTE: we can use normal oc_wrap_value / oc_unwrap / oc_catch with value options. For pointer options,
// use the following versions:
#define oc_wrap_ptr(type, ptr) ((type){ .p = ptr })
#define oc_unwrap_ptr(e) ({__typeof__(e) tmp = e; OC_ASSERT(tmp.p); tmp.p; })
#define oc_unwrap_or_ptr(e, d) ({__typeof__(e) tmp = e; (tmp.p ? tmp.p : d); })
#define oc_catch_ptr(e) \
    ({__typeof__(e) tmp = e; oc_lastCatchResult = (tmp.p != 0); tmp.p; });              \
    if(!oc_lastCatchResult)

//NOTE: additionally, the pointer options have dereferencing versions
#define oc_deref(e) ({__typeof__(e) tmp = e; OC_ASSERT(tmp.p); *tmp.p; })
#define oc_deref_or(e, d) ({__typeof__(e) tmp = e; (tmp.p ? *tmp.p : d); })
#define oc_deref_catch(e) \
    ({__typeof__(e) tmp = e; oc_lastCatchResult = (tmp.p != 0); (tmp.p ? *tmp.p : (__typeof__(*tmp.p)){0}); });                \
    if(!oc_lastCatchResult)
