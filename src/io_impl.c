/************************************************************//**
*
*	@file: io_impl.c
*	@author: Martin Fouilleul
*	@date: 09/05/2023
*
*****************************************************************/
#include"platform/platform_io.h"

io_cmp orca_io_wait_single_req(io_req* wasmReq)
{
	mem_arena* scratch = mem_scratch();

	io_cmp cmp = {0};
	io_req req = *wasmReq;
	//NOTE: convert the req->buffer wasm pointer to a native pointer
	//		for some reason, wasm3 memory doesn't start at the beginning of the block we give it.
	u64 bufferIndex = (u64)req.buffer & 0xffffffff;
	u32 memSize = 0;
	char* memory = (char*)m3_GetMemory(__orcaApp.runtime.m3Runtime, &memSize, 0);

	if(bufferIndex + req.size > memSize)
	{
		cmp.error = IO_ERR_ARG;
	}
	else
	{
		req.buffer = memory + bufferIndex;

		//TODO: do some further ownership/rights checking here, and make sure we modify flags to avoid walking out the app folder

		if(req.op == IO_OP_OPEN_AT)
		{
			////////////////////////////////////////////////////////////////////////
			//TODO: should change root to app local folder
			//	- if file handle is null, set it to pre-opened handle to app local folder
			//  - if file handle is not null, check that it is valid
			//		--> this means we probably need a second indirection: from wasm file handle to native file handle
			////////////////////////////////////////////////////////////////////////
		}

		cmp = io_wait_single_req(&req);

	}
	return(cmp);
}
