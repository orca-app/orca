/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#if OC_PLATFORM_WINDOWS
    #include "win32_system.c"
#elif OC_PLATFORM_MACOS
    #include "osx_system.c"
#elif OC_PLATFORM_LINUX
    #include "linux_system.c"
#else
    #error Unsupported platform
#endif
