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
        cmp = oc_io_wait_single_req_for_table(&req, &orca->fileTable);
    }
    else
    {
        cmp.error = OC_IO_ERR_ARG;
    }

    return (cmp);
}

oc_file oc_file_open_with_request_bridge(oc_str8 path, oc_file_access rights, oc_file_open_flags flags)
{
    oc_file file = oc_file_nil();
    oc_runtime* orca = oc_runtime_get();

    path.ptr = oc_runtime_ptr_to_native(orca, path.ptr, path.len);
    if(path.ptr)
    {
        file = oc_file_open_with_request_for_table(path, rights, flags, &orca->fileTable);
    }
    return (file);
}

typedef struct oc_wasm_list
{
    u32 first;
    u32 last;
} oc_wasm_list;

typedef struct oc_wasm_list_elt
{
    u32 prev;
    u32 next;
} oc_wasm_list_elt;

typedef struct oc_wasm_str8
{
    u64 len;
    u32 ptr;
} oc_wasm_str8;

typedef struct oc_wasm_str8_elt
{
    oc_wasm_list_elt listElt;
    oc_wasm_str8 string;

} oc_wasm_str8_elt;

typedef struct oc_wasm_str8_list
{
    oc_wasm_list list;
    u64 eltCount;
    u64 len;
} oc_wasm_str8_list;

typedef struct oc_wasm_file_dialog_desc
{
    oc_file_dialog_kind kind;
    oc_file_dialog_flags flags;
    oc_wasm_str8 title;
    oc_wasm_str8 okLabel;
    oc_file startAt;
    oc_wasm_str8 startPath;
    oc_wasm_str8_list filters;

} oc_wasm_file_dialog_desc;

typedef struct oc_wasm_file_open_with_dialog_elt
{
    oc_wasm_list_elt listElt;
    oc_file file;
} oc_wasm_file_open_with_dialog_elt;

typedef struct oc_wasm_file_open_with_dialog_result
{
    oc_file_dialog_button button;
    oc_file file;
    oc_wasm_list selection;

} oc_wasm_file_open_with_dialog_result;

oc_wasm_file_open_with_dialog_result oc_file_open_with_dialog_bridge(i32 wasmArenaIndex,
                                                                     oc_file_access rights,
                                                                     oc_file_open_flags flags,
                                                                     oc_wasm_file_dialog_desc* desc)
{
    oc_runtime* orca = oc_runtime_get();
    oc_arena_scope scratch = oc_scratch_begin();

    oc_file_dialog_desc nativeDesc = {
        .kind = desc->kind,
        .flags = desc->flags
    };

    nativeDesc.title.ptr = oc_runtime_ptr_to_native(orca, (char*)(uintptr_t)desc->title.ptr, desc->title.len);
    nativeDesc.title.len = desc->title.len;

    nativeDesc.okLabel.ptr = oc_runtime_ptr_to_native(orca, (char*)(uintptr_t)desc->okLabel.ptr, desc->okLabel.len);
    nativeDesc.okLabel.len = desc->okLabel.len;

    if(oc_file_is_nil(desc->startAt) && desc->startPath.len)
    {
        nativeDesc.startAt = orca->rootDir;
    }
    else
    {
        nativeDesc.startAt = desc->startAt;
    }
    nativeDesc.startPath.ptr = oc_runtime_ptr_to_native(orca, (char*)(uintptr_t)desc->startPath.ptr, desc->startPath.len);
    nativeDesc.startPath.len = desc->startPath.len;

    u32 eltIndex = desc->filters.list.first;
    while(eltIndex)
    {
        oc_wasm_str8_elt* elt = oc_runtime_ptr_to_native(orca, (char*)(uintptr_t)eltIndex, sizeof(oc_wasm_str8_elt));

        oc_str8 filter = { 0 };
        filter.ptr = oc_runtime_ptr_to_native(orca, (char*)(uintptr_t)elt->string.ptr, elt->string.len);
        filter.len = elt->string.len;

        oc_str8_list_push(scratch.arena, &nativeDesc.filters, filter);

        oc_log_info("filter: %.*s\n", (int)filter.len, filter.ptr);

        eltIndex = elt->listElt.next;
    }

    oc_file_open_with_dialog_result nativeResult = oc_file_open_with_dialog_for_table(scratch.arena, rights, flags, &nativeDesc, &orca->fileTable);

    oc_wasm_file_open_with_dialog_result result = {
        .button = nativeResult.button,
        .file = nativeResult.file
    };

    u32 memSize;
    char* wasmMemory = (char*)m3_GetMemory(orca->env.m3Runtime, &memSize, 0);
    oc_wasm_file_open_with_dialog_elt* lastElt = 0;

    oc_list_for(&nativeResult.selection, elt, oc_file_open_with_dialog_elt, listElt)
    {
        oc_wasm_file_open_with_dialog_elt* wasmElt = oc_wasm_arena_push(&orca->env, wasmArenaIndex, sizeof(oc_wasm_file_open_with_dialog_elt));
        wasmElt->file = elt->file;

        if(result.selection.last == 0)
        {
            result.selection.first = ((char*)wasmElt - wasmMemory);
            result.selection.last = result.selection.first;
            wasmElt->listElt.prev = 0;
            wasmElt->listElt.next = 0;
        }
        else
        {
            wasmElt->listElt.prev = result.selection.last;
            wasmElt->listElt.next = 0;
            lastElt->listElt.next = ((char*)wasmElt - wasmMemory);

            result.selection.last = ((char*)wasmElt - wasmMemory);
        }
        lastElt = wasmElt;
    }

    oc_scratch_end(scratch);
    return (result);
}
