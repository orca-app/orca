#include"canvas_api_bind_gen.c"

typedef struct wasm_str8
{
	i64 len;
	i32 offset;
} wasm_str8;

const void* mg_text_outlines_stub(IM3Runtime runtime, IM3ImportContext _ctx, uint64_t* _sp, void* _mem)
{
	wasm_str8 wasmStr = *(wasm_str8*)((char*)_mem + *(i32*)&_sp[0]);

	/////////////////////////////////////////////////////////////////////
	//TODO: bound checks
	str8 str = {.len = wasmStr.len,
	            .ptr = (char*)_mem + wasmStr.offset};
	/////////////////////////////////////////////////////////////////////

	mg_text_outlines(str);
	return(0);
}
