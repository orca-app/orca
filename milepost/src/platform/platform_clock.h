/************************************************************//**
*
*	@file: platform_clock.h
*	@author: Martin Fouilleul
*	@date: 07/03/2019
*	@revision:
*
*****************************************************************/
#ifndef __PLATFORM_CLOCK_H_
#define __PLATFORM_CLOCK_H_

#include"util/typedefs.h"
#include"platform.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum {
	MP_CLOCK_MONOTONIC, // clock that increment monotonically
    MP_CLOCK_UPTIME,    // clock that increment monotonically during uptime
	MP_CLOCK_DATE       // clock that is driven by the platform time
} mp_clock_kind;

MP_API void mp_clock_init(); // initialize the clock subsystem
MP_API u64  mp_get_timestamp(mp_clock_kind clock);
MP_API f64  mp_get_time(mp_clock_kind clock);
MP_API void mp_sleep_nanoseconds(u64 nanoseconds); // sleep for a given number of nanoseconds

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus


#endif //__PLATFORM_CLOCK_H_
