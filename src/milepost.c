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
	#include"platform/win32_clock.c"
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
// app/graphics layer
//---------------------------------------------------------------

#if defined(OS_WIN64)
	#include"win32_app.c"
	#include"graphics.c"

	#if MG_COMPILE_BACKEND_GL || MG_COMPILE_BACKEND_GLES
		#include"gl_loader.c"
	#endif

	#if MG_COMPILE_BACKEND_GL
		#include"wgl_surface.c"
		#include"gl_canvas.c"
	#endif

	#if MG_COMPILE_BACKEND_GLES
		#include"egl_surface.c"
	#endif

#elif defined(OS_MACOS)
	//NOTE: macos application layer and graphics backends are defined in milepost.m
	#include"graphics.c"
#else
	#error "Unsupported platform"
#endif
