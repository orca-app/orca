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
#include<unistd.h>	// nanosleep()

#include"platform_thread.h"

struct oc_thread
{
	bool valid;
	pthread_t pthread;
	oc_thread_start_function start;
	void* userPointer;
	oc_str8 name;
	char nameBuffer[OC_THREAD_NAME_MAX_SIZE];
};

static void* oc_thread_bootstrap(void* data)
{
	oc_thread* thread = (oc_thread*)data;
	if(thread->name.len)
	{
		pthread_setname_np(thread->nameBuffer);
	}
	i32 exitCode = thread->start(thread->userPointer);
	return((void*)(ptrdiff_t)exitCode);
}

oc_thread* oc_thread_create_with_name(oc_thread_start_function start, void* userPointer, oc_str8 name)
{
	oc_thread* thread = (oc_thread*)malloc(sizeof(oc_thread));
	if(!thread)
	{
		return(0);
	}

	if(name.len && name.ptr)
	{
		char* end = stpncpy(thread->nameBuffer, name.ptr, oc_min(name.len, OC_THREAD_NAME_MAX_SIZE-1));
		*end = '\0';
		thread->name = oc_str8_from_buffer(end - thread->nameBuffer, thread->nameBuffer);
	}
	else
	{
		thread->nameBuffer[0] = '\0';
		thread->name = oc_str8_from_buffer(0, thread->nameBuffer);
	}
	thread->start = start;
	thread->userPointer = userPointer;

	if(pthread_create(&thread->pthread, 0, oc_thread_bootstrap, thread) != 0)
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

oc_thread* oc_thread_create(oc_thread_start_function start, void* userPointer)
{
	return(oc_thread_create_with_name(start, userPointer, (oc_str8){0}));
}

oc_str8 oc_thread_get_name(oc_thread* thread)
{
	return(thread->name);
}

u64 oc_thread_unique_id(oc_thread* thread)
{
	u64 id;
	pthread_threadid_np(thread->pthread, &id);
	return(id);
}

u64 oc_thread_self_id()
{
	pthread_t thread = pthread_self();
	u64 id;
	pthread_threadid_np(thread, &id);
	return(id);
}

int oc_thread_signal(oc_thread* thread, int sig)
{
	return(pthread_kill(thread->pthread, sig));
}

int oc_thread_join(oc_thread* thread, i64* exitCode)
{
	void* ret;
	if(pthread_join(thread->pthread, &ret))
	{
		return(-1);
	}
	free(thread);

	if (exitCode)
	{
		*exitCode = (off_t)ret;
	}
	return(0);
}

int oc_thread_detach(oc_thread* thread)
{
	if(pthread_detach(thread->pthread))
	{
		return(-1);
	}
	free(thread);
	return(0);
}


struct oc_mutex
{
	pthread_mutex_t pmutex;
};

oc_mutex* oc_mutex_create()
{
	oc_mutex* mutex = (oc_mutex*)malloc(sizeof(oc_mutex));
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

int oc_mutex_destroy(oc_mutex* mutex)
{
	if(pthread_mutex_destroy(&mutex->pmutex) != 0)
	{
		return(-1);
	}
	free(mutex);
	return(0);
}

int oc_mutex_lock(oc_mutex* mutex)
{
	return(pthread_mutex_lock(&mutex->pmutex));
}

int oc_mutex_unlock(oc_mutex* mutex)
{
	return(pthread_mutex_unlock(&mutex->pmutex));
}

// oc_ticket has a mirrored implementation in win32_thread.c

void oc_ticket_init(oc_ticket* mutex)
{
	mutex->nextTicket = 0;
	mutex->serving = 0;
}

void oc_ticket_lock(oc_ticket* mutex)
{
	u64 ticket = atomic_fetch_add(&mutex->nextTicket, 1ULL);
	while(ticket != mutex->serving); //spin
}

void oc_ticket_unlock(oc_ticket* mutex)
{
	atomic_fetch_add(&mutex->serving, 1ULL);
}


struct oc_condition
{
	pthread_cond_t pcond;
};

oc_condition* oc_condition_create()
{
	oc_condition* cond = (oc_condition*)malloc(sizeof(oc_condition));
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

int oc_condition_destroy(oc_condition* cond)
{
	if(pthread_cond_destroy(&cond->pcond) != 0)
	{
		return(-1);
	}
	free(cond);
	return(0);
}

int oc_condition_wait(oc_condition* cond, oc_mutex* mutex)
{
	return(pthread_cond_wait(&cond->pcond, &mutex->pmutex));
}

int oc_condition_timedwait(oc_condition* cond, oc_mutex* mutex, f64 seconds)
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

int oc_condition_signal(oc_condition* cond)
{
	return(pthread_cond_signal(&cond->pcond));
}

int oc_condition_broadcast(oc_condition* cond)
{
	return(pthread_cond_broadcast(&cond->pcond));
}


void oc_sleep_nano(u64 nanoseconds)
{
	timespec rqtp;
	rqtp.tv_sec = nanoseconds / 1000000000;
	rqtp.tv_nsec = nanoseconds - rqtp.tv_sec * 1000000000;
	nanosleep(&rqtp, 0);
}
