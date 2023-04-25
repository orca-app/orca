/************************************************************//**
*
*	@file: milepost.c
*	@author: Martin Fouilleul
*	@date: 13/02/2021
*	@revision:
*
*****************************************************************/

//---------------------------------------------------------------
// platform implementations
//---------------------------------------------------------------
#include"platform.h"

#include"platform/std_log.c"

#if PLATFORM_WINDOWS
	#include"platform/win32_memory.c"
	#include"platform/win32_clock.c"
	//TODO
#elif PLATFORM_MACOS
	#include"platform/unix_memory.c"
	#include"platform/osx_clock.c"
	/*
	#include"platform/unix_rng.c"
	#include"platform/posix_thread.c"
	#include"platform/posix_socket.c"
	*/

#elif PLATFORM_LINUX
	#include"platform/unix_base_memory.c"
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
// utilities implementations
//---------------------------------------------------------------
#include"util/memory.c"
#include"util/strings.c"
#include"util/utf8.c"
#include"util/hash.c"
#include"util/ringbuffer.c"

//---------------------------------------------------------------
// app/graphics layer
//---------------------------------------------------------------

#if PLATFORM_WINDOWS
	#include"win32_app.c"
	#include"graphics_common.c"
	#include"graphics_surface.c"

	#if MG_COMPILE_GL || MG_COMPILE_GLES
		#include"gl_loader.c"
	#endif

	#if MG_COMPILE_GL
		#include"wgl_surface.c"
	#endif

	#if MG_COMPILE_CANVAS
		#include"gl_canvas.c"
	#endif

	#if MG_COMPILE_GLES
		#include"egl_surface.c"
	#endif

#elif PLATFORM_MACOS
	//NOTE: macos application layer and graphics backends are defined in milepost.m
#else
	#error "Unsupported platform"
#endif

#include"input_state.c"
#include"ui.c"
