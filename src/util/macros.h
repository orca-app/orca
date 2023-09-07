/************************************************************/ /**
*
*	@file: macros.h
*	@author: Martin Fouilleul
*	@date: 27/03/2020
*	@revision:
*
*****************************************************************/
#ifndef __MACROS_H_
#define __MACROS_H_

#include "platform/platform.h"
#include "util/typedefs.h"

//----------------------------------------------------------------------------------------
// utility macros
//----------------------------------------------------------------------------------------
#define OC_COMMA ,
#define OC_CAT2(a, b) a##b
#define OC_CAT3(a, b, c) a##b##c
#define OC_PASS(A, ...) A(__VA_ARGS__)
#define OC_EXPAND(...) __VA_ARGS__
#define OC_EXPAND_NIL(...)

//----------------------------------------------------------------------------------------
// Variadic macros helpers and replacement for __VA_OPT__ extension
//----------------------------------------------------------------------------------------
#define OC_ARG1_UTIL(a, ...) a
#define OC_ARG1(...) OC_ARG1_UTIL(__VA_ARGS__)
#define OC_VA_COMMA_TAIL(a, ...) , ##__VA_ARGS__

//NOTE: this expands to opt if __VA_ARGS__ is empty, and to , va1, va2, ... opt otherwise
#define OC_VA_NOPT_UTIL(opt, ...) , ##__VA_ARGS__ opt

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

//NOTE(martin): 'hygienic' max/min/square/cube macros.
#ifdef __cplusplus
    //NOTE(martin): in C++ we use templates and decltype/declval
    //              (overloaded functions would be ambiguous because of the
    //              overload resolution and conversion/promotion rules)
    #include <utility>

template <typename Ta, typename Tb>
inline decltype(std::declval<Ta>() + std::declval<Tb>()) oc_min(Ta a, Tb b)
{
    return (a < b ? a : b);
}

template <typename Ta, typename Tb>
inline decltype(std::declval<Ta>() + std::declval<Tb>()) oc_max(Ta a, Tb b)
{
    return (a > b ? a : b);
}

template <typename T>
inline T oc_square(T a)
{
    return (a * a);
}

template <typename T>
inline T oc_cube(T a)
{
    return (a * a * a);
}
#else
    //NOTE(martin): this macros helps generate variants of a generic 'template' for all arithmetic types.
    // the def parameter must be a macro that takes a type, and optional arguments
    #define oc_tga_variants(def, ...)                                                                       \
        def(u8, ##__VA_ARGS__) def(i8, ##__VA_ARGS__) def(u16, ##__VA_ARGS__) def(i16, ##__VA_ARGS__)       \
            def(u32, ##__VA_ARGS__) def(i32, ##__VA_ARGS__) def(u64, ##__VA_ARGS__) def(i64, ##__VA_ARGS__) \
                def(f32, ##__VA_ARGS__) def(f64, ##__VA_ARGS__)

    // This macro generates one _Generic association between a type and its variant
    #define oc_tga_association(type, name) , type : OC_CAT3(name, _, type)

    // This macros selects the appropriate variant for a 2 parameters functions
    #define oc_tga_select_binary(name, a, b) \
        _Generic((a + b) oc_tga_variants(oc_tga_association, name))(a, b)

    // This macros selects the appropriate variant for a 1 parameters functions
    #define oc_tga_select_unary(name, a) \
        _Generic((a)oc_tga_variants(oc_tga_association, name))(a)

    //NOTE(martin): type generic templates
    #define oc_min_def(type) \
        static inline type OC_CAT3(oc_min, _, type)(type a, type b) { return (a < b ? a : b); }
    #define oc_max_def(type) \
        static inline type OC_CAT3(oc_max, _, type)(type a, type b) { return (a > b ? a : b); }
    #define oc_square_def(type) \
        static inline type OC_CAT3(oc_square, _, type)(type a) { return (a * a); }
    #define oc_cube_def(type) \
        static inline type OC_CAT3(oc_cube, _, type)(type a) { return (a * a * a); }

//NOTE(martin): instantiante our templates for all arithmetic types
oc_tga_variants(oc_min_def)
    oc_tga_variants(oc_max_def)
        oc_tga_variants(oc_square_def)
            oc_tga_variants(oc_cube_def)

    //NOTE(martin): generate the _Generic associations between each type and its associated variant
    #define oc_min(a, b) oc_tga_select_binary(oc_min, a, b)
    #define oc_max(a, b) oc_tga_select_binary(oc_max, a, b)
    #define oc_square(a) oc_tga_select_unary(oc_square, a)
    #define oc_cube(a) oc_tga_select_unary(oc_cube, a)

#endif // __cplusplus else branch

#define oc_clamp_low(a, low) (oc_max((a), (low)))
#define oc_clamp_high(a, high) (oc_min((a), (high)))
#define oc_clamp(a, low, high) (oc_clamp_low(oc_clamp_high((a), (high)), (low)))

#endif //__MACROS_H_
