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

    void* buffer = oc_runtime_ptr_to_native(orca, req.buffer, req.size);

    if(buffer)
    {
        req.buffer = buffer;

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
    else
    {
        cmp.error = OC_IO_ERR_ARG;
    }

    return (cmp);
}
