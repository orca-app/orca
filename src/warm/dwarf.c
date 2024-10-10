
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
    X(DW_TAG_hi_user, 0xffff)

enum
{
    DW_TAG_LIST(X_ENUM)
};

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
    X(DW_AT_hi_user, 0x3fff)

enum
{
    DW_ATTR_LIST(X_ENUM)
};

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
    X(DW_FORM_addrx4, 0x2c)

enum
{
    DW_FORM_LIST(X_ENUM)
};

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

enum
{
    DW_LNS_LIST(X_ENUM)
};

enum
{
    DW_LNE_LIST(X_ENUM)
};

#define DW_LNCT_LIST(X)              \
    X(DW_LNCT_path, 0x01)            \
    X(DW_LNCT_directory_index, 0x02) \
    X(DW_LNCT_timestamp, 0x03)       \
    X(DW_LNCT_size, 0x04)            \
    X(DW_LNCT_MD5, 0x05)             \
    X(DW_LNCT_lo_user, 0x2000)       \
    X(DW_LNCT_hi_user, 0x3fff)

enum
{
    DW_LNCT_LIST(X_ENUM)
};

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

int dw_read_leb128_u32(unsigned int* res, char* data, int fileSize, int offset)
{
    int start = offset;
    char byte = 0;
    int acc = 0;
    int shift = 0;
    int size = 0;
    do
    {
        if(offset + sizeof(char) > fileSize)
        {
            printf("error: read out of bounds\n");
            exit(-1);
        }
        byte = data[offset];
        offset++;

        acc |= ((int)byte & 0x7f) << shift;
        shift += 7;
        size++;
    }
    while(byte & 0x80);

    *res = acc;
    return (offset - start);
}

int dw_read_leb128_u64(u64* res, char* data, int fileSize, int offset)
{
    int start = offset;
    char byte = 0;
    u64 acc = 0;
    int shift = 0;
    int size = 0;
    do
    {
        if(offset + sizeof(int) > fileSize)
        {
            printf("error: read out of bounds");
            exit(-1);
        }
        byte = data[offset];
        offset++;

        acc |= ((u64)byte & 0x7f) << shift;
        shift += 7;
        size++;
    }
    while(byte & 0x80);

    *res = acc;
    return (offset - start);
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

} dw_line_machine;

typedef struct dw_file_entry
{
    oc_str8 path;
    u64 dirIndex;
    u64 timestamp;
    u64 size;
    u16 md5;

} dw_file_entry;

typedef struct dw_line_program_header
{
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

    u64 dirEntryCount;
    dw_file_entry* dirEntries;

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
    dw_line_entry_flags flags;
} dw_line_entry;

typedef struct dw_line_table
{
    u64 dirEntryCount;
    dw_file_entry* dirEntries;

    u64 fileEntryCount;
    dw_file_entry* fileEntries;

    u64 entryCount;
    dw_line_entry* entries;
} dw_line_table;

typedef struct dw_line_info
{
    u64 tableCount;
    dw_line_table* tables;
} dw_line_info;

