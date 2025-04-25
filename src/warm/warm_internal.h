/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

typedef enum wa_instr_op
{
    WA_INSTR_INVALID = 0,

    /* control instructions */
    WA_INSTR_nop,
    WA_INSTR_unreachable,
    WA_INSTR_block,
    WA_INSTR_loop,
    WA_INSTR_if,
    WA_INSTR_else,
    WA_INSTR_end,
    WA_INSTR_br,
    WA_INSTR_br_if,
    WA_INSTR_br_table,
    WA_INSTR_return,
    WA_INSTR_call,
    WA_INSTR_call_indirect,

    /* reference instructions */
    WA_INSTR_ref_null,
    WA_INSTR_ref_is_null,
    WA_INSTR_ref_func,

    /* parametric instructions */
    WA_INSTR_drop,
    WA_INSTR_select,
    WA_INSTR_select_t,

    /* variable instructions */
    WA_INSTR_local_get,
    WA_INSTR_local_set,
    WA_INSTR_local_tee,
    WA_INSTR_global_get,
    WA_INSTR_global_set,

    /* table instructions */
    WA_INSTR_table_get,
    WA_INSTR_table_set,

    /* memory instructions */
    WA_INSTR_i32_load,
    WA_INSTR_i64_load,
    WA_INSTR_f32_load,
    WA_INSTR_f64_load,
    WA_INSTR_i32_load8_s,
    WA_INSTR_i32_load8_u,
    WA_INSTR_i32_load16_s,
    WA_INSTR_i32_load16_u,
    WA_INSTR_i64_load8_s,
    WA_INSTR_i64_load8_u,
    WA_INSTR_i64_load16_s,
    WA_INSTR_i64_load16_u,
    WA_INSTR_i64_load32_s,
    WA_INSTR_i64_load32_u,
    WA_INSTR_i32_store,
    WA_INSTR_i64_store,
    WA_INSTR_f32_store,
    WA_INSTR_f64_store,
    WA_INSTR_i32_store8,
    WA_INSTR_i32_store16,
    WA_INSTR_i64_store8,
    WA_INSTR_i64_store16,
    WA_INSTR_i64_store32,
    WA_INSTR_memory_size,
    WA_INSTR_memory_grow,

    /* consts */
    WA_INSTR_i32_const,
    WA_INSTR_i64_const,
    WA_INSTR_f32_const,
    WA_INSTR_f64_const,

    /* i32 comparisons */
    WA_INSTR_i32_eqz,
    WA_INSTR_i32_eq,
    WA_INSTR_i32_ne,
    WA_INSTR_i32_lt_s,
    WA_INSTR_i32_lt_u,
    WA_INSTR_i32_gt_s,
    WA_INSTR_i32_gt_u,
    WA_INSTR_i32_le_s,
    WA_INSTR_i32_le_u,
    WA_INSTR_i32_ge_s,
    WA_INSTR_i32_ge_u,

    /* i64 comparisons */
    WA_INSTR_i64_eqz,
    WA_INSTR_i64_eq,
    WA_INSTR_i64_ne,
    WA_INSTR_i64_lt_s,
    WA_INSTR_i64_lt_u,
    WA_INSTR_i64_gt_s,
    WA_INSTR_i64_gt_u,
    WA_INSTR_i64_le_s,
    WA_INSTR_i64_le_u,
    WA_INSTR_i64_ge_s,
    WA_INSTR_i64_ge_u,

    /* f32 comparisons */
    WA_INSTR_f32_eq,
    WA_INSTR_f32_ne,
    WA_INSTR_f32_lt,
    WA_INSTR_f32_gt,
    WA_INSTR_f32_le,
    WA_INSTR_f32_ge,

    /* f64 comparisons */
    WA_INSTR_f64_eq,
    WA_INSTR_f64_ne,
    WA_INSTR_f64_lt,
    WA_INSTR_f64_gt,
    WA_INSTR_f64_le,
    WA_INSTR_f64_ge,

    /* i32 arithmetic */
    WA_INSTR_i32_clz,
    WA_INSTR_i32_ctz,
    WA_INSTR_i32_popcnt,
    WA_INSTR_i32_add,
    WA_INSTR_i32_sub,
    WA_INSTR_i32_mul,
    WA_INSTR_i32_div_s,
    WA_INSTR_i32_div_u,
    WA_INSTR_i32_rem_s,
    WA_INSTR_i32_rem_u,
    WA_INSTR_i32_and,
    WA_INSTR_i32_or,
    WA_INSTR_i32_xor,
    WA_INSTR_i32_shl,
    WA_INSTR_i32_shr_s,
    WA_INSTR_i32_shr_u,
    WA_INSTR_i32_rotl,
    WA_INSTR_i32_rotr,

    /* i64 arithmetic */
    WA_INSTR_i64_clz,
    WA_INSTR_i64_ctz,
    WA_INSTR_i64_popcnt,
    WA_INSTR_i64_add,
    WA_INSTR_i64_sub,
    WA_INSTR_i64_mul,
    WA_INSTR_i64_div_s,
    WA_INSTR_i64_div_u,
    WA_INSTR_i64_rem_s,
    WA_INSTR_i64_rem_u,
    WA_INSTR_i64_and,
    WA_INSTR_i64_or,
    WA_INSTR_i64_xor,
    WA_INSTR_i64_shl,
    WA_INSTR_i64_shr_s,
    WA_INSTR_i64_shr_u,
    WA_INSTR_i64_rotl,
    WA_INSTR_i64_rotr,

    /* f32 arithmetic */
    WA_INSTR_f32_abs,
    WA_INSTR_f32_neg,
    WA_INSTR_f32_ceil,
    WA_INSTR_f32_floor,
    WA_INSTR_f32_trunc,
    WA_INSTR_f32_nearest,
    WA_INSTR_f32_sqrt,
    WA_INSTR_f32_add,
    WA_INSTR_f32_sub,
    WA_INSTR_f32_mul,
    WA_INSTR_f32_div,
    WA_INSTR_f32_min,
    WA_INSTR_f32_max,
    WA_INSTR_f32_copysign,

    /* f64 arithmetic */
    WA_INSTR_f64_abs,
    WA_INSTR_f64_neg,
    WA_INSTR_f64_ceil,
    WA_INSTR_f64_floor,
    WA_INSTR_f64_trunc,
    WA_INSTR_f64_nearest,
    WA_INSTR_f64_sqrt,
    WA_INSTR_f64_add,
    WA_INSTR_f64_sub,
    WA_INSTR_f64_mul,
    WA_INSTR_f64_div,
    WA_INSTR_f64_min,
    WA_INSTR_f64_max,
    WA_INSTR_f64_copysign,

    /* i32 conversions */
    WA_INSTR_i32_wrap_i64,
    WA_INSTR_i32_trunc_f32_s,
    WA_INSTR_i32_trunc_f32_u,
    WA_INSTR_i32_trunc_f64_s,
    WA_INSTR_i32_trunc_f64_u,

    /* i64 conversions */
    WA_INSTR_i64_extend_i32_s,
    WA_INSTR_i64_extend_i32_u,
    WA_INSTR_i64_trunc_f32_s,
    WA_INSTR_i64_trunc_f32_u,
    WA_INSTR_i64_trunc_f64_s,
    WA_INSTR_i64_trunc_f64_u,

    /* f32 conversions */
    WA_INSTR_f32_convert_i32_s,
    WA_INSTR_f32_convert_i32_u,
    WA_INSTR_f32_convert_i64_s,
    WA_INSTR_f32_convert_i64_u,
    WA_INSTR_f32_demote_f64,

    /* f64 conversions */
    WA_INSTR_f64_convert_i32_s,
    WA_INSTR_f64_convert_i32_u,
    WA_INSTR_f64_convert_i64_s,
    WA_INSTR_f64_convert_i64_u,
    WA_INSTR_f64_promote_f32,

    /* reinterpret */
    WA_INSTR_i32_reinterpret_f32,
    WA_INSTR_i64_reinterpret_f64,
    WA_INSTR_f32_reinterpret_i32,
    WA_INSTR_f64_reinterpret_i64,

    /* extend */
    WA_INSTR_i32_extend8_s,
    WA_INSTR_i32_extend16_s,
    WA_INSTR_i64_extend8_s,
    WA_INSTR_i64_extend16_s,
    WA_INSTR_i64_extend32_s,

    /* extended table instructions */
    WA_INSTR_table_init,
    WA_INSTR_elem_drop,
    WA_INSTR_table_copy,
    WA_INSTR_table_grow,
    WA_INSTR_table_size,
    WA_INSTR_table_fill,

    /* extended memory instructions */
    WA_INSTR_memory_init,
    WA_INSTR_data_drop,
    WA_INSTR_memory_copy,
    WA_INSTR_memory_fill,

    /* saturating truncation */
    WA_INSTR_i32_trunc_sat_f32_s,
    WA_INSTR_i32_trunc_sat_f32_u,
    WA_INSTR_i32_trunc_sat_f64_s,
    WA_INSTR_i32_trunc_sat_f64_u,
    WA_INSTR_i64_trunc_sat_f32_s,
    WA_INSTR_i64_trunc_sat_f32_u,
    WA_INSTR_i64_trunc_sat_f64_s,
    WA_INSTR_i64_trunc_sat_f64_u,

    /* Vector instructions */
    /* loads and stores */
    WA_INSTR_v128_load,
    WA_INSTR_v128_load8x8_s,
    WA_INSTR_v128_load8x8_u,
    WA_INSTR_v128_load16x4_s,
    WA_INSTR_v128_load16x4_u,
    WA_INSTR_v128_load32x2_s,
    WA_INSTR_v128_load32x2_u,
    WA_INSTR_v128_load8_splat,
    WA_INSTR_v128_load16_splat,
    WA_INSTR_v128_load32_splat,
    WA_INSTR_v128_load64_splat,
    WA_INSTR_v128_store,
    WA_INSTR_v128_load32_zero,
    WA_INSTR_v128_load64_zero,
    WA_INSTR_v128_load8_lane,
    WA_INSTR_v128_load16_lane,
    WA_INSTR_v128_load32_lane,
    WA_INSTR_v128_load64_lane,
    WA_INSTR_v128_store8_lane,
    WA_INSTR_v128_store16_lane,
    WA_INSTR_v128_store32_lane,
    WA_INSTR_v128_store64_lane,

    /* const */
    WA_INSTR_v128_const,

    /* shuffle */
    WA_INSTR_i8x16_shuffle,

    /* extract/replace */
    WA_INSTR_i8x16_extract_lane_s,
    WA_INSTR_i8x16_extract_lane_u,
    WA_INSTR_i8x16_replace_lane,
    WA_INSTR_i16x8_extract_lane_s,
    WA_INSTR_i16x8_extract_lane_u,
    WA_INSTR_i16x8_replace_lane,
    WA_INSTR_i32x4_extract_lane,
    WA_INSTR_i32x4_replace_lane,
    WA_INSTR_i64x2_extract_lane,
    WA_INSTR_i64x2_replace_lane,
    WA_INSTR_f32x4_extract_lane,
    WA_INSTR_f32x4_replace_lane,
    WA_INSTR_f64x2_extract_lane,
    WA_INSTR_f64x2_replace_lane,

    /* swizzle / splat */
    WA_INSTR_i8x16_swizzle,
    WA_INSTR_i8x16_splat,
    WA_INSTR_i16x8_splat,
    WA_INSTR_i32x4_splat,
    WA_INSTR_i64x2_splat,
    WA_INSTR_f32x4_splat,
    WA_INSTR_f64x2_splat,

    /* compares i8x16 */
    WA_INSTR_i8x16_eq,
    WA_INSTR_i8x16_ne,
    WA_INSTR_i8x16_lt_s,
    WA_INSTR_i8x16_lt_u,
    WA_INSTR_i8x16_gt_s,
    WA_INSTR_i8x16_gt_u,
    WA_INSTR_i8x16_le_s,
    WA_INSTR_i8x16_le_u,
    WA_INSTR_i8x16_ge_s,
    WA_INSTR_i8x16_ge_u,

    /* compares i16x8 */
    WA_INSTR_i16x8_eq,
    WA_INSTR_i16x8_ne,
    WA_INSTR_i16x8_lt_s,
    WA_INSTR_i16x8_lt_u,
    WA_INSTR_i16x8_gt_s,
    WA_INSTR_i16x8_gt_u,
    WA_INSTR_i16x8_le_s,
    WA_INSTR_i16x8_le_u,
    WA_INSTR_i16x8_ge_s,
    WA_INSTR_i16x8_ge_u,

    /* compares i32x4 */
    WA_INSTR_i32x4_eq,
    WA_INSTR_i32x4_ne,
    WA_INSTR_i32x4_lt_s,
    WA_INSTR_i32x4_lt_u,
    WA_INSTR_i32x4_gt_s,
    WA_INSTR_i32x4_gt_u,
    WA_INSTR_i32x4_le_s,
    WA_INSTR_i32x4_le_u,
    WA_INSTR_i32x4_ge_s,
    WA_INSTR_i32x4_ge_u,

    /* compares i64x2 */
    WA_INSTR_i64x2_eq,
    WA_INSTR_i64x2_ne,
    WA_INSTR_i64x2_lt_s,
    WA_INSTR_i64x2_gt_s,
    WA_INSTR_i64x2_le_s,
    WA_INSTR_i64x2_ge_s,

    /* compares f32x4 */
    WA_INSTR_f32x4_eq,
    WA_INSTR_f32x4_ne,
    WA_INSTR_f32x4_lt,
    WA_INSTR_f32x4_gt,
    WA_INSTR_f32x4_le,
    WA_INSTR_f32x4_ge,

    /* compares f64x2 */
    WA_INSTR_f64x2_eq,
    WA_INSTR_f64x2_ne,
    WA_INSTR_f64x2_lt,
    WA_INSTR_f64x2_gt,
    WA_INSTR_f64x2_le,
    WA_INSTR_f64x2_ge,

    /* bit logic */
    WA_INSTR_v128_not,
    WA_INSTR_v128_and,
    WA_INSTR_v128_andnot,
    WA_INSTR_v128_or,
    WA_INSTR_v128_xor,
    WA_INSTR_v128_bitselect,
    WA_INSTR_v128_any_true,

    /* i8x16 arithmetic */
    WA_INSTR_i8x16_abs,
    WA_INSTR_i8x16_neg,
    WA_INSTR_i8x16_popcnt,
    WA_INSTR_i8x16_all_true,
    WA_INSTR_i8x16_bitmask,
    WA_INSTR_i8x16_narrow_i16x8_s,
    WA_INSTR_i8x16_narrow_i16x8_u,
    WA_INSTR_i8x16_shl,
    WA_INSTR_i8x16_shr_s,
    WA_INSTR_i8x16_shr_u,
    WA_INSTR_i8x16_add,
    WA_INSTR_i8x16_add_sat_s,
    WA_INSTR_i8x16_add_sat_u,
    WA_INSTR_i8x16_sub,
    WA_INSTR_i8x16_sub_sat_s,
    WA_INSTR_i8x16_sub_sat_u,
    WA_INSTR_i8x16_min_s,
    WA_INSTR_i8x16_min_u,
    WA_INSTR_i8x16_max_s,
    WA_INSTR_i8x16_max_u,
    WA_INSTR_i8x16_avgr_u,

    /* i16x8 arithmetic */
    WA_INSTR_i16x8_extadd_pairwise_i8x16_s,
    WA_INSTR_i16x8_extadd_pairwise_i8x16_u,
    WA_INSTR_i16x8_abs,
    WA_INSTR_i16x8_neg,
    WA_INSTR_i16x8_q15_mulr_sat_s,
    WA_INSTR_i16x8_all_true,
    WA_INSTR_i16x8_bitmask,
    WA_INSTR_i16x8_narrow_i32x4_s,
    WA_INSTR_i16x8_narrow_i32x4_u,
    WA_INSTR_i16x8_extend_low_i8x16_s,
    WA_INSTR_i16x8_extend_high_i8x16_s,
    WA_INSTR_i16x8_extend_low_i8x16_u,
    WA_INSTR_i16x8_extend_high_i8x16_u,
    WA_INSTR_i16x8_shl,
    WA_INSTR_i16x8_shr_s,
    WA_INSTR_i16x8_shr_u,
    WA_INSTR_i16x8_add,
    WA_INSTR_i16x8_add_sat_s,
    WA_INSTR_i16x8_add_sat_u,
    WA_INSTR_i16x8_sub,
    WA_INSTR_i16x8_sub_sat_s,
    WA_INSTR_i16x8_sub_sat_u,
    WA_INSTR_i16x8_mul,
    WA_INSTR_i16x8_min_s,
    WA_INSTR_i16x8_min_u,
    WA_INSTR_i16x8_max_s,
    WA_INSTR_i16x8_max_u,
    WA_INSTR_i16x8_avgr_u,
    WA_INSTR_i16x8_extmul_low_i8x16_s,
    WA_INSTR_i16x8_extmul_high_i8x16_s,
    WA_INSTR_i16x8_extmul_low_i8x16_u,
    WA_INSTR_i16x8_extmul_high_i8x16_u,

    /* i32x4 arithmetic */
    WA_INSTR_i32x4_extadd_pairwise_i16x8_s,
    WA_INSTR_i32x4_extadd_pairwise_i16x8_u,
    WA_INSTR_i32x4_abs,
    WA_INSTR_i32x4_neg,
    WA_INSTR_i32x4_all_true,
    WA_INSTR_i32x4_bitmask,
    WA_INSTR_i32x4_extend_low_i16x8_s,
    WA_INSTR_i32x4_extend_high_i16x8_s,
    WA_INSTR_i32x4_extend_low_i16x8_u,
    WA_INSTR_i32x4_extend_high_i16x8_u,
    WA_INSTR_i32x4_shl,
    WA_INSTR_i32x4_shr_s,
    WA_INSTR_i32x4_shr_u,
    WA_INSTR_i32x4_add,
    WA_INSTR_i32x4_sub,
    WA_INSTR_i32x4_mul,
    WA_INSTR_i32x4_min_s,
    WA_INSTR_i32x4_min_u,
    WA_INSTR_i32x4_max_s,
    WA_INSTR_i32x4_max_u,
    WA_INSTR_i32x4_dot_i16x8_s,
    WA_INSTR_i32x4_extmul_low_i16x8_s,
    WA_INSTR_i32x4_extmul_high_i16x8_s,
    WA_INSTR_i32x4_extmul_low_i16x8_u,
    WA_INSTR_i32x4_extmul_high_i16x8_u,

    /* i64x2 arithmetic */
    WA_INSTR_i64x2_abs,
    WA_INSTR_i64x2_neg,
    WA_INSTR_i64x2_all_true,
    WA_INSTR_i64x2_bitmask,
    WA_INSTR_i64x2_extend_low_i32x4_s,
    WA_INSTR_i64x2_extend_high_i32x4_s,
    WA_INSTR_i64x2_extend_low_i32x4_u,
    WA_INSTR_i64x2_extend_high_i32x4_u,
    WA_INSTR_i64x2_shl,
    WA_INSTR_i64x2_shr_s,
    WA_INSTR_i64x2_shr_u,
    WA_INSTR_i64x2_add,
    WA_INSTR_i64x2_sub,
    WA_INSTR_i64x2_mul,
    WA_INSTR_i64x2_extmul_low_i32x4_s,
    WA_INSTR_i64x2_extmul_high_i32x4_s,
    WA_INSTR_i64x2_extmul_low_i32x4_u,
    WA_INSTR_i64x2_extmul_high_i32x4_u,

    /* f32x4 arithmetic */
    WA_INSTR_f32x4_ceil,
    WA_INSTR_f32x4_floor,
    WA_INSTR_f32x4_trunc,
    WA_INSTR_f32x4_nearest,
    WA_INSTR_f32x4_abs,
    WA_INSTR_f32x4_neg,
    WA_INSTR_f32x4_sqrt,
    WA_INSTR_f32x4_add,
    WA_INSTR_f32x4_sub,
    WA_INSTR_f32x4_mul,
    WA_INSTR_f32x4_div,
    WA_INSTR_f32x4_min,
    WA_INSTR_f32x4_max,
    WA_INSTR_f32x4_pmin,
    WA_INSTR_f32x4_pmax,

    /* f64x2 arithmetic */
    WA_INSTR_f64x2_ceil,
    WA_INSTR_f64x2_floor,
    WA_INSTR_f64x2_trunc,
    WA_INSTR_f64x2_nearest,
    WA_INSTR_f64x2_abs,
    WA_INSTR_f64x2_neg,
    WA_INSTR_f64x2_sqrt,
    WA_INSTR_f64x2_add,
    WA_INSTR_f64x2_sub,
    WA_INSTR_f64x2_mul,
    WA_INSTR_f64x2_div,
    WA_INSTR_f64x2_min,
    WA_INSTR_f64x2_max,
    WA_INSTR_f64x2_pmin,
    WA_INSTR_f64x2_pmax,

    /* more conversions */
    WA_INSTR_i32x4_trunc_sat_f32x4_s,
    WA_INSTR_i32x4_trunc_sat_f32x4_u,
    WA_INSTR_f32x4_convert_i32x4_s,
    WA_INSTR_f32x4_convert_i32x4_u,
    WA_INSTR_i32x4_trunc_sat_f64x2_s_zero,
    WA_INSTR_i32x4_trunc_sat_f64x2_u_zero,
    WA_INSTR_f64x2_convert_low_i32x4_s,
    WA_INSTR_f64x2_convert_low_i32x4_u,
    WA_INSTR_f32x4_demote_f64x2_zero,
    WA_INSTR_f64x2_promote_low_f32x4,

    /* Internal opcodes */

    WA_INSTR_move,
    WA_INSTR_internal_range_start = WA_INSTR_move,
    WA_INSTR_jump,
    WA_INSTR_jump_if,
    WA_INSTR_jump_if_zero,
    WA_INSTR_jump_table,

    WA_INSTR_breakpoint,

    WA_INSTR_COUNT,

} wa_instr_op;

