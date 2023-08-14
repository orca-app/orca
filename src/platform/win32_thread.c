/************************************************************//**
*
*	@file: win32_thread.c
*	@author: Reuben Dunnington
*	@date: 7/30/2023
*
*****************************************************************/
#include<processthreadsapi.h>
#include<synchapi.h>
#include<math.h> //INFINITY
#include<winuser.h> // PostMessage

#include"platform_thread.h"

struct oc_thread
{
	oc_thread_start_function start;
	HANDLE handle;
	DWORD threadId;
	void* userPointer;
	oc_str8 name;
	char nameBuffer[OC_THREAD_NAME_MAX_SIZE];
};

static DWORD WINAPI oc_thread_bootstrap(LPVOID lpParameter)
{
	oc_thread* thread = (oc_thread*)lpParameter;
	i32 exitCode = thread->start(thread->userPointer);
	return(exitCode);
}

oc_thread* oc_thread_create_with_name(oc_thread_start_function start, void* userPointer, oc_str8 name)
{
	oc_thread* thread = (oc_thread*)malloc(sizeof(oc_thread));
	thread->start = start;
	thread->handle = INVALID_HANDLE_VALUE;
	thread->userPointer = userPointer;
	if(name.len && name.ptr)
	{
		strncpy(thread->nameBuffer, name.ptr, oc_min(name.len, OC_THREAD_NAME_MAX_SIZE-1));
		thread->nameBuffer[OC_THREAD_NAME_MAX_SIZE-1] = '\0';
		thread->name = OC_STR8(thread->nameBuffer);
	}
	else
	{
		thread->nameBuffer[0] = '\0';
		thread->name = oc_str8_from_buffer(0, thread->nameBuffer);
	}

	SECURITY_ATTRIBUTES childProcessSecurity = {
		.nLength = sizeof(SECURITY_ATTRIBUTES),
		.bInheritHandle = false,
	};
	SIZE_T stackSize = 0; // uses process default
	DWORD flags = 0;
	DWORD threadId = 0;
	thread->handle = CreateThread(&childProcessSecurity, stackSize, oc_thread_bootstrap, thread, flags, &threadId);
	if (thread->handle == NULL) {
		free(thread);
		return(NULL);
	}

	thread->threadId = threadId;

	if (thread->name.len) {
		wchar_t widename[OC_THREAD_NAME_MAX_SIZE];
		size_t length = mbstowcs(widename, thread->nameBuffer, OC_THREAD_NAME_MAX_SIZE - 1);
		widename[length] = '\0';

		SetThreadDescription(thread->handle, widename);
	}

	return(thread);
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
	return(thread->threadId);
}

u64 oc_thread_self_id()
{
	return(GetCurrentThreadId());
}

int oc_thread_signal(oc_thread* thread, int sig)
{
	BOOL success = TerminateThread(thread->handle, (DWORD)sig);
	return(success ? 0 : -1);
}

int oc_thread_join(oc_thread* thread, i64* exitCode)
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

int oc_thread_detach(oc_thread* thread)
{
	if (CloseHandle(thread->handle))
	{
		free(thread);
		return(0);
	}
	return(-1);
}


struct oc_mutex
{
	u64 owningThreadId;
	SRWLOCK lock;
};

oc_mutex* oc_mutex_create()
{
	oc_mutex* mutex = (oc_mutex*)malloc(sizeof(oc_mutex));
	mutex->owningThreadId = 0;
	InitializeSRWLock(&mutex->lock);
	return mutex;
}

int oc_mutex_destroy(oc_mutex* mutex)
{
	OC_DEBUG_ASSERT(mutex->owningThreadId == 0);
	free(mutex);
	return(0);
}

int oc_mutex_lock(oc_mutex* mutex)
{
	OC_DEBUG_ASSERT(mutex->owningThreadId == 0);
	AcquireSRWLockExclusive(&mutex->lock);
	return(0);
}

int oc_mutex_unlock(oc_mutex* mutex)
{
	OC_DEBUG_ASSERT(oc_thread_self_id() == mutex->owningThreadId);
	ReleaseSRWLockExclusive(&mutex->lock);
	mutex->owningThreadId = 0;
	return(0);
}

// oc_ticket has a mirrored implementation in posix_thread.c

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
	CONDITION_VARIABLE cond;
};

oc_condition* oc_condition_create()
{
	oc_condition* cond = (oc_condition*)malloc(sizeof(oc_condition));
	InitializeConditionVariable(&cond->cond);
	return cond;
}

int oc_condition_destroy(oc_condition* cond)
{
	free(cond);
	return(0);
}

int oc_condition_wait(oc_condition* cond, oc_mutex* mutex)
{
	return oc_condition_timedwait(cond, mutex, INFINITY);
}

int oc_condition_timedwait(oc_condition* cond, oc_mutex* mutex, f64 seconds)
{
	const f32 ms = (seconds == INFINITY) ? INFINITE : seconds * 1000;
	if (!SleepConditionVariableSRW(&cond->cond, &mutex->lock, ms, 0))
	{
		return(GetLastError());
	}
	return(0);
}

int oc_condition_signal(oc_condition* cond)
{
	WakeConditionVariable(&cond->cond);
	return(0);
}

int oc_condition_broadcast(oc_condition* cond)
{
	WakeAllConditionVariable(&cond->cond);
	return(0);
}
