/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "wasm/wasm.h"

void* oc_wasm_address_to_ptr(oc_wasm_addr addr, oc_wasm_size size);
oc_wasm_addr oc_wasm_address_from_ptr(void* ptr, oc_wasm_size size);

//------------------------------------------------------------------------------------
// oc_wasm_list helpers
//------------------------------------------------------------------------------------

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

bool oc_wasm_list_empty(oc_wasm_list* list);

#define oc_wasm_list_begin(list) oc_wasm_address_to_ptr(list->first, sizeof(oc_wasm_list_elt))
#define oc_wasm_list_end(list) oc_wasm_address_to_ptr(list->last, sizeof(oc_wasm_list_elt))

#define oc_wasm_list_next(elt) oc_wasm_address_to_ptr((elt)->next, sizeof(oc_wasm_list_elt))
#define oc_wasm_list_prev(elt) oc_wasm_address_to_ptr((elt)->prev, sizeof(oc_wasm_list_elt))

#define oc_wasm_list_entry(ptr, type, member) \
    oc_container_of(ptr, type, member)

#define oc_wasm_list_next_entry(list, elt, type, member) \
    (((elt)->member.next != 0) ? oc_wasm_list_entry(oc_wasm_list_next((elt)->member), type, member) : 0)

#define oc_wasm_list_prev_entry(list, elt, type, member) \
    (((elt)->member.prev != 0) ? oc_wasm_list_entry(oc_wasm_list_prev((elt)->member), type, member) : 0)

#define oc_wasm_list_checked_entry(elt, type, member) \
    (((elt) != 0) ? oc_wasm_list_entry(elt, type, member) : 0)

#define oc_wasm_list_first_entry(list, type, member) \
    (oc_wasm_list_checked_entry(oc_wasm_list_begin(list), type, member))

#define oc_wasm_list_last_entry(list, type, member) \
    (oc_wasm_list_checked_entry(oc_wasm_list_last(list), type, member))

#define oc_wasm_list_for(list, elt, type, member)                                       \
    for(type* elt = oc_wasm_list_checked_entry(oc_wasm_list_begin(list), type, member); \
        elt != 0;                                                                       \
        elt = oc_wasm_list_checked_entry(oc_wasm_list_next((elt)->member), type, member))

void oc_wasm_list_push(oc_wasm_list* list, oc_wasm_list_elt* elt);
void oc_wasm_list_push_back(oc_wasm_list* list, oc_wasm_list_elt* elt);

//------------------------------------------------------------------------------------
// oc_wasm_str8 helpers
//------------------------------------------------------------------------------------

typedef struct oc_wasm_str8
{
    u32 ptr;
    u32 len; //NOTE: size_t is 4 bytes on wasm32
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

#define oc_wasm_str8_to_native(wasmString) ((oc_str8){ .ptr = oc_wasm_address_to_ptr(wasmString.ptr, wasmString.len), .len = wasmString.len })

//------------------------------------------------------------------------------------
// Wasm arenas helpers
//------------------------------------------------------------------------------------

oc_wasm_addr oc_wasm_arena_push(oc_wasm_addr arena, u64 size);
