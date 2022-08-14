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
	u32 cap = 1<<capExp;
	ring->mask = cap - 1;
	ring->readIndex = 0;
	ring->writeIndex = 0;
	ring->buffer = (u8*)malloc(cap);
}

void ringbuffer_cleanup(ringbuffer* ring)
{
	free(ring->buffer);
}

u32 ringbuffer_read_available(ringbuffer* ring)
{
	return((ring->writeIndex - ring->readIndex) & ring->mask);
}

u32 ringbuffer_write_available(ringbuffer* ring)
{
	//NOTE(martin): we keep one sentinel byte between write index and read index,
	//              when the buffer is full, to avoid overrunning read index.
	//              Hence, available write space is size - 1 - available read space.
	return(ring->mask - ringbuffer_read_available(ring));
}

u32 ringbuffer_write(ringbuffer* ring, u32 size, u8* data)
{
	u32 read = ring->readIndex;
	u32 write = ring->writeIndex;

	u32 writeAvailable = ringbuffer_write_available(ring);
	if(size > writeAvailable)
	{
		DEBUG_ASSERT("not enough space available");
		size = writeAvailable;
	}

	if(read <= write)
	{
		u32 copyCount = minimum(size, ring->mask + 1 - write);
		memcpy(ring->buffer + write, data, copyCount);

		data += copyCount;
		copyCount = size - copyCount;
		memcpy(ring->buffer, data, copyCount);
	}
	else
	{
		memcpy(ring->buffer + write, data, size);
	}
	ring->writeIndex = (write + size) & ring->mask;
	return(size);
}

u32 ringbuffer_read(ringbuffer* ring, u32 size, u8* data)
{
	u32 read = ring->readIndex;
	u32 write = ring->writeIndex;

	u32 readAvailable = ringbuffer_read_available(ring);
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
		u32 copyCount = minimum(size, ring->mask + 1 - read);
		memcpy(data, ring->buffer + read, copyCount);

		data += copyCount;
		copyCount = size - copyCount;
		memcpy(data, ring->buffer, copyCount);
	}
	ring->readIndex = (read + size) & ring->mask;
	return(size);
}
