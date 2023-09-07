/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __WIN32_STRING_HELPERS_H_
#define __WIN32_STRING_HELPERS_H_

#include "util/strings.h"

oc_str16 oc_win32_utf8_to_wide(oc_arena* arena, oc_str8 s);
oc_str8 oc_win32_wide_to_utf8(oc_arena* arena, oc_str16 s);

#endif // __WIN32_STRING_HELPERS_H_
