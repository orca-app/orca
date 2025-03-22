/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "hash.h"
#include "platform/platform.h"

//xxhash64, copy-pasted from https://github.com/demetri/scribbles/blob/master/hashing/ub_aware_hash_functions.c
// Thanks to Demetri Spanos

uint64_t xxh_64(const void* key, int len, uint64_t h)
{
    // primes used in mul-rot updates
    uint64_t p1 = 0x9e3779b185ebca87, p2 = 0xc2b2ae3d27d4eb4f,
             p3 = 0x165667b19e3779f9, p4 = 0x85ebca77c2b2ae63, p5 = 0x27d4eb2f165667c5;

    // inital 32-byte (4x8) wide hash state
    uint64_t s[4] = { h + p1 + p2, h + p2, h, h - p1 };

    // bulk work: process all 32 byte blocks
    for(int i = 0; i < (len / 32); i++)
    {
        uint64_t b[4];
        memcpy(b, key + 4 * i, sizeof(b));

        for(int j = 0; j < 4; j++)
            b[j] = b[j] * p2 + s[j];
        for(int j = 0; j < 4; j++)
            s[j] = ((b[j] << 31) | (b[j] >> 33)) * p1;
    }

    // mix 32-byte state down to 8-byte state, initalize to value for short keys
    uint64_t s64 = (s[2] + p5);
    if(len >= 32)
    {
        s64 = ((s[0] << 1) | (s[0] >> 63)) + ((s[1] << 7) | (s[1] >> 57)) + ((s[2] << 12) | (s[2] >> 52)) + ((s[3] << 18) | (s[3] >> 46));
        for(int i = 0; i < 4; i++)
        {
            uint64_t ps = (((s[i] * p2) << 31) | ((s[i] * p2) >> 33)) * p1;
            s64 = (s64 ^ ps) * p1 + p4;
        }
    }
    s64 += len;

    // up to 31 bytes remain, process 0-3 8 byte blocks
    uint8_t* tail = (uint8_t*)(key + (len / 32) * 32);
    for(int i = 0; i < (len & 31) / 8; i++, tail += 8)
    {
        uint64_t b;
        memcpy(&b, tail, sizeof(uint64_t));

        b *= p2;
        b = (((b << 31) | (b >> 33)) * p1) ^ s64;
        s64 = ((b << 27) | (b >> 37)) * p1 + p4;
    }

    // up to 7 bytes remain, process 0-1 4 byte block
    for(int i = 0; i < (len & 7) / 4; i++, tail += 4)
    {
        uint32_t b32;
        memcpy(&b32, tail, sizeof(b32));
        uint64_t b = b32;

        b = (s64 ^ b) * p1;
        s64 = ((b << 23) | (b >> 41)) * p2 + p3;
    }

    // up to 3 bytes remain, process 0-3 1 byte blocks
    for(int i = 0; i < (len & 3); i++, tail++)
    {
        uint64_t b = s64 ^ (*tail) * p5;
        s64 = ((b << 11) | (b >> 53)) * p1;
    }

    // finalization mix
    s64 = (s64 ^ (s64 >> 33)) * p2;
    s64 = (s64 ^ (s64 >> 29)) * p3;
    return (s64 ^ (s64 >> 32));
}

u64 oc_hash_xx64_string_seed(oc_str8 string, u64 seed)
{
    return (xxh_64(string.ptr, string.len, seed));
}

u64 oc_hash_xx64_string(oc_str8 string)
{
    return (xxh_64(string.ptr, string.len, 0));
}

#if 0 //NOTE(martin): keep that here cause we could want to use them when aes is available, but we don't for now
    #if OC_ARCH_X64
        #include <immintrin.h>

u64 oc_hash_aes_u64(u64 x)
{
    u8 seed[16] = {
        0xaa, 0x9b, 0xbd, 0xb8,
        0xa1, 0x98, 0xac, 0x3f,
        0x1f, 0x94, 0x07, 0xb3,
        0x8c, 0x27, 0x93, 0x69
    };

    __m128i hash = _mm_set_epi64x(0L, x);
    __m128i key = _mm_loadu_si128((__m128i*)seed);
    hash = _mm_aesdec_si128(hash, key);
    hash = _mm_aesdec_si128(hash, key);
    u64 result = _mm_extract_epi64(hash, 0);

    return (result);
}

u64 oc_hash_aes_u64_x2(u64 x, u64 y)
{
    u8 seed[16] = {
        0xaa, 0x9b, 0xbd, 0xb8,
        0xa1, 0x98, 0xac, 0x3f,
        0x1f, 0x94, 0x07, 0xb3,
        0x8c, 0x27, 0x93, 0x69
    };

    __m128i hash = _mm_set_epi64x(x, y);
    __m128i key = _mm_loadu_si128((__m128i*)seed);
    hash = _mm_aesdec_si128(hash, key);
    hash = _mm_aesdec_si128(hash, key);
    u64 result = _mm_extract_epi64(hash, 0);

    return (result);
}

u64 oc_hash_aes_string(oc_str8 string)
{
    u8 seed[16] = {
        0xaa, 0x9b, 0xbd, 0xb8,
        0xa1, 0x98, 0xac, 0x3f,
        0x1f, 0x94, 0x07, 0xb3,
        0x8c, 0x27, 0x93, 0x69
    };

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
    return (result);
}

u64 oc_hash_aes_string_seed(oc_str8 string, u64 seed)
{
    u8 seed16[16];
    memcpy(seed16, &seed, 8);
    memcpy(seed16 + 8, &seed, 8);

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
    return (result);
}
    #endif // OC_ARCH_X64
#endif     // 0
