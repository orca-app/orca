/************************************************************//**
*
*	@file: ringbuffer.cpp
*	@author: Martin Fouilleul
*	@date: 31/07/2022
*	@revision:
*
*****************************************************************/
#include<stdlib.h> // malloc, free
#include"ringbuffer.h"

void ringbuffer_init(ringbuffer* ring, u8 capExp)
{
	u64 cap = 1<<capExp;
	ring->mask = cap - 1;
	ring->readIndex = 0;
	ring->reserveIndex = 0;
	ring->writeIndex = 0;
	ring->buffer = (u8*)malloc(cap);
}

void ringbuffer_cleanup(ringbuffer* ring)
{
	free(ring->buffer);
}

u64 ringbuffer_read_available(ringbuffer* ring)
{
	return((ring->writeIndex - ring->readIndex) & ring->mask);
}

u64 ringbuffer_write_available(ringbuffer* ring)
{
	//NOTE(martin): we keep one sentinel byte between write index and read index,
	//              when the buffer is full, to avoid overrunning read index.
	return(((ring->readIndex - ring->reserveIndex) & ring->mask) - 1);
}

u64 ringbuffer_read(ringbuffer* ring, u64 size, u8* data)
{
	u64 read = ring->readIndex;
	u64 write = ring->writeIndex;

	u64 readAvailable = ringbuffer_read_available(ring);
	if(size > readAvailable)
	{
		size = readAvailable;
	}

	if(read <= write)
	{
		memcpy(data, ring->buffer + read, size);
	}
	else
	{
		u64 copyCount = minimum(size, ring->mask + 1 - read);
		memcpy(data, ring->buffer + read, copyCount);

		data += copyCount;
		copyCount = size - copyCount;
		memcpy(data, ring->buffer, copyCount);
	}
	ring->readIndex = (read + size) & ring->mask;
	return(size);
}

u64 ringbuffer_reserve(ringbuffer* ring, u64 size, u8* data)
{
	u64 read = ring->readIndex;
	u64 reserve = ring->reserveIndex;

	u64 writeAvailable = ringbuffer_write_available(ring);
	if(size > writeAvailable)
	{
		DEBUG_ASSERT("not enough space available");
		size = writeAvailable;
	}

	if(read <= reserve)
	{
		u64 copyCount = minimum(size, ring->mask + 1 - reserve);
		memcpy(ring->buffer + reserve, data, copyCount);

		data += copyCount;
		copyCount = size - copyCount;
		memcpy(ring->buffer, data, copyCount);
	}
	else
	{
		memcpy(ring->buffer + reserve, data, size);
	}
	ring->reserveIndex = (reserve + size) & ring->mask;
	return(size);
}

u64 ringbuffer_write(ringbuffer* ring, u64 size, u8* data)
{
	ringbuffer_commit(ring);
	u64 res = ringbuffer_reserve(ring, size, data);
	ringbuffer_commit(ring);
	return(res);
}

void ringbuffer_commit(ringbuffer* ring)
{
	ring->writeIndex = ring->reserveIndex;
}

void ringbuffer_rewind(ringbuffer* ring)
{
	ring->reserveIndex = ring->writeIndex;
}
