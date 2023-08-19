/************************************************************/ /**
*
*	@file: platform_thread.h
*	@author: Martin Fouilleul
*	@date: 21/03/2019
*	@revision:
*
*****************************************************************/
#ifndef __PLATFORM_THREAD_H_
#define __PLATFORM_THREAD_H_

#include "util/strings.h"

#ifdef __cplusplus
    #include <atomic>
    #define _Atomic(T) std::atomic<T>
#else
    #include <stdatomic.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    //---------------------------------------------------------------
    // Platform Thread API
    //---------------------------------------------------------------

    enum
    {
        OC_THREAD_NAME_MAX_SIZE = 64, // including null terminator
    };

    typedef struct oc_thread oc_thread;

    typedef i32 (*oc_thread_start_function)(void* userPointer);

    ORCA_API oc_thread* oc_thread_create(oc_thread_start_function start, void* userPointer);
    ORCA_API oc_thread* oc_thread_create_with_name(oc_thread_start_function start, void* userPointer, oc_str8 name);
    ORCA_API oc_str8 oc_thread_get_name(oc_thread* thread);
    ORCA_API u64 oc_thread_unique_id(oc_thread* thread);
    ORCA_API u64 oc_thread_self_id();
    ORCA_API int oc_thread_signal(oc_thread* thread, int sig);
    ORCA_API int oc_thread_join(oc_thread* thread, i64* exitCode);
    ORCA_API int oc_thread_detach(oc_thread* thread);

    //---------------------------------------------------------------
    // Platform Mutex API
    //---------------------------------------------------------------

    typedef struct oc_mutex oc_mutex;

    ORCA_API oc_mutex* oc_mutex_create();
    ORCA_API int oc_mutex_destroy(oc_mutex* mutex);
    ORCA_API int oc_mutex_lock(oc_mutex* mutex);
    ORCA_API int oc_mutex_unlock(oc_mutex* mutex);

    //---------------------------------------------------------------
    // Lightweight ticket mutex API
    //---------------------------------------------------------------

    typedef struct oc_ticket
    {
        volatile _Atomic(u64) nextTicket;
        volatile _Atomic(u64) serving;
    } oc_ticket;

    ORCA_API void oc_ticket_init(oc_ticket* mutex);
    ORCA_API void oc_ticket_lock(oc_ticket* mutex);
    ORCA_API void oc_ticket_unlock(oc_ticket* mutex);

    //---------------------------------------------------------------
    // Platform condition variable API
    //---------------------------------------------------------------

    typedef struct oc_condition oc_condition;

    ORCA_API oc_condition* oc_condition_create();
    ORCA_API int oc_condition_destroy(oc_condition* cond);
    ORCA_API int oc_condition_wait(oc_condition* cond, oc_mutex* mutex);
    ORCA_API int oc_condition_timedwait(oc_condition* cond, oc_mutex* mutex, f64 seconds);
    ORCA_API int oc_condition_signal(oc_condition* cond);
    ORCA_API int oc_condition_broadcast(oc_condition* cond);

    //---------------------------------------------------------------
    // Putting threads to sleep
    //---------------------------------------------------------------
    ORCA_API void oc_sleep_nano(u64 nanoseconds); // sleep for a given number of nanoseconds

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif //__PLATFORM_THREAD_H_
