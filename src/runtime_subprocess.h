/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#ifndef __RUNTIME_SUBPROCESS_H_
#define __RUNTIME_SUBPROCESS_H_

#include "runtime_memory.h"

oc_wasm_str8 oc_runtime_run_cmd(oc_wasm_addr wasmArena, oc_wasm_str8 cmd);

#endif // __RUNTIME_SUBPROCESS_H
