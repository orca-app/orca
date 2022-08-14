/************************************************************//**
*
*	@file: milepost.c
*	@author: Martin Fouilleul
*	@date: 13/02/2021
*	@revision:
*
*****************************************************************/

#include"util/debug_log.c"
#include"memory.c"
#include"strings.c"
#include"ringbuffer.c"
#include"util/utf8.c"
#include"util/hash.c"

#include"platform/unix_base_allocator.c"

#include"graphics.c"
#include"ui.c"

//TODO: guard these under platform-specific #ifdefs
#include"platform/osx_clock.c"
/*

#include"platform/unix_rng.c"
#include"platform/posix_thread.c"
#include"platform/posix_socket.c"
*/
