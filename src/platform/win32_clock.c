/************************************************************/ /**
*
*	@file: win32_clock.c
*	@author: Martin Fouilleul
*	@date: 03/02/2023
*	@revision:
*
*****************************************************************/
#include <profileapi.h>

#include "platform_clock.h"
#include "util/typedefs.h"

#ifdef __cplusplus
extern "C"
{
#endif

    static u64 __performanceCounterFreq = 0;

    void oc_clock_init()
    {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        __performanceCounterFreq = freq.QuadPart;
    }

    f64 oc_clock_time(oc_clock_kind clock)
    {
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);

        f64 time = __performanceCounterFreq ? (counter.QuadPart / (f64)__performanceCounterFreq) : 0;
        return (time);
    }

#ifdef __cplusplus
} // extern "C"
#endif
