/************************************************************//**
*
*	@file: platform_rng.h
*	@author: Martin Fouilleul
*	@date: 06/03/2020
*	@revision:
*
*****************************************************************/
#ifndef __PLATFORM_RANDOM_H_
#define __PLATFORM_RANDOM_H_

#include"typedefs.h"

int RandomSeedFromDevice();
u32 RandomU32();
u64 RandomU64();

#endif //__PLATFORM_RANDOM_H_
