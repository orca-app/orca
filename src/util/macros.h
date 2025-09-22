/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "platform/platform.h"
#include "util/typedefs.h"

//----------------------------------------------------------------------------------------
// utility macros
//----------------------------------------------------------------------------------------
#define OC_NIL
#define OC_COMMA ,
#define OC_CAT2(a, b) a##b
#define OC_CAT3(a, b, c) a##b##c
#define OC_PASS(A, ...) A(__VA_ARGS__)
#define OC_EXPAND(...) __VA_ARGS__
#define OC_EXPAND_NIL(...)

// Counting var args, up to 10 args
#define OC_COUNT10_UTIL(a, b, c, d, e, f, g, h, i, j, k, l, count, ...) count

#if OC_COMPILER_CL
    // on MSVC, there is no VA_OPT but ##__VA_ARGS__ consumes the previous token if there is no argument
    #define OC_COUNT10(...) OC_COUNT10_UTIL(, , ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#else
    // on clang, ##__VA_ARGS__ does not consume previous token if there is no argument, so we use __VA_OPT__
    #define OC_COUNT10(...) OC_COUNT10_UTIL(, __VA_OPT__(, )##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#endif

//----------------------------------------------------------------------------------------
// Variadic macros helpers and portable replacement for __VA_OPT__ extension
//----------------------------------------------------------------------------------------
#define OC_ARG1_UTIL(a, ...) a
#define OC_ARG1(...) OC_ARG1_UTIL(__VA_ARGS__)
#define OC_VA_COMMA_TAIL(a, ...) , ##__VA_ARGS__

//NOTE: this expands to opt if __VA_ARGS__ is empty, and to , va1, va2, ... opt otherwise
#if OC_COMPILER_CLANG
    // on clang we use __VA_OPT__ because ##__VA_ARGS__ does not swallow the previous token if there is _no_ arguments
    #define OC_VA_NOPT_UTIL(opt, ...) __VA_OPT__(, ) __VA_ARGS__ opt
#else
    // on msvc __VA_OPT__ does not exist in C mode, but ##__VA_ARGS__ works even when there is no arguments
    #define OC_VA_NOPT_UTIL(opt, ...) , ##__VA_ARGS__ opt
#endif

//NOTE: this expands to opt if __VA_ARGS__ is empty, and to nothing otherwise
#define OC_VA_NOPT(opt, ...) OC_PASS(OC_ARG1, OC_VA_NOPT_UTIL(opt, ##__VA_ARGS__))

//NOTE: this expands to opt if __VA_ARGS__ is non empty, and to nothing otherwise
#define OC_VA_OPT(opt, ...) OC_PASS(OC_CAT2, OC_EXPAND, OC_VA_NOPT(_NIL, ##__VA_ARGS__))(opt)

//----------------------------------------------------------------------------------------
// misc
//----------------------------------------------------------------------------------------

//NOTE: this computes the address of a struct given an address to one of its members
#ifdef __cplusplus
    #define oc_container_of(ptr, type, member) ({          \
		    const decltype( ((type *)0)->member ) *__mptr = (ptr);    \
		    (type *)( (char *)__mptr - offsetof(type,member) ); })
#else
    #define oc_container_of(ptr, type, member) (type*)((char*)(ptr)-offsetof(type, member))
#endif

#define oc_defer_loop(begin, end) \
    begin;                        \
    for(int __i__ = 0; __i__ < 1; __i__++, end)

#define oc_array_size(array) (sizeof(array) / sizeof((array)[0]))

#define oc_sizeof_member(type, member) (sizeof(((type *)NULL)->member))
#define oc_typeof_member(type, member) (sizeof(((type *)NULL)->member))
#define oc_array_size_of_member(type, member) (oc_array_size(((type *)NULL)->member))

#define U8_MAX __UINT8_MAX__
#define U16_MAX __UINT16_MAX__
#define U32_MAX __UINT32_MAX__
#define U64_MAX __UINT64_MAX__
#define I8_MAX __INT8_MAX__
#define I16_MAX __INT16_MAX__
#define I32_MAX __INT32_MAX__
#define I64_MAX __INT64_MAX__

#define I8_MIN (-I8_MAX - 1)
#define I16_MIN (-I16_MAX - 1)
#define I32_MIN (-I32_MAX - 1)
#define I64_MIN (-I64_MAX - 1)

#define OC_UNIQUE_NAME__(name, counter) oc_unique_name_ ## name ## counter
#define OC_UNIQUE_NAME_(name, counter) OC_UNIQUE_NAME__(name, counter)
#define OC_UNIQUE_NAME(name) OC_UNIQUE_NAME_(name, __COUNTER__)

#define oc_swap_(a, b, tmp_sym) \
  __extension__({          \
    __typeof__(a) tmp_sym = a; \
    a = b;                 \
    b = tmp_sym;           \
  })
#define oc_swap(a, b) oc_swap_((a), (b), OC_UNIQUE_NAME(oc_swap))

//-----------------------------------------------------------------
// endianness
//-----------------------------------------------------------------

union oc_endianness_test_detail
{
    uint32_t num;
    uint8_t bytes[4];
};

#define oc_is_little_endian() (((union oc_endianness_test_detail){ .num = 0x01020304 }).bytes[0] == 0x4)
#define oc_is_big_endian() (!OC_IS_LITTLE_ENDIAN())

//----------------------------------------------------------------------------------------
//NOTE(martin): bit-twiddling & arithmetic helpers
//----------------------------------------------------------------------------------------
#define oc_align_up_pow2(x, a) (((x) + (a)-1) & ~((a)-1))
#define oc_align_down_pow2(x, a) ((x) & ~((a)-1))

static inline u64 oc_next_pow2(u64 x)
{
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    x++;
    return (x);
}

#define oc_min(a, b) (((a) < (b)) ? (a) : (b))
#define oc_max(a, b) (((a) > (b)) ? (a) : (b))

#define oc_clamp_low(a, low) (oc_max((a), (low)))
#define oc_clamp_high(a, high) (oc_min((a), (high)))
#define oc_clamp(a, low, high) (oc_clamp_low(oc_clamp_high((a), (high)), (low)))

#define UNUSED __attribute__((unused))

#define oc_square(a) ((a) * (a))
#define oc_cube(a) ((a) * (a) * (a))
