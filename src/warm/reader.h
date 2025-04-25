/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "util/typedefs.h"
#include "util/strings.h"

typedef struct dw_reader
{
    oc_str8 contents;
    u64 offset;
    //TODO: error
} dw_reader;

dw_reader dw_reader_from_str8(oc_str8 string);
bool dw_reader_has_more(dw_reader* reader);
u64 dw_reader_offset(dw_reader* reader);
void dw_reader_seek(dw_reader* reader, u64 offset);
void dw_reader_skip(dw_reader* reader, u64 n);
dw_reader dw_reader_subreader(dw_reader* reader, u64 size);

u64 dw_read_leb128(dw_reader* reader, u32 bitWidth, bool isSigned);
u32 dw_read_leb128_u32(dw_reader* reader);
u64 dw_read_leb128_u64(dw_reader* reader);
i32 dw_read_leb128_i32(dw_reader* reader);
i64 dw_read_leb128_i64(dw_reader* reader);

u64 dw_read_u64(dw_reader* reader);
u32 dw_read_u32(dw_reader* reader);
u16 dw_read_u16(dw_reader* reader);
u8 dw_read_u8(dw_reader* reader);

oc_str8 dw_read_bytes(dw_reader* reader, u64 len);
oc_str8 dw_read_cstring(dw_reader* reader);

u8 dw_reader_peek_u8(dw_reader* reader);
