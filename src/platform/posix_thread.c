/************************************************************//**
*
*	@file: posix_thread.c
*	@author: Martin Fouilleul
*	@date: 21/03/2019
*	@revision:
*
*****************************************************************/
#include<stdlib.h>
#include<pthread.h>
#include<signal.h> //needed for pthread_kill() on linux
#include<string.h>
#include<sys/time.h>

#include"platform_thread.h"

const u32 PLATFORM_THREAD_NAME_MAX_SIZE = 64; // including null terminator

struct platform_thread
{
	bool valid;
	pthread_t pthread;
	ThreadStartFunction start;
	void* userPointer;
	char name[PLATFORM_THREAD_NAME_MAX_SIZE];
};

void* platform_thread_bootstrap(void* data)
{
	platform_thread* thread = (platform_thread*)data;
	if(strlen(thread->name))
	{
		pthread_setname_np(thread->name);
	}
	return(thread->start(thread->userPointer));
}

platform_thread* ThreadCreateWithName(ThreadStartFunction start, void* userPointer, const char* name)
{
	platform_thread* thread = (platform_thread*)malloc(sizeof(platform_thread));
	if(!thread)
	{
		return(0);
	}

	if(name)
	{
		char* end = stpncpy(thread->name, name, PLATFORM_THREAD_NAME_MAX_SIZE-1);
		*end = '\0';
	}
	else
	{
		thread->name[0] = '\0';
	}
	thread->start = start;
	thread->userPointer = userPointer;

	if(pthread_create(&thread->pthread, 0, platform_thread_bootstrap, thread) != 0)
	{
		free(thread);
		return(0);
	}
	else
	{
		thread->valid = true;
		return(thread);
	}
}

platform_thread* ThreadCreate(ThreadStartFunction start, void* userPointer)
{
	return(ThreadCreateWithName(start, userPointer, 0));
}

void ThreadCancel(platform_thread* thread)
{
	pthread_cancel(thread->pthread);
}

const char* ThreadGetName(platform_thread* thread)
{
	return(thread->name);
}


u64 ThreadUniqueID(platform_thread* thread)
{
	u64 id;
	pthread_threadid_np(thread->pthread, &id);
	return(id);
}

u64 ThreadSelfID()
{
	pthread_t thread = pthread_self();
	u64 id;
	pthread_threadid_np(thread, &id);
	return(id);
}

int ThreadSignal(platform_thread* thread, int sig)
{
	return(pthread_kill(thread->pthread, sig));
}

int ThreadJoin(platform_thread* thread, void** ret)
{
	if(pthread_join(thread->pthread, ret))
	{
		return(-1);
	}
	free(thread);
	return(0);
}

int ThreadDetach(platform_thread* thread)
{
	if(pthread_detach(thread->pthread))
	{
		return(-1);
	}
	free(thread);
	return(0);
}


struct platform_mutex
{
	pthread_mutex_t pmutex;
};

platform_mutex* MutexCreate()
{
	platform_mutex* mutex = (platform_mutex*)malloc(sizeof(platform_mutex));
	if(!mutex)
	{
		return(0);
	}
	if(pthread_mutex_init(&mutex->pmutex, 0) != 0)
	{
		free(mutex);
		return(0);
	}
	return(mutex);
}
int MutexDestroy(platform_mutex* mutex)
{
	if(pthread_mutex_destroy(&mutex->pmutex) != 0)
	{
		return(-1);
	}
	free(mutex);
	return(0);
}

int MutexLock(platform_mutex* mutex)
{
	return(pthread_mutex_lock(&mutex->pmutex));
}

int MutexUnlock(platform_mutex* mutex)
{
	return(pthread_mutex_unlock(&mutex->pmutex));
}

void TicketSpinMutexInit(ticket_spin_mutex* mutex)
{
	mutex->nextTicket = 0;
	mutex->serving = 0;
}

void TicketSpinMutexLock(ticket_spin_mutex* mutex)
{
	u64 ticket = atomic_fetch_add(&mutex->nextTicket, 1ULL);
	while(ticket != mutex->serving); //spin
}

void TicketSpinMutexUnlock(ticket_spin_mutex* mutex)
{
	atomic_fetch_add(&mutex->serving, 1ULL);
}

struct platform_condition
{
	pthread_cond_t pcond;
};

platform_condition* ConditionCreate()
{
	platform_condition* cond = (platform_condition*)malloc(sizeof(platform_condition));
	if(!cond)
	{
		return(0);
	}
	if(pthread_cond_init(&cond->pcond, 0) != 0)
	{
		free(cond);
		return(0);
	}
	return(cond);
}
int ConditionDestroy(platform_condition* cond)
{
	if(pthread_cond_destroy(&cond->pcond) != 0)
	{
		return(-1);
	}
	free(cond);
	return(0);
}
int ConditionWait(platform_condition* cond, platform_mutex* mutex)
{
	return(pthread_cond_wait(&cond->pcond, &mutex->pmutex));
}

int ConditionTimedWait(platform_condition* cond, platform_mutex* mutex, f64 seconds)
{
	struct timeval tv;
	gettimeofday(&tv, 0);

	i64 iSeconds = (i64)seconds;
	f64 fracSeconds = seconds - (f64)iSeconds;

	struct timespec ts;
	ts.tv_sec = tv.tv_sec + iSeconds;
	ts.tv_nsec = tv.tv_usec * 1000 + (i32)(fracSeconds*1e9);
	ts.tv_sec += ts.tv_nsec / 1000000000;
	ts.tv_nsec = ts.tv_nsec % 1000000000;

	return(pthread_cond_timedwait(&cond->pcond, &mutex->pmutex, &ts));
}

int ConditionSignal(platform_condition* cond)
{
	return(pthread_cond_signal(&cond->pcond));
}

int ConditionBroadcast(platform_condition* cond)
{
	return(pthread_cond_broadcast(&cond->pcond));
}
