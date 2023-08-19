/************************************************************/ /**
*
*	@file: ringbuffer.h
*	@author: Martin Fouilleul
*	@date: 31/07/2022
*	@revision:
*
*****************************************************************/
#ifndef __RINGBUFFER_H_
#define __RINGBUFFER_H_

#include <stdatomic.h>

#include "typedefs.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct ringbuffer
    {
        u64 mask;
        _Atomic(u64) readIndex;
        _Atomic(u64) writeIndex;
        u64 reserveIndex;

        u8* buffer;

    } oc_ringbuffer;

    void oc_ringbuffer_init(oc_ringbuffer* ring, u8 capExp);
    void oc_ringbuffer_cleanup(oc_ringbuffer* ring);
    u64 oc_ringbuffer_read_available(oc_ringbuffer* ring);
    u64 oc_ringbuffer_write_available(oc_ringbuffer* ring);
    u64 oc_ringbuffer_read(oc_ringbuffer* ring, u64 size, u8* data);
    u64 oc_ringbuffer_write(oc_ringbuffer* ring, u64 size, u8* data);
    u64 oc_ringbuffer_reserve(oc_ringbuffer* ring, u64 size, u8* data);
    void oc_ringbuffer_commit(oc_ringbuffer* ring);
    void oc_ringbuffer_rewind(oc_ringbuffer* ring);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__RINGBUFFER_H_
