/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <math.h>

#include "util/typedefs.h"
#include "util/strings.h"
#include "util/macros.h"
#include "reader.h"

wa_reader wa_reader_from_str8(oc_str8 string)
{
    wa_reader reader = {
        .contents = string,
    };
    return reader;
}

void wa_reader_set_error_callback(wa_reader* reader, wa_reader_error_callback callback, void* user)
{
    reader->errorCallback = callback;
    reader->errorData = user;
}

void wa_reader_error(wa_reader* reader, wa_reader_status status, const char* fmt, ...)
{
    if(reader->status == WA_READER_FATAL)
    {
        //NOTE: stop emitting errors if reader is already in fatal error state
        return;
    }

    reader->status = oc_max(reader->status, status);

    oc_arena_scope scratch = oc_scratch_begin();

    va_list ap;
    va_start(ap, fmt);
    oc_str8 message = oc_str8_pushfv(scratch.arena, fmt, ap);
    va_end(ap);

    if(reader->errorCallback)
    {
        reader->errorCallback(reader, message, reader->errorData);
    }
    else
    {
        oc_log_error("Reader error at offset 0x%08llx: %.*s\n", wa_reader_offset(reader), oc_str8_ip(message));
    }
    oc_scratch_end(scratch);
}

bool wa_reader_out_of_bounds(wa_reader* reader, u64 size)
{
    return (reader->offset + size > reader->contents.len || reader->offset + size < reader->offset);
}

bool wa_reader_has_more(wa_reader* reader)
{
    return reader->offset < reader->contents.len;
}

u64 wa_reader_offset(wa_reader* reader)
{
    return reader->offset;
}

void wa_reader_seek(wa_reader* reader, u64 offset)
{
    if(offset > reader->contents.len)
    {
        wa_reader_error(reader, WA_READER_FATAL, "Seek out of bounds.");
    }
    else
    {
        reader->offset = offset;
    }
}

void wa_reader_skip(wa_reader* reader, u64 n)
{
    if(wa_reader_out_of_bounds(reader, n))
    {
        wa_reader_error(reader, WA_READER_FATAL, "Skip out of bounds.");
    }
    else
    {
        reader->offset = reader->offset + n;
    }
}

wa_reader wa_reader_subreader(wa_reader* reader, u64 start, u64 len)
{
    wa_reader sub = {
        .errorCallback = reader->errorCallback,
        .errorData = reader->errorData,
    };

    u64 end = start + len;
    if(end < start || wa_reader_out_of_bounds(reader, end))
    {
        //NOTE: if end wrap or is out of bounds, error and restrict it to contents length
        wa_reader_error(reader, WA_READER_ERROR, "Couldn't create subreader, size out of range.");
        end = reader->contents.len;
    }
    start = oc_min(start, reader->contents.len);

    sub.contents = oc_str8_slice(reader->contents, start, end);

    //NOTE: saturate add baseloc to start
    if(reader->baseLoc + start < reader->baseLoc)
    {
        reader->baseLoc = UINT64_MAX;
    }
    else
    {
        sub.baseLoc = reader->baseLoc + start;
    }

    return sub;
}

u64 wa_read_leb128(wa_reader* reader, u32 bitWidth, bool isSigned)
{
    char byte = 0;
    u64 shift = 0;
    u64 acc = 0;
    u64 count = 0;
    u32 maxCount = (u32)ceil(bitWidth / 7.);

    do
    {
        if(wa_reader_out_of_bounds(reader, sizeof(char)))
        {
            wa_reader_error(reader, WA_READER_FATAL, "Couldn't read leb128: read out of bounds.");
            goto end;
        }

        byte = reader->contents.ptr[reader->offset];
        reader->offset++;

        acc |= ((u64)byte & 0x7f) << shift;
        shift += 7;
        count++;
    }
    while(byte & 0x80);

    if(count > maxCount)
    {
        wa_reader_error(reader, WA_READER_ERROR, "Malformed leb128: too large for bitWidth %u.", bitWidth);
        acc = 0;
        goto end;
    }

    if(isSigned)
    {
        if(count == maxCount)
        {
            //NOTE: the spec mandates that unused bits must match the sign bit,
            // so we construct a mask to select the sign bit and the unused bits,
            // and we check that they're either all 1 or all 0
            u8 bitsInLastByte = (bitWidth - (maxCount - 1) * 7);
            u8 lastByteMask = (0xff << (bitsInLastByte - 1)) & 0x7f;
            u8 bits = byte & lastByteMask;

            if(bits != 0 && bits != lastByteMask)
            {
                wa_reader_error(reader, WA_READER_ERROR, "Malformed signed leb128: unused bits don't match sign bit.");
                acc = 0;
                goto end;
            }
        }

        if(shift < 64 && (byte & 0x40))
        {
            acc |= (~0ULL << shift);
        }
    }
    else
    {
        if(count == maxCount)
        {
            //NOTE: for unsigned the spec mandates that unused bits must be zero,
            // so we construct a mask to select only unused bits,
            // and we check that they're all 0
            u8 bitsInLastByte = (bitWidth - (maxCount - 1) * 7);
            u8 lastByteMask = (0xff << (bitsInLastByte)) & 0x7f;
            u8 bits = byte & lastByteMask;

            if(bits != 0)
            {
                wa_reader_error(reader, WA_READER_ERROR, "Malformed unsigned leb128: unused bits not zero.");
                acc = 0;
                goto end;
            }
        }
    }
end:
    return acc;
}

