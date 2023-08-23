/************************************************************/ /**
*
*	@file: runtime_io.c
*	@author: Martin Fouilleul
*	@date: 09/05/2023
*
*****************************************************************/
#include "platform/platform_io_internal.h"
#include "runtime.h"

oc_io_cmp oc_runtime_io_wait_single_req(oc_io_req* wasmReq)
{
    oc_runtime* orca = oc_runtime_get();

    oc_io_cmp cmp = { 0 };
    oc_io_req req = *wasmReq;
    //NOTE: convert the req->buffer wasm pointer to a native pointer
    //		for some reason, wasm3 memory doesn't start at the beginning of the block we give it.
    u64 bufferIndex = (u64)req.buffer & 0xffffffff;
    u32 memSize = 0;
    char* memory = (char*)m3_GetMemory(orca->runtime.m3Runtime, &memSize, 0);

    if(bufferIndex + req.size > memSize)
    {
        cmp.error = OC_IO_ERR_ARG;
    }
    else
    {
        req.buffer = memory + bufferIndex;

        if(req.op == OC_IO_OPEN_AT)
        {
            if(req.handle.h == 0)
            {
                //NOTE: change root to app local folder
                req.handle = orca->rootDir;
                req.open.flags |= OC_FILE_OPEN_RESTRICT;
            }
        }
        cmp = oc_io_wait_single_req_with_table(&req, &orca->fileTable);
    }
    return (cmp);
}
