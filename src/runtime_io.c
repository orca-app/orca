/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "platform/platform_io_internal.h"
#include "runtime.h"
#include "runtime_memory.h"

oc_io_cmp oc_bridge_io_single_rect(oc_io_req* wasmReq)
{
    oc_runtime* orca = oc_runtime_get();

    oc_io_cmp cmp = { 0 };
    oc_io_req req = *wasmReq;

    //TODO have a separate oc_wasm_io_req struct
    void* buffer = oc_wasm_address_to_ptr((oc_wasm_addr)(uintptr_t)req.buffer, req.size);

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

oc_file oc_file_open_with_request_bridge(oc_wasm_str8 path, oc_file_access rights, oc_file_open_flags flags)
{
    oc_file file = oc_file_nil();
    oc_runtime* orca = oc_runtime_get();

    oc_str8 nativePath = oc_wasm_str8_to_native(path);

    if(nativePath.ptr)
    {
        file = oc_file_open_with_request_for_table(nativePath, rights, flags, &orca->fileTable);
    }
    return (file);
}

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

oc_wasm_file_open_with_dialog_result oc_file_open_with_dialog_bridge(oc_wasm_addr wasmArena,
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

    nativeDesc.title.ptr = oc_wasm_address_to_ptr(desc->title.ptr, desc->title.len);
    nativeDesc.title.len = desc->title.len;

    nativeDesc.okLabel.ptr = oc_wasm_address_to_ptr(desc->okLabel.ptr, desc->okLabel.len);
    nativeDesc.okLabel.len = desc->okLabel.len;

    if(oc_file_is_nil(desc->startAt) && desc->startPath.len)
    {
        nativeDesc.startAt = orca->rootDir;
    }
    else
    {
        nativeDesc.startAt = desc->startAt;
    }
    nativeDesc.startPath.ptr = oc_wasm_address_to_ptr(desc->startPath.ptr, desc->startPath.len);
    nativeDesc.startPath.len = desc->startPath.len;

    u32 eltIndex = desc->filters.list.first;
    while(eltIndex)
    {
        oc_wasm_str8_elt* elt = oc_wasm_address_to_ptr(eltIndex, sizeof(oc_wasm_str8_elt));
        oc_str8 filter = oc_wasm_str8_to_native(elt->string);

        oc_str8_list_push(scratch.arena, &nativeDesc.filters, filter);

        oc_log_info("filter: %.*s\n", (int)filter.len, filter.ptr);

        eltIndex = elt->listElt.next;
    }

    oc_file_open_with_dialog_result nativeResult = oc_file_open_with_dialog_for_table(scratch.arena, rights, flags, &nativeDesc, &orca->fileTable);

    oc_wasm_file_open_with_dialog_result result = {
        .button = nativeResult.button,
        .file = nativeResult.file
    };

    oc_list_for(&nativeResult.selection, elt, oc_file_open_with_dialog_elt, listElt)
    {
        oc_wasm_addr wasmEltAddr = oc_wasm_arena_push(wasmArena, sizeof(oc_wasm_file_open_with_dialog_elt));
        oc_wasm_file_open_with_dialog_elt* wasmElt = oc_wasm_address_to_ptr(wasmEltAddr, sizeof(oc_wasm_file_open_with_dialog_elt));
        wasmElt->file = elt->file;

        oc_wasm_list_push_back(&result.selection, &wasmElt->listElt);
    }

    oc_scratch_end(scratch);
    return (result);
}