typedef union wa_code
{
    u64 valU64;
    i64 valI64;
    i32 valU32;
    i32 valI32;

    f32 valF32;
    f64 valF64;

    wa_instr_op opcode;

    u32 index;
    wa_value_type valueType;

    struct
    {
        u32 align;
        u32 offset;
    } memArg;

    u8 laneIndex;

} wa_code;

enum
{
    WA_INSTR_IMM_MAX_COUNT = 2,
};

typedef struct wa_ast wa_ast;
typedef struct wa_instr wa_instr;

typedef struct wa_instr
{
    oc_list_elt listElt;

    wa_instr_op op;
    u32 immCount;
    wa_code* imm;

    wa_func_type* blockType;
    wa_instr* elseBranch;
    wa_instr* end;

    wa_ast* ast;
    u32 codeIndex;
} wa_instr;

typedef struct wa_import wa_import;
typedef struct wa_instance wa_instance;

typedef struct wa_func
{
    wa_func_type* type;

    u32 localCount;
    wa_value_type* locals;

    oc_list instructions;

    u32 codeLen;
    wa_code* code;

    wa_import* import;
    wa_host_proc proc;
    void* user;

    wa_instance* extInstance;
    u32 extIndex;

    u32 maxRegCount;
} wa_func;

typedef struct wa_global_desc
{
    wa_value_type type;
    bool mut;
    oc_list init;

    u32 codeLen;
    wa_code* code;
} wa_global_desc;

