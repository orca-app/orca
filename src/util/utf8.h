/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#ifndef __UTF8_H_
#define __UTF8_H_

#include "strings.h"
#include "typedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef u32 oc_utf32;

//-----------------------------------------------------------------
//NOTE: getting sizes / offsets / indices
//-----------------------------------------------------------------
#define oc_utf8_is_start_byte(c) (((c)&0xc0) != 0x80)

ORCA_API u32 oc_utf8_size_from_leading_char(char leadingChar);
ORCA_API u32 oc_utf8_codepoint_size(oc_utf32 codePoint);

ORCA_API u64 oc_utf8_codepoint_count_for_string(oc_str8 string);
ORCA_API u64 oc_utf8_byte_count_for_codepoints(oc_str32 codePoints);

ORCA_API u64 oc_utf8_next_offset(oc_str8 string, u64 byteOffset);
ORCA_API u64 oc_utf8_prev_offset(oc_str8 string, u64 byteOffset);

//-----------------------------------------------------------------
//NOTE: encoding / decoding
//-----------------------------------------------------------------

typedef enum oc_utf8_status
{
    OC_UTF8_OK,
    OC_UTF8_OUT_OF_BOUNDS,
    OC_UTF8_UNEXPECTED_CONTINUATION_BYTE,
    OC_UTF8_UNEXPECTED_LEADING_BYTE,
    OC_UTF8_INVALID_BYTE,
    OC_UTF8_INVALID_CODEPOINT,
    OC_UTF8_OVERLONG_ENCODING,
} oc_utf8_status;

typedef struct oc_utf8_dec
{
    oc_utf8_status status;
    oc_utf32 codepoint; //NOTE: decoded codepoint
    u32 size;           //NOTE: size of corresponding oc_utf8 sequence
} oc_utf8_dec;

ORCA_API oc_utf8_dec oc_utf8_decode(oc_str8 string);                //NOTE: decode a single oc_utf8 sequence at start of string
ORCA_API oc_utf8_dec oc_utf8_decode_at(oc_str8 string, u64 offset); //NOTE: decode a single oc_utf8 sequence starting at byte offset
ORCA_API oc_str8 oc_utf8_encode(char* dst, oc_utf32 codePoint);     //NOTE: encode codepoint into backing buffer dst

ORCA_API oc_str32 oc_utf8_to_codepoints(u64 maxCount, oc_utf32* backing, oc_str8 string);
ORCA_API oc_str8 oc_utf8_from_codepoints(u64 maxBytes, char* backing, oc_str32 codePoints);

ORCA_API oc_str32 oc_utf8_push_to_codepoints(oc_arena* arena, oc_str8 string);
ORCA_API oc_str8 oc_utf8_push_from_codepoints(oc_arena* arena, oc_str32 codePoints);

//-----------------------------------------------------------------
// oc_utf8 range struct and X-macros for defining oc_utf8 ranges
//-----------------------------------------------------------------

typedef struct oc_unicode_range
{
    oc_utf32 firstCodePoint;
    u32 count;
} oc_unicode_range;

