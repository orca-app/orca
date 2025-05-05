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

//------------------------------------------------------------------------
// Dwarf enums
//------------------------------------------------------------------------

#define X_ENUM(name, code, ...) name = code,
#define X_NAME_CASE(name, code, ...) \
    case name:                       \
        res = #name;                 \
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

static inline const char* dw_get_tag_string(u32 tag)
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

#define DW_ATTR_LIST(X)                                                \
    X(DW_AT_sibling, 0x01, reference)                                  \
    X(DW_AT_location, 0x02, exprloc, loclistptr)                       \
    X(DW_AT_name, 0x03, string)                                        \
    X(DW_AT_ordering, 0x09, constant)                                  \
    X(DW_AT_byte_size, 0x0b, constant, exprloc, reference)             \
    X(DW_AT_bit_offset, 0x0c, constant, exprloc, reference)            \
    X(DW_AT_bit_size, 0x0d, constant, exprloc, reference)              \
    X(DW_AT_stmt_list, 0x10, lineptr)                                  \
    X(DW_AT_low_pc, 0x11, address)                                     \
    X(DW_AT_high_pc, 0x12, address, constant)                          \
    X(DW_AT_language, 0x13, constant)                                  \
    X(DW_AT_discr, 0x15, reference)                                    \
    X(DW_AT_discr_value, 0x16, constant)                               \
    X(DW_AT_visibility, 0x17, constant)                                \
    X(DW_AT_import, 0x18, reference)                                   \
    X(DW_AT_string_length, 0x19, exprloc, loclistptr)                  \
    X(DW_AT_common_reference, 0x1a, reference)                         \
    X(DW_AT_comp_dir, 0x1b, string)                                    \
    X(DW_AT_const_value, 0x1c, block, constant, string)                \
    X(DW_AT_containing_type, 0x1d, reference)                          \
    X(DW_AT_default_value, 0x1e, reference)                            \
    X(DW_AT_inline, 0x20, constant)                                    \
    X(DW_AT_is_optional, 0x21, flag)                                   \
    X(DW_AT_lower_bound, 0x22, constant, exprloc, reference)           \
    X(DW_AT_producer, 0x25, string)                                    \
    X(DW_AT_prototyped, 0x27, flag)                                    \
    X(DW_AT_return_addr, 0x2a, exprloc, loclistptr)                    \
    X(DW_AT_start_scope, 0x2c, constant, rnglistptr)                   \
    X(DW_AT_bit_stride, 0x2e, constant, exprloc, reference)            \
    X(DW_AT_upper_bound, 0x2f, constant, exprloc, reference)           \
    X(DW_AT_abstract_origin, 0x31, reference)                          \
    X(DW_AT_accessibility, 0x32, constant)                             \
    X(DW_AT_address_class, 0x33, constant)                             \
    X(DW_AT_artificial, 0x34, flag)                                    \
    X(DW_AT_base_types, 0x35, reference)                               \
    X(DW_AT_calling_convention, 0x36, constant)                        \
    X(DW_AT_count, 0x37, constant, exprloc, reference)                 \
    X(DW_AT_data_member_location, 0x38, constant, exprloc, loclistptr) \
    X(DW_AT_decl_column, 0x39, constant)                               \
    X(DW_AT_decl_file, 0x3a, constant)                                 \
    X(DW_AT_decl_line, 0x3b, constant)                                 \
    X(DW_AT_declaration, 0x3c, flag)                                   \
    X(DW_AT_discr_list, 0x3d, block)                                   \
    X(DW_AT_encoding, 0x3e, constant)                                  \
    X(DW_AT_external, 0x3f, flag)                                      \
    X(DW_AT_frame_base, 0x40, exprloc, loclistptr)                     \
    X(DW_AT_friend, 0x41, reference)                                   \
    X(DW_AT_identifier_case, 0x42, constant)                           \
    X(DW_AT_macro_info, 0x43, macptr)                                  \
    X(DW_AT_namelist_item, 0x44, reference)                            \
    X(DW_AT_priority, 0x45, reference)                                 \
    X(DW_AT_segment, 0x46, exprloc, loclistptr)                        \
    X(DW_AT_specification, 0x47, reference)                            \
    X(DW_AT_static_link, 0x48, exprloc, loclistptr)                    \
    X(DW_AT_type, 0x49, reference)                                     \
    X(DW_AT_use_location, 0x4a, exprloc, loclistptr)                   \
    X(DW_AT_variable_parameter, 0x4b, flag)                            \
    X(DW_AT_virtuality, 0x4c, constant)                                \
    X(DW_AT_vtable_elem_location, 0x4d, exprloc, loclistptr)           \
    X(DW_AT_allocated, 0x4e, constant, exprloc, reference)             \
    X(DW_AT_associated, 0x4f, constant, exprloc, reference)            \
    X(DW_AT_data_location, 0x50, exprloc)                              \
    X(DW_AT_byte_stride, 0x51, constant, exprloc, reference)           \
    X(DW_AT_entry_pc, 0x52, address)                                   \
    X(DW_AT_use_UTF8, 0x53, flag)                                      \
    X(DW_AT_extension, 0x54, reference)                                \
    X(DW_AT_ranges, 0x55, rnglist)                                     \
    X(DW_AT_trampoline, 0x56, address, flag, reference, string)        \
    X(DW_AT_call_column, 0x57, constant)                               \
    X(DW_AT_call_file, 0x58, constant)                                 \
    X(DW_AT_call_line, 0x59, constant)                                 \
    X(DW_AT_description, 0x5a, string)                                 \
    X(DW_AT_binary_scale, 0x5b, constant)                              \
    X(DW_AT_decimal_scale, 0x5c, constant)                             \
    X(DW_AT_small, 0x5d, reference)                                    \
    X(DW_AT_decimal_sign, 0x5e, constant)                              \
    X(DW_AT_digit_count, 0x5f, constant)                               \
    X(DW_AT_picture_string, 0x60, string)                              \
    X(DW_AT_mutable, 0x61, flag)                                       \
    X(DW_AT_threads_scaled, 0x62, flag)                                \
    X(DW_AT_explicit, 0x63, flag)                                      \
    X(DW_AT_object_pointer, 0x64, reference)                           \
    X(DW_AT_endianity, 0x65, constant)                                 \
    X(DW_AT_elemental, 0x66, flag)                                     \
    X(DW_AT_pure, 0x67, flag)                                          \
    X(DW_AT_recursive, 0x68, flag)                                     \
    X(DW_AT_signature, 0x69, reference)                                \
    X(DW_AT_main_subprogram, 0x6a, flag)                               \
    X(DW_AT_data_bit_offset, 0x6b, constant)                           \
    X(DW_AT_const_expr, 0x6c, flag)                                    \
    X(DW_AT_enum_class, 0x6d, flag)                                    \
    X(DW_AT_linkage_name, 0x6e, string)                                \
    X(DW_AT_string_length_bit_size, 0x6f, constant)                    \
    X(DW_AT_string_length_byte_size, 0x70, constant)                   \
    X(DW_AT_rank, 0x71, constant, exprloc)                             \
    X(DW_AT_str_offsets_base, 0x72, stroffsetsptr)                     \
    X(DW_AT_addr_base, 0x73, addrptr)                                  \
    X(DW_AT_rnglists_base, 0x74, rnglistptr)                           \
    X(DW_AT_dwo_name, 0x76, string)                                    \
    X(DW_AT_reference, 0x77, flag)                                     \
    X(DW_AT_rvalue_reference, 0x78, flag)                              \
    X(DW_AT_macros, 0x79, macptr)                                      \
    X(DW_AT_call_all_calls, 0x7a, flag)                                \
    X(DW_AT_call_all_source_calls, 0x7b, flag)                         \
    X(DW_AT_call_all_tail_calls, 0x7c, flag)                           \
    X(DW_AT_call_return_pc, 0x7d, address)                             \
    X(DW_AT_call_value, 0x7e, exprloc)                                 \
    X(DW_AT_call_origin, 0x7f, exprloc)                                \
    X(DW_AT_call_parameter, 0x80, reference)                           \
    X(DW_AT_call_pc, 0x81, address)                                    \
    X(DW_AT_call_tail_call, 0x82, flag)                                \
    X(DW_AT_call_target, 0x83, exprloc)                                \
    X(DW_AT_call_target_clobbered, 0x84, exprloc)                      \
    X(DW_AT_call_data_location, 0x85, exprloc)                         \
    X(DW_AT_call_data_value, 0x86, exprloc)                            \
    X(DW_AT_noreturn, 0x87, flag)                                      \
    X(DW_AT_alignment, 0x88, constant)                                 \
    X(DW_AT_export_symbols, 0x89, flag)                                \
    X(DW_AT_deleted, 0x8a, flag)                                       \
    X(DW_AT_defaulted, 0x8b, constant)                                 \
    X(DW_AT_loclists_base, 0x8c, loclistptr)

