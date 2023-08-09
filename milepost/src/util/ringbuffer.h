/************************************************************//**
*
*	@file: ringbuffer.h
*	@author: Martin Fouilleul
*	@date: 31/07/2022
*	@revision:
*
*****************************************************************/
#ifndef __RINGBUFFER_H_
#define __RINGBUFFER_H_

#include"typedefs.h"
#include"atomic.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ringbuffer
{
	u64 mask;
	_Atomic(u64) readIndex;
	_Atomic(u64) writeIndex;
	u64 reserveIndex;

	u8* buffer;

} ringbuffer;

void ringbuffer_init(ringbuffer* ring, u8 capExp);
void ringbuffer_cleanup(ringbuffer* ring);
u64 ringbuffer_read_available(ringbuffer* ring);
u64 ringbuffer_write_available(ringbuffer* ring);
u64 ringbuffer_read(ringbuffer* ring, u64 size, u8* data);
u64 ringbuffer_write(ringbuffer* ring, u64 size, u8* data);
u64 ringbuffer_reserve(ringbuffer* ring, u64 size, u8* data);
void ringbuffer_commit(ringbuffer* ring);
void ringbuffer_rewind(ringbuffer* ring);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__RINGBUFFER_H_