typedef struct wa_table_type
{
    wa_value_type type;
    wa_limits limits;
} wa_table_type;

typedef enum wa_element_mode
{
    WA_ELEMENT_PASSIVE,
    WA_ELEMENT_ACTIVE,
    WA_ELEMENT_DECLARATIVE,
} wa_element_mode;

typedef struct wa_element
{
    wa_value_type type;
    wa_element_mode mode;
    u32 tableIndex;
    oc_list tableOffset;
    u64 initCount;
    oc_list* initInstr;

    wa_code* tableOffsetCode;
    wa_code** code;

    wa_value* refs;
} wa_element;

typedef enum wa_data_mode
{
    WA_DATA_PASSIVE,
    WA_DATA_ACTIVE,
} wa_data_mode;

typedef struct wa_data_segment
{
    wa_data_mode mode;
    u32 memoryIndex;
    oc_list memoryOffset;
    wa_code* memoryOffsetCode;
    oc_str8 init;

} wa_data_segment;

typedef enum wa_import_kind // this should be kept equal with export kinds
{
    WA_IMPORT_FUNCTION = 0x00,
    WA_IMPORT_TABLE = 0x01,
    WA_IMPORT_MEMORY = 0x02,
    WA_IMPORT_GLOBAL = 0x03,
} wa_import_kind;

