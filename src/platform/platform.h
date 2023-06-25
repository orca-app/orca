//*****************************************************************
//
//	$file: platform.h $
//	$author: Martin Fouilleul $
//	$date: 22/12/2022 $
//	$revision: $
//	$note: (C) 2022 by Martin Fouilleul - all rights reserved $
//
//*****************************************************************
#ifndef __PLATFORM_H_
#define __PLATFORM_H_

//-----------------------------------------------------------------
// Compiler identification
//-----------------------------------------------------------------
#if defined(__clang__)
	#define COMPILER_CLANG 1
	#if defined(__apple_build_version__)
		#define COMPILER_CLANG_APPLE 1
	#elif defined(_MSC_VER)
		#define COMPILER_CLANG_CL 1
	#endif

#elif defined(_MSC_VER)
	#define COMPILER_CL 1
#elif defined(__GNUC__)
	#define COMPILER_GCC 1
#else
	#error "Can't identify compiler"
#endif

//-----------------------------------------------------------------
// OS identification
//-----------------------------------------------------------------
#if defined(_WIN64)
	#define PLATFORM_WINDOWS 1
#elif defined(_WIN32)
	#error "Unsupported OS (32bit only version of Windows)"
#elif defined(__APPLE__) && defined(__MACH__)
	#define PLATFORM_MACOS 1
#elif defined(__gnu_linux__)
	#define PLATFORM_LINUX 1
#elif defined(__ORCA__)
	#define PLATFORM_ORCA 1
#else
	#error "Can't identify platform"
#endif

//-----------------------------------------------------------------
// Architecture identification
//-----------------------------------------------------------------
#if defined(COMPILER_CL)
	#if defined(_M_AMD64)
		#define ARCH_X64 1
	#elif defined(_M_I86)
		#define ARCH_X86 1
	#elif defined(_M_ARM64)
		#define ARCH_ARM64 1
	#elif defined(_M_ARM)
		#define ARCH_ARM32 1
	#else
		#error "Can't identify architecture"
	#endif
#else
	#if defined(__x86_64__)
		#define ARCH_X64 1
	#elif defined(__i386__)
		#define ARCH_X86 1
	#elif defined(__arm__)
		#define ARCH_ARM32 1
	#elif defined(__aarch64__)
		#define ARCH_ARM64 1
	#elif defined(__ORCA__)
		#define ARCH_WASM 1
	#else
		#error "Can't identify architecture"
	#endif
#endif

//-----------------------------------------------------------------
// platform helper macros
//-----------------------------------------------------------------
#if defined(COMPILER_CL)
	#if defined(MP_BUILD_DLL)
		#define MP_API __declspec(dllexport)
	#else
		#define MP_API __declspec(dllimport)
	#endif

	#define mp_thread_local __declspec(thread)

#elif defined(COMPILER_GCC) || defined(COMPILER_CLANG)
	#define MP_API
	#define mp_thread_local __thread
#endif


#if PLATFORM_ORCA
	#define ORCA_IMPORT(f) __attribute__((import_name(#f))) f
#endif

#endif // __PLATFORM_H_