int dw_read_file_entries(oc_arena* arena,
                         u64* entryCount,
                         dw_file_entry** entries,
                         dw_sections* sections,
                         dw_line_program_header* header,
                         char* data,
                         u64 fileSize,
                         u64 offset)
{
    u64 start = offset;
    u8 formatCount = 0;
    offset += dw_read_u8(&formatCount, data, fileSize, offset);

    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    dw_file_entry_format_elt* format = oc_arena_push_array(scratch.arena, dw_file_entry_format_elt, formatCount);

    for(int i = 0; i < formatCount; i++)
    {
        offset += dw_read_leb128_u64(&format[i].content, data, fileSize, offset);
        offset += dw_read_leb128_u64(&format[i].form, data, fileSize, offset);
    }

    offset += dw_read_leb128_u64(entryCount, data, fileSize, offset);

    *entries = oc_arena_push_array(arena, dw_file_entry, *entryCount);
    memset((*entries), 0, (*entryCount) * sizeof(dw_file_entry));

    for(u64 entryIndex = 0; entryIndex < *entryCount; entryIndex++)
    {
        for(int fmtIndex = 0; fmtIndex < formatCount; fmtIndex++)
        {
            u64 content = format[fmtIndex].content;
            u64 form = format[fmtIndex].form;
            dw_file_entry* entry = &((*entries)[entryIndex]);

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
                    offset += dw_read_u16(&entry->md5, data, fileSize, offset);
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
    }
    oc_scratch_end(scratch);
    return (offset - start);
}

int dw_read_line_program_header(oc_arena* arena, dw_line_program_header* header, dw_sections* sections, char* data, u64 fileSize, u64 offset)
{
    u64 start = offset;

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

    if(header->version != 5)
    {
        printf("error: DWARF version %i not supported\n", header->version);
        exit(-1);
    }

    offset += dw_read_u8(&header->addressSize, data, fileSize, offset);
    offset += dw_read_u8(&header->segmentSelectorSize, data, fileSize, offset);

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
    offset += dw_read_file_entries(arena, &header->dirEntryCount, &header->dirEntries, sections, header, data, fileSize, offset);

    // files
    offset += dw_read_file_entries(arena, &header->fileEntryCount, &header->fileEntries, sections, header, data, fileSize, offset);

    //NOTE: return offset from start to beginning of line program code
    return (headerLengthBase + header->headerLength - start);
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

void dw_line_machine_emit_row(oc_arena* arena, dw_line_machine* m, oc_list* rowList)
{
    dw_line_entry_elt* elt = oc_arena_push_type(arena, dw_line_entry_elt);
    memset(elt, 0, sizeof(dw_line_entry_elt));

    elt->entry.address = m->address;
    elt->entry.line = m->line;
    elt->entry.column = m->column;
    elt->entry.file = m->file;

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

        if(header.version != 5)
        {
            oc_log_error("DWARF version %i not supported\n", header.version);
            exit(-1);
        }
        if(header.addressSize != 4 && header.addressSize != 8)
        {
            oc_log_error("address size should be 4 or 8\n");
            exit(-1);
        }

        dw_line_table_elt* table = oc_arena_push_type(scratch.arena, dw_line_table_elt);
        oc_list_push_back(&tablesList, &table->listElt);

        table->table.dirEntryCount = header.dirEntryCount;
        table->table.dirEntries = header.dirEntries;

        table->table.fileEntryCount = header.fileEntryCount;
        table->table.fileEntries = header.fileEntries;

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

                dw_line_machine_emit_row(scratch.arena, &machine, &rowsList);
                rowCount++;

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
                        dw_line_machine_emit_row(scratch.arena, &machine, &rowsList);
                        rowCount++;
                        dw_line_machine_reset(&machine, header.defaultIsStmt);
                    }
                    break;
                    case DW_LNE_set_address:
                    {
                        u64 address = 0;
                        if(header.addressSize == 4)
                        {
                            u32 address32 = 0;
                            offset += dw_read_u32(&address32, lineSection.ptr, lineSection.len, offset);
                            address = address32;
                        }
                        else if(header.addressSize == 8)
                        {
                            offset += dw_read_u64(&address, lineSection.ptr, lineSection.len, offset);
                        }
                        else
                        {
                            OC_ASSERT(0);
                        }
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
                            printf("error: unrecognized line program opcode\n");
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
                        dw_line_machine_emit_row(scratch.arena, &machine, &rowsList);
                        rowCount++;
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
                        offset += dw_read_leb128_u64((u64*)&lineAdvance, lineSection.ptr, lineSection.len, offset);
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
                        printf("error: unrecognized line program opcode\n");
                        exit(-1);
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

void dw_print_line_table_header()
{
    printf("|   address  | line |  col | file | stmt | block | end | prolog | epilog |\n"
           "|------------------------------------------------------------------------|\n");
}

void dw_print_line_info(dw_line_info* info)
{
    for(u64 tableIndex = 0; tableIndex < info->tableCount; tableIndex++)
    {
        dw_line_table* table = &info->tables[tableIndex];

        printf("directories:\n");
        for(int i = 0; i < table->dirEntryCount; i++)
        {
            printf("[%i] %.*s\n", i, oc_str8_ip(table->dirEntries[i].path));
        }

        printf("\nfiles:\n");
        for(int i = 0; i < table->fileEntryCount; i++)
        {
            dw_file_entry* entry = &(table->fileEntries[i]);
            printf("[%i] %.*s\n"
                   "    dirIndex: %llu\n"
                   "    timestamp: %llu\n"
                   "    size: %llu\n"
                   "    md5: 0x%04x\n",
                   i, oc_str8_ip(entry->path),
                   entry->dirIndex,
                   entry->timestamp,
                   entry->size,
                   entry->md5);
        }
        printf("\n");

        dw_print_line_table_header();

        for(u64 rowIndex = 0; rowIndex < table->entryCount; rowIndex++)
        {
            dw_line_entry* entry = &table->entries[rowIndex];
            if(entry->flags & DW_LINE_SEQUENCE_END)
            {
                printf("| 0x%08llx |      |      |      |      |       |  x  |        |        |\n",
                       entry->address);
            }
            else
            {
                printf("| 0x%08llx | %4llu | %4llu | %4llu |   %s  |   %s   |  %s  |   %s    |   %s    |\n",
                       entry->address,
                       entry->line,
                       entry->column,
                       entry->file,
                       (entry->flags & DW_LINE_STMT) ? "x" : " ",
                       (entry->flags & DW_LINE_BASIC_BLOCK) ? "x" : " ",
                       (entry->flags & DW_LINE_SEQUENCE_END) ? "x" : " ",
                       (entry->flags & DW_LINE_PROLOGUE_END) ? "x" : " ",
                       (entry->flags & DW_LINE_EPILOGUE_BEGIN) ? "x" : " ");
            }
        }
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