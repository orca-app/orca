/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "orca.h"

oc_str8 oc_str8_trim_space(oc_str8 s);
oc_str8 system_orca_dir(oc_arena* a);
oc_str8 current_sdk_version(oc_arena* a, bool fail_if_not_found);
oc_str8 current_version_dir(oc_arena* a, bool fail_if_not_found);
oc_str8 get_version_dir(oc_arena* a, oc_str8 version, bool fail_if_not_found);
