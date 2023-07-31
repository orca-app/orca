/************************************************************//**
*
*	@file: platform_thread.h
*	@author: Martin Fouilleul
*	@date: 21/03/2019
*	@revision:
*
*****************************************************************/
#ifndef __PLATFORM_THREAD_H_
#define __PLATFORM_THREAD_H_


#ifdef __cplusplus
	#include<atomic>
	#define _Atomic(T) std::atomic<T>
#else
	#include<stdatomic.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//---------------------------------------------------------------
// Platform Thread API
//---------------------------------------------------------------

enum
{
	MP_THREAD_NAME_MAX_SIZE = 64, // including null terminator
};

typedef struct mp_thread mp_thread;

typedef void* (*mp_thread_start_function)(void* userPointer);

mp_thread* mp_thread_create(mp_thread_start_function start, void* userPointer);
mp_thread* mp_thread_create_with_name(mp_thread_start_function start, void* userPointer, const char* name);

const char* mp_thread_get_name(mp_thread* thread);

u64 mp_thread_unique_id(mp_thread* thread);
u64 mp_thread_self_id();

int mp_thread_signal(mp_thread* thread, int sig);
int mp_thread_join(mp_thread* thread, void** ret);
int mp_thread_detach(mp_thread* thread);

//---------------------------------------------------------------
// Platform Mutex API
//---------------------------------------------------------------

typedef struct mp_mutex mp_mutex;

mp_mutex* mp_mutex_create();
int mp_mutex_destroy(mp_mutex* mutex);
int mp_mutex_lock(mp_mutex* mutex);
int mp_mutex_unlock(mp_mutex* mutex);


//---------------------------------------------------------------
// Lightweight ticket mutex API
//---------------------------------------------------------------

typedef struct mp_ticket_spin_mutex
{
	volatile _Atomic(u64) nextTicket;
	volatile _Atomic(u64) serving;
} mp_ticket_spin_mutex;

void mp_ticket_spin_mutex_init(mp_ticket_spin_mutex* mutex);
void mp_ticket_spin_mutex_lock(mp_ticket_spin_mutex* mutex);
void mp_ticket_spin_mutex_unlock(mp_ticket_spin_mutex* mutex);

//---------------------------------------------------------------
// Platform condition variable API
//---------------------------------------------------------------

typedef struct mp_condition mp_condition;

mp_condition* mp_condition_create();
int mp_condition_destroy(mp_condition* cond);
int mp_condition_wait(mp_condition* cond, mp_mutex* mutex);
int mp_condition_timedwait(mp_condition* cond, mp_mutex* mutex, f64 seconds);
int mp_condition_signal(mp_condition* cond);
int mp_condition_broadcast(mp_condition* cond);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif //__PLATFORM_THREAD_H_