typedef struct wa_import
{
    oc_str8 moduleName;
    oc_str8 importName;

    wa_import_kind kind;
    u32 index;
    wa_value_type type;
    wa_limits limits;
    bool mut;

} wa_import;

typedef enum wa_export_kind // this should be kept equal with import kinds
{
    WA_EXPORT_FUNCTION = 0x00,
    WA_EXPORT_TABLE = 0x01,
    WA_EXPORT_MEMORY = 0x02,
    WA_EXPORT_GLOBAL = 0x03,
} wa_export_kind;

typedef struct wa_export
{
    oc_str8 name;
    wa_export_kind kind;
    u32 index;

} wa_export;

typedef struct wa_ast_loc
{
    u64 start;
    u64 len;
} wa_ast_loc;

typedef enum
{
    WA_AST_U8,
    WA_AST_U32,
    WA_AST_I32,
    WA_AST_U64,
    WA_AST_I64,
    WA_AST_F32,
    WA_AST_F64,
    WA_AST_NAME,
    WA_AST_VECTOR,

    WA_AST_ROOT,
    WA_AST_MAGIC,
    WA_AST_SECTION,
    WA_AST_TYPE,
    WA_AST_FUNC_ENTRY,
    WA_AST_TYPE_INDEX,
    WA_AST_FUNC_INDEX,
    WA_AST_LIMITS,
    WA_AST_TABLE_TYPE,
    WA_AST_ELEMENT,
    WA_AST_DATA_SEGMENT,
    WA_AST_IMPORT,
    WA_AST_EXPORT,
    WA_AST_FUNC,
    WA_AST_LOCAL_ENTRY,
    WA_AST_FUNC_BODY,
    WA_AST_INSTR,
    WA_AST_MEM_ARG,

    WA_AST_VALUE_TYPE,
    WA_AST_GLOBAL,

    WA_AST_NAME_SUBSECTION,
    WA_AST_NAME_ENTRY,
} wa_ast_kind;

