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

struct mp_thread
{
	bool valid;
	pthread_t pthread;
	mp_thread_start_function start;
	void* userPointer;
	char name[MP_THREAD_NAME_MAX_SIZE];
};

static void* mp_thread_bootstrap(void* data)
{
	mp_thread* thread = (mp_thread*)data;
	if(strlen(thread->name))
	{
		pthread_setname_np(thread->name);
	}
	i32 exitCode = thread->start(thread->userPointer);
	return((void*)(ptrdiff_t)exitCode);
}

mp_thread* mp_thread_create_with_name(mp_thread_start_function start, void* userPointer, const char* name)
{
	mp_thread* thread = (mp_thread*)malloc(sizeof(mp_thread));
	if(!thread)
	{
		return(0);
	}

	if(name)
	{
		char* end = stpncpy(thread->name, name, MP_THREAD_NAME_MAX_SIZE-1);
		*end = '\0';
	}
	else
	{
		thread->name[0] = '\0';
	}
	thread->start = start;
	thread->userPointer = userPointer;

	if(pthread_create(&thread->pthread, 0, mp_thread_bootstrap, thread) != 0)
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

mp_thread* mp_thread_create(mp_thread_start_function start, void* userPointer)
{
	return(mp_thread_create_with_name(start, userPointer, 0));
}

const char* mp_thread_get_name(mp_thread* thread)
{
	return(thread->name);
}

u64 mp_thread_unique_id(mp_thread* thread)
{
	u64 id;
	pthread_threadid_np(thread->pthread, &id);
	return(id);
}

u64 mp_thread_self_id()
{
	pthread_t thread = pthread_self();
	u64 id;
	pthread_threadid_np(thread, &id);
	return(id);
}

int mp_thread_signal(mp_thread* thread, int sig)
{
	return(pthread_kill(thread->pthread, sig));
}

int mp_thread_join(mp_thread* thread, i64* exitCode)
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

int mp_thread_detach(mp_thread* thread)
{
	if(pthread_detach(thread->pthread))
	{
		return(-1);
	}
	free(thread);
	return(0);
}


struct mp_mutex
{
	pthread_mutex_t pmutex;
};

mp_mutex* mp_mutex_create()
{
	mp_mutex* mutex = (mp_mutex*)malloc(sizeof(mp_mutex));
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

int mp_mutex_destroy(mp_mutex* mutex)
{
	if(pthread_mutex_destroy(&mutex->pmutex) != 0)
	{
		return(-1);
	}
	free(mutex);
	return(0);
}

int mp_mutex_lock(mp_mutex* mutex)
{
	return(pthread_mutex_lock(&mutex->pmutex));
}

int mp_mutex_unlock(mp_mutex* mutex)
{
	return(pthread_mutex_unlock(&mutex->pmutex));
}

// mp_ticket_spin_mutex has a mirrored implementation in win32_thread.c

void mp_ticket_spin_mutex_init(mp_ticket_spin_mutex* mutex)
{
	mutex->nextTicket = 0;
	mutex->serving = 0;
}

void mp_ticket_spin_mutex_lock(mp_ticket_spin_mutex* mutex)
{
	u64 ticket = atomic_fetch_add(&mutex->nextTicket, 1ULL);
	while(ticket != mutex->serving); //spin
}

void mp_ticket_spin_mutex_unlock(mp_ticket_spin_mutex* mutex)
{
	atomic_fetch_add(&mutex->serving, 1ULL);
}


struct mp_condition
{
	pthread_cond_t pcond;
};

mp_condition* mp_condition_create()
{
	mp_condition* cond = (mp_condition*)malloc(sizeof(mp_condition));
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

int mp_condition_destroy(mp_condition* cond)
{
	if(pthread_cond_destroy(&cond->pcond) != 0)
	{
		return(-1);
	}
	free(cond);
	return(0);
}

int mp_condition_wait(mp_condition* cond, mp_mutex* mutex)
{
	return(pthread_cond_wait(&cond->pcond, &mutex->pmutex));
}

int mp_condition_timedwait(mp_condition* cond, mp_mutex* mutex, f64 seconds)
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

int mp_condition_signal(mp_condition* cond)
{
	return(pthread_cond_signal(&cond->pcond));
}

int mp_condition_broadcast(mp_condition* cond)
{
	return(pthread_cond_broadcast(&cond->pcond));
}
