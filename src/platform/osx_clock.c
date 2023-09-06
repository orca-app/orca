/************************************************************/ /**
*
*	@file: osx_clock.cpp
*	@author: Martin Fouilleul
*	@date: 07/03/2019
*	@revision:
*
*****************************************************************/

#include <Availability.h> // availability macros
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <math.h>     //fabs()
#include <sys/time.h> // gettimeofday()
#include <time.h>

#include <sys/sysctl.h>
#include <sys/types.h>

#include "platform_clock.h"

typedef struct timeval timeval;
typedef struct timespec timespec;

//TODO(martin): measure the actual values of these constants
const f64 OC_CLOCK_FUZZ = 25e-9, // minimum time to read the clock (s)
    OC_CLOCK_TICK = 25e-9;       // minimum step between two clock readings (s)

static mach_timebase_info_data_t __machTimeBase__ = { 1, 1 };
static u64 __initialTimestamp__ = 0;

static inline u64 OSXGetUptimeNanoseconds()
{
    //NOTE(martin): according to the documentation, mach_absolute_time() does not
    //              increment when the system is asleep
    u64 now = mach_absolute_time();
    now *= __machTimeBase__.numer;
    now /= __machTimeBase__.denom;
    return (now);
}

static inline u64 OSXGetMonotonicNanoseconds()
{
    //NOTE(martin): according to the documentation, OC_CLOCK_MONOTONIC increment monotonically
    //              on systems where OC_CLOCK_MONOTONIC_RAW is present, we may want to use that instead,
    //              because OC_CLOCK_MONOTONIC seems to be subject to frequency changes ?

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200
    #ifndef CLOCK_MONOTONIC_RAW
        #error "CLOCK_MONOTONIC_RAW not found. Please verify that <time.h> is included from the MacOSX SDK rather than /usr/local/include"
    #else
    return (clock_gettime_nsec_np(CLOCK_MONOTONIC_RAW));
    #endif
#else
    //TODO(martin): quick and dirty hack is to fallback to uptime,
    //              but we should either only support macos version >= 10.12, or find a proper solution
    return (OSXGetUptimeNanoseconds());
#endif
}

static const f64 CLK_TIMESTAMPS_PER_SECOND = 4294967296.; // 2^32 as a double
static const u64 CLK_JAN_1970 = 2208988800ULL;            // seconds from january 1900 to january 1970

void oc_clock_init()
{
    mach_timebase_info(&__machTimeBase__);

    //NOTE(martin): get the date of system boot time
    timeval tv = { 0, 0 };

    int mib[2] = { CTL_KERN,
                   KERN_BOOTTIME };
    size_t size = sizeof(tv);

    if(sysctl(mib, 2, &tv, &size, 0, 0) == -1)
    {
        oc_log_error("can't read boot time\n");
    }
    //NOTE(martin): convert boot date to timestamp
    __initialTimestamp__ = (((u64)tv.tv_sec + CLK_JAN_1970) << 32)
                         + (u64)(tv.tv_usec * 1e-6 * CLK_TIMESTAMPS_PER_SECOND);

    //TODO(martin): maybe get a state vector for exclusive clock usage ?
    //RandomSeedFromDevice();
}

/*
u64 oc_clock_timestamp(oc_clock_kind clock)
{
    u64 ts = 0;
    switch(clock)
    {
        case OC_CLOCK_MONOTONIC:
        {
            //NOTE(martin): compute monotonic offset and add it to bootup timestamp
            u64 noff = OSXGetMonotonicNanoseconds();
            u64 foff = (u64)(noff * 1e-9 * CLK_TIMESTAMPS_PER_SECOND);
            ts = __initialTimestamp__ + foff;
        }
        break;

        case OC_CLOCK_UPTIME:
        {
            //TODO(martin): maybe we should warn that this date is inconsistent after a sleep ?
            //NOTE(martin): compute uptime offset and add it to bootup timestamp
            u64 noff = OSXGetUptimeNanoseconds();
            u64 foff = (u64)(noff * 1e-9 * CLK_TIMESTAMPS_PER_SECOND);
            ts = __initialTimestamp__ + foff;
        }
        break;

        case OC_CLOCK_DATE:
        {
            //NOTE(martin): get system date and convert it to a fixed-point timestamp
            timeval tv;
            gettimeofday(&tv, 0);
            ts = (((u64)tv.tv_sec + CLK_JAN_1970) << 32)
               + (u64)(tv.tv_usec * 1e-6 * CLK_TIMESTAMPS_PER_SECOND);
        }
        break;
    }

    return (ts);
}
*/

f64 oc_clock_time(oc_clock_kind clock)
{
    switch(clock)
    {
        case OC_CLOCK_MONOTONIC:
        {
            //NOTE(martin): compute monotonic offset and add it to bootup timestamp
            u64 noff = OSXGetMonotonicNanoseconds();
            return ((f64)noff * 1e-9);
        }
        break;

        case OC_CLOCK_UPTIME:
        {
            //TODO(martin): maybe we should warn that this date is inconsistent after a sleep ?
            //NOTE(martin): compute uptime offset and add it to bootup timestamp
            u64 noff = OSXGetUptimeNanoseconds();
            return ((f64)noff * 1e-9);
        }
        break;

        case OC_CLOCK_DATE:
        {
            //TODO(martin): maybe warn about precision loss ?
            //              could also change the epoch since we only promise to return a relative time
            //NOTE(martin): get system date and convert it to seconds
            timeval tv;
            gettimeofday(&tv, 0);
            return (((f64)tv.tv_sec + CLK_JAN_1970) + ((f64)tv.tv_usec * 1e-6));
        }
        break;
    }
}
