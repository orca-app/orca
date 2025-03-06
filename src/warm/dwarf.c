/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "util/typedefs.h"
#include "util/memory.h"
#include "util/strings.h"

#define X_ENUM(name, code) name = code,
#define X_NAME_CASE(name, code) \
    case name:                  \
        res = #name;            \
        break;

#define DW_TAG_LIST(X)                       \
    X(DW_TAG_array_type, 0x01)               \
    X(DW_TAG_class_type, 0x02)               \
    X(DW_TAG_entry_point, 0x03)              \
    X(DW_TAG_enumeration_type, 0x04)         \
    X(DW_TAG_formal_parameter, 0x05)         \
    X(DW_TAG_imported_declaration, 0x08)     \
    X(DW_TAG_label, 0x0a)                    \
    X(DW_TAG_lexical_block, 0x0b)            \
    X(DW_TAG_member, 0x0d)                   \
    X(DW_TAG_pointer_type, 0x0f)             \
    X(DW_TAG_reference_type, 0x10)           \
    X(DW_TAG_compile_unit, 0x11)             \
    X(DW_TAG_string_type, 0x12)              \
    X(DW_TAG_structure_type, 0x13)           \
    X(DW_TAG_subroutine_type, 0x15)          \
    X(DW_TAG_typedef, 0x16)                  \
    X(DW_TAG_union_type, 0x17)               \
    X(DW_TAG_unspecified_parameters, 0x18)   \
    X(DW_TAG_variant, 0x19)                  \
    X(DW_TAG_common_block, 0x1a)             \
    X(DW_TAG_common_inclusion, 0x1b)         \
    X(DW_TAG_inheritance, 0x1c)              \
    X(DW_TAG_inlined_subroutine, 0x1d)       \
    X(DW_TAG_module, 0x1e)                   \
    X(DW_TAG_ptr_to_member_type, 0x1f)       \
    X(DW_TAG_set_type, 0x20)                 \
    X(DW_TAG_subrange_type, 0x21)            \
    X(DW_TAG_with_stmt, 0x22)                \
    X(DW_TAG_access_declaration, 0x23)       \
    X(DW_TAG_base_type, 0x24)                \
    X(DW_TAG_catch_block, 0x25)              \
    X(DW_TAG_const_type, 0x26)               \
    X(DW_TAG_constant, 0x27)                 \
    X(DW_TAG_enumerator, 0x28)               \
    X(DW_TAG_file_type, 0x29)                \
    X(DW_TAG_friend, 0x2a)                   \
    X(DW_TAG_namelist, 0x2b)                 \
    X(DW_TAG_namelist_item, 0x2c)            \
    X(DW_TAG_packed_type, 0x2d)              \
    X(DW_TAG_subprogram, 0x2e)               \
    X(DW_TAG_template_type_parameter, 0x2f)  \
    X(DW_TAG_template_value_parameter, 0x30) \
    X(DW_TAG_thrown_type, 0x31)              \
    X(DW_TAG_try_block, 0x32)                \
    X(DW_TAG_variant_part, 0x33)             \
    X(DW_TAG_variable, 0x34)                 \
    X(DW_TAG_volatile_type, 0x35)            \
    X(DW_TAG_dwarf_procedure, 0x36)          \
    X(DW_TAG_restrict_type, 0x37)            \
    X(DW_TAG_interface_type, 0x38)           \
    X(DW_TAG_namespace, 0x39)                \
    X(DW_TAG_imported_module, 0x3a)          \
    X(DW_TAG_unspecified_type, 0x3b)         \
    X(DW_TAG_partial_unit, 0x3c)             \
    X(DW_TAG_imported_unit, 0x3d)            \
    X(DW_TAG_condition, 0x3f)                \
    X(DW_TAG_shared_type, 0x40)              \
    X(DW_TAG_type_unit, 0x41)                \
    X(DW_TAG_rvalue_reference_type, 0x42)    \
    X(DW_TAG_template_alias, 0x43)           \
    X(DW_TAG_coarray_type, 0x44)             \
    X(DW_TAG_generic_subrange, 0x45)         \
    X(DW_TAG_dynamic_type, 0x46)             \
    X(DW_TAG_atomic_type, 0x47)              \
    X(DW_TAG_call_site, 0x48)                \
    X(DW_TAG_call_site_parameter, 0x49)      \
    X(DW_TAG_skeleton_unit, 0x4a)            \
    X(DW_TAG_immutable_type, 0x4b)           \
    X(DW_TAG_lo_user, 0x4080)                \
    X(DW_TAG_hi_user, 0xffff)                \
    X(DW_TAG_force_u32, 0xffffffff)

typedef enum dw_tag
{
    DW_TAG_LIST(X_ENUM)
} dw_tag;

const char* dw_get_tag_string(u32 tag)
{
    const char* res = 0;
    switch(tag)
    {
        DW_TAG_LIST(X_NAME_CASE);

        default:
            res = "Unknown tag name";
            break;
    }
    return res;
}

enum
{
    DW_CHILDREN_no = 0x00,
    DW_CHILDREN_yes = 0x01,
};

#define DW_ATTR_LIST(X)                    \
    X(DW_AT_sibling, 0x01)                 \
    X(DW_AT_location, 0x02)                \
    X(DW_AT_name, 0x03)                    \
    X(DW_AT_ordering, 0x09)                \
    X(DW_AT_byte_size, 0x0b)               \
    X(DW_AT_bit_size, 0x0d)                \
    X(DW_AT_stmt_list, 0x10)               \
    X(DW_AT_low_pc, 0x11)                  \
    X(DW_AT_high_pc, 0x12)                 \
    X(DW_AT_language, 0x13)                \
    X(DW_AT_discr, 0x15)                   \
    X(DW_AT_discr_value, 0x16)             \
    X(DW_AT_visibility, 0x17)              \
    X(DW_AT_import, 0x18)                  \
    X(DW_AT_string_length, 0x19)           \
    X(DW_AT_common_reference, 0x1a)        \
    X(DW_AT_comp_dir, 0x1b)                \
    X(DW_AT_const_value, 0x1c)             \
    X(DW_AT_containing_type, 0x1d)         \
    X(DW_AT_default_value, 0x1e)           \
    X(DW_AT_inline, 0x20)                  \
    X(DW_AT_is_optional, 0x21)             \
    X(DW_AT_lower_bound, 0x22)             \
    X(DW_AT_producer, 0x25)                \
    X(DW_AT_prototyped, 0x27)              \
    X(DW_AT_return_addr, 0x2a)             \
    X(DW_AT_start_scope, 0x2c)             \
    X(DW_AT_bit_stride, 0x2e)              \
    X(DW_AT_upper_bound, 0x2f)             \
    X(DW_AT_abstract_origin, 0x31)         \
    X(DW_AT_accessibility, 0x32)           \
    X(DW_AT_address_class, 0x33)           \
    X(DW_AT_artificial, 0x34)              \
    X(DW_AT_base_types, 0x35)              \
    X(DW_AT_calling_convention, 0x36)      \
    X(DW_AT_count, 0x37)                   \
    X(DW_AT_data_member_location, 0x38)    \
    X(DW_AT_decl_column, 0x39)             \
    X(DW_AT_decl_file, 0x3a)               \
    X(DW_AT_decl_line, 0x3b)               \
    X(DW_AT_declaration, 0x3c)             \
    X(DW_AT_discr_list, 0x3d)              \
    X(DW_AT_encoding, 0x3e)                \
    X(DW_AT_external, 0x3f)                \
    X(DW_AT_frame_base, 0x40)              \
    X(DW_AT_friend, 0x41)                  \
    X(DW_AT_identifier_case, 0x42)         \
    X(DW_AT_namelist_item, 0x44)           \
    X(DW_AT_priority, 0x45)                \
    X(DW_AT_segment, 0x46)                 \
    X(DW_AT_specification, 0x47)           \
    X(DW_AT_static_link, 0x48)             \
    X(DW_AT_type, 0x49)                    \
    X(DW_AT_use_location, 0x4a)            \
    X(DW_AT_variable_parameter, 0x4b)      \
    X(DW_AT_virtuality, 0x4c)              \
    X(DW_AT_vtable_elem_location, 0x4d)    \
    X(DW_AT_allocated, 0x4e)               \
    X(DW_AT_associated, 0x4f)              \
    X(DW_AT_data_location, 0x50)           \
    X(DW_AT_byte_stride, 0x51)             \
    X(DW_AT_entry_pc, 0x52)                \
    X(DW_AT_use_UTF8, 0x53)                \
    X(DW_AT_extension, 0x54)               \
    X(DW_AT_ranges, 0x55)                  \
    X(DW_AT_trampoline, 0x56)              \
    X(DW_AT_call_column, 0x57)             \
    X(DW_AT_call_file, 0x58)               \
    X(DW_AT_call_line, 0x59)               \
    X(DW_AT_description, 0x5a)             \
    X(DW_AT_binary_scale, 0x5b)            \
    X(DW_AT_decimal_scale, 0x5c)           \
    X(DW_AT_small, 0x5d)                   \
    X(DW_AT_decimal_sign, 0x5e)            \
    X(DW_AT_digit_count, 0x5f)             \
    X(DW_AT_picture_string, 0x60)          \
    X(DW_AT_mutable, 0x61)                 \
    X(DW_AT_threads_scaled, 0x62)          \
    X(DW_AT_explicit, 0x63)                \
    X(DW_AT_object_pointer, 0x64)          \
    X(DW_AT_endianity, 0x65)               \
    X(DW_AT_elemental, 0x66)               \
    X(DW_AT_pure, 0x67)                    \
    X(DW_AT_recursive, 0x68)               \
    X(DW_AT_signature, 0x69)               \
    X(DW_AT_main_subprogram, 0x6a)         \
    X(DW_AT_data_bit_offset, 0x6b)         \
    X(DW_AT_const_expr, 0x6c)              \
    X(DW_AT_enum_class, 0x6d)              \
    X(DW_AT_linkage_name, 0x6e)            \
    X(DW_AT_string_length_bit_size, 0x6f)  \
    X(DW_AT_string_length_byte_size, 0x70) \
    X(DW_AT_rank, 0x71)                    \
    X(DW_AT_str_offsets_base, 0x72)        \
    X(DW_AT_addr_base, 0x73)               \
    X(DW_AT_rnglists_base, 0x74)           \
    X(DW_AT_dwo_name, 0x76)                \
    X(DW_AT_reference, 0x77)               \
    X(DW_AT_rvalue_reference, 0x78)        \
    X(DW_AT_macros, 0x79)                  \
    X(DW_AT_call_all_calls, 0x7a)          \
    X(DW_AT_call_all_source_calls, 0x7b)   \
    X(DW_AT_call_all_tail_calls, 0x7c)     \
    X(DW_AT_call_return_pc, 0x7d)          \
    X(DW_AT_call_value, 0x7e)              \
    X(DW_AT_call_origin, 0x7f)             \
    X(DW_AT_call_parameter, 0x80)          \
    X(DW_AT_call_pc, 0x81)                 \
    X(DW_AT_call_tail_call, 0x82)          \
    X(DW_AT_call_target, 0x83)             \
    X(DW_AT_call_target_clobbered, 0x84)   \
    X(DW_AT_call_data_location, 0x85)      \
    X(DW_AT_call_data_value, 0x86)         \
    X(DW_AT_noreturn, 0x87)                \
    X(DW_AT_alignment, 0x88)               \
    X(DW_AT_export_symbols, 0x89)          \
    X(DW_AT_deleted, 0x8a)                 \
    X(DW_AT_defaulted, 0x8b)               \
    X(DW_AT_loclists_base, 0x8c)           \
    X(DW_AT_lo_user, 0x2000)               \
    X(DW_AT_hi_user, 0x3fff)               \
    X(DW_AT_force_u32, 0xffffffff)

typedef enum dw_attr_name
{
    DW_ATTR_LIST(X_ENUM)
} dw_attr_name;

