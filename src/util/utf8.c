//*****************************************************************
//
//	$file: utf8.c $
//	$author: Martin Fouilleul $
//	$date: 05/11/2016 $
//	$revision: $
//	$note: (C) 2016 by Martin Fouilleul - all rights reserved $
//
//*****************************************************************
#include"utf8.h"

//-----------------------------------------------------------------
//	utf-8 gore
//-----------------------------------------------------------------
const u32 offsetsFromUTF8[6] = {
	0x00000000UL, 0x00003080UL, 0x000E2080UL,
	0x03C82080UL, 0xFA082080UL, 0x82082080UL };

const char trailingBytesForUTF8[256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

#define utf8_is_start_byte(c) (((c)&0xc0)!=0x80)

//-----------------------------------------------------------------
//NOTE: getting sizes / offsets / indices
//-----------------------------------------------------------------

u32 utf8_size_from_leading_char(char leadingChar)
{
	return(trailingBytesForUTF8[(unsigned int)(unsigned char)leadingChar] + 1);
}

u32 utf8_codepoint_size(utf32 codePoint)
{
	if(codePoint < 0x80)
	{
		return(1);
	}
	if(codePoint < 0x800)
	{
		return(2);
	}
	if(codePoint < 0x10000)
	{
		return(3);
	}
	if(codePoint < 0x110000)
	{
		return(4);
	}
	return(0);
}

u64 utf8_codepoint_count_for_string(str8 string)
{
	u64 byteOffset = 0;
	u64 codePointIndex = 0;
	for(;
	    (byteOffset < string.len) && (string.ptr[byteOffset] != 0);
	    codePointIndex++)
	{
		utf8_dec decode = utf8_decode_at(string, byteOffset);
		byteOffset += decode.size;
	}
	return(codePointIndex);
}

u64 utf8_byte_count_for_codepoints(str32 codePoints)
{
	//NOTE(martin): return the exact number of bytes taken by the encoded
	//              version of codePoints. (ie do not attempt to provision
	//              for a zero terminator).
	u64 byteCount = 0;
	for(u64 i=0; i<codePoints.len; i++)
	{
		byteCount += utf8_codepoint_size(codePoints.ptr[i]);
	}
	return(byteCount);
}

u64 utf8_next_offset(str8 string, u64 byteOffset)
{
	u64 res = 0;
	if(byteOffset >= string.len)
	{
		res = string.len;
	}
	else
	{
		u64 nextOffset = byteOffset + utf8_size_from_leading_char(string.ptr[byteOffset]);
		res = minimum(nextOffset, string.len);
	}
	return(res);
}

u64 utf8_prev_offset(str8 string, u64 byteOffset)
{
	u64 res = 0;
	if(byteOffset > string.len)
	{
		res = string.len;
	}
	else if(byteOffset)
	{
		byteOffset--;
		while(byteOffset > 0 && !utf8_is_start_byte(string.ptr[byteOffset]))
		{
			byteOffset--;
		}
		res = byteOffset;
	}
	return(res);
}

//-----------------------------------------------------------------
//NOTE: encoding / decoding
//-----------------------------------------------------------------

utf8_dec utf8_decode_at(str8 string, u64 offset)
{
	//NOTE(martin): get the first codepoint in str, and advance index to the
	//              next utf8 character
	//TODO(martin): check for utf-16 surrogate pairs
	utf32 cp = 0;
	u64   sz = 0;

	if(offset >= string.len || !string.ptr[offset])
	{
		cp = 0;
		sz = 1;
	}
	else if( !utf8_is_start_byte(string.ptr[offset]))
	{
		//NOTE(martin): unexpected continuation or invalid character.
		cp = 0xfffd;
		sz = 1;
	}
	else
	{
		int expectedSize = utf8_size_from_leading_char(string.ptr[offset]);
		do
		{
			/*NOTE(martin):
				we shift 6 bits and add the next byte at each round.
				at the end we have our utf8 codepoint, added to the shifted versions
				of the utf8 leading bits for each encoded byte. These values are
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

		} while(  offset < string.len
		       && string.ptr[offset]
		       && !utf8_is_start_byte(string.ptr[offset])
		       && sz < expectedSize);

		if(sz != expectedSize)
		{
			//NOTE(martin): if we encountered an error, we return the replacement codepoint U+FFFD
			cp = 0xfffd;
		}
		else
		{
			cp -= offsetsFromUTF8[sz-1];

			//NOTE(martin): check for invalid codepoints
			if(cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff))
			{
				cp = 0xfffd;
			}
		}
	}
	utf8_dec res = {.codepoint = cp, .size = sz};
	return(res);
}

utf8_dec utf8_decode(str8 string)
{
	return(utf8_decode_at(string, 0));
}

str8 utf8_encode(char* dest, utf32 codePoint)
{
	u64 sz = 0;
	if (codePoint < 0x80)
	{
		dest[0] = (char)codePoint;
		sz = 1;
	}
	else if (codePoint < 0x800)
	{
		dest[0] = (codePoint>>6) | 0xC0;
		dest[1] = (codePoint & 0x3F) | 0x80;
		sz = 2;
	}
	else if (codePoint < 0x10000)
	{
		dest[0] = (codePoint>>12) | 0xE0;
		dest[1] = ((codePoint>>6) & 0x3F) | 0x80;
		dest[2] = (codePoint & 0x3F) | 0x80;
		sz = 3;
	}
	else if (codePoint < 0x110000)
	{
		dest[0] = (codePoint>>18) | 0xF0;
		dest[1] = ((codePoint>>12) & 0x3F) | 0x80;
		dest[2] = ((codePoint>>6) & 0x3F) | 0x80;
		dest[3] = (codePoint & 0x3F) | 0x80;
		sz = 4;
	}
	str8 res = {.len = sz, .ptr = dest};
	return(res);
}

str32 utf8_to_codepoints(u64 maxCount, utf32* backing, str8 string)
{
	u64 codePointIndex = 0;
	u64 byteOffset = 0;
	for(; codePointIndex < maxCount && byteOffset < string.len; codePointIndex++)
	{
		utf8_dec decode = utf8_decode_at(string, byteOffset);
		backing[codePointIndex] = decode.codepoint;
		byteOffset += decode.size;
	}
	str32 res = {.len = codePointIndex, .ptr = backing};
	return(res);
}

str8 utf8_from_codepoints(u64 maxBytes, char* backing, str32 codePoints)
{
	u64 byteOffset = 0;
	for(u64 codePointIndex = 0; (codePointIndex < codePoints.len); codePointIndex++)
	{
		utf32 codePoint = codePoints.ptr[codePointIndex];
		u32 byteCount = utf8_codepoint_size(codePoint);
		if(byteOffset + byteCount > maxBytes)
		{
			break;
		}
		utf8_encode(backing+byteOffset, codePoint);
		byteOffset +=  byteCount;
	}
	str8 res = {.len = byteOffset, .ptr = backing};
	return(res);
}

str32 utf8_push_to_codepoints(mem_arena* arena, str8 string)
{
	u64 count = utf8_codepoint_count_for_string(string);
	utf32* backing = mem_arena_alloc_array(arena, utf32, count);
	str32 res = utf8_to_codepoints(count, backing, string);
	return(res);
}

str8 utf8_push_from_codepoints(mem_arena* arena, str32 codePoints)
{
	u64 count = utf8_byte_count_for_codepoints(codePoints);
	char* backing = mem_arena_alloc_array(arena, char, count);
	str8 res = utf8_from_codepoints(count, backing, codePoints);
	return(res);
}

#define UNICODE_RANGE(start, cnt, name) MP_API const unicode_range _cat2_(UNICODE_RANGE_, name) = { .firstCodePoint = start, .count = cnt };
UNICODE_RANGES
#undef UNICODE_RANGE
