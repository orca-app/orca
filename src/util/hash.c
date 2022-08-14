/************************************************************//**
*
*	@file: hash.cpp
*	@author: Martin Fouilleul
*	@date: 08/08/2022
*	@revision:
*
*****************************************************************/
#include<immintrin.h>
#include"hash.h"

u64 mp_hash_aes_u64(u64 x)
{
	u8 seed[16] = {
    	0xaa, 0x9b, 0xbd, 0xb8,
    	0xa1, 0x98, 0xac, 0x3f,
    	0x1f, 0x94, 0x07, 0xb3,
    	0x8c, 0x27, 0x93, 0x69 };

	__m128i hash = _mm_set_epi64x(0L, x);
	__m128i key = _mm_loadu_si128((__m128i*)seed);
	hash = _mm_aesdec_si128(hash, key);
	hash = _mm_aesdec_si128(hash, key);
	u64 result = _mm_extract_epi64(hash, 0);

	return(result);
}

u64 mp_hash_aes_u64_x2(u64 x, u64 y)
{
	u8 seed[16] = {
    	0xaa, 0x9b, 0xbd, 0xb8,
    	0xa1, 0x98, 0xac, 0x3f,
    	0x1f, 0x94, 0x07, 0xb3,
    	0x8c, 0x27, 0x93, 0x69 };

	__m128i hash = _mm_set_epi64x(x, y);
	__m128i key = _mm_loadu_si128((__m128i*)seed);
	hash = _mm_aesdec_si128(hash, key);
	hash = _mm_aesdec_si128(hash, key);
	u64 result = _mm_extract_epi64(hash, 0);

	return(result);
}

u64 mp_hash_aes_string(str8 string)
{
	u8 seed[16] = {
    	0xaa, 0x9b, 0xbd, 0xb8,
    	0xa1, 0x98, 0xac, 0x3f,
    	0x1f, 0x94, 0x07, 0xb3,
    	0x8c, 0x27, 0x93, 0x69 };

	__m128i hash = _mm_loadu_si128((__m128i*)seed);

	u64 chunkCount = string.len / 16;
	char* at = string.ptr;

	while(chunkCount--)
	{
		__m128i in = _mm_loadu_si128((__m128i*)at);
		at += 16;

		hash = _mm_xor_si128(hash, in);
		hash = _mm_aesdec_si128(hash, _mm_setzero_si128());
		hash = _mm_aesdec_si128(hash, _mm_setzero_si128());
		hash = _mm_aesdec_si128(hash, _mm_setzero_si128());
		hash = _mm_aesdec_si128(hash, _mm_setzero_si128());
	}

	u64 restCount = string.len % 16;
	char tmp[16];
	memset(tmp, 0, 16);
	memmove(tmp, at, restCount);

	__m128i in = _mm_loadu_si128((__m128i*)tmp);
	hash = _mm_xor_si128(hash, in);
	hash = _mm_aesdec_si128(hash, _mm_setzero_si128());
	hash = _mm_aesdec_si128(hash, _mm_setzero_si128());
	hash = _mm_aesdec_si128(hash, _mm_setzero_si128());
	hash = _mm_aesdec_si128(hash, _mm_setzero_si128());

	u64 result = _mm_extract_epi64(hash, 0);
	return(result);
}

u64 mp_hash_aes_string_seed(str8 string, u64 seed)
{
	u8 seed16[16];
	memcpy(seed16, &seed, 8);
	memcpy(seed16+8, &seed, 8);

	__m128i hash = _mm_loadu_si64(&seed16);

	u64 chunkCount = string.len / 16;
	char* at = string.ptr;

	while(chunkCount--)
	{
		__m128i in = _mm_loadu_si128((__m128i*)at);
		at += 16;

		hash = _mm_xor_si128(hash, in);
		hash = _mm_aesdec_si128(hash, _mm_setzero_si128());
		hash = _mm_aesdec_si128(hash, _mm_setzero_si128());
		hash = _mm_aesdec_si128(hash, _mm_setzero_si128());
		hash = _mm_aesdec_si128(hash, _mm_setzero_si128());
	}

	u64 restCount = string.len % 16;
	char tmp[16];
	memset(tmp, 0, 16);
	memmove(tmp, at, restCount);

	__m128i in = _mm_loadu_si128((__m128i*)tmp);
	hash = _mm_xor_si128(hash, in);
	hash = _mm_aesdec_si128(hash, _mm_setzero_si128());
	hash = _mm_aesdec_si128(hash, _mm_setzero_si128());
	hash = _mm_aesdec_si128(hash, _mm_setzero_si128());
	hash = _mm_aesdec_si128(hash, _mm_setzero_si128());

	u64 result = _mm_extract_epi64(hash, 0);
	return(result);
}