const char* dw_get_attr_name_string(u32 name)
{
    const char* res = 0;
    switch(name)
    {
        DW_ATTR_LIST(X_NAME_CASE);

        default:
            res = "Unknown attribute name";
            break;
    }
    return (res);
}

#define DW_FORM_LIST(X)             \
    X(DW_FORM_addr, 0x01)           \
    X(DW_FORM_block2, 0x03)         \
    X(DW_FORM_block4, 0x04)         \
    X(DW_FORM_data2, 0x05)          \
    X(DW_FORM_data4, 0x06)          \
    X(DW_FORM_data8, 0x07)          \
    X(DW_FORM_string, 0x08)         \
    X(DW_FORM_block, 0x09)          \
    X(DW_FORM_block1, 0x0a)         \
    X(DW_FORM_data1, 0x0b)          \
    X(DW_FORM_flag, 0x0c)           \
    X(DW_FORM_sdata, 0x0d)          \
    X(DW_FORM_strp, 0x0e)           \
    X(DW_FORM_udata, 0x0f)          \
    X(DW_FORM_ref_addr, 0x10)       \
    X(DW_FORM_ref1, 0x11)           \
    X(DW_FORM_ref2, 0x12)           \
    X(DW_FORM_ref4, 0x13)           \
    X(DW_FORM_ref8, 0x14)           \
    X(DW_FORM_ref_udata, 0x15)      \
    X(DW_FORM_indirect, 0x16)       \
    X(DW_FORM_sec_offset, 0x17)     \
    X(DW_FORM_exprloc, 0x18)        \
    X(DW_FORM_flag_present, 0x19)   \
    X(DW_FORM_strx, 0x1a)           \
    X(DW_FORM_addrx, 0x1b)          \
    X(DW_FORM_ref_sup4, 0x1c)       \
    X(DW_FORM_strp_sup, 0x1d)       \
    X(DW_FORM_data16, 0x1e)         \
    X(DW_FORM_line_strp, 0x1f)      \
    X(DW_FORM_ref_sig8, 0x20)       \
    X(DW_FORM_implicit_const, 0x21) \
    X(DW_FORM_loclistx, 0x22)       \
    X(DW_FORM_rnglistx, 0x23)       \
    X(DW_FORM_ref_sup8, 0x24)       \
    X(DW_FORM_strx1, 0x25)          \
    X(DW_FORM_strx2, 0x26)          \
    X(DW_FORM_strx3, 0x27)          \
    X(DW_FORM_strx4, 0x28)          \
    X(DW_FORM_addrx1, 0x29)         \
    X(DW_FORM_addrx2, 0x2a)         \
    X(DW_FORM_addrx3, 0x2b)         \
    X(DW_FORM_addrx4, 0x2c)         \
    X(DW_FORM_force_u32, 0xffffffff)

typedef enum dw_form
{
    DW_FORM_LIST(X_ENUM)
} dw_form;

const char* dw_get_form_string(u32 form)
{
    const char* res = 0;

    switch(form)
    {
        DW_FORM_LIST(X_NAME_CASE);
        default:
            res = "Unknown form";
    }
    return res;
}

#define DW_LNS_LIST(X)                 \
    X(DW_LNS_copy, 0x01)               \
    X(DW_LNS_advance_pc, 0x02)         \
    X(DW_LNS_advance_line, 0x03)       \
    X(DW_LNS_set_file, 0x04)           \
    X(DW_LNS_set_column, 0x05)         \
    X(DW_LNS_negate_stmt, 0x06)        \
    X(DW_LNS_set_basic_block, 0x07)    \
    X(DW_LNS_const_add_pc, 0x08)       \
    X(DW_LNS_fixed_advance_pc, 0x09)   \
    X(DW_LNS_set_prologue_end, 0x0a)   \
    X(DW_LNS_set_epilogue_begin, 0x0b) \
    X(DW_LNS_set_isa, 0x0c)

#define DW_LNE_LIST(X)                \
    X(DW_LNE_end_sequence, 0x01)      \
    X(DW_LNE_set_address, 0x02)       \
    X(DW_LNE_set_discriminator, 0x04) \
    X(DW_LNE_lo_user, 0x80)           \
    X(DW_LNE_hi_user, 0xff)

typedef enum dw_lns_opcode
{
    DW_LNS_LIST(X_ENUM)
} dw_lns_opcode;

typedef enum dw_lne_opcode
{
    DW_LNE_LIST(X_ENUM)
} dw_lne_opcode;

const char* dw_get_line_standard_opcode_string(u32 opcode)
{
    const char* res = 0;
    switch(opcode)
    {
        DW_LNS_LIST(X_NAME_CASE);
    }
    return res;
}

#define DW_LNCT_LIST(X)              \
    X(DW_LNCT_path, 0x01)            \
    X(DW_LNCT_directory_index, 0x02) \
    X(DW_LNCT_timestamp, 0x03)       \
    X(DW_LNCT_size, 0x04)            \
    X(DW_LNCT_MD5, 0x05)             \
    X(DW_LNCT_lo_user, 0x2000)       \
    X(DW_LNCT_hi_user, 0x3fff)

typedef enum dw_lnct
{
    DW_LNCT_LIST(X_ENUM)
} dw_lnct;

typedef enum dw_lnct_flags
{
    DW_LNCT_has_path = 1 << DW_LNCT_path,
    DW_LNCT_has_directory_index = 1 << DW_LNCT_directory_index,
    DW_LNCT_has_timestamp = 1 << DW_LNCT_timestamp,
    DW_LNCT_has_size = 1 << DW_LNCT_size,
    DW_LNCT_has_MD5 = 1 << DW_LNCT_MD5,
} dw_lnct_flags;

const char* dw_get_line_header_entry_format_string(u32 format)
{
    const char* res = 0;

    switch(format)
    {
        DW_LNCT_LIST(X_NAME_CASE);
        default:
            res = "Unknown line header entry format";
    }
    return res;
}

#undef X_ENUM
#undef X_NAME_CASE

const char* dw_sectionIdentifierStrings[] = {
    "custom",
    "type",
    "import",
    "function",
    "table",
    "memory",
    "global",
    "export",
    "start",
    "element",
    "code",
    "data",
    "data count",
};

const int dw_sectionIdentifierCount = sizeof(dw_sectionIdentifierStrings) / sizeof(char*);

enum
{
    DW_UT_compile = 0x01,
    DW_UT_type = 0x02,
    DW_UT_partial = 0x03,
    DW_UT_skeleton = 0x04,
    DW_UT_split_compile = 0x05,
    DW_UT_split_type = 0x06,
    DW_UT_lo_user = 0x80,
    DW_UT_hi_user = 0xff,
};

const char* dw_get_cu_type_string(u8 unitType)
{
    const char* res = 0;
    switch(unitType)
    {
        case DW_UT_compile:
            res = "DW_UT_compile";
            break;
        case DW_UT_type:
            res = "DW_UT_type";
            break;
        case DW_UT_partial:
            res = "DW_UT_partial";
            break;
        case DW_UT_skeleton:
            res = "DW_UT_skeleton";
            break;
        case DW_UT_split_compile:
            res = "DW_UT_split_compile";
            break;
        case DW_UT_split_type:
            res = "DW_UT_split_type";
            break;
        case DW_UT_lo_user:
            res = "DW_UT_lo_user";
            break;
        case DW_UT_hi_user:
            res = "DW_UT_hi_user";
            break;
        default:
            res = "Unkown CU type";
            break;
    };
    return res;
}

enum
{
    DWARF_FORMAT_DWARF32,
    DWARF_FORMAT_DWARF64,
};

typedef struct cu_header
{
    u64 length;
    uint16_t version;
    u8 unitType;

    char dwarfFormat;

    union
    {
        // full and partial compilation units
        struct
        {
            u8 addressSize;
            u64 abbrevOffset;
        };

        //TODO...
    };

} cu_header;

