#define _Addr long
#define _Int64 long long
#define _Reg long long

#define __BYTE_ORDER __BYTE_ORDER__

#define __LONG_MAX __LONG_MAX__

#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321
#define __USE_TIME_BITS64 1

#if defined(__NEED_size_t) && !defined(__DEFINED_size_t)
typedef unsigned _Addr size_t;
#define __DEFINED_size_t
#endif

#if defined(__NEED_uintptr_t) && !defined(__DEFINED_uintptr_t)
typedef unsigned _Addr uintptr_t;
#define __DEFINED_uintptr_t
#endif

#if defined(__NEED_ptrdiff_t) && !defined(__DEFINED_ptrdiff_t)
typedef _Addr ptrdiff_t;
#define __DEFINED_ptrdiff_t
#endif

#if defined(__NEED_ssize_t) && !defined(__DEFINED_ssize_t)
typedef _Addr ssize_t;
#define __DEFINED_ssize_t
#endif

#if defined(__NEED_intptr_t) && !defined(__DEFINED_intptr_t)
typedef _Addr intptr_t;
#define __DEFINED_intptr_t
#endif

#if defined(__NEED_regoff_t) && !defined(__DEFINED_regoff_t)
typedef _Addr regoff_t;
#define __DEFINED_regoff_t
#endif

#if defined(__NEED_register_t) && !defined(__DEFINED_register_t)
typedef _Reg register_t;
#define __DEFINED_register_t
#endif

#if defined(__NEED_int8_t) && !defined(__DEFINED_int8_t)
typedef signed char     int8_t;
#define __DEFINED_int8_t
#endif

#if defined(__NEED_int16_t) && !defined(__DEFINED_int16_t)
typedef signed short    int16_t;
#define __DEFINED_int16_t
#endif

#if defined(__NEED_int32_t) && !defined(__DEFINED_int32_t)
typedef signed int      int32_t;
#define __DEFINED_int32_t
#endif

#if defined(__NEED_int64_t) && !defined(__DEFINED_int64_t)
typedef signed _Int64   int64_t;
#define __DEFINED_int64_t
#endif

#if defined(__NEED_intmax_t) && !defined(__DEFINED_intmax_t)
typedef signed _Int64   intmax_t;
#define __DEFINED_intmax_t
#endif

#if defined(__NEED_uint8_t) && !defined(__DEFINED_uint8_t)
typedef unsigned char   uint8_t;
#define __DEFINED_uint8_t
#endif

#if defined(__NEED_uint16_t) && !defined(__DEFINED_uint16_t)
typedef unsigned short  uint16_t;
#define __DEFINED_uint16_t
#endif

#if defined(__NEED_uint32_t) && !defined(__DEFINED_uint32_t)
typedef unsigned int    uint32_t;
#define __DEFINED_uint32_t
#endif

#if defined(__NEED_uint64_t) && !defined(__DEFINED_uint64_t)
typedef unsigned _Int64 uint64_t;
#define __DEFINED_uint64_t
#endif

#if defined(__NEED_u_int64_t) && !defined(__DEFINED_u_int64_t)
typedef unsigned _Int64 u_int64_t;
#define __DEFINED_u_int64_t
#endif

#if defined(__NEED_uintmax_t) && !defined(__DEFINED_uintmax_t)
typedef unsigned _Int64 uintmax_t;
#define __DEFINED_uintmax_t
#endif


#if defined(__NEED_float_t) && !defined(__DEFINED_float_t)
typedef float float_t;
#define __DEFINED_float_t
#endif

#if defined(__NEED_double_t) && !defined(__DEFINED_double_t)
typedef double double_t;
#define __DEFINED_double_t
#endif


#if defined(__NEED_wchar_t) && !defined(__DEFINED_wchar_t)
typedef int wchar_t;
#define __DEFINED_wchar_t
#endif

#if defined(__NEED_off_t) && !defined(__DEFINED_off_t)
typedef _Int64 off_t;
#define __DEFINED_off_t
#endif

#if defined(__NEED_wint_t) && !defined(__DEFINED_wint_t)
typedef unsigned wint_t;
#define __DEFINED_wint_t
#endif

#if defined(__NEED_wctype_t) && !defined(__DEFINED_wctype_t)
typedef unsigned long wctype_t;
#define __DEFINED_wctype_t
#endif

#if defined(__NEED_va_list) && !defined(__DEFINED_va_list)
typedef __builtin_va_list va_list;
#define __DEFINED_va_list
#endif

#if defined(__NEED___isoc_va_list) && !defined(__DEFINED___isoc_va_list)
typedef __builtin_va_list __isoc_va_list;
#define __DEFINED___isoc_va_list
#endif

#if defined(__NEED_mbstate_t) && !defined(__DEFINED_mbstate_t)
typedef struct __mbstate_t { unsigned __opaque1, __opaque2; } mbstate_t;
#define __DEFINED_mbstate_t
#endif

#undef _Addr
#undef _Int64
#undef _Reg
