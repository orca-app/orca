/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "utf8.h"

//-----------------------------------------------------------------
//	utf-8 gore
//-----------------------------------------------------------------
static const u32 offsetsFromUTF8[6] = {
    0x00000000UL, 0x00003080UL, 0x000E2080UL,
    0x03C82080UL, 0xFA082080UL, 0x82082080UL
};

static const char trailingBytesForUTF8[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
};

#define oc_utf8_is_start_byte(c) (((c)&0xc0) != 0x80)

//-----------------------------------------------------------------
//NOTE: getting sizes / offsets / indices
//-----------------------------------------------------------------

u32 oc_utf8_size_from_leading_char(char leadingChar)
{
    return (trailingBytesForUTF8[(unsigned int)(unsigned char)leadingChar] + 1);
}

u32 oc_utf8_codepoint_size(oc_utf32 codePoint)
{
    if(codePoint < 0x80)
    {
        return (1);
    }
    if(codePoint < 0x800)
    {
        return (2);
    }
    if(codePoint < 0x10000)
    {
        return (3);
    }
    if(codePoint < 0x110000)
    {
        return (4);
    }
    return (0);
}

u64 oc_utf8_codepoint_count_for_string(oc_str8 string)
{
    u64 byteOffset = 0;
    u64 codePointIndex = 0;
    for(;
        (byteOffset < string.len) && (string.ptr[byteOffset] != 0);
        codePointIndex++)
    {
        oc_utf8_dec decode = oc_utf8_decode_at(string, byteOffset);
        byteOffset += decode.size;
    }
    return (codePointIndex);
}

u64 oc_utf8_byte_count_for_codepoints(oc_str32 codePoints)
{
    //NOTE(martin): return the exact number of bytes taken by the encoded
    //              version of codePoints. (ie do not attempt to provision
    //              for a zero terminator).
    u64 byteCount = 0;
    for(u64 i = 0; i < codePoints.len; i++)
    {
        byteCount += oc_utf8_codepoint_size(codePoints.ptr[i]);
    }
    return (byteCount);
}

u64 oc_utf8_next_offset(oc_str8 string, u64 byteOffset)
{
    u64 res = 0;
    if(byteOffset >= string.len)
    {
        res = string.len;
    }
    else
    {
        u64 nextOffset = byteOffset + oc_utf8_size_from_leading_char(string.ptr[byteOffset]);
        res = oc_min(nextOffset, string.len);
    }
    return (res);
}

u64 oc_utf8_prev_offset(oc_str8 string, u64 byteOffset)
{
    u64 res = 0;
    if(byteOffset > string.len)
    {
        res = string.len;
    }
    else if(byteOffset)
    {
        byteOffset--;
        while(byteOffset > 0 && !oc_utf8_is_start_byte(string.ptr[byteOffset]))
        {
            byteOffset--;
        }
        res = byteOffset;
    }
    return (res);
}

//-----------------------------------------------------------------
//NOTE: encoding / decoding
//-----------------------------------------------------------------

oc_utf8_dec oc_utf8_decode_at(oc_str8 string, u64 offset)
{
    //NOTE(martin): get the first codepoint in str, and advance index to the
    //              next oc_utf8 character
    //TODO(martin): check for utf-16 surrogate pairs
    oc_utf32 cp = 0;
    u64 sz = 0;

    if(offset >= string.len || !string.ptr[offset])
    {
        cp = 0;
        sz = 1;
    }
    else if(!oc_utf8_is_start_byte(string.ptr[offset]))
    {
        //NOTE(martin): unexpected continuation or invalid character.
        cp = 0xfffd;
        sz = 1;
    }
    else
    {
        int expectedSize = oc_utf8_size_from_leading_char(string.ptr[offset]);
        do
        {
            /*NOTE(martin):
				we shift 6 bits and add the next byte at each round.
				at the end we have our oc_utf8 codepoint, added to the shifted versions
				of the oc_utf8 leading bits for each encoded byte. These values are
				precomputed in offsetsFromUTF8.
			*/
            unsigned char b = string.ptr[offset];
            cp <<= 6;
            cp += b;
            offset += 1;
            sz++;

            if(b == 0xc0 || b == 0xc1 || b >= 0xc5)
            {
                //NOTE(martin): invalid byte encountered
                break;
            }
        }
        while(offset < string.len
              && string.ptr[offset]
              && !oc_utf8_is_start_byte(string.ptr[offset])
              && sz < expectedSize);

        if(sz != expectedSize)
        {
            //NOTE(martin): if we encountered an error, we return the replacement codepoint U+FFFD
            cp = 0xfffd;
        }
        else
        {
            cp -= offsetsFromUTF8[sz - 1];

            //NOTE(martin): check for invalid codepoints
            if(cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff))
            {
                cp = 0xfffd;
            }
        }
    }
    oc_utf8_dec res = { .codepoint = cp, .size = sz };
    return (res);
}

