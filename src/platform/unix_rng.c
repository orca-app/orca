/************************************************************//**
*
*	@file: unix_rng.c
*	@author: Martin Fouilleul
*	@date: 06/03/2020
*	@revision:
*
*****************************************************************/

#include<stdio.h>
#include<stdlib.h>

#include"platform_log.h"
#include"typedefs.h"

int RandomSeedFromDevice()
{
	FILE* urandom = fopen("/dev/urandom", "r");
	if(!urandom)
	{
		log_error("can't open /dev/urandom\n");
		return(-1);
	}

	union
	{
		u32 u;
		char buff[4];
	} seed;

	int size = fread(seed.buff, 1, 4, urandom);
	if(size != 4)
	{
		log_error("couldn't read from /dev/urandom\n");
		return(-1);
	}

	fclose(urandom);
	srandom(seed.u);
	return(0);
}

u32 RandomU32()
{
	u32 u1 = (u32)random();
	u32 u2 = (u32)random();
	return((u1<<1) | (u2 & 0x01));
}

u64 RandomU64()
{
	u64 u1 = (u64)random();
	u64 u2 = (u64)random();
	u64 u3 = (u64)random();
	return((u1<<33) | (u2<<2) | (u3 & 0x03));
}
