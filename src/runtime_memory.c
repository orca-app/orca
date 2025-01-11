/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <limits.h>
#include <assert.h>
#include "runtime.h"
#include "runtime_memory.h"
#include "wasm/wasm.h"

void* oc_runtime_wasm_memory_resize_callback(void* p, unsigned long newSize, void* userData)
{
    //NOTE: This is called by the wasm layer.
    //      For the wasm3 backend, the size passed includes the wasm3 memory header.
    //      We first align it on 4K page size.

    newSize = oc_align_up_pow2(newSize, WA_PAGE_SIZE);

    wa_memory* memory = (wa_memory*)userData;
    u64 committed = (u64)memory->limits.min * WA_PAGE_SIZE;
    u64 reserved = (u64)memory->limits.max * WA_PAGE_SIZE;

    if(committed >= newSize)
    {
        return (memory->ptr);
    }
    else if(newSize <= reserved)
    {
        u32 commitSize = newSize - committed;

        oc_base_allocator* allocator = oc_base_allocator_default();
        oc_base_commit(allocator, memory->ptr + committed, commitSize);
        memory->limits.min += commitSize / WA_PAGE_SIZE;

        return (memory->ptr);
    }
    else
    {
        OC_ABORT("Out of memory");
        return (0);
    }
}

void oc_runtime_wasm_memory_free_callback(void* p, void* userData)
{
    wa_memory* memory = (wa_memory*)userData;

    oc_base_allocator* allocator = oc_base_allocator_default();
    oc_base_release(allocator, memory->ptr, memory->limits.max * WA_PAGE_SIZE);
    memset(memory, 0, sizeof(wa_memory));
}

extern u32 oc_mem_grow(u64 size)
{
    oc_wasm_env* env = oc_runtime_get_env();

    u64 oldMemSize = oc_wasm_mem_size(env->wasm);

    //NOTE: compute total size and align on wasm memory page size
    OC_ASSERT(oldMemSize <= UINT_MAX - size, "Memory size overflow");
    u64 desiredMemSize = size + oldMemSize;

    desiredMemSize = oc_align_up_pow2(desiredMemSize, OC_WASM_MEM_PAGE_SIZE);

    //NOTE: call resize memory, which will call our custom resize callback... this is a bit involved because
    //      wasm3 doesn't allow resizing the memory directly
    wa_status status = oc_wasm_mem_resize(env->wasm, desiredMemSize / OC_WASM_MEM_PAGE_SIZE);
    OC_WASM_TRAP(status);

    u64 newMemSize = oc_wasm_mem_size(env->wasm);
    OC_DEBUG_ASSERT(oldMemSize + size <= newMemSize, "Memory returned by oc_mem_grow overflows wasm memory");

    return (oldMemSize);
}

void* oc_wasm_address_to_ptr(oc_wasm_addr addr, oc_wasm_size size)
{
    oc_str8 mem = oc_runtime_get_wasm_memory();
    OC_ASSERT(addr + size < mem.len, "Object overflows wasm memory");

    void* ptr = (addr == 0) ? 0 : mem.ptr + addr;
    return (ptr);
}

oc_wasm_addr oc_wasm_address_from_ptr(void* ptr, oc_wasm_size size)
{
    oc_wasm_addr addr = 0;
    if(ptr != 0)
    {
        oc_str8 mem = oc_runtime_get_wasm_memory();
        OC_ASSERT((char*)ptr > mem.ptr && (((char*)ptr - mem.ptr) + size < mem.len), "Object overflows wasm memory");

        addr = (char*)ptr - mem.ptr;
    }
    return (addr);
}

//------------------------------------------------------------------------------------
// oc_wasm_list helpers
//------------------------------------------------------------------------------------

void oc_wasm_list_push(oc_wasm_list* list, oc_wasm_list_elt* elt)
{
    elt->next = list->first;
    elt->prev = 0;

    oc_wasm_addr eltAddr = oc_wasm_address_from_ptr(elt, sizeof(oc_wasm_list_elt));

    if(list->first)
    {
        oc_wasm_list_elt* first = oc_wasm_address_to_ptr(list->first, sizeof(oc_wasm_list_elt));
        first->prev = eltAddr;
    }
    else
    {
        list->last = eltAddr;
    }
    list->first = eltAddr;
}

void oc_wasm_list_push_back(oc_wasm_list* list, oc_wasm_list_elt* elt)
{
    elt->prev = list->last;
    elt->next = 0;

    oc_wasm_addr eltAddr = oc_wasm_address_from_ptr(elt, sizeof(oc_wasm_list_elt));

    if(list->last)
    {
        oc_wasm_list_elt* last = oc_wasm_address_to_ptr(list->last, sizeof(oc_wasm_list_elt));
        last->next = eltAddr;
    }
    else
    {
        list->first = eltAddr;
    }
    list->last = eltAddr;
}

//------------------------------------------------------------------------------------
// Wasm arenas helpers
//------------------------------------------------------------------------------------

oc_wasm_addr oc_wasm_arena_push(oc_wasm_addr arena, u64 size)
{
    oc_wasm_env* env = oc_runtime_get_env();

    wa_value params[2];
    params[0].valI32 = arena;
    params[1].valI64 = size;

    wa_value returns[1];

    wa_status status = oc_wasm_function_call(env->wasm, env->exports[OC_EXPORT_ARENA_PUSH], params, 2, returns, 1);
    OC_WASM_TRAP(status);

    static_assert(sizeof(oc_wasm_addr) == sizeof(i32), "wasm addres should be 32 bits");
    return (oc_wasm_addr)returns[0].valI32;
}