oc_utf8_dec oc_utf8_decode(oc_str8 string)
{
    return (oc_utf8_decode_at(string, 0));
}

oc_str8 oc_utf8_encode(char* dest, oc_utf32 codePoint)
{
    u64 sz = 0;
    if(codePoint < 0x80)
    {
        dest[0] = (char)codePoint;
        sz = 1;
    }
    else if(codePoint < 0x800)
    {
        dest[0] = (codePoint >> 6) | 0xC0;
        dest[1] = (codePoint & 0x3F) | 0x80;
        sz = 2;
    }
    else if(codePoint < 0x10000)
    {
        dest[0] = (codePoint >> 12) | 0xE0;
        dest[1] = ((codePoint >> 6) & 0x3F) | 0x80;
        dest[2] = (codePoint & 0x3F) | 0x80;
        sz = 3;
    }
    else if(codePoint < 0x110000)
    {
        dest[0] = (codePoint >> 18) | 0xF0;
        dest[1] = ((codePoint >> 12) & 0x3F) | 0x80;
        dest[2] = ((codePoint >> 6) & 0x3F) | 0x80;
        dest[3] = (codePoint & 0x3F) | 0x80;
        sz = 4;
    }
    oc_str8 res = {.ptr = dest , .len = sz};
    return (res);
}

oc_str32 oc_utf8_to_codepoints(u64 maxCount, oc_utf32* backing, oc_str8 string)
{
    u64 codePointIndex = 0;
    u64 byteOffset = 0;
    for(; codePointIndex < maxCount && byteOffset < string.len; codePointIndex++)
    {
        oc_utf8_dec decode = oc_utf8_decode_at(string, byteOffset);
        backing[codePointIndex] = decode.codepoint;
        byteOffset += decode.size;
    }
    oc_str32 res = {.ptr = backing , .len = codePointIndex};
    return (res);
}

oc_str8 oc_utf8_from_codepoints(u64 maxBytes, char* backing, oc_str32 codePoints)
{
    u64 byteOffset = 0;
    for(u64 codePointIndex = 0; (codePointIndex < codePoints.len); codePointIndex++)
    {
        oc_utf32 codePoint = codePoints.ptr[codePointIndex];
        u32 byteCount = oc_utf8_codepoint_size(codePoint);
        if(byteOffset + byteCount > maxBytes)
        {
            break;
        }
        oc_utf8_encode(backing + byteOffset, codePoint);
        byteOffset += byteCount;
    }
    oc_str8 res = {.ptr = backing , .len = byteOffset};
    return (res);
}

oc_str32 oc_utf8_push_to_codepoints(oc_arena* arena, oc_str8 string)
{
    u64 count = oc_utf8_codepoint_count_for_string(string);
    oc_utf32* backing = oc_arena_push_array(arena, oc_utf32, count);
    oc_str32 res = oc_utf8_to_codepoints(count, backing, string);
    return (res);
}

oc_str8 oc_utf8_push_from_codepoints(oc_arena* arena, oc_str32 codePoints)
{
    u64 count = oc_utf8_byte_count_for_codepoints(codePoints);
    char* backing = oc_arena_push_array(arena, char, count);
    oc_str8 res = oc_utf8_from_codepoints(count, backing, codePoints);
    return (res);
}

#define OC_UNICODE_RANGE(start, cnt, name) ORCA_API const oc_unicode_range OC_CAT2(OC_UNICODE_, name) = { .firstCodePoint = start, .count = cnt };
OC_UNICODE_RANGES
#undef OC_UNICODE_RANGE
