/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "platform_clock.h"
#include "util/typedefs.h"
#include "util/debug.h"
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

static f64 bootTime = 0.0;

// Seconds from January 1900 to January 1970.
static const u64 CLK_JAN_1970 = 2208988800ULL;

static inline f64 tp_to_f64(struct timespec tp)
{
    return (f64)tp.tv_sec + (f64)tp.tv_nsec * 1e-9;
}

void oc_clock_init(void)
{
    struct timespec tp = {0};
    int ok = clock_gettime(CLOCK_BOOTTIME, &tp);
    OC_ASSERT(ok == 0, "Couldn't retrieve boot time: %s (%d)", strerror(errno), errno);

    bootTime = tp_to_f64(tp);
}

f64 oc_clock_time(oc_clock_kind clock)
{
    f64 ts = 0.0;
    switch(clock) {
        case OC_CLOCK_MONOTONIC:
        {
            struct timespec tp = {0};
            int ok = clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
            OC_ASSERT(ok == 0, "Couldn't read monotonic clock: %s (%d)", strerror(errno), errno);
            ts = tp_to_f64(tp);
        }
        break;
        case OC_CLOCK_UPTIME:
        {
            struct timespec tp = {0};
            int ok = clock_gettime(CLOCK_BOOTTIME, &tp);
            OC_ASSERT(ok == 0, "Couldn't read boottime clock: %s (%d)", strerror(errno), errno);
            ts = tp_to_f64(tp) - bootTime;
        }
        break;
        case OC_CLOCK_DATE:
        {
            struct timespec tp = {0};
            int ok = clock_gettime(CLOCK_REALTIME, &tp);
            OC_ASSERT(ok == 0, "Couldn't read realtime clock: %s (%d)", strerror(errno), errno);
            ts = tp_to_f64(tp) + (f64)CLK_JAN_1970;
        }
        break;
    }
    return ts;
}
