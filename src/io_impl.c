/************************************************************//**
*
*	@file: io_impl.c
*	@author: Martin Fouilleul
*	@date: 09/05/2023
*
*****************************************************************/
#include"platform/platform_io_internal.h"
#include"orca_app.h"

io_cmp orca_io_wait_single_req(io_req* wasmReq)
{
	orca_app* orca = orca_app_get();
	mem_arena* scratch = mem_scratch();

	io_cmp cmp = {0};
	io_req req = *wasmReq;
	//NOTE: convert the req->buffer wasm pointer to a native pointer
	//		for some reason, wasm3 memory doesn't start at the beginning of the block we give it.
	u64 bufferIndex = (u64)req.buffer & 0xffffffff;
	u32 memSize = 0;
	char* memory = (char*)m3_GetMemory(orca->runtime.m3Runtime, &memSize, 0);

	if(bufferIndex + req.size > memSize)
	{
		cmp.error = IO_ERR_ARG;
	}
	else
	{
		req.buffer = memory + bufferIndex;

		if(req.op == IO_OP_OPEN_AT)
		{
			if(req.handle.h == 0)
			{
				//NOTE: change root to app local folder
				req.handle = orca->rootDir;
				req.open.flags |= FILE_OPEN_RESTRICT;
			}
		}
		cmp = io_wait_single_req_with_table(&req, &orca->fileTable);
	}
	return(cmp);
}