u32 wa_read_leb128_u32(wa_reader* reader)
{
    return (u32)wa_read_leb128(reader, 32, false);
}

u64 wa_read_leb128_u64(wa_reader* reader)
{
    return wa_read_leb128(reader, 64, false);
}

i32 wa_read_leb128_i32(wa_reader* reader)
{
    return (i32)wa_read_leb128(reader, 32, true);
}

i64 wa_read_leb128_i64(wa_reader* reader)
{
    return (i64)wa_read_leb128(reader, 64, true);
}

u64 wa_read_u64(wa_reader* reader)
{
    u64 res = 0;
    if(wa_reader_out_of_bounds(reader, sizeof(u64)))
    {
        wa_reader_error(reader, WA_READER_FATAL, "Couldn't read u64: read out of bounds.");
    }
    else
    {
        memcpy(&res, reader->contents.ptr + reader->offset, sizeof(u64));
        reader->offset += sizeof(u64);
    }
    return res;
}

u32 wa_read_u32(wa_reader* reader)
{
    u32 res = 0;
    if(wa_reader_out_of_bounds(reader, sizeof(u32)))
    {
        wa_reader_error(reader, WA_READER_FATAL, "Couldn't read u32: read out of bounds.");
    }
    else
    {
        memcpy(&res, reader->contents.ptr + reader->offset, sizeof(u32));
        reader->offset += sizeof(u32);
    }
    return res;
}

u16 wa_read_u16(wa_reader* reader)
{
    u16 res = 0;
    if(wa_reader_out_of_bounds(reader, sizeof(u16)))
    {
        wa_reader_error(reader, WA_READER_FATAL, "Couldn't read u16: read out of bounds.");
    }
    else
    {
        memcpy(&res, reader->contents.ptr + reader->offset, sizeof(u16));
        reader->offset += sizeof(u16);
    }
    return res;
}

u8 wa_read_u8(wa_reader* reader)
{
    u8 res = 0;
    if(wa_reader_out_of_bounds(reader, sizeof(u8)))
    {
        wa_reader_error(reader, WA_READER_FATAL, "Couldn't read u8: read out of bounds.");
    }
    else
    {
        memcpy(&res, reader->contents.ptr + reader->offset, sizeof(u8));
        reader->offset += sizeof(u8);
    }
    return res;
}

f32 wa_read_f32(wa_reader* reader)
{
    f32 res = 0;
    if(wa_reader_out_of_bounds(reader, sizeof(f32)))
    {
        wa_reader_error(reader, WA_READER_FATAL, "Couldn't read f32: read out of bounds.");
    }
    else
    {
        memcpy(&res, reader->contents.ptr + reader->offset, sizeof(f32));
        reader->offset += sizeof(f32);
    }
    return res;
}

f64 wa_read_f64(wa_reader* reader)
{
    f64 res = 0;
    if(wa_reader_out_of_bounds(reader, sizeof(f64)))
    {
        wa_reader_error(reader, WA_READER_FATAL, "Couldn't read f64: read out of bounds.");
    }
    else
    {
        memcpy(&res, reader->contents.ptr + reader->offset, sizeof(f64));
        reader->offset += sizeof(f64);
    }
    return res;
}

u8 wa_reader_peek_u8(wa_reader* reader)
{
    u8 res = 0;
    if(wa_reader_out_of_bounds(reader, sizeof(u8)))
    {
        wa_reader_error(reader, WA_READER_FATAL, "Couldn't peek u8: read out of bounds.");
    }
    else
    {
        memcpy(&res, reader->contents.ptr + reader->offset, sizeof(u8));
    }
    return res;
}

oc_str8 wa_read_bytes(wa_reader* reader, u64 len)
{
    oc_str8 res = { 0 };
    if(wa_reader_out_of_bounds(reader, len))
    {
        wa_reader_error(reader, WA_READER_FATAL, "Couldn't read bytes: read out of bounds.");
    }
    else
    {
        res = (oc_str8){
            .ptr = reader->contents.ptr + reader->offset,
            .len = len,
        };
        reader->offset += len;
    }
    return res;
}

oc_str8 wa_read_cstring(wa_reader* reader)
{
    oc_str8 res = { 0 };
    if(reader->offset >= reader->contents.len)
    {
        wa_reader_error(reader, WA_READER_FATAL, "Couldn't read null-terminated string: read out of bounds.");
    }
    else
    {
        size_t len = strnlen(reader->contents.ptr + reader->offset,
                             reader->contents.len - reader->offset);

        if(wa_reader_out_of_bounds(reader, len + 1))
        {
            //NOTE len+1 since we also need to fit a null byte
            wa_reader_error(reader, WA_READER_FATAL, "Couldn't read null-terminated string: read out of bounds.");
        }
        else
        {
            res = (oc_str8){
                .ptr = reader->contents.ptr + reader->offset,
                .len = len,
            };
            reader->offset += len + 1;
        }
    }
    return res;
}