static const char* wa_ast_kind_strings[] = {
    "u8",
    "u32",
    "i32",
    "u64",
    "i64",
    "f32",
    "f64",
    "name",
    "vector",
    "root",
    "magic number",
    "section",
    "type",
    "function entry",
    "type index",
    "func index",
    "limits",
    "table type",
    "element",
    "data segment",
    "import",
    "export",
    "function",
    "local entry",
    "function body",
    "instruction",
    "memArg",
    "value type",
    "global",
    "name subsection",
    "name entry",
};

typedef struct wa_ast
{
    oc_list_elt parentElt;
    wa_ast* parent;
    oc_list children;

    wa_ast_loc loc;
    wa_ast_kind kind;
    oc_str8 label;

    oc_list errors;

    union
    {
        u8 valU8;
        u32 valU32;
        i32 valI32;
        u64 valU64;
        i64 valI64;
        f32 valF32;
        f64 valF64;
        wa_value_type valueType;
        oc_str8 str8;
        wa_instr* instr;
        wa_func* func;
        wa_func_type* type;
    };

} wa_ast;

typedef struct wa_section
{
    oc_list_elt listElt;
    u64 id;
    u64 len;
    u64 offset;
    wa_ast* ast;
    oc_str8 name;

} wa_section;

typedef struct dw_info dw_info;

