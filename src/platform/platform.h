/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

//-----------------------------------------------------------------
// Compiler identification
//-----------------------------------------------------------------
#if defined(__clang__)
    #define OC_COMPILER_CLANG 1
    #if defined(__apple_build_version__)
        #define OC_COMPILER_CLANG_APPLE 1
    #elif defined(_MSC_VER)
        #define OC_COMPILER_CLANG_CL 1
    #endif

#elif defined(_MSC_VER)
    #define OC_COMPILER_CL 1
#elif defined(__GNUC__)
    #define OC_COMPILER_GCC 1
#else
    #error "Can't identify compiler"
#endif

//-----------------------------------------------------------------
// OS identification
//-----------------------------------------------------------------
#if defined(_WIN64)
    #define OC_PLATFORM_WINDOWS 1
#elif defined(_WIN32)
    #error "Unsupported OS (32bit only version of Windows)"
#elif defined(__APPLE__) && defined(__MACH__)
    #define OC_PLATFORM_MACOS 1
#elif defined(__gnu_linux__)
    #define PLATFORM_LINUX 1
#elif defined(__wasm__)
    #define OC_PLATFORM_ORCA 1
#else
    #error "Can't identify platform"
#endif

//-----------------------------------------------------------------
// Architecture identification
//-----------------------------------------------------------------
#if defined(OC_COMPILER_CL)
    #if defined(_M_AMD64)
        #define OC_ARCH_X64 1
    #elif defined(_M_I86)
        #define OC_ARCH_X86 1
    #elif defined(_M_ARM64)
        #define OC_ARCH_ARM64 1
    #elif defined(_M_ARM)
        #define OC_ARCH_ARM32 1
    #else
        #error "Can't identify architecture"
    #endif
#else
    #if defined(__x86_64__)
        #define OC_ARCH_X64 1
    #elif defined(__i386__)
        #define OC_ARCH_X86 1
    #elif defined(__arm__)
        #define OC_ARCH_ARM32 1
    #elif defined(__aarch64__)
        #define OC_ARCH_ARM64 1
    #elif defined(__wasm32__)
        #define OC_ARCH_WASM32 1
    #else
        #error "Can't identify architecture"
    #endif
#endif

//-----------------------------------------------------------------
// platform helper macros
//-----------------------------------------------------------------
#if defined(OC_COMPILER_CL)
    #if defined(OC_BUILD_DLL)
        #define ORCA_API __declspec(dllexport)
    #else
        #define ORCA_API __declspec(dllimport)
    #endif
#elif defined(OC_COMPILER_GCC) || defined(OC_COMPILER_CLANG)
    #define ORCA_API
#endif

#if OC_PLATFORM_ORCA
    #define oc_thread_local // no tls (or threads) for now on wasm orca
#elif defined(OC_COMPILER_CL)
    #define oc_thread_local __declspec(thread)
#elif defined(OC_COMPILER_GCC) || defined(OC_COMPILER_CLANG)
    #define oc_thread_local __thread
#endif

#if OC_PLATFORM_ORCA
    #define ORCA_IMPORT(f) __attribute__((import_name(#f))) f

    #if OC_COMPILER_CLANG
        #ifdef __cplusplus
            #define ORCA_EXPORT __attribute__((visibility("default"))) extern "C"
        #else
            #define ORCA_EXPORT __attribute__((visibility("default")))
        #endif
    #else
        #error "Orca apps can only be compiled with clang for now"
    #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    OC_HOST_PLATFORM_MACOS,
    OC_HOST_PLATFORM_WINDOWS,
} oc_host_platform;

ORCA_API oc_host_platform oc_get_host_platform(void);

#ifdef __cplusplus
} // extern "C"
#endif
