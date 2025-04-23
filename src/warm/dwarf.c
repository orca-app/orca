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

#define X_NAME_CASE_STR8(name, ...) \
    case name:                      \
        res = OC_STR8(#name);       \
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

#define DW_ATE_LIST(X)              \
    X(DW_ATE_address, 0x01)         \
    X(DW_ATE_boolean, 0x02)         \
    X(DW_ATE_complex_float, 0x03)   \
    X(DW_ATE_float, 0x04)           \
    X(DW_ATE_signed, 0x05)          \
    X(DW_ATE_signed_char, 0x06)     \
    X(DW_ATE_unsigned, 0x07)        \
    X(DW_ATE_unsigned_char, 0x08)   \
    X(DW_ATE_imaginary_float, 0x09) \
    X(DW_ATE_packed_decimal, 0x0a)  \
    X(DW_ATE_numeric_string, 0x0b)  \
    X(DW_ATE_edited, 0x0c)          \
    X(DW_ATE_signed_fixed, 0x0d)    \
    X(DW_ATE_unsigned_fixed, 0x0e)  \
    X(DW_ATE_decimal_float, 0x0f)   \
    X(DW_ATE_UTF, 0x10)             \
    X(DW_ATE_UCS, 0x11)             \
    X(DW_ATE_ASCII, 0x12)           \
    X(DW_ATE_lo_user, 0x80)         \
    X(DW_ATE_hi_user, 0xff)

typedef enum dw_encoding
{
    DW_ATE_LIST(X_ENUM)
} dw_encoding;

const char* dw_get_encoding_string(u32 name)
{
    const char* res = 0;
    switch(name)
    {
        DW_ATE_LIST(X_NAME_CASE);

        default:
            res = "Unknown encoding";
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

#define DW_OP_LIST(_)                  \
    _(DW_OP_addr, 0x03)                \
    _(DW_OP_deref, 0x06)               \
    _(DW_OP_const1u, 0x08)             \
    _(DW_OP_const1s, 0x09)             \
    _(DW_OP_const2u, 0x0a)             \
    _(DW_OP_const2s, 0x0b)             \
    _(DW_OP_const4u, 0x0c)             \
    _(DW_OP_const4s, 0x0d)             \
    _(DW_OP_const8u, 0x0e)             \
    _(DW_OP_const8s, 0x0f)             \
    _(DW_OP_constu, 0x10)              \
    _(DW_OP_consts, 0x11)              \
    _(DW_OP_dup, 0x12)                 \
    _(DW_OP_drop, 0x13)                \
    _(DW_OP_over, 0x14)                \
    _(DW_OP_pick, 0x15)                \
    _(DW_OP_swap, 0x16)                \
    _(DW_OP_rot, 0x17)                 \
    _(DW_OP_xderef, 0x18)              \
    _(DW_OP_abs, 0x19)                 \
    _(DW_OP_and, 0x1a)                 \
    _(DW_OP_div, 0x1b)                 \
    _(DW_OP_minus, 0x1c)               \
    _(DW_OP_mod, 0x1d)                 \
    _(DW_OP_mul, 0x1e)                 \
    _(DW_OP_neg, 0x1f)                 \
    _(DW_OP_not, 0x20)                 \
    _(DW_OP_or, 0x21)                  \
    _(DW_OP_plus, 0x22)                \
    _(DW_OP_plus_uconst, 0x23)         \
    _(DW_OP_shl, 0x24)                 \
    _(DW_OP_shr, 0x25)                 \
    _(DW_OP_shra, 0x26)                \
    _(DW_OP_xor, 0x27)                 \
    _(DW_OP_bra, 0x28)                 \
    _(DW_OP_eq, 0x29)                  \
    _(DW_OP_ge, 0x2a)                  \
    _(DW_OP_gt, 0x2b)                  \
    _(DW_OP_le, 0x2c)                  \
    _(DW_OP_lt, 0x2d)                  \
    _(DW_OP_ne, 0x2e)                  \
    _(DW_OP_skip, 0x2f)                \
    _(DW_OP_lit0, 0x30)                \
    _(DW_OP_lit1, 0x31)                \
    _(DW_OP_lit2, 0x32)                \
    _(DW_OP_lit3, 0x33)                \
    _(DW_OP_lit4, 0x34)                \
    _(DW_OP_lit5, 0x35)                \
    _(DW_OP_lit6, 0x36)                \
    _(DW_OP_lit7, 0x37)                \
    _(DW_OP_lit8, 0x38)                \
    _(DW_OP_lit9, 0x39)                \
    _(DW_OP_lit10, 0x3a)               \
    _(DW_OP_lit11, 0x3b)               \
    _(DW_OP_lit12, 0x3c)               \
    _(DW_OP_lit13, 0x3d)               \
    _(DW_OP_lit14, 0x3e)               \
    _(DW_OP_lit15, 0x3f)               \
    _(DW_OP_lit16, 0x40)               \
    _(DW_OP_lit17, 0x41)               \
    _(DW_OP_lit18, 0x42)               \
    _(DW_OP_lit19, 0x43)               \
    _(DW_OP_lit20, 0x44)               \
    _(DW_OP_lit21, 0x45)               \
    _(DW_OP_lit22, 0x46)               \
    _(DW_OP_lit23, 0x47)               \
    _(DW_OP_lit24, 0x48)               \
    _(DW_OP_lit25, 0x49)               \
    _(DW_OP_lit26, 0x4a)               \
    _(DW_OP_lit27, 0x4b)               \
    _(DW_OP_lit28, 0x4c)               \
    _(DW_OP_lit29, 0x4d)               \
    _(DW_OP_lit30, 0x4e)               \
    _(DW_OP_lit31, 0x4f)               \
    _(DW_OP_reg0, 0x50)                \
    _(DW_OP_reg1, 0x51)                \
    _(DW_OP_reg2, 0x52)                \
    _(DW_OP_reg3, 0x53)                \
    _(DW_OP_reg4, 0x54)                \
    _(DW_OP_reg5, 0x55)                \
    _(DW_OP_reg6, 0x56)                \
    _(DW_OP_reg7, 0x57)                \
    _(DW_OP_reg8, 0x58)                \
    _(DW_OP_reg9, 0x59)                \
    _(DW_OP_reg10, 0x5a)               \
    _(DW_OP_reg11, 0x5b)               \
    _(DW_OP_reg12, 0x5c)               \
    _(DW_OP_reg13, 0x5d)               \
    _(DW_OP_reg14, 0x5e)               \
    _(DW_OP_reg15, 0x5f)               \
    _(DW_OP_reg16, 0x60)               \
    _(DW_OP_reg17, 0x61)               \
    _(DW_OP_reg18, 0x62)               \
    _(DW_OP_reg19, 0x63)               \
    _(DW_OP_reg20, 0x64)               \
    _(DW_OP_reg21, 0x65)               \
    _(DW_OP_reg22, 0x66)               \
    _(DW_OP_reg23, 0x67)               \
    _(DW_OP_reg24, 0x68)               \
    _(DW_OP_reg25, 0x69)               \
    _(DW_OP_reg26, 0x6a)               \
    _(DW_OP_reg27, 0x6b)               \
    _(DW_OP_reg28, 0x6c)               \
    _(DW_OP_reg29, 0x6d)               \
    _(DW_OP_reg30, 0x6e)               \
    _(DW_OP_reg31, 0x6f)               \
    _(DW_OP_breg0, 0x70)               \
    _(DW_OP_breg1, 0x71)               \
    _(DW_OP_breg2, 0x72)               \
    _(DW_OP_breg3, 0x73)               \
    _(DW_OP_breg4, 0x74)               \
    _(DW_OP_breg5, 0x75)               \
    _(DW_OP_breg6, 0x76)               \
    _(DW_OP_breg7, 0x77)               \
    _(DW_OP_breg8, 0x78)               \
    _(DW_OP_breg9, 0x79)               \
    _(DW_OP_breg10, 0x7a)              \
    _(DW_OP_breg11, 0x7b)              \
    _(DW_OP_breg12, 0x7c)              \
    _(DW_OP_breg13, 0x7d)              \
    _(DW_OP_breg14, 0x7e)              \
    _(DW_OP_breg15, 0x7f)              \
    _(DW_OP_breg16, 0x80)              \
    _(DW_OP_breg17, 0x81)              \
    _(DW_OP_breg18, 0x82)              \
    _(DW_OP_breg19, 0x83)              \
    _(DW_OP_breg20, 0x84)              \
    _(DW_OP_breg21, 0x85)              \
    _(DW_OP_breg22, 0x86)              \
    _(DW_OP_breg23, 0x87)              \
    _(DW_OP_breg24, 0x88)              \
    _(DW_OP_breg25, 0x89)              \
    _(DW_OP_breg26, 0x8a)              \
    _(DW_OP_breg27, 0x8b)              \
    _(DW_OP_breg28, 0x8c)              \
    _(DW_OP_breg29, 0x8d)              \
    _(DW_OP_breg30, 0x8e)              \
    _(DW_OP_breg31, 0x8f)              \
    _(DW_OP_regx, 0x90)                \
    _(DW_OP_fbreg, 0x91)               \
    _(DW_OP_bregx, 0x92)               \
    _(DW_OP_piece, 0x93)               \
    _(DW_OP_deref_size, 0x94)          \
    _(DW_OP_xderef_size, 0x95)         \
    _(DW_OP_nop, 0x96)                 \
    _(DW_OP_push_object_address, 0x97) \
    _(DW_OP_call2, 0x98)               \
    _(DW_OP_call4, 0x99)               \
    _(DW_OP_call_ref, 0x9a)            \
    _(DW_OP_form_tls_address, 0x9b)    \
    _(DW_OP_call_frame_cfa, 0x9c)      \
    _(DW_OP_bit_piece, 0x9d)           \
    _(DW_OP_implicit_value, 0x9e)      \
    _(DW_OP_stack_value, 0x9f)         \
    _(DW_OP_implicit_pointer, 0xa0)    \
    _(DW_OP_addrx, 0xa1)               \
    _(DW_OP_constx, 0xa2)              \
    _(DW_OP_entry_value, 0xa3)         \
    _(DW_OP_const_type, 0xa4)          \
    _(DW_OP_regval_type, 0xa5)         \
    _(DW_OP_deref_type, 0xa6)          \
    _(DW_OP_xderef_type, 0xa7)         \
    _(DW_OP_convert, 0xa8)             \
    _(DW_OP_reinterpret, 0xa9)         \
    _(DW_OP_lo_user, 0xe0)             \
    _(DW_OP_hi_user, 0xff)             \
    _(DW_OP_WASM_location, 0xed)

typedef enum dw_op
{
    DW_OP_LIST(X_ENUM)
} dw_op;

oc_str8 dw_op_get_string(u32 opcode)
{
    oc_str8 res = { 0 };
    switch(opcode)
    {
        DW_OP_LIST(X_NAME_CASE_STR8);
    }
    return res;
}

#define DW_LLE_LIST(_)               \
    _(DW_LLE_end_of_list, 0x00)      \
    _(DW_LLE_base_addressx, 0x01)    \
    _(DW_LLE_startx_endx, 0x02)      \
    _(DW_LLE_startx_length, 0x03)    \
    _(DW_LLE_offset_pair, 0x04)      \
    _(DW_LLE_default_location, 0x05) \
    _(DW_LLE_base_address, 0x06)     \
    _(DW_LLE_start_end, 0x07)        \
    _(DW_LLE_start_length, 0x08)

typedef enum dw_lle
{
    DW_LLE_LIST(X_ENUM)
} dw_lle;

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
    DW_DWARF32,
    DW_DWARF64,
};

typedef struct dw_abbrev_attr
{
    dw_attr_name name;
    dw_form form;
    i64 implicitConst;
} dw_abbrev_attr;

typedef struct dw_abbrev_entry
{
    u64 code;
    u8 hasChildren;
    dw_tag tag;
    u32 attrCount;
    dw_abbrev_attr* attributes;
} dw_abbrev_entry;

typedef struct dw_abbrev_table
{
    u64 entryCount;
    dw_abbrev_entry* entries;
} dw_abbrev_table;

typedef struct dw_loc_entry
{
    u64 start;
    u64 end;
    oc_str8 desc;
} dw_loc_entry;

typedef struct dw_loc
{
    bool single;
    u64 entryCount;
    dw_loc_entry* entries;
} dw_loc;

typedef struct dw_attr
{
    dw_abbrev_attr* abbrev;

    union
    {
        dw_loc loc;
        oc_str8 string;
        u8 valU8;
        u16 valU16;
        u32 valU32;
        u64 valU64;
        i64 valI64;
    };
} dw_attr;

typedef struct dw_die dw_die;

typedef struct dw_die
{
    oc_list_elt parentElt;
    dw_die* parent;
    oc_list children;

    u64 start;
    u64 abbrevCode;
    dw_abbrev_entry* abbrev;
    dw_attr* attributes;
} dw_die;

typedef struct dw_unit
{
    u64 start;
    u64 initialLength;
    u16 version;
    u8 format;
    u8 type;

    union
    {
        // full and partial compilation units
        struct
        {
            u8 addressSize;
            u64 abbrevOffset;
        };

        //TODO: other types of units
    };

    dw_abbrev_table* abbrev;

    dw_die* rootDie;

} dw_unit;

typedef struct dw_reader
{
    oc_str8 contents;
    u64 offset;
    //TODO: error
} dw_reader;

bool dw_reader_has_more(dw_reader* reader)
{
    return reader->offset < reader->contents.len;
}

void dw_reader_seek(dw_reader* reader, u64 offset)
{
    //TODO: check offset
    reader->offset = oc_min(reader->contents.len, offset);
}

dw_reader dw_reader_subreader(dw_reader* reader, u64 size)
{
    if(reader->offset + size > reader->contents.len)
    {
        oc_log_error("Couldn't create subreader, size out of range\n");
    }

    dw_reader sub = {
        .contents = oc_str8_slice(reader->contents, reader->offset, reader->offset + size),
        .offset = 0,
    };
    return sub;
}

u64 dw_read_leb128(dw_reader* reader, u32 bitWidth, bool isSigned)
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

u32 dw_read_leb128_u32(dw_reader* reader)
{
    return (u32)dw_read_leb128(reader, 32, false);
}

u64 dw_read_leb128_u64(dw_reader* reader)
{
    return dw_read_leb128(reader, 64, false);
}

i32 dw_read_leb128_i32(dw_reader* reader)
{
    return (i32)dw_read_leb128(reader, 32, true);
}

i64 dw_read_leb128_i64(dw_reader* reader)
{
    return (i64)dw_read_leb128(reader, 64, true);
}

u64 dw_read_u64(dw_reader* reader)
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

u32 dw_read_u32(dw_reader* reader)
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

u16 dw_read_u16(dw_reader* reader)
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

u8 dw_read_u8(dw_reader* reader)
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

oc_str8 dw_read_bytes(dw_reader* reader, u64 len)
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

oc_str8 dw_read_cstring(dw_reader* reader)
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

typedef struct dw_sections
{
    oc_str8 abbrev;
    oc_str8 info;
    oc_str8 strOffsets;
    oc_str8 str;
    oc_str8 addr;
    oc_str8 line;
    oc_str8 lineStr;
    oc_str8 loc;
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
    u64 unitCount;
    dw_unit* units;

    dw_line_info* line;

} dw_info;

typedef struct dw_file_entry_elt
{
    oc_list_elt listElt;
    dw_file_entry entry;
} dw_file_entry_elt;

void dw_read_file_entries(oc_arena* arena,
                          dw_reader* reader,
                          u64* entryCount,
                          dw_file_entry** entries,
                          dw_sections* sections,
                          dw_line_program_header* header,
                          bool directories)
{
    u8 formatCount = 0;
    dw_file_entry_format_elt* format = 0;

    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    if(header->version == 5)
    {
        formatCount = dw_read_u8(reader);
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
            format[i].content = dw_read_leb128_u64(reader);
            format[i].form = dw_read_leb128_u64(reader);

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
        *entryCount = dw_read_leb128_u64(reader);
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

    while(dw_reader_has_more(reader))
    {
        if(header->version == 5 && entryIndex >= *entryCount)
        {
            break;
        }
        else if(header->version == 4 && reader->contents.ptr[reader->offset] == 0) //TODO: use reader peek
        {
            reader->offset++;
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
                            entry->path = dw_read_cstring(reader);
                        }
                        break;
                        case DW_FORM_line_strp:
                        case DW_FORM_strp:
                        case DW_FORM_strp_sup:
                        {
                            u64 strp = 0;
                            if(header->addressSize == 4)
                            {
                                strp = dw_read_u32(reader);
                            }
                            else
                            {
                                strp = dw_read_u64(reader);
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

                            dw_reader strReader = {
                                .contents = section,
                            };
                            dw_reader_seek(&strReader, strp);
                            entry->path = dw_read_cstring(&strReader);
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
                            entry->dirIndex = dw_read_u8(reader);
                        }
                        break;
                        case DW_FORM_data2:
                        {
                            entry->dirIndex = dw_read_u16(reader);
                        }
                        break;
                        case DW_FORM_udata:
                        {
                            entry->dirIndex = dw_read_leb128_u64(reader);
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
                            entry->timestamp = dw_read_u32(reader);
                        }
                        break;
                        case DW_FORM_data8:
                        {
                            entry->timestamp = dw_read_u64(reader);
                        }
                        break;
                        case DW_FORM_udata:
                        {
                            entry->timestamp = dw_read_leb128_u64(reader);
                        }
                        break;
                        case DW_FORM_block:
                        {
                            //NOTE(martin): I don't know how to interpret the block, so just warn and skip it
                            u64 len = dw_read_leb128_u64(reader);
                            oc_str8 str = dw_read_bytes(reader, len);

                            printf("warning: unsupported form DW_FORM_block in %s file entry format\n",
                                   dw_get_line_header_entry_format_string(content));
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
                            entry->size = dw_read_u8(reader);
                        }
                        break;
                        case DW_FORM_data2:
                        {
                            entry->size = dw_read_u16(reader);
                        }
                        break;
                        case DW_FORM_data4:
                        {
                            entry->size = dw_read_u32(reader);
                        }
                        break;
                        case DW_FORM_data8:
                        {
                            entry->size = dw_read_u64(reader);
                        }
                        break;
                        case DW_FORM_udata:
                        {
                            entry->size = dw_read_leb128_u64(reader);
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
                    oc_str8 md5 = dw_read_bytes(reader, 16);
                    if(md5.len == 16)
                    {
                        memcpy(entry->md5, md5.ptr, 16);
                    }
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
        oc_list_for_indexed(entryList, it, dw_file_entry_elt, listElt)
        {
            (*entries)[it.index] = it.elt->entry;
        }
    }

    oc_scratch_end(scratch);
}

int dw_read_line_program_header(oc_arena* arena, dw_reader* reader, dw_line_program_header* header, dw_sections* sections)
{
    header->offset = reader->offset;

    u8 dwarfFormat = DW_DWARF32;
    header->unitLength = dw_read_u32(reader);

    if(header->unitLength >= 0xfffffff0)
    {
        dwarfFormat = DW_DWARF64;
        header->unitLength = dw_read_u64(reader);
    }

    header->version = dw_read_u16(reader);

    if(header->version != 5 && header->version != 4)
    {
        printf("error: DWARF version %i not supported\n", header->version);
        exit(-1);
    }

    if(header->version == 5)
    {
        header->addressSize = dw_read_u8(reader);
        if(header->addressSize != 4 && header->addressSize != 8)
        {
            oc_log_error("address size should be 4 or 8\n");
            exit(-1);
        }

        header->segmentSelectorSize = dw_read_u8(reader);
    }
    else
    {
        //NOTE: we set 4 by default as it is the address size on wasm.
        //TODO: THIS SHOULD CHANGE IF WE SWITCH TO WASM64!
        //TODO: allow configuring the "default target address size" from outside
        header->addressSize = 4;
    }

    if(dwarfFormat == DW_DWARF32)
    {
        header->headerLength = dw_read_u32(reader);
    }
    else
    {
        header->headerLength = dw_read_u64(reader);
    }
    u64 headerLengthBase = reader->offset;

    header->minInstructionLength = dw_read_u8(reader);
    header->maxOperationsPerInstruction = dw_read_u8(reader);
    header->defaultIsStmt = dw_read_u8(reader);
    header->lineBase = dw_read_u8(reader);
    header->lineRange = dw_read_u8(reader);
    header->opcodeBase = dw_read_u8(reader);

    //TODO: support non-12 sizes
    for(int i = 0; i < 12; i++)
    {
        header->standardOpcodeLength[i] = dw_read_u8(reader);
    }

    // directories
    dw_read_file_entries(arena, reader, &header->dirEntryCount, &header->dirEntries, sections, header, true);

    // files
    dw_read_file_entries(arena, reader, &header->fileEntryCount, &header->fileEntries, sections, header, false);

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

    dw_reader reader = {
        .contents = sections->line,
    };

    oc_arena_scope scratch = oc_scratch_begin_next(arena);
    oc_list tablesList = { 0 };
    u64 tableCount = 0;

    while(dw_reader_has_more(&reader))
    {
        u64 unitStart = reader.offset;
        dw_line_program_header header = { 0 };

        dw_read_line_program_header(arena, &reader, &header, sections);

        u64 unitLineInfoEnd = unitStart + header.addressSize + header.unitLength;

        if(unitLineInfoEnd > reader.contents.len)
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

        //TODO: use subreader
        while(reader.offset < unitLineInfoEnd)
        {
            u8 opcode = dw_read_u8(&reader);

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
                u32 opcodeSize = dw_read_leb128_u32(&reader);
                opcode = dw_read_u8(&reader);

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
                            address = dw_read_u32(&reader);
                        }
                        else if(header.addressSize == 8)
                        {
                            tombstoneAddress = 0xffffffffffffffff;
                            address = dw_read_u64(&reader);
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
                        u64 disc = dw_read_leb128_u64(&reader);
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
                        u64 opAdvance = dw_read_leb128_u64(&reader);
                        machine.address += header.minInstructionLength * ((machine.opIndex + opAdvance) / header.maxOperationsPerInstruction);
                        machine.opIndex = (machine.opIndex + opAdvance) % header.maxOperationsPerInstruction;
                    }
                    break;
                    case DW_LNS_advance_line:
                    {
                        int64_t lineAdvance = dw_read_leb128_i64(&reader);
                        machine.line += lineAdvance;
                    }
                    break;
                    case DW_LNS_set_file:
                    {
                        u64 file = dw_read_leb128_u64(&reader);
                        machine.file = file;
                    }
                    break;
                    case DW_LNS_set_column:
                    {
                        u64 column = dw_read_leb128_u64(&reader);
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
                        uint16_t inc = dw_read_u16(&reader);
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
                        u64 isa = dw_read_leb128_u64(&reader);
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

        oc_list_for_indexed(rowsList, rowIt, dw_line_entry_elt, listElt)
        {
            table->table.entries[rowIt.index] = rowIt.elt->entry;
        }

        tableCount++;
    }

    lineInfo.tableCount = tableCount;
    lineInfo.tables = oc_arena_push_array(arena, dw_line_table, tableCount);

    oc_list_for_indexed(tablesList, tableIt, dw_line_table_elt, listElt)
    {
        lineInfo.tables[tableIt.index] = tableIt.elt->table;
    }

    oc_scratch_end(scratch);

    return lineInfo;
}

void dw_print_expr(dw_unit* unit, oc_str8 data)
{
    dw_reader reader = {
        .contents = data,
    };

    while(dw_reader_has_more(&reader))
    {
        dw_op op = dw_read_u8(&reader);

        printf("%.*s ", oc_str8_ip(dw_op_get_string(op)));

        switch(op)
        {
            case DW_OP_addr:
            {
                reader.offset += 4; //TODO: size is target specific
            }
            break;
            case DW_OP_deref:
            {
            }
            break;
            case DW_OP_const1u:
            {
                reader.offset += 1;
            }
            break;
            case DW_OP_const1s:
            {
                reader.offset += 1;
            }
            break;
            case DW_OP_const2u:
            {
                reader.offset += 2;
            }
            break;
            case DW_OP_const2s:
            {
                reader.offset += 2;
            }
            break;
            case DW_OP_const4u:
            {
                reader.offset += 4;
            }
            break;
            case DW_OP_const4s:
            {
                reader.offset += 4;
            }
            break;
            case DW_OP_const8u:
            {
                reader.offset += 8;
            }
            break;
            case DW_OP_const8s:
            {
                reader.offset += 8;
            }
            break;
            case DW_OP_constu:
            {
                u64 opd = dw_read_leb128_u64(&reader);
            }
            break;
            case DW_OP_consts:
            {
                i64 opd = dw_read_leb128_i64(&reader);
            }
            break;
            case DW_OP_dup:
            {
            }
            break;
            case DW_OP_drop:
            {
            }
            break;
            case DW_OP_over:
            {
            }
            break;
            case DW_OP_pick:
            {
                reader.offset += 1;
            }
            break;
            case DW_OP_swap:
            {
            }
            break;
            case DW_OP_rot:
            {
            }
            break;
            case DW_OP_xderef:
            {
            }
            break;
            case DW_OP_abs:
            {
            }
            break;
            case DW_OP_and:
            {
            }
            break;
            case DW_OP_div:
            {
            }
            break;
            case DW_OP_minus:
            {
            }
            break;
            case DW_OP_mod:
            {
            }
            break;
            case DW_OP_mul:
            {
            }
            break;
            case DW_OP_neg:
            {
            }
            break;
            case DW_OP_not:
            {
            }
            break;
            case DW_OP_or:
            {
            }
            break;
            case DW_OP_plus:
            {
            }
            break;
            case DW_OP_plus_uconst:
            {
                u64 opd = dw_read_leb128_u64(&reader);
            }
            break;
            case DW_OP_shl:
            {
            }
            break;
            case DW_OP_shr:
            {
            }
            break;
            case DW_OP_shra:
            {
            }
            break;
            case DW_OP_xor:
            {
            }
            break;
            case DW_OP_bra:
            {
                reader.offset += 2;
            }
            break;
            case DW_OP_eq:
            {
            }
            break;
            case DW_OP_ge:
            {
            }
            break;
            case DW_OP_gt:
            {
            }
            break;
            case DW_OP_le:
            {
            }
            break;
            case DW_OP_lt:
            {
            }
            break;
            case DW_OP_ne:
            {
            }
            break;
            case DW_OP_skip:
            {
                reader.offset += 2;
            }
            break;
            case DW_OP_lit0:
            case DW_OP_lit1:
            case DW_OP_lit2:
            case DW_OP_lit3:
            case DW_OP_lit4:
            case DW_OP_lit5:
            case DW_OP_lit6:
            case DW_OP_lit7:
            case DW_OP_lit8:
            case DW_OP_lit9:
            case DW_OP_lit10:
            case DW_OP_lit11:
            case DW_OP_lit12:
            case DW_OP_lit13:
            case DW_OP_lit14:
            case DW_OP_lit15:
            case DW_OP_lit16:
            case DW_OP_lit17:
            case DW_OP_lit18:
            case DW_OP_lit19:
            case DW_OP_lit20:
            case DW_OP_lit21:
            case DW_OP_lit22:
            case DW_OP_lit23:
            case DW_OP_lit24:
            case DW_OP_lit25:
            case DW_OP_lit26:
            case DW_OP_lit27:
            case DW_OP_lit28:
            case DW_OP_lit29:
            case DW_OP_lit30:
            case DW_OP_lit31:
            {
            }
            break;

            case DW_OP_reg0:
            case DW_OP_reg1:
            case DW_OP_reg2:
            case DW_OP_reg3:
            case DW_OP_reg4:
            case DW_OP_reg5:
            case DW_OP_reg6:
            case DW_OP_reg7:
            case DW_OP_reg8:
            case DW_OP_reg9:
            case DW_OP_reg10:
            case DW_OP_reg11:
            case DW_OP_reg12:
            case DW_OP_reg13:
            case DW_OP_reg14:
            case DW_OP_reg15:
            case DW_OP_reg16:
            case DW_OP_reg17:
            case DW_OP_reg18:
            case DW_OP_reg19:
            case DW_OP_reg20:
            case DW_OP_reg21:
            case DW_OP_reg22:
            case DW_OP_reg23:
            case DW_OP_reg24:
            case DW_OP_reg25:
            case DW_OP_reg26:
            case DW_OP_reg27:
            case DW_OP_reg28:
            case DW_OP_reg29:
            case DW_OP_reg30:
            case DW_OP_reg31:
            {
            }
            break;

            case DW_OP_breg0:
            case DW_OP_breg1:
            case DW_OP_breg2:
            case DW_OP_breg3:
            case DW_OP_breg4:
            case DW_OP_breg5:
            case DW_OP_breg6:
            case DW_OP_breg7:
            case DW_OP_breg8:
            case DW_OP_breg9:
            case DW_OP_breg10:
            case DW_OP_breg11:
            case DW_OP_breg12:
            case DW_OP_breg13:
            case DW_OP_breg14:
            case DW_OP_breg15:
            case DW_OP_breg16:
            case DW_OP_breg17:
            case DW_OP_breg18:
            case DW_OP_breg19:
            case DW_OP_breg20:
            case DW_OP_breg21:
            case DW_OP_breg22:
            case DW_OP_breg23:
            case DW_OP_breg24:
            case DW_OP_breg25:
            case DW_OP_breg26:
            case DW_OP_breg27:
            case DW_OP_breg28:
            case DW_OP_breg29:
            case DW_OP_breg30:
            case DW_OP_breg31:
            {
                i64 opd = dw_read_leb128_i64(&reader);
            }
            break;

            case DW_OP_regx:
            {
                u64 opd = dw_read_leb128_u64(&reader);
            }
            break;
            case DW_OP_fbreg:
            {
                i64 opd = dw_read_leb128_i64(&reader);
            }
            break;
            case DW_OP_bregx:
            {
                u64 reg = dw_read_leb128_u64(&reader);
                i64 offset = dw_read_leb128_i64(&reader);
            }
            break;
            case DW_OP_piece:
            {
                u64 opd = dw_read_leb128_u64(&reader);
            }
            break;
            case DW_OP_deref_size:
            {
                reader.offset += 1;
            }
            break;
            case DW_OP_xderef_size:
            {
                reader.offset += 1;
            }
            break;
            case DW_OP_nop:
            {
            }
            break;
            case DW_OP_push_object_address:
            {
            }
            break;
            case DW_OP_call2:
            {
                reader.offset += 2;
            }
            break;
            case DW_OP_call4:
            {
                reader.offset += 4;
            }
            break;
            case DW_OP_call_ref:
            {
                reader.offset += (unit->format == DW_DWARF32) ? 4 : 8;
            }
            break;
            case DW_OP_form_tls_address:
            {
            }
            break;
            case DW_OP_call_frame_cfa:
            {
            }
            break;
            case DW_OP_bit_piece:
            {
                u64 size = dw_read_leb128_u64(&reader);
                u64 offset = dw_read_leb128_u64(&reader);
            }
            break;
            case DW_OP_implicit_value:
            {
                u64 size = dw_read_leb128_u64(&reader);

                reader.offset += size;
            }
            break;
            case DW_OP_stack_value:
            {
            }
            break;
            case DW_OP_implicit_pointer:
            {
                reader.offset += (unit->format == DW_DWARF32) ? 4 : 8;
                u64 offset = dw_read_leb128_u64(&reader);
            }
            break;
            case DW_OP_addrx:
            {
                u64 opd = dw_read_leb128_u64(&reader);
            }
            break;
            case DW_OP_constx:
            {
                u64 opd = dw_read_leb128_u64(&reader);
            }
            break;
            case DW_OP_entry_value:
            {
                u64 size = dw_read_leb128_u64(&reader);

                reader.offset += size;
            }
            break;
            case DW_OP_const_type:
            {
                u64 offset = dw_read_leb128_u64(&reader);
                u8 size = dw_read_u8(&reader);

                reader.offset += size;
            }
            break;
            case DW_OP_regval_type:
            {
                u64 reg = dw_read_leb128_u64(&reader);
                u64 offset = dw_read_leb128_u64(&reader);
            }
            break;
            case DW_OP_deref_type:
            {
                reader.offset += 1;
                u64 offset = dw_read_leb128_u64(&reader);
            }
            break;
            case DW_OP_xderef_type:
            {
                reader.offset += 1;
                u64 offset = dw_read_leb128_u64(&reader);
            }
            break;
            case DW_OP_convert:
            {
                u64 offset = dw_read_leb128_u64(&reader);
            }
            break;
            case DW_OP_reinterpret:
            {
                u64 offset = dw_read_leb128_u64(&reader);
            }
            break;

            case DW_OP_WASM_location:
            {
                u8 code = dw_read_u8(&reader);
                if(code == 3)
                {
                    reader.offset += 4;
                }
                else
                {
                    i64 i = dw_read_leb128_i64(&reader);
                }
            }
            break;

            default:
            {
            }
            break;
        }
    }
}

void dw_print_indent(u64 indent)
{
    for(u64 i = 0; i < indent; i++)
    {
        printf("  ");
    }
}

void dw_print_loc(dw_unit* unit, dw_loc loc, int indent)
{
    if(loc.single)
    {
        dw_print_expr(unit, loc.entries[0].desc);
    }
    else
    {
        u64 baseOffset = 0; //TODO: should be unit's base offset
        for(u64 i = 0; i < loc.entryCount; i++)
        {
            if(i)
            {
                printf("            "); // space of address
                dw_print_indent(indent);
            }
            dw_loc_entry* entry = &loc.entries[i];
            if(entry->start == 0xffffffffffffffff)
            {
                printf("0x%08llx:", entry->end);
                baseOffset = entry->end;
            }
            else
            {
                printf("[0x%08llx, 0x%08llx): ", entry->start + baseOffset, entry->end + baseOffset);
                dw_print_expr(unit, entry->desc);
            }
            if(i < loc.entryCount - 1)
            {
                printf("\n");
            }
        }
    }
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

/*
typedef struct dw_loclist_table
{
    u64 unitLength;
    dw_format format;
    u16 version;
    u8 addressSize;
    u8 segmentSelectorSize;
    u32 offsetEntryCount;
    u64* offsets;

    //...

} dw_loclist_table;

dw_loclist_table* dw_parse_loclist_table(oc_arean* arena, oc_str8 section)
{
    dw_loclist_table* table = oc_arena_push_type(arena, dw_loclist_table);
    memset(table, 0, sizeof(dw_loclist_table));

    u64 offset = 0;
    u32 length32 = 0;
    u8 dwarfFormat = DW_DWARF32;

    offset += dw_read_u32(&length32, section.ptr, section.len, offset);

    if(length32 >= 0xfffffff0)
    {
        offset += dw_read_u64(&table->unitLength, section.ptr, section.len, offset);
        dwarfFormat = DW_DWARF64;
    }
    else
    {
        table->unitLength = length32;
    }

    //TODO

    return table;
}
*/

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

    dw_reader reader = {
        .contents = section,
    };
    dw_reader_seek(&reader, offset);

    while(dw_reader_has_more(&reader))
    {
        u64 abbrevCode = dw_read_leb128_u64(&reader);
        if(abbrevCode == 0)
        {
            //NOTE: null entry, this is the end of the table
            break;
        }

        dw_abbrev_entry_elt* entryElt = oc_arena_push_type(scratch.arena, dw_abbrev_entry_elt);
        dw_abbrev_entry* entry = &entryElt->entry;

        entry->code = abbrevCode;

        entry->tag = dw_read_leb128_u32(&reader);
        entry->hasChildren = dw_read_u8(&reader);

        //NOTE: parse attributes
        oc_list attributes = { 0 };
        entry->attrCount = 0;

        while(dw_reader_has_more(&reader))
        {
            u32 attrName = dw_read_leb128_u32(&reader);
            u32 attrForm = dw_read_leb128_u32(&reader);

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
                attr->implicitConst = dw_read_leb128_i64(&reader);
            }

            oc_list_push_back(&attributes, &attrElt->listElt);
            entry->attrCount++;
        }

        //NOTE: copy attributes to fixed array
        entry->attributes = oc_arena_push_array(arena, dw_abbrev_attr, entry->attrCount);

        oc_list_for_indexed(attributes, attrIt, dw_abbrev_attr_elt, listElt)
        {
            entry->attributes[attrIt.index] = attrIt.elt->attr;
        }

        oc_list_push_back(&entries, &entryElt->listElt);
        table->entryCount++;
    }

    //NOTE: copy entries to fixed array
    table->entries = oc_arena_push_array(arena, dw_abbrev_entry, table->entryCount);
    u64 entryIndex = 0;

    oc_list_for_indexed(entries, entryIt, dw_abbrev_entry_elt, listElt)
    {
        table->entries[entryIt.index] = entryIt.elt->entry;
    }

    oc_scratch_end(scratch);

    return table;
}

dw_loc dw_parse_loclist(oc_arena* arena, dw_unit* unit, oc_str8 section, u64 offset)
{
    //TODO: parse from debug loclist.
    // in v4, offset is an offset from the beginning of the debug_loc section
    // in v5, offset in an offset from the beginning of the debug_loclists section
    dw_loc loc = { 0 };

    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    dw_reader reader = {
        .contents = section,
    };
    dw_reader_seek(&reader, offset);

    if(unit->version == 4)
    {
        typedef struct dw_loc_entry_elt
        {
            oc_list_elt listElt;
            dw_loc_entry entry;
        } dw_loc_entry_elt;

        oc_list list = { 0 };

        while(offset < section.len)
        {
            u64 start = 0;
            u64 end = 0;
            oc_str8 desc = { 0 };

            if(unit->addressSize == 4)
            {
                start = dw_read_u32(&reader);
                end = dw_read_u32(&reader);
                if(start == 0xffffffff)
                {
                    start = 0xffffffffffffffff;
                }
            }
            else if(unit->addressSize == 8)
            {
                start = dw_read_u64(&reader);
                end = dw_read_u64(&reader);
            }
            else
            {
                OC_ASSERT(0);
            }

            if(start == 0 && end == 0)
            {
                //NOTE end of list entry
                break;
            }
            else if(start != 0xffffffffffffffff)
            {
                //NOTE normal entry
                u16 length = dw_read_u16(&reader);
                desc = dw_read_bytes(&reader, length);
            }

            dw_loc_entry_elt* elt = oc_arena_push_type(scratch.arena, dw_loc_entry_elt);
            elt->entry = (dw_loc_entry){
                .start = start,
                .end = end,
                .desc = desc,
            };
            oc_list_push_back(&list, &elt->listElt);
            loc.entryCount++;
        }

        loc.entries = oc_arena_push_array(arena, dw_loc_entry, loc.entryCount);
        oc_list_for_indexed(list, it, dw_loc_entry_elt, listElt)
        {
            loc.entries[it.index] = it.elt->entry;
        }
    }
    else
    {
        //TODO
        OC_ASSERT(0);
    }

    oc_scratch_end(scratch);

    return loc;
}

void dw_parse_form_value(oc_arena* arena, dw_reader* reader, dw_attr* res, dw_unit* unit, dw_sections* sections, dw_attr_name name, dw_form form)
{
    switch(form)
    {
        //-----------------------
        // address class
        //-----------------------
        case DW_FORM_addr:
        {
            if(unit->addressSize == 4)
            {
                res->valU64 = dw_read_u32(reader);
            }
            else
            {
                res->valU64 = dw_read_u64(reader);
            }
        }
        break;
        case DW_FORM_addrx1:
        {
            u8 indOffset = dw_read_u8(reader);
            //TODO: get address from debug_addr section
        }
        break;
        case DW_FORM_addrx2:
        {
            u16 indOffset = dw_read_u16(reader);
            //TODO: get address from debug_addr section
        }
        break;
        case DW_FORM_addrx3:
        {
            u64 indOffset = 0;
            u8* indOffset8 = (u8*)&indOffset;
            indOffset8[0] = dw_read_u8(reader);
            indOffset8[1] = dw_read_u8(reader);
            indOffset8[2] = dw_read_u8(reader);
            //TODO: get address from debug_addr section
        }
        break;
        case DW_FORM_addrx4:
        {
            u32 indOffset = dw_read_u32(reader);
            //TODO: get address from debug_addr section
        }
        break;
        case DW_FORM_addrx:
        {
            u64 indOffset = dw_read_leb128_u64(reader);
            //TODO: get address from debug_addr section
        }
        break;

        //-----------------------
        // block class
        //-----------------------
        case DW_FORM_block1:
        {
            u8 len = dw_read_u8(reader);
            res->string = dw_read_bytes(reader, len);
        }
        break;
        case DW_FORM_block2:
        {
            u16 len = dw_read_u16(reader);
            res->string = dw_read_bytes(reader, len);
        }
        break;
        case DW_FORM_block4:
        {
            u32 len = dw_read_u32(reader);
            res->string = dw_read_bytes(reader, len);
        }
        break;
        case DW_FORM_block:
        {
            u64 len = dw_read_leb128_u64(reader);
            res->string = dw_read_bytes(reader, len);
        }
        break;

        //-----------------------
        // constant class
        //-----------------------
        case DW_FORM_data1:
        case DW_FORM_data2:
        case DW_FORM_data4:
        case DW_FORM_data8:
        case DW_FORM_data16:
        {
            switch(form)
            {
                case DW_FORM_data1:
                {
                    res->valU64 = dw_read_u8(reader);
                }
                break;
                case DW_FORM_data2:
                {
                    res->valU64 = dw_read_u16(reader);
                }
                break;
                case DW_FORM_data4:
                {
                    res->valU64 = dw_read_u32(reader);
                }
                break;
                case DW_FORM_data8:
                {
                    res->valU64 = dw_read_u64(reader);
                }
                break;

                case DW_FORM_data16:
                {
                    oc_log_error("DW_FORM_data16 unsupported for now\n");
                }
                break;

                default:
                    OC_ASSERT(0, "unreachable");
                    break;
            }
        }
        break;
        case DW_FORM_sdata:
        {
            res->valI64 = dw_read_leb128_i64(reader);
        }
        break;
        case DW_FORM_udata:
        {
            res->valU64 = dw_read_leb128_u64(reader);
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
            u64 len = dw_read_leb128_u64(reader);
            oc_str8 expr = dw_read_bytes(reader, len);

            res->loc = (dw_loc){
                .single = true,
                .entryCount = 1,
                .entries = oc_arena_push_type(arena, dw_loc_entry),
            };
            res->loc.entries[0] = (dw_loc_entry){
                .desc = expr,
            };
        }
        break;

        //-----------------------
        // flag class
        //-----------------------
        case DW_FORM_flag:
        {
            res->valU8 = dw_read_u8(reader);
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
            u64 listOffset = dw_read_leb128_u64(reader);
            //TODO: extract list of entries form debug_loclists section
        }
        break;

        //-----------------------
        // rnglist class
        //-----------------------
        case DW_FORM_rnglistx:
        {
            u64 rngListIndex = dw_read_leb128_u64(reader);
            //TODO: extract rnglist of entries form debug_rnglists section
        }
        break;

        //-----------------------
        // reference class
        //-----------------------
        //NOTE: we store refs as u64 offset relative to the start of the debug section
        case DW_FORM_ref1:
        {
            res->valU64 = unit->start + dw_read_u8(reader);
        }
        break;
        case DW_FORM_ref2:
        {
            res->valU64 = unit->start + dw_read_u16(reader);
        }
        break;
        case DW_FORM_ref4:
        {
            res->valU64 = unit->start + dw_read_u32(reader);
        }
        break;
        case DW_FORM_ref8:
        {
            res->valU64 = unit->start + dw_read_u64(reader);
        }
        break;
        case DW_FORM_ref_udata:
        {
            res->valU64 = unit->start + dw_read_leb128_u64(reader);
        }
        break;

        case DW_FORM_ref_addr:
        {
            if(unit->format == DW_DWARF32)
            {
                res->valU64 = dw_read_u32(reader);
            }
            else
            {
                res->valU64 = dw_read_u64(reader);
            }
        }
        break;

        case DW_FORM_ref_sig8:
        {
            u64 sig = dw_read_u64(reader);
            //TODO: store the signature and find corresponding def later?
        }
        break;

        case DW_FORM_ref_sup4:
        {
            u32 supOffset = dw_read_u32(reader);
            //TODO: support supplementary object files??
        }
        break;

        case DW_FORM_ref_sup8:
        {
            u64 supOffset = dw_read_u64(reader);
            //TODO: support supplementary object files??
        }
        break;

        //-----------------------
        // string class
        //-----------------------
        case DW_FORM_string:
        {
            res->string = dw_read_cstring(reader);
        }
        break;

        //TODO extract strings from string section
        case DW_FORM_strp:
        case DW_FORM_line_strp:
        case DW_FORM_strp_sup:
        {
            u64 strOffset = 0;
            if(unit->format == DW_DWARF32)
            {
                strOffset = dw_read_u32(reader);
            }
            else
            {
                strOffset = dw_read_u64(reader);
            }

            oc_str8* strSection = 0;
            if(form == DW_FORM_strp)
            {
                strSection = &sections->str;
            }
            else if(form == DW_FORM_line_strp)
            {
                strSection = &sections->lineStr;
            }
            else
            {
                oc_log_warning("unsupported form DW_FORM_str_sup (string from supplementary object file)\n");
            }

            if(strSection)
            {
                dw_reader strReader = {
                    .contents = *strSection,
                };
                dw_reader_seek(&strReader, strOffset);
                res->string = dw_read_cstring(&strReader);
            }
        }
        break;

        case DW_FORM_strx1:
        case DW_FORM_strx2:
        case DW_FORM_strx3:
        case DW_FORM_strx4:
        case DW_FORM_strx:
        {
            u64 index = 0;
            switch(form)
            {
                case DW_FORM_strx1:
                {
                    index = dw_read_u8(reader);
                }
                break;
                case DW_FORM_strx2:
                {
                    index = dw_read_u16(reader);
                }
                break;
                case DW_FORM_strx3:
                {
                    u16 index16 = dw_read_u16(reader);
                    u8 index8 = dw_read_u8(reader);
                    memcpy(&index, &index16, sizeof(u16));
                    memcpy(((char*)&index) + sizeof(u16), &index8, sizeof(u8));
                }
                break;
                case DW_FORM_strx4:
                {
                    index = dw_read_u32(reader);
                }
                break;
                case DW_FORM_strx:
                {
                    index = dw_read_leb128_u64(reader);
                }
                break;

                default:
                    OC_ASSERT(0, "unreachable");
            }

            u64 strOffset = 0;

            //NOTE: compute start of offsets table
            //TODO: take into account string offset base...

            dw_reader strOffsetReader = {
                .contents = sections->strOffsets,
            };

            u32 strOffsetLengthSize = 4;
            u64 strOffsetLength = dw_read_u32(&strOffsetReader);

            if(strOffsetLength >= 0xfffffff0)
            {
                strOffsetLengthSize = 8;
                strOffsetLength = dw_read_u64(&strOffsetReader);
            }

            u16 strOffsetVersion = dw_read_u16(&strOffsetReader);
            u16 padding = dw_read_u16(&strOffsetReader);

            strOffsetReader = dw_reader_subreader(&strOffsetReader, strOffsetReader.contents.len - strOffsetReader.offset);

            if(strOffsetLengthSize == 4)
            {
                dw_reader_seek(&strOffsetReader, index * 4);
                strOffset = dw_read_u32(&strOffsetReader);
            }
            else
            {
                dw_reader_seek(&strOffsetReader, index * 8);
                strOffset = dw_read_u64(&strOffsetReader);
            }

            dw_reader strReader = {
                .contents = sections->str,
            };
            res->string = dw_read_cstring(&strReader);
        }
        break;

        //-----------------------
        // sec_offset (belongs to multiple classes)
        //-----------------------
        //TODO use sec offset according to class
        case DW_FORM_sec_offset:
        {
            u64 addrOffset = 0;
            if(unit->format == DW_DWARF32)
            {
                addrOffset = dw_read_u32(reader);
            }
            else
            {
                addrOffset = dw_read_u64(reader);
            }

            switch(name)
            {
                case DW_AT_location:
                {
                    res->loc = dw_parse_loclist(arena, unit, sections->loc, addrOffset);
                }
                break;

                default:
                    //TODO
                    break;
            }
        }
        break;

        //-----------------------
        // indirect is a special case, see p207 of Dwarf5 spec.
        //-----------------------
        case DW_FORM_indirect:
        {
            u64 indForm = dw_read_leb128_u64(reader);
            dw_parse_form_value(arena, reader, res, unit, sections, name, indForm);
        }
        break;

        default:
            //TODO
            oc_log_error("unsupported form %s\n", dw_get_form_string(form));
            exit(-1);
            break;
    }
}

/*
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
    return (offset - startOffset);
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
    return (offset - startOffset);
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

    return (offset - startOffset);
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

    return (offset - startOffset);
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
    return (offset - startOffset);
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
    return (offset - startOffset);
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
    return (offset - startOffset);
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
    return (offset - startOffset);
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
    return (offset - startOffset);
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
    return (offset - startOffset);
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
    return (offset - startOffset);
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
    return (offset - startOffset);
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

    return (offset - startOffset);
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

    return (offset - startOffset);
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
    return (offset - startOffset);
}

u64 dw_parse_attr(oc_arena* arena, dw_attr* res, u32 addressSize, char* data, u64 fileSize, u64 offset)
{
    u64 startOffset = offset;

    dw_form form = res->abbrev->form;


    return (offset - startOffset);
}
*/

void dw_parse_die(oc_arena* arena, dw_reader* reader, dw_die* die, dw_sections* sections, dw_unit* unit)
{
    //NOTE: find abbreviation
    die->abbrevCode = dw_read_leb128_u64(reader);
    if(die->abbrevCode == 0)
    {
        goto end;
    }

    die->abbrev = 0;
    for(u64 abbrevIndex = 0; abbrevIndex < unit->abbrev->entryCount; abbrevIndex++)
    {
        if(unit->abbrev->entries[abbrevIndex].code == die->abbrevCode)
        {
            die->abbrev = &unit->abbrev->entries[abbrevIndex];
            break;
        }
    }

    if(!die->abbrev)
    {
        oc_log_error("Couldn't find abbrev code %llu\n", die->abbrevCode);
        exit(-1);
    }

    die->attributes = oc_arena_push_array(arena, dw_attr, die->abbrev->attrCount);
    memset(die->attributes, 0, sizeof(dw_attr) * die->abbrev->attrCount);

    for(u32 attrIndex = 0; attrIndex < die->abbrev->attrCount; attrIndex++)
    {
        dw_attr* attr = &die->attributes[attrIndex];
        attr->abbrev = &die->abbrev->attributes[attrIndex];

        dw_parse_form_value(arena, reader, attr, unit, sections, attr->abbrev->name, attr->abbrev->form);

        //TODO: some forms need interpretation based on the attr name,
        //      review how we parse forms w/ context specific meaning
    }

end:
    return;
}

dw_die* dw_die_next(dw_die* root, dw_die* die)
{
    if(!oc_list_empty(die->children))
    {
        die = oc_list_first_entry(die->children, dw_die, parentElt);
    }
    else if(die->parentElt.next)
    {
        die = oc_list_entry(die->parentElt.next, dw_die, parentElt);
    }
    else if(die->parent && die->parent != root && die->parent->parentElt.next)
    {
        die = oc_list_entry(die->parent->parentElt.next, dw_die, parentElt);
    }
    else
    {
        die = 0;
    }

    return die;
}

dw_die* dw_die_find_next_with_tag(dw_die* root, dw_die* start, dw_tag tag)
{
    dw_die* die = start;

    while(die)
    {
        if(die != start && die->abbrev && die->abbrev->tag == tag)
        {
            return die;
        }

        if(!oc_list_empty(die->children))
        {
            die = oc_list_first_entry(die->children, dw_die, parentElt);
        }
        else if(die->parentElt.next)
        {
            die = oc_list_entry(die->parentElt.next, dw_die, parentElt);
        }
        else if(die->parent && die->parent != root && die->parent->parentElt.next)
        {
            die = oc_list_entry(die->parent->parentElt.next, dw_die, parentElt);
        }
        else
        {
            break;
        }
    }
    return 0;
}

void dw_print_die(dw_unit* unit, dw_die* die, u32 indent)
{
    printf("0x%08llx: ", die->start);
    dw_print_indent(indent);

    if(die->abbrevCode == 0)
    {
        printf("NULL\n\n");
    }
    else
    {
        printf("%s\n", dw_get_tag_string(die->abbrev->tag));

        for(u64 attrIndex = 0; attrIndex < die->abbrev->attrCount; attrIndex++)
        {
            dw_attr* attr = &die->attributes[attrIndex];

            dw_print_indent(indent + 7);
            printf("%s", dw_get_attr_name_string(attr->abbrev->name));
            printf("\t[%s]", dw_get_form_string(attr->abbrev->form));

            if(attr->abbrev->form == DW_FORM_string
               || attr->abbrev->form == DW_FORM_strp
               || attr->abbrev->form == DW_FORM_line_strp
               || attr->abbrev->form == DW_FORM_strx1
               || attr->abbrev->form == DW_FORM_strx2
               || attr->abbrev->form == DW_FORM_strx3
               || attr->abbrev->form == DW_FORM_strx4
               || attr->abbrev->form == DW_FORM_strx)
            {
                printf("\t(\"%.*s\")", oc_str8_ip(attr->string));
            }
            else if(attr->abbrev->name == DW_AT_location || attr->abbrev->name == DW_AT_frame_base)
            {
                printf("\t(");
                dw_print_loc(unit, attr->loc, indent + 2);
                printf(")");
            }
            printf("\n");
        }

        printf("\n");
        oc_list_for(die->children, child, dw_die, parentElt)
        {
            dw_print_die(unit, child, indent + 1);
        }
    }
}

void dw_print_debug_info(dw_info* info)
{
    for(u64 unitIndex = 0; unitIndex < info->unitCount; unitIndex++)
    {
        dw_unit* unit = &info->units[unitIndex];

        printf("0x%016llx\n", unit->start);
        printf("    type: %s\n", dw_get_cu_type_string(unit->type));
        printf("    version: %i\n", unit->version);

        dw_print_die(unit, unit->rootDie, 0);
    }
}

void dw_parse_info(oc_arena* arena, dw_sections* sections, dw_info* info)
{
    dw_reader reader = {
        .contents = sections->info,
    };

    typedef struct dw_unit_elt
    {
        oc_list_elt listElt;
        dw_unit unit;
    } dw_unit_elt;

    oc_list units = { 0 };
    info->unitCount = 0;

    while(dw_reader_has_more(&reader))
    {
        dw_unit_elt* unitElt = oc_arena_push_type(arena, dw_unit_elt);
        memset(unitElt, 0, sizeof(dw_unit_elt));
        dw_unit* unit = &unitElt->unit;

        unit->start = reader.offset;

        unit->format = DW_DWARF32;
        unit->initialLength = dw_read_u32(&reader);

        if(unit->initialLength >= 0xfffffff0)
        {
            unit->format = DW_DWARF64;
            unit->initialLength = dw_read_u64(&reader);
        }

        unit->version = dw_read_u16(&reader);

        if(unit->version >= 5)
        {
            unit->type = dw_read_u8(&reader);
        }
        else
        {
            unit->type = DW_UT_compile;
        }

        if(unit->type == DW_UT_compile || unit->type == DW_UT_partial)
        {
            u8 addressSize;
            u64 abbrevOffset = 0;

            if(unit->version >= 5)
            {
                //NOTE: debug abbrev offset and address size are ordered differently in dwarf v4 and v5, because.. why not?
                unit->addressSize = dw_read_u8(&reader);

                if(unit->format == DW_DWARF32)
                {
                    unit->abbrevOffset = dw_read_u32(&reader);
                }
                else
                {
                    unit->abbrevOffset = dw_read_u64(&reader);
                }
            }
            else
            {
                if(unit->format == DW_DWARF32)
                {
                    unit->abbrevOffset = dw_read_u32(&reader);
                }
                else
                {
                    unit->abbrevOffset = dw_read_u64(&reader);
                }
                unit->addressSize = dw_read_u8(&reader);
            }

            oc_arena_scope scratch = oc_scratch_begin_next(arena);

            unit->abbrev = dw_load_abbrev_table(scratch.arena, sections->abbrev, unit->abbrevOffset);

            //NOTE(martin): read DIEs for unit
            dw_die* parentDIE = 0;
            do
            {
                dw_die* die = oc_arena_push_type(arena, dw_die);
                die->start = reader.offset;
                if(parentDIE)
                {
                    die->parent = parentDIE;
                    oc_list_push_back(&parentDIE->children, &die->parentElt);
                }
                else
                {
                    //NOTE: This is the root DIE
                    unit->rootDie = die;
                }

                dw_parse_die(scratch.arena, &reader, die, sections, unit);

                if(die->abbrevCode == 0)
                {
                    //NOTE: this is a null entry that marks the end of a siblings chain
                    parentDIE = parentDIE->parent;
                }
                else
                {
                    if(die->abbrev->hasChildren)
                    {
                        parentDIE = die;
                    }
                }
            }
            while(parentDIE != 0);
        }
        else
        {
            oc_log_warning("first DIE is not a compile unit DIE\n");
        }

        //NOTE: Add unit to unit list
        oc_list_push_back(&units, &unitElt->listElt);
        info->unitCount++;

        // skip to next unit
        dw_reader_seek(&reader, unit->start + unit->initialLength + (unit->format == DW_DWARF32 ? 4 : 8));
    }
    //NOTE: copy units in fixed array
    info->units = oc_arena_push_array(arena, dw_unit, info->unitCount);
    oc_list_for_indexed(units, unitIt, dw_unit_elt, listElt)
    {
        info->units[unitIt.index] = unitIt.elt->unit;
    }
}

dw_attr* dw_die_get_attr(dw_die* die, dw_attr_name name)
{
    dw_attr* res = 0;
    for(u64 attrIndex = 0; attrIndex < die->abbrev->attrCount; attrIndex++)
    {
        dw_attr* attr = &die->attributes[attrIndex];
        if(attr->abbrev->name == name)
        {
            res = attr;
            break;
        }
    }
    return res;
}

/*
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

                if(format == DW_DWARF32)
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
                if(format == DW_DWARF32)
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

                printf("    0x%016llx: %s\n", offset, dw_get_attr_name_string(abbrevAttr->name));
                printf("        form: %s\n", dw_get_form_string(abbrevAttr->form));
                dw_attr attr = { 0 };
                attr.abbrev = abbrevAttr;
                //offset += dw_parse_attr(scratch.arena, &attr, addressSize, contents.ptr, contents.len, offset);
                offset += dw_parse_form_value(&attr, &sections, abbrevAttr->form, addressSize, lengthSize, contents.ptr, contents.len, offset);

                if(abbrevAttr->form == DW_FORM_string
                   || abbrevAttr->form == DW_FORM_strp
                   || abbrevAttr->form == DW_FORM_line_strp
                   || abbrevAttr->form == DW_FORM_strx1
                   || abbrevAttr->form == DW_FORM_strx2
                   || abbrevAttr->form == DW_FORM_strx3
                   || abbrevAttr->form == DW_FORM_strx4
                   || abbrevAttr->form == DW_FORM_strx)
                {
                    printf("        value: %.*s\n", oc_str8_ip(attr.string));
                }
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
*/

void dw_dump_abbrev_table(oc_str8 contents)
{
    dw_reader reader = {
        .contents = contents,
    };

    while(dw_reader_has_more(&reader))
    {
        u32 abbrevCode = 0;
        u32 abbrevTag = 0;
        u8 children = 0;

        abbrevCode = dw_read_leb128_u32(&reader);
        if(abbrevCode == 0)
        {
            //null entry
            continue;
        }

        abbrevTag = dw_read_leb128_u32(&reader);
        children = dw_read_u8(&reader);

        printf("0x%08x %s %s\n", abbrevCode, dw_get_tag_string(abbrevTag), children == 0 ? "DW_CHILDREN_no" : "DW_CHILDREN_yes");

        // attributes
        u32 attrName = 0;
        u32 attrForm = 0;
        do
        {
            attrName = dw_read_leb128_u32(&reader);
            attrForm = dw_read_leb128_u32(&reader);

            if(attrName != 0 && attrForm != 0)
            {
                printf("  %s %s\n", dw_get_attr_name_string(attrName), dw_get_form_string(attrForm));
            }
        }
        while((attrName != 0 || attrForm != 0)
              && dw_reader_has_more(&reader));
    }
}