typedef struct wa_name_entry
{
    u32 index;
    oc_str8 name;
} wa_name_entry;

typedef struct wa_wasm_to_line_entry
{
    u64 wasmOffset;
    wa_line_loc loc;
} wa_wasm_to_line_entry;

typedef struct wa_register_range
{
    u64 start;
    u64 end;
    wa_value_type type;
} wa_register_range;

typedef struct wa_register_map
{
    u32 count;
    wa_register_range* ranges;
} wa_register_map;

typedef enum wa_debug_type_kind
{
    WA_DEBUG_TYPE_VOID,
    WA_DEBUG_TYPE_BASIC,
    WA_DEBUG_TYPE_POINTER,
    WA_DEBUG_TYPE_STRUCT,
    WA_DEBUG_TYPE_UNION,
    WA_DEBUG_TYPE_ENUM,
    WA_DEBUG_TYPE_ARRAY,
    WA_DEBUG_TYPE_NAMED,
    //...
} wa_debug_type_kind;

typedef enum wa_debug_type_encoding
{
    WA_DEBUG_TYPE_SIGNED,
    WA_DEBUG_TYPE_UNSIGNED,
    WA_DEBUG_TYPE_FLOAT,
    WA_DEBUG_TYPE_BOOL,
} wa_debug_type_encoding;

