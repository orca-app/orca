/************************************************************//**
*
*	@file: milepost.c
*	@author: Martin Fouilleul
*	@date: 13/02/2021
*	@revision:
*
*****************************************************************/

//---------------------------------------------------------------
// utilities implementations
//---------------------------------------------------------------
#include"util/debug_log.c"
#include"util/memory.c"
#include"util/strings.c"
#include"util/utf8.c"
#include"util/hash.c"
#include"util/ringbuffer.c"

//---------------------------------------------------------------
// platform implementations
//---------------------------------------------------------------
#include"platform.h"

#if defined(OS_WIN64)
	#include"platform/win32_base_allocator.c"
	//TODO
#elif defined(OS_MACOS)
	#include"platform/unix_base_allocator.c"
	#include"platform/osx_clock.c"
	/*
	#include"platform/unix_rng.c"
	#include"platform/posix_thread.c"
	#include"platform/posix_socket.c"
	*/

#elif defined(OS_LINUX)
	#include"platform/unix_base_allocator.c"
	#include"platform/linux_clock.c"
	/*
	#include"platform/unix_rng.c"
	#include"platform/posix_thread.c"
	#include"platform/posix_socket.c"
	*/
#else
	#error "Unsupported platform"
#endif

//---------------------------------------------------------------
// application layer
//---------------------------------------------------------------

#if defined(OS_WIN64)
	#include"win32_app.c"
//	#include"win32_gl_surface.c"
	#include"win32_gles_surface.c"
	#include"gles_canvas.c"
#elif defined(OS_MACOS)
	//NOTE: macos application layer is defined in milepost.m
#else
	#error "Unsupported platform"
#endif

//---------------------------------------------------------------
// graphics/ui layer
//---------------------------------------------------------------
#include"graphics.c"
//#include"ui.c"
