/************************************************************//**
*
*	@file: hash.h
*	@author: Martin Fouilleul
*	@date: 08/08/2022
*	@revision:
*
*****************************************************************/
#ifndef __HASH_H_
#define __HASH_H_

#include"typedefs.h"
#include"strings.h"

#ifdef __cplusplus
extern "C" {
#endif

MP_API u64 mp_hash_aes_u64(u64 x);
MP_API u64 mp_hash_aes_u64_x2(u64 x, u64 y);
MP_API u64 mp_hash_aes_string(str8 string);
MP_API u64 mp_hash_aes_string_seed(str8 string, u64 seed);

#ifdef __cplusplus
} // extern "C"
#endif


#endif //__HASH_H_