typedef struct wa_debug_type wa_debug_type;

typedef struct wa_debug_type_field
{
    oc_list_elt listElt;
    oc_str8 name;
    wa_debug_type* type;
    u64 offset;
} wa_debug_type_field;

typedef struct wa_debug_type
{
    oc_list_elt listElt;
    u64 dwarfRef;
    oc_str8 name;
    wa_debug_type_kind kind;
    u64 size;

    union
    {
        wa_debug_type_encoding encoding;
        wa_debug_type* type;
        oc_list fields;

        struct
        {
            wa_debug_type* type;
            u64 count;
        } array;

        //TODO enum, etc...
    };
} wa_debug_type;

typedef struct dw_loc dw_loc;

typedef struct wa_debug_variable
{
    oc_str8 name;
    dw_loc* loc;
    wa_debug_type* type;
} wa_debug_variable;

typedef struct wa_debug_function
{
    dw_loc* frameBase;
    u32 count;
    wa_debug_variable* vars;
} wa_debug_function;

typedef struct wa_debug_info
{
    u32 warmToWasmMapLen;
    oc_list* warmToWasmMap;

    u32 wasmToWarmMapLen;
    oc_list* wasmToWarmMap;

    dw_info* dwarf;

    wa_source_info sourceInfo;

    u64 wasmToLineCount;
    wa_wasm_to_line_entry* wasmToLine;

    wa_register_map** registerMaps;

    wa_debug_function* functionLocals;

} wa_debug_info;

