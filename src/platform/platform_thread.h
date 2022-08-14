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

typedef struct platform_thread platform_thread;

typedef void* (*ThreadStartFunction)(void* userPointer);

platform_thread* ThreadCreate(ThreadStartFunction start, void* userPointer);
platform_thread* ThreadCreateWithName(ThreadStartFunction start, void* userPointer, const char* name);

const char* ThreadGetName(platform_thread* thread);

u64 ThreadSelfID();
u64 ThreadUniqueID(platform_thread* thread);

int ThreadSignal(platform_thread* thread, int sig);
void ThreadCancel(platform_thread* thread);
int ThreadJoin(platform_thread* thread, void** ret);
int ThreadDetach(platform_thread* thread);

//---------------------------------------------------------------
// Platform Mutex API
//---------------------------------------------------------------

typedef struct platform_mutex platform_mutex;

platform_mutex* MutexCreate();
int MutexDestroy(platform_mutex* mutex);
int MutexLock(platform_mutex* mutex);
int MutexUnlock(platform_mutex* mutex);


//---------------------------------------------------------------
// Lightweight ticket mutex API
//---------------------------------------------------------------

typedef struct ticket_spin_mutex
{
	volatile _Atomic(u64) nextTicket;
	volatile _Atomic(u64) serving;
} ticket_spin_mutex;

void TicketSpinMutexInit(ticket_spin_mutex* mutex);
void TicketSpinMutexLock(ticket_spin_mutex* mutex);
void TicketSpinMutexUnlock(ticket_spin_mutex* mutex);

//---------------------------------------------------------------
// Platform condition variable API
//---------------------------------------------------------------

typedef struct platform_condition platform_condition;

platform_condition* ConditionCreate();
int ConditionDestroy(platform_condition* cond);
int ConditionWait(platform_condition* cond, platform_mutex* mutex);
int ConditionTimedWait(platform_condition* cond, platform_mutex* mutex, f64 seconds);
int ConditionSignal(platform_condition* cond);
int ConditionBroadcast(platform_condition* cond);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif //__PLATFORM_THREAD_H_