int dw_read_leb128(u64* res, char* data, u64 fileSize, u64 offset, u32 bitWidth, bool isSigned)
{
    char byte = 0;
    u64 shift = 0;
    u64 acc = 0;
    u32 count = 0;
    u32 maxCount = (u32)ceil(bitWidth / 7.);

    u64 startOffset = offset;
    do
    {
        if(offset + sizeof(char) > fileSize)
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

        byte = data[offset];
        offset++;

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
    *res = acc;
    return (offset - startOffset);
}

int dw_read_leb128_u32(u32* res, char* data, int fileSize, int offset)
{
    u64 res64 = 0;
    int delta = dw_read_leb128(&res64, data, fileSize, offset, 32, false);
    *res = res64;
    return delta;
}

int dw_read_leb128_u64(u64* res, char* data, int fileSize, int offset)
{
    return dw_read_leb128(res, data, fileSize, offset, 64, false);
}

int dw_read_leb128_i32(i32* res, char* data, int fileSize, int offset)
{
    u64 res64 = 0;
    int delta = dw_read_leb128(&res64, data, fileSize, offset, 32, true);
    *res = (i32)res64;
    return delta;
}

int dw_read_leb128_i64(i64* res, char* data, int fileSize, int offset)
{
    return dw_read_leb128((u64*)res, data, fileSize, offset, 64, true);
}

int dw_read_u64(u64* res, char* data, int fileSize, int offset)
{
    if(offset + sizeof(u64) > fileSize)
    {
        printf("error: read out of bounds");
        exit(-1);
    }
    *res = *(u64*)(data + offset);
    return (sizeof(u64));
}

int dw_read_u32(u32* res, char* data, int fileSize, int offset)
{
    if(offset + sizeof(u32) > fileSize)
    {
        printf("error: read out of bounds");
        exit(-1);
    }
    *res = *(u32*)(data + offset);
    return (sizeof(u32));
}

int dw_read_u16(uint16_t* res, char* data, int fileSize, int offset)
{
    if(offset + sizeof(uint16_t) > fileSize)
    {
        printf("error: read out of bounds");
        exit(-1);
    }
    *res = *(uint16_t*)(data + offset);
    return (sizeof(uint16_t));
}

int dw_read_u8(u8* res, char* data, int fileSize, int offset)
{
    if(offset + sizeof(u8) > fileSize)
    {
        printf("error: read out of bounds");
        exit(-1);
    }
    *res = data[offset];
    return (sizeof(u8));
}

int dw_read_str8(char** res, int len, char* data, int fileSize, int offset)
{
    if(offset + len > fileSize)
    {
        printf("error: read out of bounds");
        exit(-1);
    }
    *res = data + offset;
    return (len);
}

int dw_read_null_string(oc_str8* res, char* data, int fileSize, int offset)
{
    if(offset >= fileSize)
    {
        printf("error: read out of bounds");
        exit(-1);
    }
    int len = strnlen(data + offset, fileSize - offset);
    if(offset + len + 1 >= fileSize)
    {
        printf("error: read out of bounds");
        exit(-1);
    }
    *res = (oc_str8){
        .ptr = data + offset,
        .len = len,
    };
    return len + 1;
}

int dw_read_cu_header(cu_header* header, char* data, int fileSize, int offset)
{
    size_t start = offset;
    memset(header, 0, sizeof(cu_header));

    u32 length32 = 0;
    offset += dw_read_u32(&length32, data, fileSize, offset);
    if(length32 > 0xfffffff0)
    {
        //TODO: if header length is not 0xffffffff at this point this should be an error
        header->dwarfFormat = DWARF_FORMAT_DWARF64;
        offset += dw_read_u64(&header->length, data, fileSize, offset);
    }
    else
    {
        header->length = length32;
    }

    offset += dw_read_u16(&header->version, data, fileSize, offset);
    if(header->version != 5)
    {
        goto end;
    }

    offset += dw_read_u8(&header->unitType, data, fileSize, offset);

    switch(header->unitType)
    {
        case DW_UT_compile:
        case DW_UT_partial:
        {
            offset += dw_read_u8(&header->addressSize, data, fileSize, offset);
            if(header->dwarfFormat == DWARF_FORMAT_DWARF32)
            {
                u32 abbrevOffset = 0;
                offset += dw_read_u32(&abbrevOffset, data, fileSize, offset);
                header->abbrevOffset = abbrevOffset;
            }
            else
            {
                offset += dw_read_u64(&header->abbrevOffset, data, fileSize, offset);
            }
        }
        break;

        default:
            //TODO
            break;
    }

end:
    return (offset - start);
}

typedef struct dw_sections
{
    oc_str8 abbrev;
    oc_str8 info;
    oc_str8 strOffsets;
    oc_str8 str;
    oc_str8 addr;
    oc_str8 line;
    oc_str8 lineStr;
} dw_sections;

typedef struct dw_line_machine
{
    u64 address;
    u64 opIndex; // always 0 for non vliw
    u64 file;
    u64 line;
    u64 column;
    bool isStmt;
    bool basicBlock;
    bool endSequence;
    bool prologueEnd;
    bool epilogueBegin;
    u64 isa; // could omit ?
    u64 discriminator;

    /*NOTE(martin):
        linkers may remove entities from a linked object, and instead of fixing up dwarf info, they
        can emit "tombstone addresses" (= the max address for the target address size).
        A DW_LNE_set_address opcode with a tombstone address invalidates all entries up to the next
        DW_LNE_set_address or DW_LNE_end_sequence opcode.
        This is actually a Dwarf v6 proposal (see https://dwarfstd.org/issues/200609.1.html), and
        it isn't mentionned anywhere in the Dwarf version <= 5 specs, but lld uses this anyway.

        Seems like the "spec" is more what you'd call guidelines than actual rules.
    */
    bool tombstone;

} dw_line_machine;

typedef struct dw_file_entry
{
    oc_str8 path;
    u64 dirIndex;
    u64 timestamp;
    u64 size;
    u8 md5[16];

} dw_file_entry;

typedef struct dw_line_program_header
{
    u64 offset;
    u64 unitLength;
    uint16_t version;
    u8 addressSize;
    u8 segmentSelectorSize;
    u64 headerLength;
    u8 minInstructionLength;
    u8 maxOperationsPerInstruction;
    u8 defaultIsStmt;
    int8_t lineBase;
    u8 lineRange;
    u8 opcodeBase;
    u8 standardOpcodeLength[12]; //TODO not always 12, should point to allocated array

    dw_lnct_flags dirFlags;
    u64 dirEntryCount;
    dw_file_entry* dirEntries;

    dw_lnct_flags fileFlags;
    u64 fileEntryCount;
    dw_file_entry* fileEntries;

} dw_line_program_header;

typedef struct dw_file_entry_format_elt
{
    u64 content;
    u64 form;
} dw_file_entry_format_elt;

typedef u32 dw_line_entry_flags;

enum dw_line_entry_flags
{
    DW_LINE_NONE = 0,
    DW_LINE_STMT = 1 << 0,
    DW_LINE_BASIC_BLOCK = 1 << 1,
    DW_LINE_PROLOGUE_END = 1 << 2,
    DW_LINE_EPILOGUE_BEGIN = 1 << 3,
    DW_LINE_SEQUENCE_END = 1 << 4,
};

typedef struct dw_line_entry
{
    u64 address;
    dw_file_entry* fileEntry;
    u64 file;
    u64 line;
    u64 column;
    u64 discriminator;
    u64 isa;
    u64 opIndex;
    dw_line_entry_flags flags;
} dw_line_entry;

typedef struct dw_line_table
{
    dw_line_program_header header;
    u64 entryCount;
    dw_line_entry* entries;
} dw_line_table;

typedef struct dw_line_info
{
    u64 tableCount;
    dw_line_table* tables;
} dw_line_info;

typedef struct dw_info
{
    dw_line_info* line;

} dw_info;

typedef struct dw_file_entry_elt
{
    oc_list_elt listElt;
    dw_file_entry entry;
} dw_file_entry_elt;

int dw_read_file_entries(oc_arena* arena,
                         u64* entryCount,
                         dw_file_entry** entries,
                         dw_sections* sections,
                         dw_line_program_header* header,
                         char* data,
                         u64 fileSize,
                         u64 offset,
                         bool directories)
{
    u64 start = offset;
    u8 formatCount = 0;
    dw_file_entry_format_elt* format = 0;

    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    if(header->version == 5)
    {
        offset += dw_read_u8(&formatCount, data, fileSize, offset);
    }
    else
    {
        if(directories)
        {
            formatCount = 1;
        }
        else
        {
            formatCount = 4;
        }
    }

    format = oc_arena_push_array(scratch.arena, dw_file_entry_format_elt, formatCount);

    if(header->version == 5)
    {
        for(int i = 0; i < formatCount; i++)
        {
            offset += dw_read_leb128_u64(&format[i].content, data, fileSize, offset);
            offset += dw_read_leb128_u64(&format[i].form, data, fileSize, offset);

            if(format[i].content <= DW_LNCT_MD5)
            {
                if(directories)
                {
                    header->dirFlags |= (1 << format[i].content);
                }
                else
                {
                    header->fileFlags |= (1 << format[i].content);
                }
            }
        }
        offset += dw_read_leb128_u64(entryCount, data, fileSize, offset);
    }
    else
    {
        if(directories)
        {
            header->dirFlags = DW_LNCT_has_path;

            format[0].content = DW_LNCT_path;
            format[0].form = DW_FORM_string;
        }
        else
        {
            header->dirFlags = DW_LNCT_has_path
                             | DW_LNCT_has_directory_index
                             | DW_LNCT_has_timestamp
                             | DW_LNCT_has_size;

            format[0].content = DW_LNCT_path;
            format[0].form = DW_FORM_string;
            format[1].content = DW_LNCT_directory_index;
            format[1].form = DW_FORM_udata;

            format[2].content = DW_LNCT_timestamp;
            format[2].form = DW_FORM_udata;
            format[3].content = DW_LNCT_size;
            format[3].form = DW_FORM_udata;
        }
    }

    oc_list entryList = { 0 };
    u64 entryIndex = 0;

    while(offset < fileSize)
    {
        if(header->version == 5 && entryIndex >= *entryCount)
        {
            break;
        }
        else if(header->version == 4 && data[offset] == 0)
        {
            offset++;
            break;
        }

        dw_file_entry_elt* elt = oc_arena_push_type(scratch.arena, dw_file_entry_elt);
        memset(elt, 0, sizeof(dw_file_entry_elt));
        oc_list_push_back(&entryList, &elt->listElt);

        dw_file_entry* entry = &elt->entry;

        for(int fmtIndex = 0; fmtIndex < formatCount; fmtIndex++)
        {
            u64 content = format[fmtIndex].content;
            u64 form = format[fmtIndex].form;

            switch(content)
            {
                case DW_LNCT_path:
                {
                    switch(form)
                    {
                        case DW_FORM_string:
                        {
                            u64 len = strnlen(data + offset, fileSize - offset);
                            if(len == fileSize - offset)
                            {
                                printf("error: unterminated string in file entry");
                                exit(-1);
                            }
                            entry->path = oc_str8_from_buffer(len, data + offset);
                            offset += len + 1;
                        }
                        break;
                        case DW_FORM_line_strp:
                        case DW_FORM_strp:
                        case DW_FORM_strp_sup:
                        {
                            u64 strp = 0;
                            if(header->addressSize == 4)
                            {
                                u32 strp32 = 0;
                                offset += dw_read_u32(&strp32, data, fileSize, offset);
                                strp = strp32;
                            }
                            else
                            {
                                offset += dw_read_u64(&strp, data, fileSize, offset);
                            }
                            oc_str8 section = { 0 };
                            if(form == DW_FORM_line_strp)
                            {
                                section = sections->lineStr;
                            }
                            else if(form == DW_FORM_strp)
                            {
                                section = sections->str;
                            }
                            else
                            {
                                //TODO: supplementary string section
                                printf("error: unsupported supplementary string section\n");
                                exit(-1);
                            }

                            if(strp < section.len)
                            {
                                char* ptr = section.ptr + strp;
                                u64 maxLen = section.len - strp;
                                u64 len = strnlen(ptr, maxLen);

                                if(len == maxLen)
                                {
                                    printf("error: unterminated string in file entry");
                                    exit(-1);
                                }
                                entry->path = oc_str8_from_buffer(len, ptr);
                            }
                            else
                            {
                                printf("error: invalid string offset in file entry\n");
                                exit(-1);
                            }
                        }
                        break;

                        default:
                            printf("unsupported form code for %s file entry format\n",
                                   dw_get_line_header_entry_format_string(content));
                            exit(-1);
                            break;
                    }
                }
                break;
                case DW_LNCT_directory_index:
                {
                    switch(form)
                    {
                        case DW_FORM_data1:
                        {
                            u8 index8 = 0;
                            offset += dw_read_u8(&index8, data, fileSize, offset);
                            entry->dirIndex = index8;
                        }
                        break;
                        case DW_FORM_data2:
                        {
                            u16 index16 = 0;
                            offset += dw_read_u16(&index16, data, fileSize, offset);
                            entry->dirIndex = index16;
                        }
                        break;
                        case DW_FORM_udata:
                        {
                            offset += dw_read_leb128_u64(&entry->dirIndex, data, fileSize, offset);
                        }
                        break;

                        default:
                            printf("unsupported form code for %s file entry format\n",
                                   dw_get_line_header_entry_format_string(content));
                            exit(-1);
                            break;
                    }
                }
                break;
                case DW_LNCT_timestamp:
                {
                    switch(form)
                    {
                        case DW_FORM_data4:
                        {
                            u32 timestamp32 = 0;
                            offset += dw_read_u32(&timestamp32, data, fileSize, offset);
                            entry->timestamp = timestamp32;
                        }
                        break;
                        case DW_FORM_data8:
                        {
                            offset += dw_read_u64(&entry->timestamp, data, fileSize, offset);
                        }
                        break;
                        case DW_FORM_udata:
                        {
                            offset += dw_read_leb128_u64(&entry->timestamp, data, fileSize, offset);
                        }
                        break;
                        case DW_FORM_block:
                        {
                            //NOTE(martin): I don't know how to interpret the block, so just warn and skip it
                            u64 len = 0;
                            offset += dw_read_leb128_u64(&len, data, fileSize, offset);
                            printf("warning: unsupported form DW_FORM_block in %s file entry format\n",
                                   dw_get_line_header_entry_format_string(content));
                            offset += len;
                        }
                        break;

                        default:
                            printf("unsupported form code for %s file entry format\n",
                                   dw_get_line_header_entry_format_string(content));
                            exit(-1);
                            break;
                    }
                }
                break;
                case DW_LNCT_size:
                {
                    switch(form)
                    {
                        case DW_FORM_data1:
                        {
                            u8 size8 = 0;
                            offset += dw_read_u8(&size8, data, fileSize, offset);
                            entry->size = size8;
                        }
                        break;
                        case DW_FORM_data2:
                        {
                            u16 size16 = 0;
                            offset += dw_read_u16(&size16, data, fileSize, offset);
                            entry->size = size16;
                        }
                        break;
                        case DW_FORM_data4:
                        {
                            u32 size32 = 0;
                            offset += dw_read_u32(&size32, data, fileSize, offset);
                            entry->size = size32;
                        }
                        break;
                        case DW_FORM_data8:
                        {
                            offset += dw_read_u64(&entry->size, data, fileSize, offset);
                        }
                        break;
                        case DW_FORM_udata:
                        {
                            offset += dw_read_leb128_u64(&entry->size, data, fileSize, offset);
                        }
                        break;

                        default:
                            printf("unsupported form code for %s file entry format\n",
                                   dw_get_line_header_entry_format_string(content));
                            exit(-1);
                            break;
                    }
                }
                break;
                case DW_LNCT_MD5:
                {
                    if(form != DW_FORM_data16)
                    {
                        printf("unsupported form code for %s file entry format\n",
                               dw_get_line_header_entry_format_string(content));
                        exit(-1);
                    }
                    char* md5 = 0;
                    offset += dw_read_str8(&md5, 16, data, fileSize, offset);
                    memcpy(entry->md5, md5, 16);
                }
                break;
                case DW_LNCT_lo_user:
                {
                }
                break;
                case DW_LNCT_hi_user:
                {
                }
                break;

                default:
                {
                    if(content >= DW_LNCT_lo_user && content <= DW_LNCT_hi_user)
                    {
                        printf("error: unsupported vendor-defined content description\n");
                        //TODO: just skip
                        exit(-1);
                    }
                    else
                    {
                        printf("error: unrecognized directory entry content description\n");
                        exit(-1);
                    }
                }
                break;
            }
        }

        OC_DEBUG_ASSERT(entry->path.ptr);

        entryIndex++;
    }
    OC_DEBUG_ASSERT(header->version == 4 || *entryCount == entryIndex);

    *entryCount = entryIndex;

    *entries = oc_arena_push_array(arena, dw_file_entry, *entryCount);
    {
        u64 i = 0;
        oc_list_for(entryList, elt, dw_file_entry_elt, listElt)
        {
            (*entries)[i] = elt->entry;
            i++;
        }
    }

    oc_scratch_end(scratch);
    return (offset - start);
}

int dw_read_line_program_header(oc_arena* arena, dw_line_program_header* header, dw_sections* sections, char* data, u64 fileSize, u64 offset)
{
    header->offset = offset;

    u32 length32 = 0;
    u8 dwarfFormat = DWARF_FORMAT_DWARF32;

    offset += dw_read_u32(&length32, data, fileSize, offset);

    if(length32 >= 0xfffffff0)
    {
        offset += dw_read_u64(&header->unitLength, data, fileSize, offset);
        dwarfFormat = DWARF_FORMAT_DWARF64;
    }
    else
    {
        header->unitLength = length32;
    }

    offset += dw_read_u16(&header->version, data, fileSize, offset);

    if(header->version != 5 && header->version != 4)
    {
        printf("error: DWARF version %i not supported\n", header->version);
        exit(-1);
    }

    if(header->version == 5)
    {
        offset += dw_read_u8(&header->addressSize, data, fileSize, offset);
        if(header->addressSize != 4 && header->addressSize != 8)
        {
            oc_log_error("address size should be 4 or 8\n");
            exit(-1);
        }

        offset += dw_read_u8(&header->segmentSelectorSize, data, fileSize, offset);
    }
    else
    {
        //NOTE: we set 4 by default as it is the address size on wasm.
        //TODO: THIS SHOULD CHANGE IF WE SWITCH TO WASM64!
        //TODO: allow configuring the "default target address size" from outside
        header->addressSize = 4;
    }

    if(dwarfFormat == DWARF_FORMAT_DWARF32)
    {
        u32 len32 = 0;
        offset += dw_read_u32(&len32, data, fileSize, offset);
        header->headerLength = len32;
    }
    else
    {
        offset += dw_read_u64(&header->headerLength, data, fileSize, offset);
    }
    u64 headerLengthBase = offset;

    offset += dw_read_u8(&header->minInstructionLength, data, fileSize, offset);
    offset += dw_read_u8(&header->maxOperationsPerInstruction, data, fileSize, offset);
    offset += dw_read_u8(&header->defaultIsStmt, data, fileSize, offset);
    offset += dw_read_u8((u8*)&header->lineBase, data, fileSize, offset);
    offset += dw_read_u8(&header->lineRange, data, fileSize, offset);
    offset += dw_read_u8(&header->opcodeBase, data, fileSize, offset);

    //TODO: support non-12 sizes
    for(int i = 0; i < 12; i++)
    {
        offset += dw_read_u8(&header->standardOpcodeLength[i], data, fileSize, offset);
    }

    // directories
    offset += dw_read_file_entries(arena, &header->dirEntryCount, &header->dirEntries, sections, header, data, fileSize, offset, true);

    // files
    offset += dw_read_file_entries(arena, &header->fileEntryCount, &header->fileEntries, sections, header, data, fileSize, offset, false);

    //NOTE: return offset from start to beginning of line program code
    return (headerLengthBase + header->headerLength - header->offset);
}

void dw_line_machine_reset(dw_line_machine* m, bool defaultIsStmt)
{
    memset(m, 0, sizeof(dw_line_machine));
    m->file = 1;
    m->line = 1;
    m->isStmt = defaultIsStmt;
}

typedef struct dw_line_entry_elt
{
    oc_list_elt listElt;
    dw_line_entry entry;

} dw_line_entry_elt;

typedef struct dw_line_table_elt
{
    oc_list_elt listElt;
    dw_line_table table;

} dw_line_table_elt;

void dw_line_machine_emit_row(oc_arena* arena, dw_line_machine* m, oc_list* rowList, u32* rowCount)
{
    if(!m->tombstone)
    {
        dw_line_entry_elt* elt = oc_arena_push_type(arena, dw_line_entry_elt);
        memset(elt, 0, sizeof(dw_line_entry_elt));

        elt->entry.address = m->address;
        elt->entry.line = m->line;
        elt->entry.column = m->column;
        elt->entry.file = m->file;
        elt->entry.isa = m->isa;
        elt->entry.opIndex = m->opIndex;

        if(m->isStmt)
        {
            elt->entry.flags |= DW_LINE_STMT;
        }
        if(m->basicBlock)
        {
            elt->entry.flags |= DW_LINE_BASIC_BLOCK;
        }
        if(m->endSequence)
        {
            elt->entry.flags |= DW_LINE_SEQUENCE_END;
        }
        if(m->prologueEnd)
        {
            elt->entry.flags |= DW_LINE_PROLOGUE_END;
        }
        if(m->epilogueBegin)
        {
            elt->entry.flags |= DW_LINE_EPILOGUE_BEGIN;
        }

        oc_list_push_back(rowList, &elt->listElt);

        (*rowCount)++;
    }
}

dw_line_info dw_load_line_info(oc_arena* arena, dw_sections* sections)
{
    dw_line_info lineInfo = { 0 };

    oc_str8 lineSection = sections->line;
    u64 offset = 0;

    oc_arena_scope scratch = oc_scratch_begin_next(arena);
    oc_list tablesList = { 0 };
    u64 tableCount = 0;

    while(offset < lineSection.len)
    {
        u64 unitStart = offset;
        dw_line_program_header header = { 0 };

        offset += dw_read_line_program_header(arena, &header, sections, lineSection.ptr, lineSection.len, offset);

        u64 unitLineInfoEnd = unitStart + header.addressSize + header.unitLength;

        if(unitLineInfoEnd > lineSection.len)
        {
            oc_log_error("inconsistent size information in line program header\n");
            exit(-1);
        }

        dw_line_table_elt* table = oc_arena_push_type(scratch.arena, dw_line_table_elt);
        oc_list_push_back(&tablesList, &table->listElt);

        table->table.header = header;

        oc_list rowsList = { 0 };
        u32 rowCount = 0;

        dw_line_machine machine;
        dw_line_machine_reset(&machine, header.defaultIsStmt);

        while(offset < unitLineInfoEnd)
        {
            u8 opcode;
            offset += dw_read_u8(&opcode, lineSection.ptr, lineSection.len, offset);

            if(opcode >= header.opcodeBase)
            {
                // special opcode
                opcode -= header.opcodeBase;
                u64 opAdvance = opcode / header.lineRange;

                machine.line += header.lineBase + (opcode % header.lineRange);
                machine.address += header.minInstructionLength * ((machine.opIndex + opAdvance) / header.maxOperationsPerInstruction);
                machine.opIndex = (machine.opIndex + opAdvance) % header.maxOperationsPerInstruction;

                dw_line_machine_emit_row(scratch.arena, &machine, &rowsList, &rowCount);

                machine.basicBlock = false;
                machine.prologueEnd = false;
                machine.epilogueBegin = false;
                machine.discriminator = 0;
            }
            else if(opcode == 0)
            {
                // extended opcode
                u32 opcodeSize = 0;
                offset += dw_read_leb128_u32(&opcodeSize, lineSection.ptr, lineSection.len, offset);
                offset += dw_read_u8(&opcode, lineSection.ptr, lineSection.len, offset);

                switch(opcode)
                {
                    case DW_LNE_end_sequence:
                    {
                        machine.endSequence = true;
                        dw_line_machine_emit_row(scratch.arena, &machine, &rowsList, &rowCount);
                        dw_line_machine_reset(&machine, header.defaultIsStmt);
                    }
                    break;
                    case DW_LNE_set_address:
                    {
                        u64 tombstoneAddress = 0;
                        u64 address = 0;
                        if(header.addressSize == 4)
                        {
                            tombstoneAddress = 0xffffffff;
                            u32 address32 = 0;
                            offset += dw_read_u32(&address32, lineSection.ptr, lineSection.len, offset);
                            address = address32;
                        }
                        else if(header.addressSize == 8)
                        {
                            tombstoneAddress = 0xffffffffffffffff;
                            offset += dw_read_u64(&address, lineSection.ptr, lineSection.len, offset);
                        }
                        else
                        {
                            OC_ASSERT(0);
                        }
                        machine.tombstone = (address == tombstoneAddress);
                        machine.address = address;
                        machine.opIndex = 0;
                    }
                    break;
                    case DW_LNE_set_discriminator:
                    {
                        u64 disc = 0;
                        offset += dw_read_leb128_u64(&disc, lineSection.ptr, lineSection.len, offset);
                        machine.discriminator = disc;
                    }
                    break;

                    default:
                    {
                        if(opcode >= DW_LNE_lo_user && opcode <= DW_LNE_hi_user)
                        {
                            printf("error: unsupported user opcode\n");
                            exit(-1);
                        }
                        else
                        {
                            oc_log_error("unrecognized line program opcode\n");
                            exit(-1);
                        }
                    }
                    break;
                }
            }
            else
            {
                // standard opcode
                switch(opcode)
                {
                    case DW_LNS_copy:
                    {
                        dw_line_machine_emit_row(scratch.arena, &machine, &rowsList, &rowCount);
                        machine.discriminator = 0;
                        machine.basicBlock = false;
                        machine.prologueEnd = false;
                        machine.epilogueBegin = false;
                    }
                    break;
                    case DW_LNS_advance_pc:
                    {
                        u64 opAdvance = 0;
                        offset += dw_read_leb128_u64(&opAdvance, lineSection.ptr, lineSection.len, offset);
                        machine.address += header.minInstructionLength * ((machine.opIndex + opAdvance) / header.maxOperationsPerInstruction);
                        machine.opIndex = (machine.opIndex + opAdvance) % header.maxOperationsPerInstruction;
                    }
                    break;
                    case DW_LNS_advance_line:
                    {
                        int64_t lineAdvance = 0;
                        offset += dw_read_leb128_i64(&lineAdvance, lineSection.ptr, lineSection.len, offset);
                        machine.line += lineAdvance;
                    }
                    break;
                    case DW_LNS_set_file:
                    {
                        u64 file = 0;
                        offset += dw_read_leb128_u64(&file, lineSection.ptr, lineSection.len, offset);
                        machine.file = file;
                    }
                    break;
                    case DW_LNS_set_column:
                    {
                        u64 column = 0;
                        offset += dw_read_leb128_u64(&column, lineSection.ptr, lineSection.len, offset);
                        machine.column = column;
                    }
                    break;
                    case DW_LNS_negate_stmt:
                    {
                        machine.isStmt = !machine.isStmt;
                    }
                    break;
                    case DW_LNS_set_basic_block:
                    {
                        machine.basicBlock = true;
                    }
                    break;
                    case DW_LNS_const_add_pc:
                    {
                        // advance line and opIndex by same increments as special opcode 255
                        opcode = 255 - header.opcodeBase;
                        u64 opAdvance = opcode / header.lineRange;

                        machine.address += header.minInstructionLength * ((machine.opIndex + opAdvance) / header.maxOperationsPerInstruction);
                        machine.opIndex = (machine.opIndex + opAdvance) % header.maxOperationsPerInstruction;
                    }
                    break;
                    case DW_LNS_fixed_advance_pc:
                    {
                        uint16_t inc = 0;
                        offset += dw_read_u16(&inc, lineSection.ptr, lineSection.len, offset);
                        machine.address += inc;
                        machine.opIndex = 0;
                    }
                    break;
                    case DW_LNS_set_prologue_end:
                    {
                        machine.prologueEnd = true;
                    }
                    break;
                    case DW_LNS_set_epilogue_begin:
                    {
                        machine.epilogueBegin = true;
                    }
                    break;
                    case DW_LNS_set_isa:
                    {
                        u64 isa = 0;
                        offset += dw_read_leb128_u64(&isa, lineSection.ptr, lineSection.len, offset);
                        machine.isa = isa;
                    }
                    break;

                    default:
                    {
                        oc_log_error("unrecognized line program opcode\n");
                        exit(-1);
                    }
                    break;
                }
            }
        }

        table->table.entryCount = rowCount;
        table->table.entries = oc_arena_push_array(arena, dw_line_entry, rowCount);
        u64 rowIndex = 0;
        oc_list_for(rowsList, rowElt, dw_line_entry_elt, listElt)
        {
            table->table.entries[rowIndex] = rowElt->entry;
            rowIndex++;
        }

        tableCount++;
    }

    lineInfo.tableCount = tableCount;
    lineInfo.tables = oc_arena_push_array(arena, dw_line_table, tableCount);
    u64 tableIndex = 0;
    oc_list_for(tablesList, tableElt, dw_line_table_elt, listElt)
    {
        lineInfo.tables[tableIndex] = tableElt->table;
        tableIndex++;
    }

    oc_scratch_end(scratch);

    return lineInfo;
}

void dw_print_line_table_header(void)
{
    printf("Address            Line   Column File   ISA Discriminator OpIndex Flags\n"
           "------------------ ------ ------ ------ --- ------------- ------- -------------\n");
}

void dw_print_line_info(dw_line_info* info)
{
    printf(".debug_line contents:\n");
    for(u64 tableIndex = 0; tableIndex < info->tableCount; tableIndex++)
    {
        dw_line_table* table = &info->tables[tableIndex];

        printf("debug_line[0x%08llx]\n", table->header.offset);
        printf("Line table prologue:\n");
        printf("    total_length: 0x%08llx\n", table->header.unitLength);
        printf("          format: %s\n", table->header.addressSize == 4 ? "DWARF32" : "DWARF64");
        printf("         version: %i\n", table->header.version);
        if(table->header.version >= 5)
        {
            printf("    address_size: %i\n", table->header.addressSize);
            printf(" seg_select_size: %i\n", table->header.segmentSelectorSize);
        }
        printf(" prologue_length: 0x%08llx\n", table->header.headerLength);
        printf(" min_inst_length: %i\n", table->header.minInstructionLength);
        printf("max_ops_per_inst: %i\n", table->header.maxOperationsPerInstruction);
        printf(" default_is_stmt: %i\n", table->header.defaultIsStmt ? 1 : 0);
        printf("       line_base: %i\n", table->header.lineBase);
        printf("      line_range: %i\n", table->header.lineRange);
        printf("     opcode_base: %i\n", table->header.opcodeBase);

        for(int i = 0; i < 12; i++)
        {
            printf("standard_opcode_lengths[%s] = %i\n",
                   dw_get_line_standard_opcode_string(i + 1),
                   table->header.standardOpcodeLength[i]);
        }

        for(int i = 0; i < table->header.dirEntryCount; i++)
        {
            int displayIndex = (table->header.version >= 5) ? i : i + 1;
            printf("include_directories[%3i] = \"%.*s\"\n", displayIndex, oc_str8_ip(table->header.dirEntries[i].path));
        }
        for(int i = 0; i < table->header.fileEntryCount; i++)
        {
            dw_file_entry* entry = &(table->header.fileEntries[i]);

            if(table->header.version < 5)
            {
                printf("file_names[%3i]:\n"
                       "           name: \"%.*s\"\n"
                       "      dir_index: %llu\n"
                       "       mod_time: 0x%08llx\n"
                       "         length: 0x%08llx\n",
                       i + 1,
                       oc_str8_ip(entry->path),
                       entry->dirIndex,
                       entry->timestamp,
                       entry->size);
            }
            else
            {
                printf("file_names[%3i]:\n"
                       "           name: \"%.*s\"\n"
                       "      dir_index: %llu\n",
                       i,
                       oc_str8_ip(entry->path),
                       entry->dirIndex);

                if(table->header.fileFlags & DW_LNCT_has_timestamp)
                {
                    printf("       mod_time: 0x%08llx\n",
                           entry->timestamp);
                }

                if(table->header.fileFlags & DW_LNCT_has_size)
                {
                    printf("         length: 0x%08llx\n",
                           entry->size);
                }

                if(table->header.fileFlags & DW_LNCT_has_MD5)
                {
                    printf("   md5_checksum: ");
                    for(int i = 0; i < 16; i++)
                    {
                        printf("%02hhx", entry->md5[i]);
                    }
                    printf("\n");
                }
            }
        }

        {
            /*NOTE(martin):
                This reproduces the behaviour of llvm-dwarfdump, which only prints the
                table headings if the program code is non empty.
                This is NOT be the same as an _empty table_: eg llvm-dwarfdump does produce
                headings even if the table is entirely tombstoned.
            */
            u64 unitLengthSize = (table->header.addressSize == 4) ? 4 : 12;
            u64 programEnd = unitLengthSize + table->header.unitLength;
            u64 prologueStart = unitLengthSize + 2 + table->header.addressSize;
            if(table->header.version >= 5)
            {
                prologueStart += 2;
            }
            u64 prologueEnd = prologueStart + table->header.headerLength;

            if(prologueEnd < programEnd)
            {
                printf("\n");
                dw_print_line_table_header();
            }
        }

        for(u64 rowIndex = 0; rowIndex < table->entryCount; rowIndex++)
        {
            dw_line_entry* entry = &table->entries[rowIndex];

            printf("0x%016llx %6llu %6llu %6llu %3llu %13llu %7llu %s%s%s%s%s\n",
                   entry->address,
                   entry->line,
                   entry->column,
                   entry->file,
                   entry->isa,
                   entry->discriminator,
                   entry->opIndex,
                   (entry->flags & DW_LINE_STMT) ? " is_stmt" : "",
                   (entry->flags & DW_LINE_BASIC_BLOCK) ? " basic_block" : "",
                   (entry->flags & DW_LINE_SEQUENCE_END) ? " end_sequence" : "",
                   (entry->flags & DW_LINE_PROLOGUE_END) ? " prologue_end" : "",
                   (entry->flags & DW_LINE_EPILOGUE_BEGIN) ? " epilogue_begin" : "");
        }

        printf("\n");
    }
}

typedef struct dw_abbrev_attr
{
    dw_attr_name name;
    dw_form form;
    i64 implicitConst;
} dw_abbrev_attr;

typedef struct dw_abbrev_entry
{
    u64 code;
    u8 children;
    dw_tag tag;
    u32 attrCount;
    dw_abbrev_attr* attributes;
} dw_abbrev_entry;

typedef struct dw_abbrev_table
{
    u64 entryCount;
    dw_abbrev_entry* entries;
} dw_abbrev_table;

dw_abbrev_table* dw_load_abbrev_table(oc_arena* arena, oc_str8 section, u64 offset)
{
    //TODO: check if we already loaded this table

    dw_abbrev_table* table = oc_arena_push_type(arena, dw_abbrev_table);
    memset(table, 0, sizeof(dw_abbrev_table));

    //NOTE: we don't know the number of entries or the number of attributes for
    //      each entry before we parse them, so we first push parsed struct to
    //      linked list, and copy them at the end to fixed arrays.

    typedef struct dw_abbrev_entry_elt
    {
        oc_list_elt listElt;
        dw_abbrev_entry entry;

    } dw_abbrev_entry_elt;

    typedef struct dw_abbrev_attr_elt
    {
        oc_list_elt listElt;
        dw_abbrev_attr attr;
    } dw_abbrev_attr_elt;

    oc_list entries = { 0 };
    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    while(offset < section.len)
    {
        u64 abbrevCode = 0;
        offset += dw_read_leb128_u64(&abbrevCode, section.ptr, section.len, offset);
        if(abbrevCode == 0)
        {
            //NOTE: null entry, this is the end of the table
            break;
        }

        dw_abbrev_entry_elt* entryElt = oc_arena_push_type(scratch.arena, dw_abbrev_entry_elt);
        dw_abbrev_entry* entry = &entryElt->entry;

        entry->code = abbrevCode;

        offset += dw_read_leb128_u32(&entry->tag, section.ptr, section.len, offset);
        offset += dw_read_u8(&entry->children, section.ptr, section.len, offset);

        //NOTE: parse attributes
        oc_list attributes = { 0 };
        entry->attrCount = 0;

        while(offset < section.len)
        {
            u32 attrName = 0;
            u32 attrForm = 0;
            offset += dw_read_leb128_u32(&attrName, section.ptr, section.len, offset);
            offset += dw_read_leb128_u32(&attrForm, section.ptr, section.len, offset);

            if(attrName == 0 && attrForm == 0)
            {
                //NOTE: end of attributes
                break;
            }

            dw_abbrev_attr_elt* attrElt = oc_arena_push_type(scratch.arena, dw_abbrev_attr_elt);
            dw_abbrev_attr* attr = &attrElt->attr;

            attr->name = attrName;
            attr->form = attrForm;

            if(attr->form == DW_FORM_implicit_const)
            {
                offset += dw_read_leb128_i64(&attr->implicitConst, section.ptr, section.len, offset);
            }

            oc_list_push_back(&attributes, &attrElt->listElt);
            entry->attrCount++;
        }

        //NOTE: copy attributes to fixed array
        entry->attributes = oc_arena_push_array(arena, dw_abbrev_attr, entry->attrCount);
        u64 attrIndex = 0;
        oc_list_for(attributes, elt, dw_abbrev_attr_elt, listElt)
        {
            entry->attributes[attrIndex] = elt->attr;
            attrIndex++;
        }

        oc_list_push_back(&entries, &entryElt->listElt);
        table->entryCount++;
    }

    //NOTE: copy entries to fixed array
    table->entries = oc_arena_push_array(arena, dw_abbrev_entry, table->entryCount);
    u64 entryIndex = 0;
    oc_list_for(entries, elt, dw_abbrev_entry_elt, listElt)
    {
        table->entries[entryIndex] = elt->entry;
        entryIndex++;
    }

    oc_scratch_end(scratch);

    return table;
}

typedef struct dw_attr
{
    dw_abbrev_attr* abbrev;

    union
    {
        oc_str8 string;
        u8 valU8;
        u16 valU16;
        u32 valU32;
        u64 valU64;
        i64 valI64;
    };
} dw_attr;

u64 dw_parse_form_value(dw_attr* res, dw_form form, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    switch(form)
    {
        //-----------------------
        // address class
        //-----------------------
        case DW_FORM_addr:
        {
            if(addressSize == 4)
            {
                u32 address32 = 0;
                offset += dw_read_u32(&address32, data, fileSize, offset);
                res->valU64 = address32;
            }
            else
            {
                offset += dw_read_u64(&res->valU64, data, fileSize, offset);
            }
        }
        break;
        case DW_FORM_addrx1:
        {
            u8 indOffset = 0;
            offset += dw_read_u8(&indOffset, data, fileSize, offset);
            //TODO: get address from debug_addr section
        }
        break;
        case DW_FORM_addrx2:
        {
            u16 indOffset = 0;
            offset += dw_read_u16(&indOffset, data, fileSize, offset);
            //TODO: get address from debug_addr section
        }
        break;
        case DW_FORM_addrx3:
        {
            u64 indOffset = 0;
            u8* indOffset8 = (u8*)&indOffset;
            offset += dw_read_u8(indOffset8, data, fileSize, offset);
            offset += dw_read_u8(indOffset8 + 1, data, fileSize, offset);
            offset += dw_read_u8(indOffset8 + 2, data, fileSize, offset);
            //TODO: get address from debug_addr section
        }
        break;
        case DW_FORM_addrx4:
        {
            u32 indOffset = 0;
            offset += dw_read_u32(&indOffset, data, fileSize, offset);
            //TODO: get address from debug_addr section
        }
        break;
        case DW_FORM_addrx:
        {
            u64 indOffset = 0;
            offset += dw_read_leb128_u64(&indOffset, data, fileSize, offset);
            //TODO: get address from debug_addr section
        }
        break;

        //-----------------------
        // block class
        //-----------------------
        case DW_FORM_block1:
        {
            u8 len = 0;
            offset += dw_read_u8(&len, data, fileSize, offset);
            res->string.len = len;
        }
        break;
        case DW_FORM_block2:
        {
            u16 len = 0;
            offset += dw_read_u16(&len, data, fileSize, offset);
            res->string.len = len;
        }
        break;
        case DW_FORM_block4:
        {
            u32 len = 0;
            offset += dw_read_u32(&len, data, fileSize, offset);
            res->string.len = len;
        }
        break;
        case DW_FORM_block:
        {
            u64 len = 0;
            offset += dw_read_leb128_u64(&len, data, fileSize, offset);
            res->string.len = len;
        }
        break;

        //-----------------------
        // constant class
        //-----------------------
        case DW_FORM_data1:
        {
            res->string.len = 1;
        }
        break;
        case DW_FORM_data2:
        {
            res->string.len = 2;
        }
        break;
        case DW_FORM_data4:
        {
            res->string.len = 4;
        }
        break;
        case DW_FORM_data8:
        {
            res->string.len = 4;
        }
        break;
        case DW_FORM_data16:
        {
            res->string.len = 4;
        }
        break;

        case DW_FORM_sdata:
        {
            offset += dw_read_leb128_i64(&res->valI64, data, fileSize, offset);
        }
        break;
        case DW_FORM_udata:
        {
            offset += dw_read_leb128_u64(&res->valU64, data, fileSize, offset);
        }
        break;
        case DW_FORM_implicit_const:
        {
            res->valI64 = res->abbrev->implicitConst;
        }
        break;

        //-----------------------
        // exprloc class
        //-----------------------
        case DW_FORM_exprloc:
        {
            u64 len = 0;
            offset += dw_read_leb128_u64(&len, data, fileSize, offset);
            res->string.len = len;
            offset += dw_read_str8(&res->string.ptr, res->string.len, data, fileSize, offset);
        }
        break;

        //-----------------------
        // flag class
        //-----------------------
        case DW_FORM_flag:
        {
            offset += dw_read_u8(&res->valU8, data, fileSize, offset);
        }
        break;
        case DW_FORM_flag_present:
        {
            res->valU8 = 1;
        }
        break;

        //-----------------------
        // loclist class
        //-----------------------
        case DW_FORM_loclistx:
        {
            u64 listOffset = 0;
            offset += dw_read_leb128_u64(&listOffset, data, fileSize, offset);
            //TODO: extract list of entries form debug_loclists section
        }
        break;

        //-----------------------
        // rnglist class
        //-----------------------
        case DW_FORM_rnglistx:
        {
            u64 rngListIndex = 0;
            offset += dw_read_leb128_u64(&rngListIndex, data, fileSize, offset);
            //TODO: extract rnglist of entries form debug_rnglists section
        }
        break;

        //-----------------------
        // reference class
        //-----------------------
        //TODO use/extract references
        case DW_FORM_ref1:
        {
            u8 ref8 = 0;
            offset += dw_read_u8(&ref8, data, fileSize, offset);
        }
        break;
        case DW_FORM_ref2:
        {
            u16 ref16 = 0;
            offset += dw_read_u16(&ref16, data, fileSize, offset);
        }
        break;
        case DW_FORM_ref4:
        {
            u32 ref32 = 0;
            offset += dw_read_u32(&ref32, data, fileSize, offset);
        }
        break;
        case DW_FORM_ref8:
        {
            u64 ref64 = 0;
            offset += dw_read_u64(&ref64, data, fileSize, offset);
        }
        break;
        case DW_FORM_ref_udata:
        {
            u64 ref64 = 0;
            offset += dw_read_leb128_u64(&ref64, data, fileSize, offset);
        }
        break;

        case DW_FORM_ref_addr:
        {
            u64 ref64 = 0;
            if(addressSize == 4)
            {
                u32 ref32 = 0;
                offset += dw_read_u32(&ref32, data, fileSize, offset);
                ref64 = ref32;
            }
            else
            {
                offset += dw_read_u64(&ref64, data, fileSize, offset);
            }
        }
        break;

        case DW_FORM_ref_sig8:
        {
            u64 sig = 0;
            offset += dw_read_u64(&sig, data, fileSize, offset);
        }
        break;

        case DW_FORM_ref_sup4:
        {
            u32 supOffset = 0;
            offset += dw_read_u32(&supOffset, data, fileSize, offset);
        }
        break;

        case DW_FORM_ref_sup8:
        {
            u64 supOffset = 0;
            offset += dw_read_u64(&supOffset, data, fileSize, offset);
        }
        break;

        //-----------------------
        // string class
        //-----------------------
        case DW_FORM_string:
        {
            offset += dw_read_null_string(&res->string, data, fileSize, offset);
        }
        break;

        //TODO extract strings from string section
        case DW_FORM_strp:
        case DW_FORM_line_strp:
        case DW_FORM_strp_sup:
        {
            u64 strOffset64 = 0;
            if(addressSize == 4)
            {
                u32 strOffset32 = 0;
                offset += dw_read_u32(&strOffset32, data, fileSize, offset);
                strOffset64 = strOffset32;
            }
            else
            {
                offset += dw_read_u64(&strOffset64, data, fileSize, offset);
            }
        }
        break;

        case DW_FORM_strx1:
        {
            u8 indOffset = 0;
            offset += dw_read_u8(&indOffset, data, fileSize, offset);
        }
        break;
        case DW_FORM_strx2:
        {
            u16 indOffset = 0;
            offset += dw_read_u16(&indOffset, data, fileSize, offset);
        }
        break;
        case DW_FORM_strx3:
        {
            u16 indOffset16 = 0;
            offset += dw_read_u16(&indOffset16, data, fileSize, offset);
            u8 indOffset8 = 0;
            offset += dw_read_u8(&indOffset8, data, fileSize, offset);
            //TODO: combine 3 bytes
        }
        break;
        case DW_FORM_strx4:
        {
            u32 indOffset = 0;
            offset += dw_read_u32(&indOffset, data, fileSize, offset);
        }
        break;
        case DW_FORM_strx:
        {
            u64 indOffset = 0;
            offset += dw_read_leb128_u64(&indOffset, data, fileSize, offset);
        }
        break;

        //-----------------------
        // sec_offset (belongs to multiple classes)
        //-----------------------
        //TODO use sec offset according to class
        case DW_FORM_sec_offset:
        {
            u64 addrOffset = 0;
            if(addressSize == 4)
            {
                u32 addrOffset32 = 0;
                offset += dw_read_u32(&addrOffset32, data, fileSize, offset);
                addrOffset = addrOffset32;
            }
            else
            {
                offset += dw_read_u64(&addrOffset, data, fileSize, offset);
            }
        }
        break;

        //-----------------------
        // indirect is a special case, see p207 of Dwarf5 spec.
        //-----------------------
        case DW_FORM_indirect:
        {
            u64 indForm = 0;
            offset += dw_read_leb128_u64(&indForm, data, fileSize, offset);
            offset += dw_parse_form_value(res, indForm, addressSize, data, fileSize, offset);
        }
        break;

        default:
            //TODO
            oc_log_error("unsupported form %s\n", dw_get_form_string(form));
            exit(-1);
            break;
    }

    return (startOffset - offset);
}

u64 dw_parse_address(dw_attr* res, dw_form form, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;
    switch(form)
    {
        case DW_FORM_addr:
        {
            if(addressSize == 4)
            {
                u32 address32 = 0;
                offset += dw_read_u32(&address32, data, fileSize, offset);
                res->valU64 = address32;
            }
            else
            {
                offset += dw_read_u64(&res->valU64, data, fileSize, offset);
            }
        }
        break;

        default:
            //TODO
            oc_log_error("unsupported form %s for address\n", dw_get_form_string(form));
            exit(-1);
            break;
    }
    return (startOffset - offset);
}

u64 dw_parse_address_ptr(dw_attr* res, dw_form form, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    if(form == DW_FORM_sec_offset)
    {
        u64 addrOffset = 0;
        if(addressSize == 4)
        {
            u32 addrOffset32 = 0;
            offset += dw_read_u32(&addrOffset32, data, fileSize, offset);
            addrOffset = addrOffset32;
        }
        else
        {
            offset += dw_read_u64(&addrOffset, data, fileSize, offset);
        }
        //TODO: we now need to get the address in the debug_addr section
        oc_log_warning("partially supported addressptr attribute (skipped)\n");
    }
    else
    {
        oc_log_error("unsupported form %s for addressptr\n", dw_get_form_string(form));
        exit(-1);
    }
    return (startOffset - offset);
}

u64 dw_parse_block(dw_attr* res, dw_form form, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    switch(form)
    {
        case DW_FORM_block1:
        {
            u8 len = 0;
            offset += dw_read_u8(&len, data, fileSize, offset);
            res->string.len = len;
        }
        break;
        case DW_FORM_block2:
        {
            u16 len = 0;
            offset += dw_read_u16(&len, data, fileSize, offset);
            res->string.len = len;
        }
        break;
        case DW_FORM_block4:
        {
            u32 len = 0;
            offset += dw_read_u32(&len, data, fileSize, offset);
            res->string.len = len;
        }
        break;
        case DW_FORM_block:
        {
            u64 len = 0;
            offset += dw_read_leb128_u64(&len, data, fileSize, offset);
            res->string.len = len;
        }
        break;

        default:
            oc_log_error("unsupported form %s for addressptr\n", dw_get_form_string(form));
            exit(-1);
            break;
    }
    offset += dw_read_str8(&res->string.ptr, res->string.len, data, fileSize, offset);

    return (startOffset - offset);
}

u64 dw_parse_constant(dw_attr* res, dw_form form, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    if(form == DW_FORM_udata)
    {
        offset += dw_read_leb128_u64(&res->valU64, data, fileSize, offset);
    }
    else if(form == DW_FORM_sdata)
    {
        offset += dw_read_leb128_i64(&res->valI64, data, fileSize, offset);
    }
    else
    {
        //NOTE: just get the uninterpreted bytes
        switch(form)
        {
            case DW_FORM_data1:
            {
                res->string.len = 1;
            }
            break;
            case DW_FORM_data2:
            {
                res->string.len = 2;
            }
            break;
            case DW_FORM_data4:
            {
                res->string.len = 4;
            }
            break;
            case DW_FORM_data8:
            {
                res->string.len = 4;
            }
            break;
            case DW_FORM_data16:
            {
                res->string.len = 4;
            }
            break;

            default:
                oc_log_error("unsupported form %s for addressptr\n", dw_get_form_string(form));
                exit(-1);
                break;
        }
        offset += dw_read_str8(&res->string.ptr, res->string.len, data, fileSize, offset);
    }

    return (startOffset - offset);
}

u64 dw_parse_exprloc(dw_attr* res, dw_form form, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    if(form == DW_FORM_exprloc)
    {
        // just read the uninterpreted expr for now

        u64 len = 0;
        offset += dw_read_leb128_u64(&len, data, fileSize, offset);
        res->string.len = len;

        offset += dw_read_str8(&res->string.ptr, res->string.len, data, fileSize, offset);
    }
    else
    {
        oc_log_error("unsupported form %s for exprloc\n", dw_get_form_string(form));
        exit(-1);
    }
    return (startOffset - offset);
}

u64 dw_parse_flag(dw_attr* res, dw_form form, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    if(form == DW_FORM_flag)
    {
        offset += dw_read_u8(&res->valU8, data, fileSize, offset);
    }
    else if(form == DW_FORM_flag_present)
    {
        res->valU8 = 1;
    }
    else
    {
        oc_log_error("unsupported form %s for flag\n", dw_get_form_string(form));
        exit(-1);
    }
    return (startOffset - offset);
}

u64 dw_parse_lineptr(dw_attr* res, dw_form form, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    if(form == DW_FORM_sec_offset)
    {
        u64 addrOffset = 0;
        if(addressSize == 4)
        {
            u32 addrOffset32 = 0;
            offset += dw_read_u32(&addrOffset32, data, fileSize, offset);
            addrOffset = addrOffset32;
        }
        else
        {
            offset += dw_read_u64(&addrOffset, data, fileSize, offset);
        }
        //TODO:
        oc_log_warning("partially supported lineptr attribute (skipped)\n");
    }
    else
    {
        oc_log_error("unsupported form %s for lineptr\n", dw_get_form_string(form));
        exit(-1);
    }
    return (startOffset - offset);
}

u64 dw_parse_loclist(dw_attr* res, dw_form form, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    if(form == DW_FORM_loclistx)
    {
        u64 lineOffset = 0;
        offset += dw_read_leb128_u64(&lineOffset, data, fileSize, offset);
        //TODO:
        oc_log_warning("partially supported loclist attribute (skipped)\n");
    }
    else if(form == DW_FORM_sec_offset)
    {
        u64 addrOffset = 0;
        if(addressSize == 4)
        {
            u32 addrOffset32 = 0;
            offset += dw_read_u32(&addrOffset32, data, fileSize, offset);
            addrOffset = addrOffset32;
        }
        else
        {
            offset += dw_read_u64(&addrOffset, data, fileSize, offset);
        }
        //TODO:
        oc_log_warning("partially supported loclist attribute (skipped)\n");
    }
    else
    {
        oc_log_error("unsupported form %s for loclist\n", dw_get_form_string(form));
        exit(-1);
    }
    return (startOffset - offset);
}

u64 dw_parse_loclistptr(dw_attr* res, dw_form form, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    if(form == DW_FORM_sec_offset)
    {
        u64 addrOffset = 0;
        if(addressSize == 4)
        {
            u32 addrOffset32 = 0;
            offset += dw_read_u32(&addrOffset32, data, fileSize, offset);
            addrOffset = addrOffset32;
        }
        else
        {
            offset += dw_read_u64(&addrOffset, data, fileSize, offset);
        }
        //TODO:
        oc_log_warning("partially supported loclistptr attribute (skipped)\n");
    }
    else
    {
        oc_log_error("unsupported form %s for loclistptr\n", dw_get_form_string(form));
        exit(-1);
    }
    return (startOffset - offset);
}

u64 dw_parse_macptr(dw_attr* res, dw_form form, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    if(form == DW_FORM_sec_offset)
    {
        u64 addrOffset = 0;
        if(addressSize == 4)
        {
            u32 addrOffset32 = 0;
            offset += dw_read_u32(&addrOffset32, data, fileSize, offset);
            addrOffset = addrOffset32;
        }
        else
        {
            offset += dw_read_u64(&addrOffset, data, fileSize, offset);
        }
        //TODO:
        oc_log_warning("partially supported macptr attribute (skipped)\n");
    }
    else
    {
        oc_log_error("unsupported form %s for macptr\n", dw_get_form_string(form));
        exit(-1);
    }
    return (startOffset - offset);
}

u64 dw_parse_rnglist(dw_attr* res, dw_form form, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    if(form == DW_FORM_rnglistx)
    {
        u64 rngOffset = 0;
        offset += dw_read_leb128_u64(&rngOffset, data, fileSize, offset);
        oc_log_warning("partially supported rnglist attribute (skipped)\n");
    }
    else if(form == DW_FORM_sec_offset)
    {
        u64 addrOffset = 0;
        if(addressSize == 4)
        {
            u32 addrOffset32 = 0;
            offset += dw_read_u32(&addrOffset32, data, fileSize, offset);
            addrOffset = addrOffset32;
        }
        else
        {
            offset += dw_read_u64(&addrOffset, data, fileSize, offset);
        }
        //TODO:
        oc_log_warning("partially supported rnglist attribute (skipped)\n");
    }
    else
    {
        oc_log_error("unsupported form %s for rnglist\n", dw_get_form_string(form));
        exit(-1);
    }
    return (startOffset - offset);
}

u64 dw_parse_rnglistptr(dw_attr* res, dw_form form, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    if(form == DW_FORM_sec_offset)
    {
        u64 addrOffset = 0;
        if(addressSize == 4)
        {
            u32 addrOffset32 = 0;
            offset += dw_read_u32(&addrOffset32, data, fileSize, offset);
            addrOffset = addrOffset32;
        }
        else
        {
            offset += dw_read_u64(&addrOffset, data, fileSize, offset);
        }
        //TODO:
        oc_log_warning("partially supported rnglistptr attribute (skipped)\n");
    }
    else
    {
        oc_log_error("unsupported form %s for rnglistptr\n", dw_get_form_string(form));
        exit(-1);
    }
    return (startOffset - offset);
}

u64 dw_parse_reference(dw_attr* res, dw_form form, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    switch(form)
    {
        case DW_FORM_ref1:
        {
            u8 ref8 = 0;
            offset += dw_read_u8(&ref8, data, fileSize, offset);
        }
        break;
        case DW_FORM_ref2:
        {
            u16 ref16 = 0;
            offset += dw_read_u16(&ref16, data, fileSize, offset);
        }
        break;
        case DW_FORM_ref4:
        {
            u32 ref32 = 0;
            offset += dw_read_u32(&ref32, data, fileSize, offset);
        }
        break;
        case DW_FORM_ref8:
        {
            u64 ref64 = 0;
            offset += dw_read_u64(&ref64, data, fileSize, offset);
        }
        break;
        case DW_FORM_ref_udata:
        {
            u64 ref64 = 0;
            offset += dw_read_leb128_u64(&ref64, data, fileSize, offset);
        }
        break;

        case DW_FORM_ref_addr:
        {
            u64 ref64 = 0;
            if(addressSize == 4)
            {
                u32 ref32 = 0;
                offset += dw_read_u32(&ref32, data, fileSize, offset);
                ref64 = ref32;
            }
            else
            {
                offset += dw_read_u64(&ref64, data, fileSize, offset);
            }
        }
        break;

        case DW_FORM_ref_sig8:
        {
            u64 sig = 0;
            offset += dw_read_u64(&sig, data, fileSize, offset);
        }
        break;

        case DW_FORM_ref_sup4:
        {
            u32 supOffset = 0;
            offset += dw_read_u32(&supOffset, data, fileSize, offset);
        }
        break;

        case DW_FORM_ref_sup8:
        {
            u64 supOffset = 0;
            offset += dw_read_u64(&supOffset, data, fileSize, offset);
        }
        break;

        default:
        {
            oc_log_error("unsupported form %s for reference\n", dw_get_form_string(form));
            exit(-1);
        }
        break;
    }
    //TODO:
    oc_log_warning("partially supported reference attribute (skipped)\n");

    return (startOffset - offset);
}

u64 dw_parse_string(dw_attr* res, dw_form form, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    switch(form)
    {
        case DW_FORM_string:
        {
            offset += dw_read_null_string(&res->string, data, fileSize, offset);
        }
        break;
        case DW_FORM_strp:
        case DW_FORM_line_strp:
        case DW_FORM_strp_sup:
        {
            u64 strOffset64 = 0;
            if(addressSize == 4)
            {
                u32 strOffset32 = 0;
                offset += dw_read_u32(&strOffset32, data, fileSize, offset);
                strOffset64 = strOffset32;
            }
            else
            {
                offset += dw_read_u64(&strOffset64, data, fileSize, offset);
            }
            //TODO:
            oc_log_warning("partially supported string attribute (skipped)\n");
        }
        break;

        case DW_FORM_strx1:
        {
            u8 indOffset = 0;
            offset += dw_read_u8(&indOffset, data, fileSize, offset);
            //TODO:
            oc_log_warning("partially supported string attribute (skipped)\n");
        }
        break;
        case DW_FORM_strx2:
        {
            u16 indOffset = 0;
            offset += dw_read_u16(&indOffset, data, fileSize, offset);
            //TODO:
            oc_log_warning("partially supported string attribute (skipped)\n");
        }
        break;
        case DW_FORM_strx3:
        {
            u16 indOffset16 = 0;
            offset += dw_read_u16(&indOffset16, data, fileSize, offset);
            u8 indOffset8 = 0;
            offset += dw_read_u8(&indOffset8, data, fileSize, offset);
            //TODO: combine 3 bytes
            //TODO:
            oc_log_warning("partially supported string attribute (skipped)\n");
        }
        break;
        case DW_FORM_strx4:
        {
            u32 indOffset = 0;
            offset += dw_read_u32(&indOffset, data, fileSize, offset);
            //TODO:
            oc_log_warning("partially supported string attribute (skipped)\n");
        }
        break;
        case DW_FORM_strx:
        {
            u64 indOffset = 0;
            offset += dw_read_leb128_u64(&indOffset, data, fileSize, offset);
            //TODO:
            oc_log_warning("partially supported string attribute (skipped)\n");
        }
        break;

        default:
        {
            oc_log_error("unsupported form %s for string\n", dw_get_form_string(form));
            exit(-1);
        }
        break;
    }

    return (startOffset - offset);
}

u64 dw_parse_stroffsetsptr(dw_attr* res, dw_form form, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    if(form == DW_FORM_sec_offset)
    {
        u64 addrOffset = 0;
        if(addressSize == 4)
        {
            u32 addrOffset32 = 0;
            offset += dw_read_u32(&addrOffset32, data, fileSize, offset);
            addrOffset = addrOffset32;
        }
        else
        {
            offset += dw_read_u64(&addrOffset, data, fileSize, offset);
        }
        //TODO:
        oc_log_warning("partially supported stroffsetsptr attribute (skipped)\n");
    }
    else
    {
        oc_log_error("unsupported form %s for stroffsetsptr\n", dw_get_form_string(form));
        exit(-1);
    }
    return (startOffset - offset);
}

u64 dw_parse_attr(oc_arena* arena, dw_attr* res, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    dw_form form = res->abbrev->form;

    switch(res->abbrev->name)
    {
        case DW_AT_sibling:
        {
            offset += dw_parse_reference(res, form, addressSize, data, fileSize, offset);
        }
        break;
        case DW_AT_location:
        {
            if(form == DW_FORM_exprloc)
            {
                offset += dw_parse_exprloc(res, form, addressSize, data, fileSize, offset);
            }
            else
            {
                offset += dw_parse_loclist(res, form, addressSize, data, fileSize, offset);
            }
        }
        break;
        case DW_AT_name:
        {
            offset += dw_parse_string(res, form, addressSize, data, fileSize, offset);
        }
        break;
        case DW_AT_ordering:
        {
            offset += dw_parse_constant(res, form, addressSize, data, fileSize, offset);
        }
        break;
        case DW_AT_byte_size:
        {
            if(form == DW_FORM_exprloc)
            {
                offset += dw_parse_exprloc(res, form, addressSize, data, fileSize, offset);
            }
        }
        break;
        case DW_AT_bit_size:
        {
        }
        break;
        case DW_AT_stmt_list:
        {
        }
        break;
        case DW_AT_low_pc:
        {
        }
        break;
        case DW_AT_high_pc:
        {
        }
        break;
        case DW_AT_language:
        {
        }
        break;
        case DW_AT_discr:
        {
        }
        break;
        case DW_AT_discr_value:
        {
        }
        break;
        case DW_AT_visibility:
        {
        }
        break;
        case DW_AT_import:
        {
        }
        break;
        case DW_AT_string_length:
        {
        }
        break;
        case DW_AT_common_reference:
        {
        }
        break;
        case DW_AT_comp_dir:
        {
        }
        break;
        case DW_AT_const_value:
        {
        }
        break;
        case DW_AT_containing_type:
        {
        }
        break;
        case DW_AT_default_value:
        {
        }
        break;
        case DW_AT_inline:
        {
        }
        break;
        case DW_AT_is_optional:
        {
        }
        break;
        case DW_AT_lower_bound:
        {
        }
        break;
        case DW_AT_producer:
        {
        }
        break;
        case DW_AT_prototyped:
        {
        }
        break;
        case DW_AT_return_addr:
        {
        }
        break;
        case DW_AT_start_scope:
        {
        }
        break;
        case DW_AT_bit_stride:
        {
        }
        break;
        case DW_AT_upper_bound:
        {
        }
        break;
        case DW_AT_abstract_origin:
        {
        }
        break;
        case DW_AT_accessibility:
        {
        }
        break;
        case DW_AT_address_class:
        {
        }
        break;
        case DW_AT_artificial:
        {
        }
        break;
        case DW_AT_base_types:
        {
        }
        break;
        case DW_AT_calling_convention:
        {
        }
        break;
        case DW_AT_count:
        {
        }
        break;
        case DW_AT_data_member_location:
        {
        }
        break;
        case DW_AT_decl_column:
        {
        }
        break;
        case DW_AT_decl_file:
        {
        }
        break;
        case DW_AT_decl_line:
        {
        }
        break;
        case DW_AT_declaration:
        {
        }
        break;
        case DW_AT_discr_list:
        {
        }
        break;
        case DW_AT_encoding:
        {
        }
        break;
        case DW_AT_external:
        {
        }
        break;
        case DW_AT_frame_base:
        {
        }
        break;
        case DW_AT_friend:
        {
        }
        break;
        case DW_AT_identifier_case:
        {
        }
        break;
        case DW_AT_namelist_item:
        {
        }
        break;
        case DW_AT_priority:
        {
        }
        break;
        case DW_AT_segment:
        {
        }
        break;
        case DW_AT_specification:
        {
        }
        break;
        case DW_AT_static_link:
        {
        }
        break;
        case DW_AT_type:
        {
        }
        break;
        case DW_AT_use_location:
        {
        }
        break;
        case DW_AT_variable_parameter:
        {
        }
        break;
        case DW_AT_virtuality:
        {
        }
        break;
        case DW_AT_vtable_elem_location:
        {
        }
        break;
        case DW_AT_allocated:
        {
        }
        break;
        case DW_AT_associated:
        {
        }
        break;
        case DW_AT_data_location:
        {
        }
        break;
        case DW_AT_byte_stride:
        {
        }
        break;
        case DW_AT_entry_pc:
        {
        }
        break;
        case DW_AT_use_UTF8:
        {
        }
        break;
        case DW_AT_extension:
        {
        }
        break;
        case DW_AT_ranges:
        {
        }
        break;
        case DW_AT_trampoline:
        {
        }
        break;
        case DW_AT_call_column:
        {
        }
        break;
        case DW_AT_call_file:
        {
        }
        break;
        case DW_AT_call_line:
        {
        }
        break;
        case DW_AT_description:
        {
        }
        break;
        case DW_AT_binary_scale:
        {
        }
        break;
        case DW_AT_decimal_scale:
        {
        }
        break;
        case DW_AT_small:
        {
        }
        break;
        case DW_AT_decimal_sign:
        {
        }
        break;
        case DW_AT_digit_count:
        {
        }
        break;
        case DW_AT_picture_string:
        {
        }
        break;
        case DW_AT_mutable:
        {
        }
        break;
        case DW_AT_threads_scaled:
        {
        }
        break;
        case DW_AT_explicit:
        {
        }
        break;
        case DW_AT_object_pointer:
        {
        }
        break;
        case DW_AT_endianity:
        {
        }
        break;
        case DW_AT_elemental:
        {
        }
        break;
        case DW_AT_pure:
        {
        }
        break;
        case DW_AT_recursive:
        {
        }
        break;
        case DW_AT_signature:
        {
        }
        break;
        case DW_AT_main_subprogram:
        {
        }
        break;
        case DW_AT_data_bit_offset:
        {
        }
        break;
        case DW_AT_const_expr:
        {
        }
        break;
        case DW_AT_enum_class:
        {
        }
        break;
        case DW_AT_linkage_name:
        {
        }
        break;
        case DW_AT_string_length_bit_size:
        {
        }
        break;
        case DW_AT_string_length_byte_size:
        {
        }
        break;
        case DW_AT_rank:
        {
        }
        break;
        case DW_AT_str_offsets_base:
        {
        }
        break;
        case DW_AT_addr_base:
        {
        }
        break;
        case DW_AT_rnglists_base:
        {
        }
        break;
        case DW_AT_dwo_name:
        {
        }
        break;
        case DW_AT_reference:
        {
        }
        break;
        case DW_AT_rvalue_reference:
        {
        }
        break;
        case DW_AT_macros:
        {
        }
        break;
        case DW_AT_call_all_calls:
        {
        }
        break;
        case DW_AT_call_all_source_calls:
        {
        }
        break;
        case DW_AT_call_all_tail_calls:
        {
        }
        break;
        case DW_AT_call_return_pc:
        {
        }
        break;
        case DW_AT_call_value:
        {
        }
        break;
        case DW_AT_call_origin:
        {
        }
        break;
        case DW_AT_call_parameter:
        {
        }
        break;
        case DW_AT_call_pc:
        {
        }
        break;
        case DW_AT_call_tail_call:
        {
        }
        break;
        case DW_AT_call_target:
        {
        }
        break;
        case DW_AT_call_target_clobbered:
        {
        }
        break;
        case DW_AT_call_data_location:
        {
        }
        break;
        case DW_AT_call_data_value:
        {
        }
        break;
        case DW_AT_noreturn:
        {
        }
        break;
        case DW_AT_alignment:
        {
        }
        break;
        case DW_AT_export_symbols:
        {
        }
        break;
        case DW_AT_deleted:
        {
        }
        break;
        case DW_AT_defaulted:
        {
        }
        break;
        case DW_AT_loclists_base:
        {
        }
        break;

        default:
        {
            oc_log_error("unsupported attribute name\n");
            exit(-1);
        }
        break;
    }

    return (offset - startOffset);
}

void dw_dump_info(dw_sections sections)
{
    oc_str8 contents = sections.info;
    u64 offset = 0;
    while(offset < contents.len)
    {
        u64 unitStart = offset;

        u32 lengthSize = 4;
        u32 initialLength = 0;

        {
            u32 length32 = 0;
            offset += dw_read_u32(&length32, contents.ptr, contents.len, offset);
            if(length32 >= 0xfffffff0)
            {
                lengthSize = 8;
                offset += dw_read_u32(&initialLength, contents.ptr, contents.len, offset);
            }
            else
            {
                initialLength = length32;
            }
        }

        u16 version = 0;
        offset += dw_read_u16(&version, contents.ptr, contents.len, offset);

        u8 unitType = DW_UT_compile;
        if(version >= 5)
        {
            offset += dw_read_u8(&unitType, contents.ptr, contents.len, offset);
        }

        printf("0x%016llx\n", offset);
        printf("    type: %s\n", dw_get_cu_type_string(unitType));
        printf(" version: %i\n", version);

        if(unitType == DW_UT_compile)
        {
            u8 addressSize;
            u64 abbrevOffset = 0;

            if(version >= 5)
            {
                //NOTE: debug abbrev offset and address size are ordered differently in dwarf v4 and v5, because.. why not?
                offset += dw_read_u8(&addressSize, contents.ptr, contents.len, offset);

                if(lengthSize == 4)
                {
                    u32 abbrevOffset32 = 0;
                    offset += dw_read_u32(&abbrevOffset32, contents.ptr, contents.len, offset);
                    abbrevOffset = abbrevOffset32;
                }
                else
                {
                    offset += dw_read_u64(&abbrevOffset, contents.ptr, contents.len, offset);
                }
            }
            else
            {
                if(lengthSize == 4)
                {
                    u32 abbrevOffset32 = 0;
                    offset += dw_read_u32(&abbrevOffset32, contents.ptr, contents.len, offset);
                    abbrevOffset = abbrevOffset32;
                }
                else
                {
                    offset += dw_read_u64(&abbrevOffset, contents.ptr, contents.len, offset);
                }

                offset += dw_read_u8(&addressSize, contents.ptr, contents.len, offset);
            }

            oc_arena_scope scratch = oc_scratch_begin();

            dw_abbrev_table* abbrevTable = dw_load_abbrev_table(scratch.arena, sections.abbrev, abbrevOffset);

            // read the first DIE, hopefully the CU DIE?
            u64 abbrevCode = 0;
            offset += dw_read_leb128_u64(&abbrevCode, contents.ptr, contents.len, offset);

            // find abbreviation
            dw_abbrev_entry* abbrev = 0;
            for(u64 abbrevIndex = 0; abbrevIndex < abbrevTable->entryCount; abbrevIndex++)
            {
                if(abbrevTable->entries[abbrevIndex].code == abbrevCode)
                {
                    abbrev = &abbrevTable->entries[abbrevIndex];
                    break;
                }
            }

            if(!abbrev)
            {
                oc_log_error("Couldn't find abbrev code %llu\n", abbrevCode);
                exit(-1);
            }

            for(u32 attrIndex = 0; attrIndex < abbrev->attrCount; attrIndex++)
            {
                dw_abbrev_attr* abbrevAttr = &abbrev->attributes[attrIndex];

                printf("    attr: %s\n", dw_get_attr_name_string(abbrevAttr->name));
                printf("    form: %s\n", dw_get_form_string(abbrevAttr->form));

                dw_attr attr = { 0 };
                attr.abbrev = abbrevAttr;
                offset += dw_parse_attr(scratch.arena, &attr, addressSize, contents.ptr, contents.len, offset);
            }

            oc_scratch_end(scratch);
        }
        else
        {
            oc_log_warning("first DIE is not a compile unit DIE\n");
        }

        // skip to next unit
        offset = unitStart + initialLength + lengthSize;
    }
}

void dw_dump_abbrev_table(oc_str8 contents)
{
    u64 offset = 0;

    while(offset < contents.len)
    {
        u32 abbrevCode = 0;
        u32 abbrevTag = 0;
        u8 children = 0;

        offset += dw_read_leb128_u32(&abbrevCode, contents.ptr, contents.len, offset);
        if(abbrevCode == 0)
        {
            //null entry
            continue;
        }

        offset += dw_read_leb128_u32(&abbrevTag, contents.ptr, contents.len, offset);
        offset += dw_read_u8(&children, contents.ptr, contents.len, offset);

        printf("0x%08x %s %s\n", abbrevCode, dw_get_tag_string(abbrevTag), children == 0 ? "DW_CHILDREN_no" : "DW_CHILDREN_yes");

        // attributes
        u32 attrName = 0;
        u32 attrForm = 0;
        do
        {
            offset += dw_read_leb128_u32(&attrName, contents.ptr, contents.len, offset);
            offset += dw_read_leb128_u32(&attrForm, contents.ptr, contents.len, offset);

            if(attrName != 0 && attrForm != 0)
            {
                printf("  %s %s\n", dw_get_attr_name_string(attrName), dw_get_form_string(attrForm));
            }
        }
        while((attrName != 0 || attrForm != 0)
              && (offset < contents.len));
    }
}
