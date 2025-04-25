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
    //TODO: check offset and overflows
    reader->offset = oc_min(reader->contents.len, offset);
}

void wa_reader_skip(wa_reader* reader, u64 n)
{
    //TODO: check offset and overflows
    reader->offset = oc_min(reader->contents.len, reader->offset + n);
}

wa_reader wa_reader_subreader(wa_reader* reader, u64 size)
{
    if(reader->offset + size > reader->contents.len)
    {
        oc_log_error("Couldn't create subreader, size out of range\n");
    }

    wa_reader sub = {
        .contents = oc_str8_slice(reader->contents, reader->offset, reader->offset + size),
        .offset = 0,
    };
    return sub;
}

u64 wa_read_leb128(wa_reader* reader, u32 bitWidth, bool isSigned)
{
    char byte = 0;
    u64 shift = 0;
    u64 acc = 0;
    u32 count = 0;
    u32 maxCount = (u32)ceil(bitWidth / 7.);

    do
    {
        if(reader->offset + sizeof(char) > reader->contents.len)
        {
            oc_log_error("Couldn't read leb128: unexpected end of file.\n");
            goto end;
        }

        if(count >= maxCount)
        {
            oc_log_error("Couldn't read leb128: too large for bitWidth.\n");
            acc = 0;
            goto end;
        }

        byte = reader->contents.ptr[reader->offset];
        reader->offset++;

        acc |= ((u64)byte & 0x7f) << shift;
        shift += 7;
        count++;
    }
    while(byte & 0x80);

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
                oc_log_error("Couldn't read signed leb128: unused bits don't match sign bit.\n");
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
                oc_log_error("Couldn't read unsigned leb128: unused bits not zero.\n");
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
    if(reader->offset + sizeof(u64) > reader->contents.len)
    {
        oc_log_error("read out of bounds\n");
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
    if(reader->offset + sizeof(u32) > reader->contents.len)
    {
        oc_log_error("read out of bounds\n");
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
    if(reader->offset + sizeof(u16) > reader->contents.len)
    {
        oc_log_error("read out of bounds\n");
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
    if(reader->offset + sizeof(u8) > reader->contents.len)
    {
        oc_log_error("read out of bounds\n");
    }
    else
    {
        memcpy(&res, reader->contents.ptr + reader->offset, sizeof(u8));
        reader->offset += sizeof(u8);
    }
    return res;
}

u64 wa_read_f32(wa_reader* reader)
{
    f32 res = 0;
    if(reader->offset + sizeof(f32) > reader->contents.len)
    {
        oc_log_error("read out of bounds\n");
    }
    else
    {
        memcpy(&res, reader->contents.ptr + reader->offset, sizeof(f32));
        reader->offset += sizeof(f32);
    }
    return res;
}

u64 wa_read_f64(wa_reader* reader)
{
    f64 res = 0;
    if(reader->offset + sizeof(f64) > reader->contents.len)
    {
        oc_log_error("read out of bounds\n");
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
    if(reader->offset + sizeof(u8) > reader->contents.len)
    {
        oc_log_error("read out of bounds\n");
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
    if(reader->offset + len > reader->contents.len)
    {
        oc_log_error("read out of bounds\n");
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
        oc_log_error("read out of bounds\n");
    }
    else
    {
        size_t len = strnlen(reader->contents.ptr + reader->offset,
                             reader->contents.len - reader->offset);

        if(reader->offset + len >= reader->contents.len)
        {
            //NOTE >= since we also need to fit a null byte
            oc_log_error("read out of bounds\n");
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
