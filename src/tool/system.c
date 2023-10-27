/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#ifdef OC_PLATFORM_WINDOWS
    #include "win32_system.c"
#else
    #error Unsupported platform
#endif