typedef enum dw_attr_name
{
    DW_ATTR_LIST(X_ENUM)
    //
    DW_AT_loc_user = 0x2000,
    DW_AT_hi_user = 0x3fff,
    DW_AT_force_u32 = 0xffffffff,
} dw_attr_name;

static inline const char* dw_get_attr_name_string(u32 name)
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

#define DW_ATTR_CLASS_LIST(X)                                                                                                                                       \
    X(address, DW_FORM_addr, DW_FORM_addrx, DW_FORM_addrx1, DW_FORM_addrx2, DW_FORM_addrx3, DW_FORM_addrx4)                                                         \
    X(addrptr, DW_FORM_sec_offset)                                                                                                                                  \
    X(block, DW_FORM_block1, DW_FORM_block2, DW_FORM_block4, DW_FORM_block)                                                                                         \
    X(constant, DW_FORM_data1, DW_FORM_data2, DW_FORM_data4, DW_FORM_data8, DW_FORM_data16, DW_FORM_sdata, DW_FORM_udata, DW_FORM_implicit_const)                   \
    X(exprloc, DW_FORM_exprloc)                                                                                                                                     \
    X(flag, DW_FORM_flag, DW_FORM_flag_present)                                                                                                                     \
    X(lineptr, DW_FORM_sec_offset)                                                                                                                                  \
    X(loclist, DW_FORM_loclistx, DW_FORM_sec_offset)                                                                                                                \
    X(loclistptr, DW_FORM_sec_offset)                                                                                                                               \
    X(macptr, DW_FORM_sec_offset)                                                                                                                                   \
    X(rnglist, DW_FORM_rnglistx, DW_FORM_sec_offset)                                                                                                                \
    X(rnglistptr, DW_FORM_sec_offset)                                                                                                                               \
    X(reference, DW_FORM_ref1, DW_FORM_ref2, DW_FORM_ref4, DW_FORM_ref8, DW_FORM_ref_udata, DW_FORM_ref_addr, DW_FORM_ref_sig8, DW_FORM_ref_sup4, DW_FORM_ref_sup8) \
    X(string, DW_FORM_string, DW_FORM_strp, DW_FORM_line_strp, DW_FORM_strp_sup, DW_FORM_strx, DW_FORM_strx1, DW_FORM_strx2, DW_FORM_strx3, DW_FORM_strx4)          \
    X(stroffsetsptr, DW_FORM_sec_offset)

