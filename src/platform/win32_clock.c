/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <profileapi.h>

#include "platform_clock.h"
#include "util/typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

static u64 __performanceCounterFreq = 0;

const u64 WIN_EPOCH_TO_UNIX_EPOCH_100NS = 116444736000000000; // number of 100ns from Jan 1, 1601 (windows epoch) to  Jan 1, 1970 (unix epoch)

void oc_clock_init()
{
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    __performanceCounterFreq = freq.QuadPart;
}

f64 oc_clock_time(oc_clock_kind clock)
{
    switch(clock)
    {
        case OC_CLOCK_MONOTONIC:
        {
            LARGE_INTEGER counter;
            QueryPerformanceCounter(&counter);

            f64 time = __performanceCounterFreq ? (counter.QuadPart / (f64)__performanceCounterFreq) : 0;
            return (time);
        }

        case OC_CLOCK_UPTIME:
        {
            ULONGLONG unbiasedTime100Ns;
            if(QueryUnbiasedInterruptTime(&unbiasedTime100Ns))
            {
                f64 unbiasedTimeSecs = ((f64)unbiasedTime100Ns) / (10 * 1000 * 1000);
                return unbiasedTimeSecs;
            }
            // TODO: Find another way to handle this that doesn't depend on oc_log_error.
            // oc_log_error("OC_CLOCK_UPTIME syscall failed with error: %d", GetLastError());
            return 0;
        }

        case OC_CLOCK_DATE:
        {
            FILETIME ft;
            GetSystemTimeAsFileTime(&ft);

            u64 systemTime100Ns = ((u64)ft.dwHighDateTime << 32) | (u64)ft.dwLowDateTime;
            u64 timestamp100Ns = systemTime100Ns - WIN_EPOCH_TO_UNIX_EPOCH_100NS;
            f64 timeSecs = ((f64)timestamp100Ns) / (10 * 1000 * 1000);
            return timeSecs;
        }
    }

    return 0.0;
}

#ifdef __cplusplus
} // extern "C"
#endif
