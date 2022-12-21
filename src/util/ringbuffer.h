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

#if defined(_MSC_VER) && defined(__STDC_NO_ATOMICS__)
	#define _Atomic(t) volatile t
#else
	#include<stdatomic.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef struct ringbuffer
{
	u32 mask;
	_Atomic(u32) readIndex;
	_Atomic(u32) writeIndex;

	u8* buffer;

} ringbuffer;

void ringbuffer_init(ringbuffer* ring, u8 capExp);
void ringbuffer_cleanup(ringbuffer* ring);
u32 ringbuffer_read_available(ringbuffer* ring);
u32 ringbuffer_write_available(ringbuffer* ring);
u32 ringbuffer_write(ringbuffer* ring, u32 size, u8* data);
u32 ringbuffer_read(ringbuffer* ring, u32 size, u8* data);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__RINGBUFFER_H_
