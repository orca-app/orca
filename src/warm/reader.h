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

typedef enum wa_reader_status
{
    WA_READER_OK = 0,
    WA_READER_ERROR,
    WA_READER_FATAL,
} wa_reader_status;

typedef struct wa_reader wa_reader;
typedef void (*wa_reader_error_callback)(wa_reader* reader, oc_str8 message, void* user);

typedef struct wa_reader
{
    oc_str8 contents;
    u64 baseLoc;
    u64 offset;

    wa_reader_status status;
    wa_reader_error_callback errorCallback;
    void* errorData;
} wa_reader;

wa_reader wa_reader_from_str8(oc_str8 contents);

wa_reader wa_reader_subreader(wa_reader* reader, u64 start, u64 len);
void wa_reader_set_error_callback(wa_reader* reader, wa_reader_error_callback callback, void* user);

bool wa_reader_has_more(wa_reader* reader);
u64 wa_reader_offset(wa_reader* reader);
void wa_reader_seek(wa_reader* reader, u64 offset);
void wa_reader_skip(wa_reader* reader, u64 n);

u64 wa_read_leb128(wa_reader* reader, u32 bitWidth, bool isSigned);
u32 wa_read_leb128_u32(wa_reader* reader);
u64 wa_read_leb128_u64(wa_reader* reader);
i32 wa_read_leb128_i32(wa_reader* reader);
i64 wa_read_leb128_i64(wa_reader* reader);

u8 wa_read_u8(wa_reader* reader);
u16 wa_read_u16(wa_reader* reader);
u32 wa_read_u32(wa_reader* reader);
u64 wa_read_u64(wa_reader* reader);
f32 wa_read_f32(wa_reader* reader);
f64 wa_read_f64(wa_reader* reader);

oc_str8 wa_read_bytes(wa_reader* reader, u64 len);
oc_str8 wa_read_cstring(wa_reader* reader);

u8 wa_reader_peek_u8(wa_reader* reader);