//NOTE(martin): range declared here are defined in utf8.cpp
//              they can be used by prefixing them with UTF8_RANGE_, as in 'UTF8_RANGE_BASIC_LATIN'
#define OC_UNICODE_RANGES                                                   \
    OC_UNICODE_RANGE(0x0000, 127, BASIC_LATIN)                              \
    OC_UNICODE_RANGE(0x0080, 127, C1_CONTROLS_AND_LATIN_1_SUPPLEMENT)       \
    OC_UNICODE_RANGE(0x0100, 127, LATIN_EXTENDED_A)                         \
    OC_UNICODE_RANGE(0x0180, 207, LATIN_EXTENDED_B)                         \
    OC_UNICODE_RANGE(0x0250, 95, IPA_EXTENSIONS)                            \
    OC_UNICODE_RANGE(0x02b0, 79, SPACING_MODIFIER_LETTERS)                  \
    OC_UNICODE_RANGE(0x0300, 111, COMBINING_DIACRITICAL_MARKS)              \
    OC_UNICODE_RANGE(0x0370, 143, GREEK_COPTIC)                             \
    OC_UNICODE_RANGE(0x0400, 255, CYRILLIC)                                 \
    OC_UNICODE_RANGE(0x0500, 47, CYRILLIC_SUPPLEMENT)                       \
    OC_UNICODE_RANGE(0x0530, 95, ARMENIAN)                                  \
    OC_UNICODE_RANGE(0x0590, 111, HEBREW)                                   \
    OC_UNICODE_RANGE(0x0600, 255, ARABIC)                                   \
    OC_UNICODE_RANGE(0x0700, 79, SYRIAC)                                    \
    OC_UNICODE_RANGE(0x0780, 63, THAANA)                                    \
    OC_UNICODE_RANGE(0x0900, 127, DEVANAGARI)                               \
    OC_UNICODE_RANGE(0x0980, 127, BENGALI_ASSAMESE)                         \
    OC_UNICODE_RANGE(0x0a00, 127, GURMUKHI)                                 \
    OC_UNICODE_RANGE(0x0a80, 127, GUJARATI)                                 \
    OC_UNICODE_RANGE(0x0b00, 127, ORIYA)                                    \
    OC_UNICODE_RANGE(0x0b80, 127, TAMIL)                                    \
    OC_UNICODE_RANGE(0x0c00, 127, TELUGU)                                   \
    OC_UNICODE_RANGE(0x0c80, 127, KANNADA)                                  \
    OC_UNICODE_RANGE(0x0d00, 255, MALAYALAM)                                \
    OC_UNICODE_RANGE(0x0d80, 127, SINHALA)                                  \
    OC_UNICODE_RANGE(0x0e00, 127, THAI)                                     \
    OC_UNICODE_RANGE(0x0e80, 127, LAO)                                      \
    OC_UNICODE_RANGE(0x0f00, 255, TIBETAN)                                  \
    OC_UNICODE_RANGE(0x1000, 159, MYANMAR)                                  \
    OC_UNICODE_RANGE(0x10a0, 95, GEORGIAN)                                  \
    OC_UNICODE_RANGE(0x1100, 255, HANGUL_JAMO)                              \
    OC_UNICODE_RANGE(0x1200, 383, ETHIOPIC)                                 \
    OC_UNICODE_RANGE(0x13a0, 95, CHEROKEE)                                  \
    OC_UNICODE_RANGE(0x1400, 639, UNIFIED_CANADIAN_ABORIGINAL_SYLLABICS)    \
    OC_UNICODE_RANGE(0x1680, 31, OGHAM)                                     \
    OC_UNICODE_RANGE(0x16a0, 95, RUNIC)                                     \
    OC_UNICODE_RANGE(0x1700, 31, TAGALOG)                                   \
    OC_UNICODE_RANGE(0x1720, 31, HANUNOO)                                   \
    OC_UNICODE_RANGE(0x1740, 31, BUHID)                                     \
    OC_UNICODE_RANGE(0x1760, 31, TAGBANWA)                                  \
    OC_UNICODE_RANGE(0x1780, 127, KHMER)                                    \
    OC_UNICODE_RANGE(0x1800, 175, MONGOLIAN)                                \
    OC_UNICODE_RANGE(0x1900, 79, LIMBU)                                     \
    OC_UNICODE_RANGE(0x1950, 47, TAI_LE)                                    \
    OC_UNICODE_RANGE(0x19e0, 31, KHMER_SYMBOLS)                             \
    OC_UNICODE_RANGE(0x1d00, 127, PHONETIC_EXTENSIONS)                      \
    OC_UNICODE_RANGE(0x1e00, 255, LATIN_EXTENDED_ADDITIONAL)                \
    OC_UNICODE_RANGE(0x1f00, 255, GREEK_EXTENDED)                           \
    OC_UNICODE_RANGE(0x2000, 111, GENERAL_PUNCTUATION)                      \
    OC_UNICODE_RANGE(0x2070, 47, SUPERSCRIPTS_AND_SUBSCRIPTS)               \
    OC_UNICODE_RANGE(0x20a0, 47, CURRENCY_SYMBOLS)                          \
    OC_UNICODE_RANGE(0x20d0, 47, COMBINING_DIACRITICAL_MARKS_FOR_SYMBOLS)   \
    OC_UNICODE_RANGE(0x2100, 79, LETTERLIKE_SYMBOLS)                        \
    OC_UNICODE_RANGE(0x2150, 63, NUMBER_FORMS)                              \
    OC_UNICODE_RANGE(0x2190, 111, ARROWS)                                   \
    OC_UNICODE_RANGE(0x2200, 255, MATHEMATICAL_OPERATORS)                   \
    OC_UNICODE_RANGE(0x2300, 255, MISCELLANEOUS_TECHNICAL)                  \
    OC_UNICODE_RANGE(0x2400, 63, CONTROL_PICTURES)                          \
    OC_UNICODE_RANGE(0x2440, 31, OPTICAL_CHARACTER_RECOGNITION)             \
    OC_UNICODE_RANGE(0x2460, 159, ENCLOSED_ALPHANUMERICS)                   \
    OC_UNICODE_RANGE(0x2500, 127, BOX_DRAWING)                              \
    OC_UNICODE_RANGE(0x2580, 31, BLOCK_ELEMENTS)                            \
    OC_UNICODE_RANGE(0x25a0, 95, GEOMETRIC_SHAPES)                          \
    OC_UNICODE_RANGE(0x2600, 255, MISCELLANEOUS_SYMBOLS)                    \
    OC_UNICODE_RANGE(0x2700, 191, DINGBATS)                                 \
    OC_UNICODE_RANGE(0x27c0, 47, MISCELLANEOUS_MATHEMATICAL_SYMBOLS_A)      \
    OC_UNICODE_RANGE(0x27f0, 15, SUPPLEMENTAL_ARROWS_A)                     \
    OC_UNICODE_RANGE(0x2800, 255, BRAILLE_PATTERNS)                         \
    OC_UNICODE_RANGE(0x2900, 127, SUPPLEMENTAL_ARROWS_B)                    \
    OC_UNICODE_RANGE(0x2980, 127, MISCELLANEOUS_MATHEMATICAL_SYMBOLS_B)     \
    OC_UNICODE_RANGE(0x2a00, 255, SUPPLEMENTAL_MATHEMATICAL_OPERATORS)      \
    OC_UNICODE_RANGE(0x2b00, 255, MISCELLANEOUS_SYMBOLS_AND_ARROWS)         \
    OC_UNICODE_RANGE(0x2e80, 127, CJK_RADICALS_SUPPLEMENT)                  \
    OC_UNICODE_RANGE(0x2f00, 223, KANGXI_RADICALS)                          \
    OC_UNICODE_RANGE(0x2ff0, 15, IDEOGRAPHIC_DESCRIPTION_CHARACTERS)        \
    OC_UNICODE_RANGE(0x3000, 63, CJK_SYMBOLS_AND_PUNCTUATION)               \
    OC_UNICODE_RANGE(0x3040, 95, HIRAGANA)                                  \
    OC_UNICODE_RANGE(0x30a0, 95, KATAKANA)                                  \
    OC_UNICODE_RANGE(0x3100, 47, BOPOMOFO)                                  \
    OC_UNICODE_RANGE(0x3130, 95, HANGUL_COMPATIBILITY_JAMO)                 \
    OC_UNICODE_RANGE(0x3190, 15, KANBUN_KUNTEN)                             \
    OC_UNICODE_RANGE(0x31a0, 31, BOPOMOFO_EXTENDED)                         \
    OC_UNICODE_RANGE(0x31f0, 15, KATAKANA_PHONETIC_EXTENSIONS)              \
    OC_UNICODE_RANGE(0x3200, 255, ENCLOSED_CJK_LETTERS_AND_MONTHS)          \
    OC_UNICODE_RANGE(0x3300, 255, CJK_COMPATIBILITY)                        \
    OC_UNICODE_RANGE(0x3400, 6591, CJK_UNIFIED_IDEOGRAPHS_EXTENSION_A)      \
    OC_UNICODE_RANGE(0x4dc0, 63, YIJING_HEXAGRAM_SYMBOLS)                   \
    OC_UNICODE_RANGE(0x4e00, 20911, CJK_UNIFIED_IDEOGRAPHS)                 \
    OC_UNICODE_RANGE(0xa000, 1167, YI_SYLLABLES)                            \
    OC_UNICODE_RANGE(0xa490, 63, YI_RADICALS)                               \
    OC_UNICODE_RANGE(0xac00, 11183, HANGUL_SYLLABLES)                       \
    OC_UNICODE_RANGE(0xd800, 1023, HIGH_SURROGATE_AREA)                     \
    OC_UNICODE_RANGE(0xdc00, 1023, LOW_SURROGATE_AREA)                      \
    OC_UNICODE_RANGE(0xe000, 6399, PRIVATE_USE_AREA)                        \
    OC_UNICODE_RANGE(0xf900, 511, CJK_COMPATIBILITY_IDEOGRAPHS)             \
    OC_UNICODE_RANGE(0xfb00, 79, ALPHABETIC_PRESENTATION_FORMS)             \
    OC_UNICODE_RANGE(0xfb50, 687, ARABIC_PRESENTATION_FORMS_A)              \
    OC_UNICODE_RANGE(0xfe00, 15, VARIATION_SELECTORS)                       \
    OC_UNICODE_RANGE(0xfe20, 15, COMBINING_HALF_MARKS)                      \
    OC_UNICODE_RANGE(0xfe30, 31, CJK_COMPATIBILITY_FORMS)                   \
    OC_UNICODE_RANGE(0xfe50, 31, SMALL_FORM_VARIANTS)                       \
    OC_UNICODE_RANGE(0xfe70, 143, ARABIC_PRESENTATION_FORMS_B)              \
    OC_UNICODE_RANGE(0xff00, 239, HALFWIDTH_AND_FULLWIDTH_FORMS)            \
    OC_UNICODE_RANGE(0xfff0, 15, SPECIALS)                                  \
    OC_UNICODE_RANGE(0x10000, 127, LINEAR_B_SYLLABARY)                      \
    OC_UNICODE_RANGE(0x10080, 127, LINEAR_B_IDEOGRAMS)                      \
    OC_UNICODE_RANGE(0x10100, 63, AEGEAN_NUMBERS)                           \
    OC_UNICODE_RANGE(0x10300, 47, OLD_ITALIC)                               \
    OC_UNICODE_RANGE(0x10330, 31, GOTHIC)                                   \
    OC_UNICODE_RANGE(0x10380, 31, UGARITIC)                                 \
    OC_UNICODE_RANGE(0x10400, 79, DESERET)                                  \
    OC_UNICODE_RANGE(0x10450, 47, SHAVIAN)                                  \
    OC_UNICODE_RANGE(0x10480, 47, OSMANYA)                                  \
    OC_UNICODE_RANGE(0x10800, 63, CYPRIOT_SYLLABARY)                        \
    OC_UNICODE_RANGE(0x1d000, 255, BYZANTINE_MUSICAL_SYMBOLS)               \
    OC_UNICODE_RANGE(0x1d100, 255, MUSICAL_SYMBOLS)                         \
    OC_UNICODE_RANGE(0x1d300, 95, TAI_XUAN_JING_SYMBOLS)                    \
    OC_UNICODE_RANGE(0x1d400, 1023, MATHEMATICAL_ALPHANUMERIC_SYMBOLS)      \
    OC_UNICODE_RANGE(0x20000, 42719, CJK_UNIFIED_IDEOGRAPHS_EXTENSION_B)    \
    OC_UNICODE_RANGE(0x2f800, 543, CJK_COMPATIBILITY_IDEOGRAPHS_SUPPLEMENT) \
    OC_UNICODE_RANGE(0xe0000, 127, TAGS)                                    \
    OC_UNICODE_RANGE(0xe0100, 239, VARIATION_SELECTORS_SUPPLEMENT)          \
    OC_UNICODE_RANGE(0xf0000, 65533, SUPPLEMENTARY_PRIVATE_USE_AREA_A)      \
    OC_UNICODE_RANGE(0x100000, 65533, SUPPLEMENTARY_PRIVATE_USE_AREA_B)

#define OC_UNICODE_RANGE(start, count, name) \
    ORCA_API extern const oc_unicode_range OC_CAT2(OC_UNICODE_, name);
OC_UNICODE_RANGES
#undef OC_UNICODE_RANGE

#ifdef __cplusplus
} // extern "C"
#endif

#endif //__UTF8_H_