typedef struct wa_module_toc
{
    wa_section types;
    wa_section imports;
    wa_section functions;
    wa_section tables;
    wa_section memory;
    wa_section globals;
    wa_section exports;
    wa_section start;
    wa_section elements;
    wa_section dataCount;
    wa_section code;
    wa_section data;

    wa_section names;
    oc_list customSections;
} wa_module_toc;

typedef struct wa_module
{
    oc_arena* arena;
    oc_list errors;
    wa_ast* root;

    wa_module_toc toc;

    u32 functionNameCount;
    wa_name_entry* functionNames;

    u32 typeCount;
    wa_func_type* types;

    u32 importCount;
    wa_import* imports;

    u32 functionImportCount;
    u32 functionCount;
    wa_func* functions;

    bool hasStart;
    u32 startIndex;

    u32 globalImportCount;
    u32 globalCount;
    wa_global_desc* globals;

    u32 tableImportCount;
    u32 tableCount;
    wa_table_type* tables;

    u32 elementCount;
    wa_element* elements;

    u32 exportCount;
    wa_export* exports;

    u32 memoryImportCount;
    u32 memoryCount;
    wa_limits* memories;

    u32 dataCount;
    wa_data_segment* data;

    wa_debug_info debugInfo;

} wa_module;

enum
{
    WA_TYPE_STACK_MAX_LEN = 512,
    WA_CONTROL_STACK_MAX_LEN = 512,
    WA_MAX_REG = 4096,
    WA_MAX_SLOT_COUNT = 4096,
};

typedef struct wa_instance
{
    wa_status status;

    oc_arena* arena;
    wa_module* module;

    wa_func* functions;
    wa_global** globals;
    wa_table** tables;
    wa_memory** memories;

    wa_data_segment* data;
    wa_element* elements;
} wa_instance;