typedef enum dw_attr_class
{
    DW_AT_CLASS_address,
    DW_AT_CLASS_addrptr,
    DW_AT_CLASS_block,
    DW_AT_CLASS_constant,
    DW_AT_CLASS_exprloc,
    DW_AT_CLASS_flag,
    DW_AT_CLASS_lineptr,
    DW_AT_CLASS_loclist,
    DW_AT_CLASS_loclistptr,
    DW_AT_CLASS_macptr,
    DW_AT_CLASS_reference,
    DW_AT_CLASS_rnglist,
    DW_AT_CLASS_rnglistptr,
    DW_AT_CLASS_string,
    DW_AT_CLASS_stroffsetsptr,
} dw_attr_class;

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

static inline const char* dw_get_encoding_string(u32 name)
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

static inline const char* dw_get_form_string(u32 form)
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

static inline const char* dw_get_line_standard_opcode_string(u32 opcode)
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

static inline const char* dw_get_line_header_entry_format_string(u32 format)
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

#define DW_OP_LIST(_)                                           \
    _(DW_OP_addr, 0x03, 1, DW_OPD_ADDR)                         \
    _(DW_OP_deref, 0x06, 0)                                     \
    _(DW_OP_const1u, 0x08, 1, DW_OPD_U8)                        \
    _(DW_OP_const1s, 0x09, 1, DW_OPD_I8)                        \
    _(DW_OP_const2u, 0x0a, 1, DW_OPD_U16)                       \
    _(DW_OP_const2s, 0x0b, 1, DW_OPD_I16)                       \
    _(DW_OP_const4u, 0x0c, 1, DW_OPD_U32)                       \
    _(DW_OP_const4s, 0x0d, 1, DW_OPD_I32)                       \
    _(DW_OP_const8u, 0x0e, 1, DW_OPD_U64)                       \
    _(DW_OP_const8s, 0x0f, 1, DW_OPD_I64)                       \
    _(DW_OP_constu, 0x10, 1, DW_OPD_ULEB)                       \
    _(DW_OP_consts, 0x11, 1, DW_OPD_SLEB)                       \
    _(DW_OP_dup, 0x12, 0)                                       \
    _(DW_OP_drop, 0x13, 0)                                      \
    _(DW_OP_over, 0x14, 0)                                      \
    _(DW_OP_pick, 0x15, DW_OPD_U8)                              \
    _(DW_OP_swap, 0x16, 0)                                      \
    _(DW_OP_rot, 0x17, 0)                                       \
    _(DW_OP_xderef, 0x18, 0)                                    \
    _(DW_OP_abs, 0x19, 0)                                       \
    _(DW_OP_and, 0x1a, 0)                                       \
    _(DW_OP_div, 0x1b, 0)                                       \
    _(DW_OP_minus, 0x1c, 0)                                     \
    _(DW_OP_mod, 0x1d, 0)                                       \
    _(DW_OP_mul, 0x1e, 0)                                       \
    _(DW_OP_neg, 0x1f, 0)                                       \
    _(DW_OP_not, 0x20, 0)                                       \
    _(DW_OP_or, 0x21, 0)                                        \
    _(DW_OP_plus, 0x22, 0)                                      \
    _(DW_OP_plus_uconst, 0x23, 1, DW_OPD_ULEB)                  \
    _(DW_OP_shl, 0x24, 0)                                       \
    _(DW_OP_shr, 0x25, 0)                                       \
    _(DW_OP_shra, 0x26, 0)                                      \
    _(DW_OP_xor, 0x27, 0)                                       \
    _(DW_OP_bra, 0x28, 1, DW_OPD_I16)                           \
    _(DW_OP_eq, 0x29, 0)                                        \
    _(DW_OP_ge, 0x2a, 0)                                        \
    _(DW_OP_gt, 0x2b, 0)                                        \
    _(DW_OP_le, 0x2c, 0)                                        \
    _(DW_OP_lt, 0x2d, 0)                                        \
    _(DW_OP_ne, 0x2e, 0)                                        \
    _(DW_OP_skip, 0x2f, 0)                                      \
    _(DW_OP_lit0, 0x30, 0)                                      \
    _(DW_OP_lit1, 0x31, 0)                                      \
    _(DW_OP_lit2, 0x32, 0)                                      \
    _(DW_OP_lit3, 0x33, 0)                                      \
    _(DW_OP_lit4, 0x34, 0)                                      \
    _(DW_OP_lit5, 0x35, 0)                                      \
    _(DW_OP_lit6, 0x36, 0)                                      \
    _(DW_OP_lit7, 0x37, 0)                                      \
    _(DW_OP_lit8, 0x38, 0)                                      \
    _(DW_OP_lit9, 0x39, 0)                                      \
    _(DW_OP_lit10, 0x3a, 0)                                     \
    _(DW_OP_lit11, 0x3b, 0)                                     \
    _(DW_OP_lit12, 0x3c, 0)                                     \
    _(DW_OP_lit13, 0x3d, 0)                                     \
    _(DW_OP_lit14, 0x3e, 0)                                     \
    _(DW_OP_lit15, 0x3f, 0)                                     \
    _(DW_OP_lit16, 0x40, 0)                                     \
    _(DW_OP_lit17, 0x41, 0)                                     \
    _(DW_OP_lit18, 0x42, 0)                                     \
    _(DW_OP_lit19, 0x43, 0)                                     \
    _(DW_OP_lit20, 0x44, 0)                                     \
    _(DW_OP_lit21, 0x45, 0)                                     \
    _(DW_OP_lit22, 0x46, 0)                                     \
    _(DW_OP_lit23, 0x47, 0)                                     \
    _(DW_OP_lit24, 0x48, 0)                                     \
    _(DW_OP_lit25, 0x49, 0)                                     \
    _(DW_OP_lit26, 0x4a, 0)                                     \
    _(DW_OP_lit27, 0x4b, 0)                                     \
    _(DW_OP_lit28, 0x4c, 0)                                     \
    _(DW_OP_lit29, 0x4d, 0)                                     \
    _(DW_OP_lit30, 0x4e, 0)                                     \
    _(DW_OP_lit31, 0x4f, 0)                                     \
    _(DW_OP_reg0, 0x50, 0)                                      \
    _(DW_OP_reg1, 0x51, 0)                                      \
    _(DW_OP_reg2, 0x52, 0)                                      \
    _(DW_OP_reg3, 0x53, 0)                                      \
    _(DW_OP_reg4, 0x54, 0)                                      \
    _(DW_OP_reg5, 0x55, 0)                                      \
    _(DW_OP_reg6, 0x56, 0)                                      \
    _(DW_OP_reg7, 0x57, 0)                                      \
    _(DW_OP_reg8, 0x58, 0)                                      \
    _(DW_OP_reg9, 0x59, 0)                                      \
    _(DW_OP_reg10, 0x5a, 0)                                     \
    _(DW_OP_reg11, 0x5b, 0)                                     \
    _(DW_OP_reg12, 0x5c, 0)                                     \
    _(DW_OP_reg13, 0x5d, 0)                                     \
    _(DW_OP_reg14, 0x5e, 0)                                     \
    _(DW_OP_reg15, 0x5f, 0)                                     \
    _(DW_OP_reg16, 0x60, 0)                                     \
    _(DW_OP_reg17, 0x61, 0)                                     \
    _(DW_OP_reg18, 0x62, 0)                                     \
    _(DW_OP_reg19, 0x63, 0)                                     \
    _(DW_OP_reg20, 0x64, 0)                                     \
    _(DW_OP_reg21, 0x65, 0)                                     \
    _(DW_OP_reg22, 0x66, 0)                                     \
    _(DW_OP_reg23, 0x67, 0)                                     \
    _(DW_OP_reg24, 0x68, 0)                                     \
    _(DW_OP_reg25, 0x69, 0)                                     \
    _(DW_OP_reg26, 0x6a, 0)                                     \
    _(DW_OP_reg27, 0x6b, 0)                                     \
    _(DW_OP_reg28, 0x6c, 0)                                     \
    _(DW_OP_reg29, 0x6d, 0)                                     \
    _(DW_OP_reg30, 0x6e, 0)                                     \
    _(DW_OP_reg31, 0x6f, 0)                                     \
    _(DW_OP_breg0, 0x70, 1, DW_OPD_SLEB)                        \
    _(DW_OP_breg1, 0x71, 1, DW_OPD_SLEB)                        \
    _(DW_OP_breg2, 0x72, 1, DW_OPD_SLEB)                        \
    _(DW_OP_breg3, 0x73, 1, DW_OPD_SLEB)                        \
    _(DW_OP_breg4, 0x74, 1, DW_OPD_SLEB)                        \
    _(DW_OP_breg5, 0x75, 1, DW_OPD_SLEB)                        \
    _(DW_OP_breg6, 0x76, 1, DW_OPD_SLEB)                        \
    _(DW_OP_breg7, 0x77, 1, DW_OPD_SLEB)                        \
    _(DW_OP_breg8, 0x78, 1, DW_OPD_SLEB)                        \
    _(DW_OP_breg9, 0x79, 1, DW_OPD_SLEB)                        \
    _(DW_OP_breg10, 0x7a, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg11, 0x7b, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg12, 0x7c, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg13, 0x7d, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg14, 0x7e, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg15, 0x7f, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg16, 0x80, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg17, 0x81, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg18, 0x82, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg19, 0x83, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg20, 0x84, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg21, 0x85, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg22, 0x86, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg23, 0x87, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg24, 0x88, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg25, 0x89, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg26, 0x8a, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg27, 0x8b, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg28, 0x8c, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg29, 0x8d, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg30, 0x8e, 1, DW_OPD_SLEB)                       \
    _(DW_OP_breg31, 0x8f, 1, DW_OPD_SLEB)                       \
    _(DW_OP_regx, 0x90, 1, DW_OPD_ULEB)                         \
    _(DW_OP_fbreg, 0x91, 1, DW_OPD_SLEB)                        \
    _(DW_OP_bregx, 0x92, 2, DW_OPD_ULEB, DW_OPD_SLEB)           \
    _(DW_OP_piece, 0x93, 1, DW_OPD_ULEB)                        \
    _(DW_OP_deref_size, 0x94, 1, DW_OPD_U8)                     \
    _(DW_OP_xderef_size, 0x95, DW_OPD_U8)                       \
    _(DW_OP_nop, 0x96, 0)                                       \
    _(DW_OP_push_object_address, 0x97, 0)                       \
    _(DW_OP_call2, 0x98, 1, DW_OPD_U16)                         \
    _(DW_OP_call4, 0x99, 1, DW_OPD_U32)                         \
    _(DW_OP_call_ref, 0x9a, 1, DW_OPD_REF)                      \
    _(DW_OP_form_tls_address, 0x9b, 0)                          \
    _(DW_OP_call_frame_cfa, 0x9c, 0)                            \
    _(DW_OP_bit_piece, 0x9d, 2, DW_OPD_ULEB, DW_OPD_ULEB)       \
    _(DW_OP_implicit_value, 0x9e, 1, DW_OPD_ULEB)               \
    _(DW_OP_stack_value, 0x9f, 0)                               \
    _(DW_OP_implicit_pointer, 0xa0, 2, DW_OPD_REF, DW_OPD_ULEB) \
    _(DW_OP_addrx, 0xa1, 1, DW_OPD_ULEB)                        \
    _(DW_OP_constx, 0xa2, 1, DW_OPD_ULEB)                       \
    _(DW_OP_entry_value, 0xa3, 1, DW_OPD_ULEB)                  \
    _(DW_OP_const_type, 0xa4, 2, DW_OPD_ULEB, DW_OPD_U8)        \
    _(DW_OP_regval_type, 0xa5, 2, DW_OPD_ULEB, DW_OPD_ULEB)     \
    _(DW_OP_deref_type, 0xa6, 2, DW_OPD_U8, DW_OPD_ULEB)        \
    _(DW_OP_xderef_type, 0xa7, 2, DW_OPD_U8, DW_OPD_ULEB)       \
    _(DW_OP_convert, 0xa8, DW_OPD_ULEB)                         \
    _(DW_OP_reinterpret, 0xa9, DW_OPD_ULEB)                     \
    _(DW_OP_lo_user, 0xe0, 0)                                   \
    _(DW_OP_hi_user, 0xff, 0)                                   \
    _(DW_OP_WASM_location, 0xed, 2, DW_OPD_U8, DW_OPD_ULEB)

