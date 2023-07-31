/************************************************************//**
*
*	@file: win32_thread.c
*	@author: Reuben Dunnington
*	@date: 7/30/2023
*	@revision:
*
*****************************************************************/

#include<processthreadsapi.h>
#include<synchapi.h>
#include<math.h> //INFINITY

#include"platform_thread.h"

struct mp_thread
{
	mp_thread_start_function start;
	HANDLE handle;
	DWORD threadId;
	void* userPointer;
	char name[MP_THREAD_NAME_MAX_SIZE];
};

static DWORD WINAPI mp_thread_bootstrap(LPVOID lpParameter)
{
	mp_thread* thread = (mp_thread*)lpParameter;
	i32 exitCode = thread->start(thread->userPointer);
	return(exitCode);
}

mp_thread* mp_thread_create_with_name(mp_thread_start_function start, void* userPointer, const char* name)
{
	mp_thread* thread = (mp_thread*)malloc(sizeof(mp_thread));
	thread->start = start;
	thread->handle = INVALID_HANDLE_VALUE;
	thread->userPointer = userPointer;
	if(name)
	{
		char* end = strncpy(thread->name, name, MP_THREAD_NAME_MAX_SIZE-1);
		*end = '\0';
	}
	else
	{
		thread->name[0] = '\0';
	}

	SECURITY_ATTRIBUTES childProcessSecurity = { 
		.nLength = sizeof(SECURITY_ATTRIBUTES),
		.bInheritHandle = false,
	};
	SIZE_T stackSize = 0; // uses process default
	DWORD flags = 0;
	DWORD threadId = 0;
	thread->handle = CreateThread(&childProcessSecurity, stackSize, mp_thread_bootstrap, thread, flags, &threadId);
	if (thread->handle == NULL) {
		free(thread);
		return(NULL);
	}

	thread->threadId = threadId;

	if (thread->name[0]) {
		wchar_t widename[MP_THREAD_NAME_MAX_SIZE];
		size_t length = mbstowcs(widename, thread->name, MP_THREAD_NAME_MAX_SIZE - 1);
		widename[length] = '\0';

		SetThreadDescription(thread->handle, widename);
	}

	return(thread);
}

mp_thread* mp_thread_create(mp_thread_start_function start, void* userPointer)
{
	return(mp_thread_create_with_name(start, userPointer, NULL));
}

const char* mp_thread_get_name(mp_thread* thread)
{
	return(thread->name);
}

u64 mp_thread_unique_id(mp_thread* thread)
{
	return(thread->threadId);
}

u64 mp_thread_self_id()
{
	return(GetCurrentThreadId());
}

int mp_thread_signal(mp_thread* thread, int sig)
{
	BOOL success = TerminateThread(thread->handle, (DWORD)sig);
	return(success ? 0 : -1);
}

int mp_thread_join(mp_thread* thread, i64* exitCode)
{
	DWORD result = WaitForSingleObject(thread->handle, INFINITE);
	if (result == WAIT_FAILED) {
		return(-1);
	}

	if (exitCode)
	{
		DWORD exitCodeWin32 = 0;
		if (GetExitCodeThread(thread->handle, &exitCodeWin32))
		{
			*exitCode = exitCodeWin32;
		}
	}

	free(thread);
	return(0);
}

int mp_thread_detach(mp_thread* thread)
{
	if (CloseHandle(thread->handle))
	{
		free(thread);
		return(0);
	}
	return(-1);
}


struct mp_mutex
{
	u64 owningThreadId;
	SRWLOCK lock;
};

mp_mutex* mp_mutex_create()
{
	mp_mutex* mutex = (mp_mutex*)malloc(sizeof(mp_mutex));
	mutex->owningThreadId = 0;
	InitializeSRWLock(&mutex->lock);
	return mutex;
}

int mp_mutex_destroy(mp_mutex* mutex)
{
	DEBUG_ASSERT(mutex->owningThreadId == 0);
	free(mutex);
	return(0);
}

int mp_mutex_lock(mp_mutex* mutex)
{
	DEBUG_ASSERT(mutex->owningThreadId == 0);
	AcquireSRWLockExclusive(&mutex->lock);
	return(0);
}

int mp_mutex_unlock(mp_mutex* mutex)
{
	DEBUG_ASSERT(mp_thread_self_id() == mutex->owningThreadId);
	ReleaseSRWLockExclusive(&mutex->lock);
	mutex->owningThreadId = 0;
	return(0);
}

// mp_ticket_spin_mutex has a mirrored implementation in posix_thread.c

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
	CONDITION_VARIABLE cond;
};

mp_condition* mp_condition_create()
{
	mp_condition* cond = (mp_condition*)malloc(sizeof(mp_condition));
	InitializeConditionVariable(&cond->cond);
	return cond;
}

int mp_condition_destroy(mp_condition* cond)
{
	free(cond);
	return(0);
}

int mp_condition_wait(mp_condition* cond, mp_mutex* mutex)
{
	return mp_condition_timedwait(cond, mutex, INFINITY);
}

int mp_condition_timedwait(mp_condition* cond, mp_mutex* mutex, f64 seconds)
{
	const f32 ms = (seconds == INFINITY) ? INFINITE : seconds * 1000;
	if (!SleepConditionVariableSRW(&cond->cond, &mutex->lock, ms, 0))
	{
		return(GetLastError());
	}
	return(0);
}

int mp_condition_signal(mp_condition* cond)
{
	WakeConditionVariable(&cond->cond);
	return(0);
}

int mp_condition_broadcast(mp_condition* cond)
{
	WakeAllConditionVariable(&cond->cond);
	return(0);
}
