/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "runtime_subprocess.h"
// #include "memory.h"

oc_arena cmdArena;

oc_wasm_str8 oc_runtime_run_cmd(oc_wasm_addr wasmArena, oc_wasm_str8 cmd)
{
    if(!cmdArena.currentChunk)
    {
        oc_arena_init(&cmdArena);
    }

    oc_arena_clear(&cmdArena);
    oc_str8 nativeCmd = oc_wasm_str8_to_native(cmd);
    oc_str8 out = oc_run_cmd(&cmdArena, nativeCmd);
    oc_wasm_addr outAddr = oc_wasm_arena_push(wasmArena, out.len);
    char* outPtr = (char*)oc_wasm_address_to_ptr(outAddr, out.len);
    memcpy(outPtr, out.ptr, out.len);
    return (oc_wasm_str8){ .ptr = outAddr,
                           .len = out.len };
}