typedef enum dw_op
{
    DW_OP_LIST(X_ENUM)
} dw_op;

static inline oc_str8 dw_op_get_string(u32 opcode)
{
    oc_str8 res = { 0 };
    switch(opcode)
    {
        DW_OP_LIST(X_NAME_CASE_STR8);
    }
    return res;
}

enum
{
    DW_MAX_OPD_COUNT = 4,
};

typedef enum dw_opd_kind
{
    DW_OPD_U8,
    DW_OPD_I8,
    DW_OPD_U16,
    DW_OPD_I16,
    DW_OPD_U32,
    DW_OPD_I32,
    DW_OPD_U64,
    DW_OPD_I64,
    DW_OPD_ULEB,
    DW_OPD_SLEB,
    DW_OPD_ADDR, //NOTE: operand size depends on address size
    DW_OPD_REF,  //NOTE: operand size depends on dwarf format
} dw_opd_kind;

typedef struct dw_op_info
{
    u32 count;
    dw_opd_kind opd[DW_MAX_OPD_COUNT];
} dw_op_info;

extern const dw_op_info DW_OP_INFO[];

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

static const char* dw_sectionIdentifierStrings[] = {
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

static const int dw_sectionIdentifierCount = sizeof(dw_sectionIdentifierStrings) / sizeof(char*);

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

static inline const char* dw_get_cu_type_string(u8 unitType)
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

//------------------------------------------------------------------------
// DIEs
//------------------------------------------------------------------------

typedef enum dw_dwarf_format
{
    DW_DWARF32,
    DW_DWARF64,
} dw_dwarf_format;

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

typedef struct dw_val
{
    union
    {
        oc_str8 string;
        u8 valU8;
        i8 valI8;
        u16 valU16;
        i16 valI16;
        u32 valU32;
        i32 valI32;
        u64 valU64;
        i64 valI64;
    };
} dw_val;

typedef struct dw_expr_instr
{
    dw_op op;
    dw_val* operands;
} dw_expr_instr;

typedef struct dw_expr
{
    u64 codeLen;
    dw_expr_instr* code;
} dw_expr;

typedef struct dw_loc_entry
{
    u64 start;
    u64 end;
    dw_expr expr;

} dw_loc_entry;

typedef struct dw_loc
{
    bool single;
    u64 entryCount;
    dw_loc_entry* entries;
} dw_loc;

typedef struct dw_range_entry
{
    u64 start;
    u64 end;
} dw_range_entry;

typedef struct dw_range_list
{
    u64 entryCount;
    dw_range_entry* entries;
} dw_range_list;

typedef struct dw_attr
{
    dw_abbrev_attr* abbrev;

    union
    {
        dw_loc loc;
        dw_range_list ranges;
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

//------------------------------------------------------------------------
// Line info
//------------------------------------------------------------------------

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

//------------------------------------------------------------------------
// dwarf info
//------------------------------------------------------------------------

typedef struct dw_info
{
    u64 unitCount;
    dw_unit* units;

    dw_line_info line;

} dw_info;

typedef struct dw_section
{
    u64 offset;
    u64 len;
} dw_section;

typedef struct dw_sections
{
    dw_section abbrev;
    dw_section info;
    dw_section strOffsets;
    dw_section str;
    dw_section addr;
    dw_section line;
    dw_section lineStr;
    dw_section loc;
    dw_section ranges;
} dw_sections;
