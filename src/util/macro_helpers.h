/************************************************************//**
*
*	@file: macro_helpers.h
*	@author: Martin Fouilleul
*	@date: 27/03/2020
*	@revision:
*
*****************************************************************/
#ifndef __MACRO_HELPERS_H_
#define __MACRO_HELPERS_H_

#include"util/typedefs.h"
#include"platform/platform.h"

//NOTE(martin): macro concatenation
#define _cat2_(a, b) a##b
#define _cat3_(a, b, c) a##b##c

//NOTE(martin): inline, but still generate code
//		(eg. use the inline version inside a library, but still exports the function for client code)
//TODO(martin): this is a compiler-specific attribute, recognized by clang and gcc. See if there's a more portable approach
//#define INLINE_GEN __attribute__((used)) static inline

//NOTE(martin): 'hygienic' templates, to replace macros and avoid multiple evaluation problems.
#ifdef __cplusplus
	//NOTE(martin): in C++ we use templates and decltype/declval
	//              (overloaded functions would be ambiguous because of the
	//              overload resolution and conversion/promotion rules)

	#include<utility>

	template<typename Ta, typename Tb>
	inline decltype(std::declval<Ta>()+std::declval<Tb>()) minimum_safe(Ta a, Tb b)
	{
		return(a < b ? a : b);
	}

	template<typename Ta, typename Tb>
	inline decltype(std::declval<Ta>()+std::declval<Tb>()) maximum_safe(Ta a, Tb b)
	{
		return(a > b ? a : b);
	}

	template<typename T>
	inline T square_safe(T a) {return(a*a);}

	template<typename T>
	inline T cube_safe(T a) {return(a*a*a);}

#else // (__cplusplus not defined)

	//NOTE(martin): Type generic arithmetic functions helpers
	// this macros helps generate variants of a generic 'template' for all arithmetic types.
	// the def parameter must be a macro that take a type, and optional arguments
	#define tga_generate_variants(def, ...) \
			def(u8, ##__VA_ARGS__) def(i8, ##__VA_ARGS__ ) def(u16, ##__VA_ARGS__) def(i16, ##__VA_ARGS__)  \
			def(u32, ##__VA_ARGS__) def(i32, ##__VA_ARGS__) def(u64, ##__VA_ARGS__) def(i64, ##__VA_ARGS__) \
			def(f32, ##__VA_ARGS__) def(f64, ##__VA_ARGS__)

	// This macro generates one _Generic association between a type and its variant
	#define tga_variant_association(type, name) , type: _cat3_(name, _, type)

	// This macros selects the appropriate variant for a 2 parameters functions
	#define tga_select_binary(name, a, b) \
			_Generic((a+b) tga_generate_variants(tga_variant_association, name))(a, b)

	// This macros selects the appropriate variant for a 1 parameters functions
	#define tga_select_unary(name, a) \
			_Generic((a) tga_generate_variants(tga_variant_association, name))(a)

	//NOTE(martin): type generic templates
	#define minimum_def(type) static inline type _cat3_(minimum_safe, _, type)(type a, type b) {return(a < b ? a : b);}
	#define maximum_def(type) static inline type _cat3_(maximum_safe, _, type)(type a, type b) {return(a > b ? a : b);}
	#define square_def(type) static inline type _cat3_(square_safe, _, type)(type a) {return(a*a);}
	#define cube_def(type) static inline type _cat3_(cube_safe, _, type)(type a) {return(a*a*a);}

	//NOTE(martin): instantiante our templates for all arithmetic types
	tga_generate_variants(minimum_def)
	tga_generate_variants(maximum_def)
	tga_generate_variants(square_def)
	tga_generate_variants(cube_def)

	//NOTE(martin): generate the _Generic associations between each type and its associated variant
	#define minimum_safe(a, b) tga_select_binary(minimum_safe, a, b)
	#define maximum_safe(a, b) tga_select_binary(maximum_safe, a, b)
	#define square_safe(a) tga_select_unary(square_safe, a)
	#define cube_safe(a) tga_select_unary(cube_safe, a)

#endif // __cplusplus else branch


//NOTE(martin): these macros are calling the safe functions defined above, so they don't evaluate their
//              arguments twice

#define minimum(a, b) minimum_safe(a, b)
#define maximum(a, b) maximum_safe(a, b)

#define ClampLowBound(a, low)	(maximum((a), (low)))
#define ClampHighBound(a, high) (minimum((a), (high)))
#define Clamp(a, low, high)	(ClampLowBound(ClampHighBound((a), (high)), (low)))

#define Square(a) square_safe(a)
#define Cube(a) cube_safe(a)

#define AlignUpOnPow2(x, a) (((x) + (a) - 1) & ~((a)-1))
#define AlignDownOnPow2(x, a) ((x) & ~((a)-1))

static inline u64 next_pow2_u64(u64 x)
{
	x--;
	x |= x>>1;
	x |= x>>2;
	x |= x>>4;
	x |= x>>8;
	x |= x>>16;
	x |= x>>32;
	x++;
	return(x);
}

#define defer_loop(begin, end) begin; for(int __i__=0; __i__<1; __i__++, end)

//NOTE: assert macros

#ifndef NO_ASSERT
	#ifdef PLATFORM_ORCA
		//TODO add a runtime-provided assert
		extern void orca_assert(bool x);

		#define _ASSERT_(x, msg) orca_assert(x)
	#else
		#include<assert.h>
		#define _ASSERT_(x, msg) assert(x && msg)
	#endif

	#define ASSERT(x, ...) _ASSERT_(x, #__VA_ARGS__)

	#ifdef DEBUG
		#define DEBUG_ASSERT(x, ...) ASSERT(x, ##__VA_ARGS__ )
	#else
		#define DEBUG_ASSERT(x, ...)
	#endif
#else
	#define ASSERT(x, ...)
	#define DEBUG_ASSERT(x, ...)
#endif


#endif //__MACRO_HELPERS_H_
