/************************************************************//**
*
*	@file: linux_clock.c
*	@author: Martin Fouilleul
*	@date: 20/04/2020
*	@revision:
*
*****************************************************************/

#include<math.h> //fabs()
#include<time.h>
#include<sys/time.h>	// gettimeofday()

#include<sys/types.h>
#include<sys/sysctl.h>

#include<unistd.h>	// nanosleep()

#include"platform_rng.h"
#include"platform_clock.h"

extern "C" {

//TODO(martin): measure the actual values of these constants
const f64 SYSTEM_FUZZ = 25e-9,		// minimum time to read the clock (s)
          SYSTEM_TICK = 25e-9;		// minimum step between two clock readings (s)

static u64 __initialTimestamp__ = 0;
static u64 __initialMonotonicNanoseconds__ = 0;

static inline u64 LinuxGetMonotonicNanoseconds()
{
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	//WARN(martin): do not multiply ts.tv_sec directly (implicit conversion will overflow)
	u64 r = ts.tv_sec;
	r *= 1000000000;
	r += ts.tv_nsec;
	return(r);
}

static inline u64 LinuxGetUptimeNanoseconds()
{
	timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	//WARN(martin): do not multiply ts.tv_sec directly (implicit conversion will overflow)
	u64 r = ts.tv_sec;
	r *= 1000000000;
	r += ts.tv_nsec;
	return(r);
}


void ClockSystemInit()
{
	//NOTE(martin): we don't know of a widely supported way of getting the boot time on linux, so
	//              we fallback to our second best choice, which is taking an initial timestamp and
	//              the initial monotonic time now

	timeval tv;
	gettimeofday(&tv, 0);
	__initialMonotonicNanoseconds__ = LinuxGetMonotonicNanoseconds();

	//NOTE(martin): convert boot date to timestamp
	__initialTimestamp__ =   (((u64)tv.tv_sec + JAN_1970) << 32)
			       + (u64)(tv.tv_usec * 1e-6 * TIMESTAMPS_PER_SECOND);

	//TODO(martin): maybe get a state vector for exclusive clock usage ?
	RandomSeedFromDevice();
}

fx_timestamp ClockGetTimestamp(clock_kind clock)
{
	fx_timestamp ts = {0};
	switch(clock)
	{
		case SYS_CLOCK_MONOTONIC:
		{
			//NOTE(martin): compute monotonic offset and add it to bootup timestamp
			u64 noff = LinuxGetMonotonicNanoseconds() - __initialMonotonicNanoseconds__;
			u64 foff = (u64)(noff * 1e-9 * TIMESTAMPS_PER_SECOND);
			ts.ts = __initialTimestamp__ + foff;
		} break;

		case SYS_CLOCK_UPTIME:
		{
			//TODO(martin): maybe we should warn that this date is inconsistent after a sleep ?
			//NOTE(martin): compute uptime offset and add it to bootup timestamp
			u64 noff = LinuxGetUptimeNanoseconds() - __initialMonotonicNanoseconds__ ;
			u64 foff = (u64)(noff * 1e-9 * TIMESTAMPS_PER_SECOND);
			ts.ts = __initialTimestamp__ + foff;
		} break;

		case SYS_CLOCK_DATE:
		{
			//NOTE(martin): get system date and convert it to a fixed-point timestamp
			timeval tv;
			gettimeofday(&tv, 0);
			ts.ts = (((u64)tv.tv_sec + JAN_1970) << 32)
			        + (u64)(tv.tv_usec * 1e-6 * TIMESTAMPS_PER_SECOND);
		} break;
	}

	//NOTE(martin): add a random fuzz between 0 and 1 times the system fuzz
	f64 fuzz = RandomU32()/(f64)(~(0UL)) * SYSTEM_FUZZ;
	fx_timediff tdfuzz = TimediffFromSeconds(fuzz);
	ts = TimestampAdd(ts, tdfuzz);
	//TODO(martin): ensure that we always return a value greater than the last value

	return(ts);
}


f64 ClockGetTime(clock_kind clock)
{
	switch(clock)
	{
		case SYS_CLOCK_MONOTONIC:
		{
			//NOTE(martin): compute monotonic offset and add it to bootup timestamp
			u64 noff = LinuxGetMonotonicNanoseconds();
			return((f64)noff * 1e-9);
		} break;

		case SYS_CLOCK_UPTIME:
		{
			//TODO(martin): maybe we should warn that this date is inconsistent after a sleep ?
			//NOTE(martin): compute uptime offset and add it to bootup timestamp
			u64 noff = LinuxGetUptimeNanoseconds();
			return((f64)noff * 1e-9);
		} break;

		case SYS_CLOCK_DATE:
		{
			//TODO(martin): maybe warn about precision loss ?
			//              could also change the epoch since we only promise to return a relative time
			//NOTE(martin): get system date and convert it to seconds
			timeval tv;
			gettimeofday(&tv, 0);
			return(((f64)tv.tv_sec + JAN_1970) + ((f64)tv.tv_usec * 1e-6));
		} break;
	}
	return(0);
}

void ClockSleepNanoseconds(u64 nanoseconds)
{
	timespec rqtp;
	rqtp.tv_sec = nanoseconds / 1000000000;
	rqtp.tv_nsec = nanoseconds - rqtp.tv_sec * 1000000000;
	nanosleep(&rqtp, 0);
}



////////////////////////////////////////////////////////////////////
//TODO: update these functions for various clocks besides monotonic
////////////////////////////////////////////////////////////////////
f64 ClockGetGranularity(clock_kind clock)
{
	u64 minDiff = ~(0ULL);

	const int GRANULARITY_NUM_ITERATION = 100000;
	for(int i=0; i<GRANULARITY_NUM_ITERATION; i++)
	{
		u64 a = LinuxGetMonotonicNanoseconds();
		u64 b = LinuxGetMonotonicNanoseconds();
		u64 diff = b - a;
		if(diff != 0 && diff < minDiff)
		{
			minDiff = diff;
		}
	}
	return(minDiff * 1e-9);
}

f64 ClockGetMeanReadTime(clock_kind clock)
{
	u64 start = LinuxGetMonotonicNanoseconds();
	fx_timestamp time;
	const int READ_NUM_ITERATION = 1000000;
	for(int i=0; i<READ_NUM_ITERATION; i++)
	{
		time = ClockGetTimestamp(SYS_CLOCK_MONOTONIC);
	}
	volatile u64 ts = time.ts;
	u64 end = LinuxGetMonotonicNanoseconds();
	f64 mean = (end - start)/(f64)READ_NUM_ITERATION;

	return(mean * 1e-9);
}

} // extern "C"
