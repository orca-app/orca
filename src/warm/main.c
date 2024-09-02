#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define OC_NO_APP_LAYER 1
#include "orca.h"

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

    WA_INSTR_COUNT,

} wa_instr_op;

typedef enum wa_value_type
{
    WA_TYPE_I32 = 0x7f,
    WA_TYPE_I64 = 0x7e,
    WA_TYPE_F32 = 0x7d,
    WA_TYPE_F64 = 0x7c,
    WA_TYPE_V128 = 0x7b,
    WA_TYPE_FUNC_REF = 0x70,
    WA_TYPE_EXTERN_REF = 0x6f,

    WA_TYPE_UNKNOWN = 0x100,
    WA_TYPE_ANY = 0x101,
    WA_TYPE_REF = 0x102,
    WA_TYPE_NUM_OR_VEC = 0x103,
}

wa_value_type;

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

typedef struct wa_instance wa_instance;

typedef union wa_value
{
    i32 valI32;
    i64 valI64;
    f32 valF32;
    f64 valF64;

    //TODO v128, funcref, externref...
    struct
    {
        wa_instance* refInstance;
        u32 refIndex;
    };
} wa_value;

typedef struct wa_typed_value
{
    wa_value_type type;
    wa_value value;

} wa_typed_value;

enum
{
    WA_INSTR_IMM_MAX_COUNT = 2,
};

typedef struct wa_func_type
{
    u32 paramCount;
    wa_value_type* params;

    u32 returnCount;
    wa_value_type* returns;

} wa_func_type;

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
} wa_instr;

typedef void (*wa_host_proc)(wa_value* args, wa_value* returns); //TODO: complete with memory, return status / etc

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

    wa_instance* extInstance;
    u32 extIndex;

} wa_func;

typedef struct wa_global
{
    wa_value_type type;
    bool mut;
    oc_list init;

    u32 codeLen;
    wa_code* code;
} wa_global;

typedef enum wa_limits_kind
{
    WA_LIMIT_MIN = 0,
    WA_LIMIT_MIN_MAX = 1,
} wa_limits_kind;

typedef struct wa_limits
{
    wa_limits_kind kind;
    u32 min;
    u32 max;

} wa_limits;

typedef struct wa_table_type
{
    wa_value_type type;
    wa_limits limits;
} wa_table_type;

typedef struct wa_table
{
    u64 size;
    wa_value* contents;
} wa_table;

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

typedef struct wa_memory
{
    wa_limits limits;
    u64 cap;
    u64 size;
    char* ptr;
} wa_memory;

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

typedef struct wa_loc
{
    u64 start;
    u64 len;
} wa_loc;

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
};

typedef struct wa_ast
{
    oc_list_elt parentElt;
    wa_ast* parent;
    oc_list children;

    wa_loc loc;
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

typedef struct wa_module_toc_entry
{
    u64 len;
    u64 offset;
    wa_ast* ast;
} wa_module_toc_entry;

typedef struct wa_module
{
    oc_list errors;
    wa_ast* root;

    struct
    {
        wa_module_toc_entry types;
        wa_module_toc_entry imports;
        wa_module_toc_entry functions;
        wa_module_toc_entry tables;
        wa_module_toc_entry memory;
        wa_module_toc_entry globals;
        wa_module_toc_entry exports;
        wa_module_toc_entry start;
        wa_module_toc_entry elements;
        wa_module_toc_entry dataCount;
        wa_module_toc_entry code;
        wa_module_toc_entry data;
    } toc;

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
    wa_global* globals;

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

} wa_module;

typedef struct wa_parser
{
    oc_arena* arena;
    wa_module* module;
    char* contents;
    u64 len;
    u64 offset;

} wa_parser;

static const wa_value_type WA_BLOCK_TYPE_I32_VAL = WA_TYPE_I32;
static const wa_value_type WA_BLOCK_TYPE_I64_VAL = WA_TYPE_I64;
static const wa_value_type WA_BLOCK_TYPE_F32_VAL = WA_TYPE_F32;
static const wa_value_type WA_BLOCK_TYPE_F64_VAL = WA_TYPE_F64;
static const wa_value_type WA_BLOCK_TYPE_V128_VAL = WA_TYPE_V128;
static const wa_value_type WA_BLOCK_TYPE_FUNC_REF_VAL = WA_TYPE_FUNC_REF;
static const wa_value_type WA_BLOCK_TYPE_EXTERN_REF_VAL = WA_TYPE_EXTERN_REF;

static const wa_func_type WA_BLOCK_VALUE_TYPES[] = {
    [0] = { 0 },
    [1] = {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_I32_VAL,
    },
    [2] = {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_I64_VAL,
    },
    [3] = {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_F32_VAL,
    },
    [4] = {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_F64_VAL,
    },
    [5] = {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_V128_VAL,
    },
    [16] = {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_FUNC_REF_VAL,
    },
    [17] = {
        .returnCount = 1,
        .returns = (wa_value_type*)&WA_BLOCK_TYPE_EXTERN_REF_VAL,
    },
};

bool wa_is_value_type(u64 t)
{
    switch(t)
    {
        case WA_TYPE_I32:
        case WA_TYPE_I64:
        case WA_TYPE_F32:
        case WA_TYPE_F64:
        case WA_TYPE_V128:
        case WA_TYPE_FUNC_REF:
        case WA_TYPE_EXTERN_REF:
            return true;
        default:
            return false;
    }
}

bool wa_is_value_type_numeric(u64 t)
{
    switch(t)
    {
        case WA_TYPE_UNKNOWN:
        case WA_TYPE_I32:
        case WA_TYPE_I64:
        case WA_TYPE_F32:
        case WA_TYPE_F64:
            return true;
        default:
            return false;
    }
}

const char* wa_value_type_string(wa_value_type t)
{
    switch(t)
    {
        case WA_TYPE_I32:
            return "i32";
        case WA_TYPE_I64:
            return "i64";
        case WA_TYPE_F32:
            return "f32";
        case WA_TYPE_F64:
            return "f64";
        case WA_TYPE_V128:
            return "vec128";
        case WA_TYPE_FUNC_REF:
            return "funcref";
        case WA_TYPE_EXTERN_REF:
            return "externref";
        default:
            return "invalid type";
    }
}

#include "wasm_tables.c"

enum
{
    WA_PAGE_SIZE = 64 * 1 << 10,
};

//-------------------------------------------------------------------------
// errors
//-------------------------------------------------------------------------

/*
examples of assert_invalid strings:

type mismatch
unknown global
unknown local
unknown function
unknown label
unknown elem segment ...
unknown table ...

*/

typedef enum wa_status
{
    WA_OK = 0,

    WA_PARSE_ERROR,

    WA_VALIDATION_TYPE_MISMATCH,
    WA_VALIDATION_INVALID_TYPE,
    WA_VALIDATION_INVALID_FUNCTION,
    WA_VALIDATION_INVALID_GLOBAL,
    WA_VALIDATION_INVALID_LOCAL,
    WA_VALIDATION_INVALID_TABLE,
    WA_VALIDATION_INVALID_MEMORY,
    /*...*/

    WA_ERROR_INVALID_ARGS,
    WA_FAIL_MISSING_IMPORT,
    WA_FAIL_IMPORT_TYPE_MISMATCH,

    WA_TRAP_UNREACHABLE,
    WA_TRAP_INVALID_OP,
    WA_TRAP_DIVIDE_BY_ZERO,
    WA_TRAP_INTEGER_OVERFLOW,
    WA_TRAP_INVALID_INTEGER_CONVERSION,
    WA_TRAP_STACK_OVERFLOW,
    WA_TRAP_MEMORY_OUT_OF_BOUNDS,
    WA_TRAP_TABLE_OUT_OF_BOUNDS,
    WA_TRAP_REF_NULL,
    WA_TRAP_INDIRECT_CALL_TYPE_MISMATCH,
} wa_status;

static const char* wa_status_strings[] = {
    "ok",
    "parse error",
    "type mismatch",
    "invalid type index",
    "invalid function index",
    "invalid global index",
    "invalid local index",
    "invalid table index",
    "invalid memory index",
    "invalid arguments",
    "binding type mismatch",
    "missing import",

    "unreachable",
    "invalid opcode",
    "divide by zero",
    "integer overflow",
    "invalid integer conversion",
    "stack overflow",
    "out-of-bounds memory access",
    "null reference",
};

typedef struct wa_module_error
{
    oc_list_elt moduleElt;
    oc_list_elt astElt;

    wa_ast* ast;
    bool blockEnd;

    wa_status status;
    oc_str8 string;

} wa_module_error;

void wa_parse_error(wa_parser* parser, wa_ast* ast, const char* fmt, ...)
{
    wa_module_error* error = oc_arena_push_type(parser->arena, wa_module_error);

    error->status = WA_PARSE_ERROR;

    va_list ap;
    va_start(ap, fmt);
    error->string = oc_str8_pushfv(parser->arena, fmt, ap);
    va_end(ap);

    error->ast = ast;

    oc_list_push_back(&parser->module->errors, &error->moduleElt);
    oc_list_push_back(&ast->errors, &error->astElt);
}

void wa_module_print_errors(wa_module* module)
{
    oc_list_for(module->errors, err, wa_module_error, moduleElt)
    {
        printf("%.*s", oc_str8_ip(err->string));
    }
}

bool wa_ast_has_errors(wa_ast* ast)
{
    return (!oc_list_empty(ast->errors));
}

//-------------------------------------------------------------------------
// Input
//-------------------------------------------------------------------------

wa_ast* wa_ast_alloc(wa_parser* parser, wa_ast_kind kind)
{
    wa_ast* ast = oc_arena_push_type(parser->arena, wa_ast);
    memset(ast, 0, sizeof(wa_ast));

    ast->kind = kind;

    return (ast);
}

void wa_ast_add_child(wa_ast* parent, wa_ast* child)
{
    child->parent = parent;
    oc_list_push_back(&parent->children, &child->parentElt);
}

wa_ast* wa_ast_begin(wa_parser* parser, wa_ast* parent, wa_ast_kind kind)
{
    wa_ast* ast = oc_arena_push_type(parser->arena, wa_ast);
    memset(ast, 0, sizeof(wa_ast));

    ast->kind = kind;
    ast->loc.start = parser->offset;

    if(parent)
    {
        wa_ast_add_child(parent, ast);
    }

    return ast;
}

void wa_ast_end(wa_parser* parser, wa_ast* ast)
{
    ast->loc.len = parser->offset - ast->loc.start;
}

char* wa_parser_head(wa_parser* parser)
{
    return (parser->contents + parser->offset);
}

bool wa_parser_end(wa_parser* parser)
{
    return (parser->offset >= parser->len);
}

void wa_parser_seek(wa_parser* parser, u64 offset, oc_str8 label)
{
    parser->offset = offset;
}

wa_ast* wa_read_byte(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_U8);
    ast->label = label;

    if(parser->offset + sizeof(u8) > parser->len)
    {
        wa_parse_error(parser,
                       ast,
                       "Couldn't read %.*s: unexpected end of parser.\n",
                       oc_str8_ip(label));
    }
    else
    {
        ast->valU8 = *(u8*)&parser->contents[parser->offset];
        parser->offset += sizeof(u8);
    }

    wa_ast_end(parser, ast);
    return (ast);
}

wa_ast* wa_read_raw_u32(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_U32);
    ast->label = label;

    if(parser->offset + sizeof(u32) > parser->len)
    {
        wa_parse_error(parser,
                       ast,
                       "Couldn't read %.*s: unexpected end of parser.\n",
                       oc_str8_ip(label));
    }
    else
    {
        ast->valU32 = *(u32*)&parser->contents[parser->offset];
        parser->offset += sizeof(u32);
    }

    wa_ast_end(parser, ast);
    return (ast);
}

wa_ast* wa_read_leb128_u64(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_U64);
    ast->label = label;

    char byte = 0;
    u64 shift = 0;
    u64 res = 0;

    do
    {
        if(parser->offset + sizeof(char) > parser->len)
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: unexpected end of parser.\n",
                           oc_str8_ip(label));
            res = 0;
            break;
        }

        byte = parser->contents[parser->offset];
        parser->offset++;

        if(shift >= 64)
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: leb128 overflow.\n",
                           oc_str8_ip(label));
            res = 0;
            break;
        }

        res |= ((u64)byte & 0x7f) << shift;
        shift += 7;
    }
    while(byte & 0x80);

    ast->valU64 = res;
    wa_ast_end(parser, ast);

    return (ast);
}

wa_ast* wa_read_leb128_i64(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_I64);
    ast->label = label;

    char byte = 0;
    u64 shift = 0;
    i64 res = 0;

    do
    {
        if(parser->offset + sizeof(char) > parser->len)
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: unexpected end of parser.\n",
                           oc_str8_ip(label));
            byte = 0;
            res = 0;
            break;
        }
        byte = parser->contents[parser->offset];
        parser->offset++;

        if(shift >= 64)
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: leb128 overflow.\n",
                           oc_str8_ip(label));
            byte = 0;
            res = 0;
            break;
        }

        res |= ((u64)byte & 0x7f) << shift;
        shift += 7;
    }
    while(byte & 0x80);

    if(shift < 64 && (byte & 0x40))
    {
        res |= (~0ULL << shift);
    }

    ast->valI64 = res;

    wa_ast_end(parser, ast);
    return (ast);
}

wa_ast* wa_read_leb128_u32(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_read_leb128_u64(parser, parent, label);
    ast->kind = WA_AST_U32;
    ast->label = label;

    if(ast->valU64 > UINT32_MAX)
    {
        ast->valU64 = 0;
        if(!wa_ast_has_errors(ast))
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: leb128 overflow.\n",
                           oc_str8_ip(label));
        }
    }
    else
    {
        ast->valU32 = (u32)ast->valU64;
    }
    return ast;
}

wa_ast* wa_read_leb128_i32(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_read_leb128_i64(parser, parent, label);
    ast->kind = WA_AST_I32;
    ast->label = label;

    if(ast->valI64 > INT32_MAX || ast->valI64 < INT32_MIN)
    {
        ast->valI64 = 0;
        if(!wa_ast_has_errors(ast))
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: leb128 overflow.\n",
                           oc_str8_ip(label));
        }
    }
    else
    {
        ast->valI32 = (i32)ast->valI64;
    }
    return ast;
}

wa_ast* wa_read_f32(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_F32);
    ast->label = label;

    if(parser->offset + sizeof(f32) > parser->len)
    {
        wa_parse_error(parser,
                       ast,
                       "Couldn't read %.*s: unexpected end of parser.\n",
                       oc_str8_ip(label));
    }
    else
    {
        ast->valF32 = *(f32*)&parser->contents[parser->offset];
        parser->offset += sizeof(f32);
    }

    wa_ast_end(parser, ast);
    return ast;
}

wa_ast* wa_read_f64(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_F64);
    ast->label = label;

    if(parser->offset + sizeof(f64) > parser->len)
    {
        wa_parse_error(parser,
                       ast,
                       "Couldn't read %.*s: unexpected end of parser.\n",
                       oc_str8_ip(label));
    }
    else
    {
        ast->valF64 = *(f64*)&parser->contents[parser->offset];
        parser->offset += sizeof(f64);
    }

    wa_ast_end(parser, ast);
    return ast;
}

wa_ast* wa_read_name(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_NAME);
    ast->label = label;

    wa_ast* lenAst = wa_read_leb128_u32(parser, ast, label);
    if(!wa_ast_has_errors(lenAst))
    {
        u32 len = lenAst->valU32;

        if(parser->offset + len > parser->len)
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: unexpected end of parser.\n",
                           oc_str8_ip(label));
        }
        else
        {
            ast->str8 = oc_str8_push_buffer(parser->arena, len, &parser->contents[parser->offset]);
            parser->offset += len;
        }
    }
    wa_ast_end(parser, ast);
    return (ast);
}

wa_ast* wa_parse_value_type(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_read_leb128_u32(parser, parent, label);
    ast->kind = WA_AST_VALUE_TYPE;
    ast->valueType = (wa_value_type)ast->valU32;

    if(!wa_is_value_type(ast->valU32))
    {
        wa_parse_error(parser,
                       ast,
                       "unrecognized value type 0x%02x\n",
                       ast->valU32);
    }
    return (ast);
}

void wa_parse_sections(wa_parser* parser, wa_module* module)
{
    while(!wa_parser_end(parser))
    {
        wa_ast* section = wa_ast_begin(parser, module->root, WA_AST_SECTION);
        wa_ast* sectionID = wa_read_byte(parser, section, OC_STR8("section ID"));
        if(wa_ast_has_errors(sectionID))
        {
            return;
        }

        wa_ast* sectionLen = wa_read_leb128_u32(parser, section, OC_STR8("section length"));
        if(wa_ast_has_errors(sectionLen))
        {
            return;
        }

        u64 contentOffset = parser->offset;

        wa_module_toc_entry* entry = 0;
        switch(sectionID->valU8)
        {
            case 0:
                break;
                //TODO: check if section was already defined...

            case 1:
                entry = &module->toc.types;
                section->label = OC_STR8("Types section");
                break;

            case 2:
                entry = &module->toc.imports;
                section->label = OC_STR8("Imports section");
                break;

            case 3:
                entry = &module->toc.functions;
                section->label = OC_STR8("Functions section");
                break;

            case 4:
                entry = &module->toc.tables;
                section->label = OC_STR8("Tables section");
                break;

            case 5:
                entry = &module->toc.memory;
                section->label = OC_STR8("Memory section");
                break;

            case 6:
                entry = &module->toc.globals;
                section->label = OC_STR8("Globals section");
                break;

            case 7:
                entry = &module->toc.exports;
                section->label = OC_STR8("Exports section");
                break;

            case 8:
                entry = &module->toc.start;
                section->label = OC_STR8("Start section");
                break;

            case 9:
                entry = &module->toc.elements;
                section->label = OC_STR8("Elements section");
                break;

            case 10:
                entry = &module->toc.code;
                section->label = OC_STR8("Code section");
                break;

            case 11:
                entry = &module->toc.data;
                section->label = OC_STR8("Data section");
                break;

            case 12:
                entry = &module->toc.dataCount;
                section->label = OC_STR8("Data count section");
                break;

            default:
            {
                wa_parse_error(parser,
                               section,
                               "Unknown section identifier %i.\n",
                               sectionID);
            }
            break;
        }

        if(entry)
        {
            entry->offset = parser->offset;
            entry->len = sectionLen->valU32;
            entry->ast = section;
        }
        wa_parser_seek(parser, contentOffset + sectionLen->valU32, OC_STR8("next section"));
        wa_ast_end(parser, section);
    }
}

wa_ast* wa_ast_begin_vector(wa_parser* parser, wa_ast* parent, u32* count)
{
    wa_ast* vectorAst = wa_ast_begin(parser, parent, WA_AST_VECTOR);
    wa_ast* countAst = wa_read_leb128_u32(parser, vectorAst, OC_STR8("count"));

    if(wa_ast_has_errors(countAst))
    {
        *count = 0;
    }
    else
    {
        *count = countAst->valU32;
    }
    return vectorAst;
}

void wa_parse_types(wa_parser* parser, wa_module* module)
{
    //NOTE: parse types section
    wa_ast* section = module->toc.types.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.types.offset, OC_STR8("types section"));
    u64 startOffset = parser->offset;

    wa_ast* vector = wa_ast_begin_vector(parser, section, &module->typeCount);

    module->types = oc_arena_push_array(parser->arena, wa_func_type, module->typeCount);

    for(u32 typeIndex = 0; typeIndex < module->typeCount; typeIndex++)
    {
        wa_func_type* type = &module->types[typeIndex];

        wa_ast* typeAst = wa_ast_begin(parser, vector, WA_AST_TYPE);
        typeAst->type = type;

        wa_ast* b = wa_read_byte(parser, typeAst, OC_STR8("type prefix"));

        if(b->valU8 != 0x60)
        {
            wa_parse_error(parser,
                           b,
                           "Unexpected prefix 0x%02x for function type.\n",
                           b->valU8);
            return;
        }

        wa_ast* paramCountAst = wa_read_leb128_u32(parser, typeAst, OC_STR8("parameter count"));
        type->paramCount = paramCountAst->valU32;
        type->params = oc_arena_push_array(parser->arena, wa_value_type, type->paramCount);

        for(u32 typeIndex = 0; typeIndex < type->paramCount; typeIndex++)
        {
            wa_ast* paramAst = wa_parse_value_type(parser, typeAst, OC_STR8("parameter type"));
            type->params[typeIndex] = paramAst->valU32;
        }

        wa_ast* returnCountAst = wa_read_leb128_u32(parser, typeAst, OC_STR8("return count"));
        type->returnCount = returnCountAst->valU32;
        type->returns = oc_arena_push_array(parser->arena, wa_value_type, type->returnCount);

        for(u32 typeIndex = 0; typeIndex < type->returnCount; typeIndex++)
        {
            wa_ast* returnAst = wa_parse_value_type(parser, typeAst, OC_STR8("return type"));
            type->returns[typeIndex] = returnAst->valU32;
        }

        wa_ast_end(parser, typeAst);
    }
    wa_ast_end(parser, vector);

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.types.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.types.len,
                       parser->offset - startOffset);
    }
}

wa_ast* wa_parse_limits(wa_parser* parser, wa_ast* parent, wa_limits* limits)
{
    wa_ast* limitsAst = wa_ast_begin(parser, parent, WA_AST_LIMITS);
    wa_ast* kindAst = wa_read_byte(parser, limitsAst, OC_STR8("limit kind"));
    u8 kind = kindAst->valU8;

    if(kind != WA_LIMIT_MIN && kind != WA_LIMIT_MIN_MAX)
    {
        wa_parse_error(parser,
                       kindAst,
                       "Invalid limit kind 0x%02x\n",
                       kind);
    }
    else
    {
        limits->kind = kind;

        wa_ast* minAst = wa_read_leb128_u32(parser, limitsAst, OC_STR8("min limit"));
        limits->min = minAst->valU32;

        if(limits->kind == WA_LIMIT_MIN_MAX)
        {
            wa_ast* maxAst = wa_read_leb128_u32(parser, limitsAst, OC_STR8("max limit"));
            limits->max = maxAst->valU32;
        }
    }
    wa_ast_end(parser, limitsAst);
    return limitsAst;
}

void wa_parse_imports(wa_parser* parser, wa_module* module)
{
    //NOTE: parse import section
    wa_ast* section = module->toc.imports.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.imports.offset, OC_STR8("import section"));
    u64 startOffset = parser->offset;

    wa_ast* vector = wa_ast_begin_vector(parser, section, &module->importCount);

    module->imports = oc_arena_push_array(parser->arena, wa_import, module->importCount);

    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];

        wa_ast* importAst = wa_ast_begin(parser, vector, WA_AST_IMPORT);
        wa_ast* moduleNameAst = wa_read_name(parser, importAst, OC_STR8("module name"));
        wa_ast* importNameAst = wa_read_name(parser, importAst, OC_STR8("import name"));
        wa_ast* kindAst = wa_read_byte(parser, importAst, OC_STR8("import kind"));

        import->moduleName = moduleNameAst->str8;
        import->importName = importNameAst->str8;
        import->kind = kindAst->valU8;

        switch((u32)import->kind)
        {
            case WA_IMPORT_FUNCTION:
            {
                wa_ast* indexAst = wa_read_leb128_u32(parser, importAst, OC_STR8("type index"));
                indexAst->kind = WA_AST_TYPE_INDEX;
                import->index = indexAst->valU8;

                if(import->index >= module->typeCount)
                {
                    wa_parse_error(parser,
                                   indexAst,
                                   "Out of bounds type index in function import (type count: %u, got index %u)\n",
                                   module->functionCount,
                                   import->index);
                }
                module->functionImportCount++;
            }
            break;
            case WA_IMPORT_TABLE:
            {
                wa_ast* typeAst = wa_read_byte(parser, importAst, OC_STR8("table type"));
                import->type = typeAst->valU8;

                if(import->type != WA_TYPE_FUNC_REF && import->type != WA_TYPE_EXTERN_REF)
                {
                    wa_parse_error(parser,
                                   typeAst,
                                   "Invalid type 0x%02x in table import \n",
                                   import->type);
                }
                wa_ast* limitsAst = wa_parse_limits(parser, importAst, &import->limits);
                module->tableImportCount++;
            }
            break;

            case WA_IMPORT_MEMORY:
            {
                wa_ast* limitAst = wa_parse_limits(parser, importAst, &import->limits);
                module->memoryImportCount++;
            }
            break;

            case WA_IMPORT_GLOBAL:
            {
                //TODO: coalesce with globals section parsing

                wa_ast* typeAst = wa_parse_value_type(parser, importAst, OC_STR8("type"));
                wa_ast* mutAst = wa_read_byte(parser, importAst, OC_STR8("mut"));

                import->type = typeAst->valueType;

                if(mutAst->valU8 == 0x00)
                {
                    import->mut = false;
                }
                else if(mutAst->valU8 == 0x01)
                {
                    import->mut = true;
                }
                else
                {
                    wa_parse_error(parser,
                                   mutAst,
                                   "invalid byte 0x%02hhx as global mutability.",
                                   mutAst->valU8);
                }
                module->globalImportCount++;
            }
            break;
            default:
            {
                wa_parse_error(parser,
                               importAst,
                               "Unknown import kind 0x%02x\n",
                               (u8)import->kind);
                return;
            }
        }
        wa_ast_end(parser, importAst);
    }
    wa_ast_end(parser, vector);

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.imports.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.imports.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_functions(wa_parser* parser, wa_module* module)
{
    //NOTE: parse function section
    wa_parser_seek(parser, module->toc.functions.offset, OC_STR8("functions section"));
    u64 startOffset = parser->offset;

    wa_ast* section = module->toc.functions.ast;
    wa_ast* vector = 0;
    if(section)
    {
        vector = wa_ast_begin_vector(parser, section, &module->functionCount);
    }

    module->functions = oc_arena_push_array(parser->arena, wa_func, module->functionImportCount + module->functionCount);
    memset(module->functions, 0, (module->functionImportCount + module->functionCount) * sizeof(wa_func));

    //NOTE: re-read imports, because the format is kinda stupid -- they should have included imports in the function section
    u32 funcImportIndex = 0;
    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];
        if(import->kind == WA_IMPORT_FUNCTION)
        {
            wa_func* func = &module->functions[funcImportIndex];
            func->type = &module->types[import->index];
            func->import = import;
            funcImportIndex++;
        }
    }

    //NOTE: read non-import functions
    if(section)
    {
        for(u32 funcIndex = 0; funcIndex < module->functionCount; funcIndex++)
        {
            wa_func* func = &module->functions[module->functionImportCount + funcIndex];

            wa_ast* typeIndexAst = wa_read_leb128_u32(parser, vector, OC_STR8("type index"));
            typeIndexAst->kind = WA_AST_FUNC_ENTRY;
            u32 typeIndex = typeIndexAst->valU32;

            if(typeIndex >= module->typeCount)
            {
                wa_parse_error(parser,
                               typeIndexAst,
                               "Invalid type index %i in function section\n",
                               typeIndex);
            }
            else
            {
                func->type = &module->types[typeIndex];
            }
        }
        wa_ast_end(parser, vector);
    }
    module->functionCount += module->functionImportCount;

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.functions.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.functions.len,
                       parser->offset - startOffset);
    }
}

wa_ast* wa_parse_constant_expression(wa_parser* parser, wa_ast* parent, oc_list* list);

void wa_parse_globals(wa_parser* parser, wa_module* module)
{
    //NOTE: parse global section
    wa_parser_seek(parser, module->toc.globals.offset, OC_STR8("globals section"));
    u64 startOffset = parser->offset;

    wa_ast* section = module->toc.globals.ast;
    wa_ast* vector = 0;
    if(section)
    {
        vector = wa_ast_begin_vector(parser, section, &module->globalCount);
    }

    module->globals = oc_arena_push_array(parser->arena, wa_global, module->globalCount + module->globalImportCount);
    memset(module->globals, 0, (module->globalCount + module->globalImportCount) * sizeof(wa_global));

    //NOTE: re-read imports, because the format is kinda stupid -- they should have included imports in the global section
    u32 globalImportIndex = 0;
    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];
        if(import->kind == WA_IMPORT_GLOBAL)
        {
            wa_global* global = &module->globals[globalImportIndex];
            global->type = import->type;
            global->mut = import->mut;
            globalImportIndex++;
        }
    }

    if(section)
    {
        for(u32 globalIndex = 0; globalIndex < module->globalCount; globalIndex++)
        {
            wa_global* global = &module->globals[globalIndex + module->globalImportCount];

            wa_ast* globalAst = wa_ast_begin(parser, vector, WA_AST_GLOBAL);
            wa_ast* typeAst = wa_parse_value_type(parser, globalAst, OC_STR8("type"));
            wa_ast* mutAst = wa_read_byte(parser, globalAst, OC_STR8("mut"));

            global->type = typeAst->valueType;

            if(mutAst->valU8 == 0x00)
            {
                global->mut = false;
            }
            else if(mutAst->valU8 == 0x01)
            {
                global->mut = true;
            }
            else
            {
                wa_parse_error(parser,
                               mutAst,
                               "invalid byte 0x%02hhx as global mutability.",
                               mutAst->valU8);
            }
            wa_parse_constant_expression(parser, globalAst, &global->init);

            wa_ast_end(parser, globalAst);
        }
        wa_ast_end(parser, vector);
    }
    module->globalCount += module->globalImportCount;

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.globals.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.globals.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_tables(wa_parser* parser, wa_module* module)
{
    //NOTE: parse table section
    wa_parser_seek(parser, module->toc.tables.offset, OC_STR8("tables section"));
    u64 startOffset = parser->offset;

    wa_ast* section = module->toc.tables.ast;
    wa_ast* vector = 0;

    if(section)
    {
        vector = wa_ast_begin_vector(parser, section, &module->tableCount);
    }

    module->tables = oc_arena_push_array(parser->arena, wa_table_type, module->tableImportCount + module->tableCount);
    memset(module->tables, 0, (module->tableImportCount + module->tableCount) * sizeof(wa_table_type));

    //NOTE: re-read imports, because the format is kinda stupid -- they should have included imports in the tables section
    u32 tableImportIndex = 0;
    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];
        if(import->kind == WA_IMPORT_TABLE)
        {
            wa_table_type* table = &module->tables[tableImportIndex];
            table->type = import->type;
            table->limits = import->limits;
            tableImportIndex++;
        }
    }

    if(section)
    {
        //NOTE: read non-import tables
        for(u32 tableIndex = 0; tableIndex < module->tableCount; tableIndex++)
        {
            wa_table_type* table = &module->tables[tableIndex + module->tableImportCount];

            wa_ast* tableAst = wa_ast_begin(parser, vector, WA_AST_TABLE_TYPE);

            //TODO coalesce with parsing of table in imports
            wa_ast* typeAst = wa_read_byte(parser, tableAst, OC_STR8("table type"));
            table->type = typeAst->valU8;

            if(table->type != WA_TYPE_FUNC_REF && table->type != WA_TYPE_EXTERN_REF)
            {
                wa_parse_error(parser,
                               typeAst,
                               "Invalid type 0x%02x in table import \n",
                               table->type);
            }
            wa_ast* limitsAst = wa_parse_limits(parser, tableAst, &table->limits);

            wa_ast_end(parser, tableAst);
        }
        wa_ast_end(parser, vector);
    }
    module->tableCount += module->tableImportCount;

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.tables.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.tables.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_memories(wa_parser* parser, wa_module* module)
{
    //NOTE: parse table section
    wa_parser_seek(parser, module->toc.memory.offset, OC_STR8("memory section"));
    u64 startOffset = parser->offset;

    wa_ast* section = module->toc.memory.ast;
    wa_ast* vector = 0;

    if(section)
    {
        vector = wa_ast_begin_vector(parser, section, &module->memoryCount);
    }

    module->memories = oc_arena_push_array(parser->arena, wa_limits, module->memoryImportCount + module->memoryCount);
    memset(module->memories, 0, (module->memoryImportCount + module->memoryCount) * sizeof(wa_limits));

    //NOTE: re-read imports, because the format is kinda stupid -- they should have included imports in the memories section
    u32 memoryImportIndex = 0;
    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];
        if(import->kind == WA_IMPORT_MEMORY)
        {
            module->memories[memoryImportIndex] = import->limits;
            memoryImportIndex++;
        }
    }

    if(section)
    {
        //NOTE: read non-import memories
        for(u32 memoryIndex = 0; memoryIndex < module->memoryCount; memoryIndex++)
        {
            wa_limits* memory = &module->memories[memoryIndex + module->memoryImportCount];
            wa_ast* memoryAst = wa_parse_limits(parser, vector, memory);
        }
        wa_ast_end(parser, vector);
    }
    module->memoryCount += module->memoryImportCount;

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.memory.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.memory.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_exports(wa_parser* parser, wa_module* module)
{
    //NOTE: parse export section
    wa_ast* section = module->toc.exports.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.exports.offset, OC_STR8("exports section"));
    u64 startOffset = parser->offset;

    wa_ast* vector = wa_ast_begin_vector(parser, section, &module->exportCount);

    module->exports = oc_arena_push_array(parser->arena, wa_export, module->exportCount);

    for(u32 exportIndex = 0; exportIndex < module->exportCount; exportIndex++)
    {
        wa_export* export = &module->exports[exportIndex];

        wa_ast* exportAst = wa_ast_begin(parser, vector, WA_AST_EXPORT);
        wa_ast* nameAst = wa_read_name(parser, exportAst, OC_STR8("export name"));
        wa_ast* kindAst = wa_read_byte(parser, exportAst, OC_STR8("export kind"));
        wa_ast* indexAst = wa_read_leb128_u32(parser, exportAst, OC_STR8("export index"));

        export->name = nameAst->str8;
        export->kind = kindAst->valU8;
        export->index = indexAst->valU32;

        switch((u32) export->kind)
        {
            case WA_EXPORT_FUNCTION:
            {
                if(export->index >= module->functionCount)
                {
                    wa_parse_error(parser,
                                   indexAst,
                                   "Invalid type index in function export (function count: %u, got index %u)\n",
                                   module->functionCount,
                                   export->index);
                }
            }
            break;
            case WA_EXPORT_TABLE:
            {
                //TODO
            }
            break;
            case WA_EXPORT_MEMORY:
            {
                //TODO
            }
            break;
            case WA_EXPORT_GLOBAL:
            {
                //TODO
            }
            break;
            default:
            {
                wa_parse_error(parser,
                               kindAst,
                               "Unknown export kind 0x%02x\n",
                               (u8) export->kind);
                //TODO end parsing section?
            }
        }
        wa_ast_end(parser, exportAst);
    }
    wa_ast_end(parser, vector);

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.exports.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.exports.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_start(wa_parser* parser, wa_module* module)
{
    //NOTE: parse export section
    wa_ast* section = module->toc.start.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.start.offset, OC_STR8("start section"));
    u64 startOffset = parser->offset;

    wa_ast* startAst = wa_read_leb128_u32(parser, section, OC_STR8("start index"));

    module->hasStart = true;
    module->startIndex = startAst->valU32;

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.start.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.start.len,
                       parser->offset - startOffset);
    }
}

wa_ast* wa_parse_expression(wa_parser* parser, wa_ast* parent, u32 localCount, oc_list* list, bool constant)
{
    wa_module* module = parser->module;

    wa_ast* exprAst = wa_ast_begin(parser, parent, WA_AST_FUNC_BODY);

    //TODO: we should validate block nesting here?

    i64 blockDepth = 0;

    while(!wa_parser_end(parser) && blockDepth >= 0)
    {
        wa_instr* instr = oc_arena_push_type(parser->arena, wa_instr);
        memset(instr, 0, sizeof(wa_instr));
        oc_list_push_back(list, &instr->listElt);

        wa_ast* instrAst = wa_ast_begin(parser, exprAst, WA_AST_INSTR);
        instrAst->instr = instr;

        instr->ast = instrAst;

        wa_ast* byteAst = wa_read_byte(parser, instrAst, OC_STR8("opcode"));
        u8 byte = byteAst->valU8;

        if(byte == WA_INSTR_PREFIX_EXTENDED)
        {
            wa_ast* extAst = wa_read_leb128_u32(parser, instrAst, OC_STR8("extended instruction"));
            u32 code = extAst->valU32;

            if(code >= wa_instr_decode_extended_len)
            {
                wa_parse_error(parser,
                               extAst,
                               "Invalid extended instruction %i\n",
                               code);
                break;
            }
            instr->op = wa_instr_decode_extended[code];
        }
        else if(byte == WA_INSTR_PREFIX_VECTOR)
        {
            wa_ast* extAst = wa_read_leb128_u32(parser, instrAst, OC_STR8("vector instruction"));
            u32 code = extAst->valU32;

            if(code >= wa_instr_decode_vector_len)
            {
                wa_parse_error(parser,
                               extAst,
                               "Invalid vector instruction %i\n",
                               code);
                break;
            }
            instr->op = wa_instr_decode_vector[code];
        }
        else
        {
            if(byte >= wa_instr_decode_basic_len)
            {
                wa_parse_error(parser,
                               byteAst,
                               "Invalid basic instruction 0x%02x\n",
                               byte);
                break;
            }
            instr->op = wa_instr_decode_basic[byte];
        }

        const wa_instr_info* info = &wa_instr_infos[instr->op];

        if(!info->defined)
        {
            wa_parse_error(parser, instrAst, "undefined instruction %s.\n", wa_instr_strings[instr->op]);
            break;
        }

        if(constant)
        {
            if(instr->op != WA_INSTR_i32_const
               && instr->op != WA_INSTR_i64_const
               && instr->op != WA_INSTR_f32_const
               && instr->op != WA_INSTR_f64_const
               && instr->op != WA_INSTR_ref_null
               && instr->op != WA_INSTR_ref_func
               && instr->op != WA_INSTR_global_get
               && instr->op != WA_INSTR_end)
            {
                wa_parse_error(parser,
                               instrAst,
                               "found non-constant instruction %s while parsing constant expression.\n",
                               wa_instr_strings[instr->op]);
            }
            //TODO add constraint on global get
        }

        //NOTE: parse immediates, special cases first, then generic

        if(instr->op == WA_INSTR_block
           || instr->op == WA_INSTR_loop
           || instr->op == WA_INSTR_if)
        {
            //NOTE: parse block type
            wa_ast* blockTypeAst = wa_read_leb128_i64(parser, instrAst, OC_STR8("block type"));
            i64 t = blockTypeAst->valI64;
            if(t >= 0)
            {
                u64 typeIndex = (u64)t;

                if(typeIndex >= module->typeCount)
                {
                    wa_parse_error(parser,
                                   blockTypeAst,
                                   "unexpected type index %u (type count: %u)\n",
                                   typeIndex,
                                   module->typeCount);
                    break;
                }
                instr->blockType = &module->types[typeIndex];
            }
            else
            {
                if(t != -64 && !wa_is_value_type(t & 0x7f))
                {
                    wa_parse_error(parser,
                                   blockTypeAst,
                                   "unrecognized value type 0x%02hhx\n",
                                   t & 0x7f);
                    break;
                }
                t = (t == -64) ? 0 : -t;

                instr->blockType = (wa_func_type*)&WA_BLOCK_VALUE_TYPES[t];
            }
            blockDepth++;
        }
        else if(instr->op == WA_INSTR_end)
        {
            blockDepth--;
        }
        else if(instr->op == WA_INSTR_select_t)
        {
            wa_ast* vector = wa_ast_begin_vector(parser, instrAst, &instr->immCount);

            if(instr->immCount != 1)
            {
                //TODO: should set the error on the count rather than the vector?
                wa_parse_error(parser,
                               vector,
                               "select instruction can have at most one immediate\n");
                break;
            }

            wa_ast* immAst = wa_parse_value_type(parser, vector, OC_STR8("type"));

            instr->imm = oc_arena_push_type(parser->arena, wa_code);
            instr->imm[0].valueType = immAst->valU32;

            wa_ast_end(parser, vector);
        }
        else if(instr->op == WA_INSTR_br_table)
        {
            wa_ast* vector = wa_ast_begin_vector(parser, instrAst, &instr->immCount);

            instr->immCount += 1;
            instr->imm = oc_arena_push_array(parser->arena, wa_code, instr->immCount);

            for(u32 i = 0; i < instr->immCount - 1; i++)
            {
                wa_ast* immAst = wa_read_leb128_u32(parser, vector, OC_STR8("label"));
                instr->imm[i].index = immAst->valU32;
            }
            wa_ast* immAst = wa_read_leb128_u32(parser, vector, OC_STR8("label"));
            instr->imm[instr->immCount - 1].index = immAst->valU32;

            wa_ast_end(parser, vector);
        }
        else
        {
            //generic case

            instr->immCount = info->immCount;
            instr->imm = oc_arena_push_array(parser->arena, wa_code, instr->immCount);

            for(int immIndex = 0; immIndex < info->immCount; immIndex++)
            {
                switch(info->imm[immIndex])
                {
                    case WA_IMM_ZERO:
                    {
                        wa_read_byte(parser, instrAst, OC_STR8("zero"));
                    }
                    break;
                    case WA_IMM_I32:
                    {
                        wa_ast* immAst = wa_read_leb128_i32(parser, instrAst, OC_STR8("i32"));
                        instr->imm[immIndex].valI32 = immAst->valI32;
                    }
                    break;
                    case WA_IMM_I64:
                    {
                        wa_ast* immAst = wa_read_leb128_i64(parser, instrAst, OC_STR8("i64"));
                        instr->imm[immIndex].valI64 = immAst->valI64;
                    }
                    break;
                    case WA_IMM_F32:
                    {
                        wa_ast* immAst = wa_read_f32(parser, instrAst, OC_STR8("f32"));
                        instr->imm[immIndex].valF32 = immAst->valF32;
                    }
                    break;
                    case WA_IMM_F64:
                    {
                        wa_ast* immAst = wa_read_f64(parser, instrAst, OC_STR8("f64"));
                        instr->imm[immIndex].valF64 = immAst->valF64;
                    }
                    break;
                    case WA_IMM_VALUE_TYPE:
                    {
                        wa_ast* immAst = wa_parse_value_type(parser, instrAst, OC_STR8("value type"));
                        instr->imm[immIndex].valueType = immAst->valU32;
                    }
                    break;
                    case WA_IMM_REF_TYPE:
                    {
                        wa_ast* immAst = wa_read_byte(parser, instrAst, OC_STR8("ref type"));
                        instr->imm[immIndex].valueType = immAst->valU8;
                    }
                    break;

                    case WA_IMM_LOCAL_INDEX:
                    {
                        wa_ast* immAst = wa_read_leb128_u32(parser, instrAst, OC_STR8("index"));
                        instr->imm[immIndex].index = immAst->valU32;
                    }
                    break;

                    case WA_IMM_FUNC_INDEX:
                    {
                        wa_ast* immAst = wa_read_leb128_u32(parser, instrAst, OC_STR8("function index"));
                        instr->imm[immIndex].index = immAst->valU32;
                        immAst->kind = WA_AST_FUNC_INDEX;
                    }
                    break;

                    case WA_IMM_GLOBAL_INDEX:
                    case WA_IMM_TYPE_INDEX:
                    case WA_IMM_TABLE_INDEX:
                    case WA_IMM_ELEM_INDEX:
                    case WA_IMM_DATA_INDEX:
                    case WA_IMM_LABEL:
                    {
                        wa_ast* immAst = wa_read_leb128_u32(parser, instrAst, OC_STR8("index"));
                        instr->imm[immIndex].index = immAst->valU32;
                    }
                    break;
                    case WA_IMM_MEM_ARG:
                    {
                        wa_ast* memArgAst = wa_ast_begin(parser, instrAst, WA_AST_MEM_ARG);
                        wa_ast* alignAst = wa_read_leb128_u32(parser, memArgAst, OC_STR8("mem arg"));
                        wa_ast* offsetAst = wa_read_leb128_u32(parser, memArgAst, OC_STR8("mem arg"));

                        instr->imm[immIndex].memArg.align = alignAst->valU32;
                        instr->imm[immIndex].memArg.offset = offsetAst->valU32;

                        wa_ast_end(parser, memArgAst);
                    }
                    break;
                    case WA_IMM_LANE_INDEX:
                    {
                        wa_ast* immAst = wa_read_byte(parser, instrAst, OC_STR8("lane index"));
                        instr->imm[immIndex].laneIndex = immAst->valU8;
                    }
                    break;
                    /*
                    case WA_IMM_V128:
                    {
                        //TODO
                    }
                    break;
                    */
                    default:
                        OC_ASSERT(0, "unsupported immediate type");
                        break;
                }
            }
        }
        wa_ast_end(parser, instrAst);
    }
    //TODO check that we exited from an end instruction

    wa_ast_end(parser, exprAst);
    return (exprAst);
}

wa_ast* wa_parse_constant_expression(wa_parser* parser, wa_ast* parent, oc_list* list)
{
    return wa_parse_expression(parser, parent, 0, list, true);
}

void wa_parse_elements(wa_parser* parser, wa_module* module)
{
    wa_ast* section = module->toc.elements.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.elements.offset, OC_STR8("elements section"));
    u64 startOffset = parser->offset;

    wa_ast* vector = wa_ast_begin_vector(parser, section, &module->elementCount);

    module->elements = oc_arena_push_array(parser->arena, wa_element, module->elementCount);
    memset(module->elements, 0, module->elementCount * sizeof(wa_element));

    for(u32 eltIndex = 0; eltIndex < module->elementCount; eltIndex++)
    {
        wa_element* element = &module->elements[eltIndex];

        wa_ast* elementAst = wa_ast_begin(parser, vector, WA_AST_ELEMENT);
        wa_ast* prefixAst = wa_read_leb128_u32(parser, elementAst, OC_STR8("prefix"));
        u32 prefix = prefixAst->valU32;

        if(prefix > 7)
        {
            wa_parse_error(parser, prefixAst, "invalid element prefix %u\n", prefix);
            return;
        }

        if(prefix & 0x01)
        {
            if(prefix & 0x02)
            {
                element->mode = WA_ELEMENT_DECLARATIVE;
                //NOTE(martin): what the f* are they used for??
            }
            else
            {
                element->mode = WA_ELEMENT_PASSIVE;
            }
        }
        else
        {
            element->mode = WA_ELEMENT_ACTIVE;
            if(prefix & 0x02)
            {
                //NOTE: explicit table index
                wa_ast* tableIndexAst = wa_read_leb128_u32(parser, elementAst, OC_STR8("table index"));
                //TODO validate index
                element->tableIndex = tableIndexAst->valU32;
            }
            wa_parse_constant_expression(parser, elementAst, &element->tableOffset);
        }

        element->type = WA_TYPE_FUNC_REF;

        if(prefix & 0x04)
        {
            //NOTE: reftype? vec(expr)
            if(prefix & 0x03)
            {
                //NOTE ref type
                wa_ast* refTypeAst = wa_read_byte(parser, elementAst, OC_STR8("refType"));
                if(refTypeAst->valU8 != WA_TYPE_FUNC_REF && refTypeAst->valU8 != WA_TYPE_EXTERN_REF)
                {
                    wa_parse_error(parser, refTypeAst, "ref type should be externref or funcref.");
                }
                element->type = refTypeAst->valU8;
            }

            wa_ast* exprVec = wa_ast_begin(parser, elementAst, WA_AST_VECTOR);
            wa_ast* exprVecCount = wa_read_leb128_u32(parser, exprVec, OC_STR8("count"));
            element->initCount = exprVecCount->valU32;
            element->initInstr = oc_arena_push_array(parser->arena, oc_list, element->initCount);
            memset(element->initInstr, 0, element->initCount * sizeof(oc_list));

            for(u32 i = 0; i < element->initCount; i++)
            {
                wa_parse_constant_expression(parser, elementAst, &element->initInstr[i]);
            }
        }
        else
        {
            //NOTE: refkind? vec(funcIdx)
            if(prefix & 0x03)
            {
                //NOTE refkind
                wa_ast* refKindAst = wa_read_byte(parser, elementAst, OC_STR8("refKind"));
                if(refKindAst->valU8 != 0x00)
                {
                    wa_parse_error(parser, refKindAst, "ref kind should be 0.");
                }
            }

            wa_ast* funcVec = wa_ast_begin(parser, elementAst, WA_AST_VECTOR);
            wa_ast* funcVecCount = wa_read_leb128_u32(parser, funcVec, OC_STR8("count"));
            element->initCount = funcVecCount->valU32;
            element->initInstr = oc_arena_push_array(parser->arena, oc_list, element->initCount);
            memset(element->initInstr, 0, element->initCount * sizeof(oc_list));

            for(u32 i = 0; i < element->initCount; i++)
            {
                //TODO validate index
                wa_ast* funcIndexAst = wa_read_leb128_u32(parser, funcVec, OC_STR8("index"));
                funcIndexAst->kind = WA_AST_FUNC_INDEX;

                wa_instr* init = oc_arena_push_array(parser->arena, wa_instr, 2);
                memset(init, 0, 2 * sizeof(wa_instr));

                init[0].op = WA_INSTR_ref_func;
                init[0].immCount = 1;
                init[0].imm = oc_arena_push_type(parser->arena, wa_code);
                init[0].imm[0].index = funcIndexAst->valU32;
                oc_list_push_back(&element->initInstr[i], &init[0].listElt);

                init[1].op = WA_INSTR_end;
                oc_list_push_back(&element->initInstr[i], &init[1].listElt);
            }
        }
        wa_ast_end(parser, elementAst);
    }
    wa_ast_end(parser, vector);

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.elements.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.elements.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_data(wa_parser* parser, wa_module* module)
{
    wa_ast* section = module->toc.data.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.data.offset, OC_STR8("data section"));
    u64 startOffset = parser->offset;

    wa_ast* vector = wa_ast_begin_vector(parser, section, &module->dataCount);

    module->data = oc_arena_push_array(parser->arena, wa_data_segment, module->dataCount);
    memset(module->data, 0, module->dataCount * sizeof(wa_data_segment));

    for(u32 segIndex = 0; segIndex < module->dataCount; segIndex++)
    {
        wa_data_segment* seg = &module->data[segIndex];

        wa_ast* segmentAst = wa_ast_begin(parser, vector, WA_AST_DATA_SEGMENT);
        wa_ast* prefixAst = wa_read_leb128_u32(parser, segmentAst, OC_STR8("prefix"));
        u32 prefix = prefixAst->valU32;

        if(prefix > 2)
        {
            wa_parse_error(parser, prefixAst, "invalid segment prefix %u\n", prefix);
            return;
        }

        if(prefix & 0x01)
        {
            seg->mode = WA_DATA_PASSIVE;
        }
        else
        {
            seg->mode = WA_DATA_ACTIVE;
            if(prefix & 0x02)
            {
                //NOTE: explicit memory index
                wa_ast* memoryIndexAst = wa_read_leb128_u32(parser, segmentAst, OC_STR8("memory index"));
                //TODO validate index
                seg->memoryIndex = memoryIndexAst->valU32;
            }
            wa_parse_constant_expression(parser, segmentAst, &seg->memoryOffset);
        }

        //NOTE: parse vec(bytes)
        wa_ast* initVec = wa_read_name(parser, segmentAst, OC_STR8("init"));
        seg->init = initVec->str8;

        wa_ast_end(parser, segmentAst);
    }
    wa_ast_end(parser, vector);

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.data.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.data.len,
                       parser->offset - startOffset);
    }
}

void wa_parse_code(wa_parser* parser, wa_module* module)
{
    wa_ast* section = module->toc.code.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.code.offset, OC_STR8("code section"));
    u64 startOffset = parser->offset;

    u32 functionCount = 0;
    wa_ast* vector = wa_ast_begin_vector(parser, section, &functionCount);

    if(functionCount != module->functionCount - module->functionImportCount)
    {
        //TODO should set the error on the count, not the vector?
        wa_parse_error(parser,
                       vector,
                       "Function count mismatch (function section: %i, code section: %i\n",
                       module->functionCount - module->functionImportCount,
                       functionCount);
    }
    functionCount = oc_min(functionCount + module->functionImportCount, module->functionCount);

    for(u32 funcIndex = module->functionImportCount; funcIndex < functionCount; funcIndex++)
    {
        wa_func* func = &module->functions[funcIndex];

        wa_ast* funcAst = wa_ast_begin(parser, vector, WA_AST_FUNC);
        funcAst->func = func;

        oc_arena_scope scratch = oc_scratch_begin();

        wa_ast* lenAst = wa_read_leb128_u32(parser, funcAst, OC_STR8("function length"));

        u32 funcLen = lenAst->valU32;
        u32 funcStartOffset = parser->offset;

        //NOTE: parse locals
        u32 localEntryCount = 0;
        wa_ast* localsVector = wa_ast_begin_vector(parser, funcAst, &localEntryCount);

        u32* counts = oc_arena_push_array(scratch.arena, u32, localEntryCount);
        wa_value_type* types = oc_arena_push_array(scratch.arena, wa_value_type, localEntryCount);

        func->localCount = func->type->paramCount;
        for(u32 localEntryIndex = 0; localEntryIndex < localEntryCount; localEntryIndex++)
        {
            wa_ast* localEntryAst = wa_ast_begin(parser, localsVector, WA_AST_LOCAL_ENTRY);
            wa_ast* countAst = wa_read_leb128_u32(parser, localEntryAst, OC_STR8("count"));
            wa_ast* typeAst = wa_read_byte(parser, localEntryAst, OC_STR8("index"));
            typeAst->kind = WA_AST_TYPE_INDEX;

            counts[localEntryIndex] = countAst->valU32;
            types[localEntryIndex] = typeAst->valU8;
            func->localCount += counts[localEntryIndex];

            wa_ast_end(parser, localEntryAst);
            //TODO: validate types? or validate later?
        }
        wa_ast_end(parser, localsVector);

        //NOTE: expand locals
        func->locals = oc_arena_push_array(parser->arena, wa_value_type, func->localCount);

        for(u32 paramIndex = 0; paramIndex < func->type->paramCount; paramIndex++)
        {
            func->locals[paramIndex] = func->type->params[paramIndex];
        }

        u32 localIndex = func->type->paramCount;
        for(u32 localEntryIndex = 0; localEntryIndex < localEntryCount; localEntryIndex++)
        {
            u32 count = counts[localEntryIndex];
            wa_value_type type = types[localEntryIndex];

            for(int i = 0; i < count; i++)
            {
                func->locals[localIndex + i] = type;
            }
            localIndex += count;
        }

        //NOTE: parse body
        wa_ast* bodyAst = wa_parse_expression(parser, funcAst, func->localCount, &func->instructions, false);

        wa_ast_end(parser, funcAst);

        //NOTE: check entry length
        if(parser->offset - funcStartOffset != funcLen)
        {
            wa_parse_error(parser,
                           funcAst,
                           "Size of code entry %i does not match declared size (declared %u, got %u)\n",
                           funcIndex,
                           funcLen,
                           parser->offset - funcStartOffset);
            goto parse_function_end;
        }

    parse_function_end:
        oc_scratch_end(scratch);
        wa_parser_seek(parser, funcStartOffset + funcLen, OC_STR8("next function"));
    }

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.code.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.code.len,
                       parser->offset - startOffset);
    }
}

//-------------------------------------------------------------------------
// Compile
//-------------------------------------------------------------------------

enum
{
    WA_TYPE_STACK_MAX_LEN = 512,
    WA_CONTROL_STACK_MAX_LEN = 512,
    WA_MAX_REG = 4096,
};

typedef enum wa_operand_slot_kind
{
    WA_OPERAND_SLOT_NIL = 0,
    WA_OPERAND_SLOT_LOCAL,
    WA_OPERAND_SLOT_REG,
} wa_operand_slot_kind;

typedef struct wa_operand_slot
{
    wa_operand_slot_kind kind;
    wa_value_type type;
    u32 index;
    u64 count;
    wa_instr* originInstr;
    u64 originOpd;
} wa_operand_slot;

typedef struct wa_jump_target
{
    oc_list_elt listElt;
    u64 offset;
} wa_jump_target;

typedef struct wa_block
{
    wa_instr* begin;
    u64 beginOffset;
    u64 elseOffset;
    oc_list jumpTargets;

    u64 scopeBase;
    bool polymorphic;
    bool prevPolymorphic;

} wa_block;

typedef struct wa_build_context
{
    oc_arena* arena;     // the module's arena
    oc_arena checkArena; // temp arena for checking
    oc_arena codeArena;  // temp arena for building code
    wa_module* module;

    u64 opdStackLen;
    u64 opdStackCap;
    wa_operand_slot* opdStack;

    u64 controlStackLen;
    u64 controlStackCap;
    wa_block* controlStack;

    u64 nextRegIndex;
    u64 freeRegLen;
    u64 freeRegs[WA_MAX_REG];

    wa_func_type* exprType;

    u64 codeCap;
    u64 codeLen;
    wa_code* code;

} wa_build_context;

void wa_compile_error(wa_build_context* context, wa_ast* ast, const char* fmt, ...)
{
    wa_module_error* error = oc_arena_push_type(context->arena, wa_module_error);
    memset(error, 0, sizeof(wa_module_error));

    va_list ap;
    va_start(ap, fmt);
    error->string = oc_str8_pushfv(context->arena, fmt, ap);
    va_end(ap);

    error->ast = ast;
    oc_list_push_back(&ast->errors, &error->astElt);

    oc_list_push_back(&context->module->errors, &error->moduleElt);
}

void wa_compile_error_block_end(wa_build_context* context, wa_ast* ast, const char* fmt, ...)
{
    wa_module_error* error = oc_arena_push_type(context->arena, wa_module_error);
    memset(error, 0, sizeof(wa_module_error));

    va_list ap;
    va_start(ap, fmt);
    error->string = oc_str8_pushfv(context->arena, fmt, ap);
    va_end(ap);

    error->ast = ast;
    error->blockEnd = true;

    oc_list_push_back(&ast->errors, &error->astElt);

    oc_list_push_back(&context->module->errors, &error->moduleElt);
}

bool wa_operand_slot_is_nil(wa_operand_slot* slot)
{
    return (slot->kind == WA_OPERAND_SLOT_NIL);
}

bool wa_block_is_nil(wa_block* block)
{
    return (block->begin == 0);
}

void wa_control_stack_push(wa_build_context* context, wa_instr* instr)
{
    if(context->controlStack == 0 || context->controlStackLen >= context->controlStackCap)
    {
        context->controlStackCap = (context->controlStackCap + 8) * 2;
        wa_block* tmp = context->controlStack;
        context->controlStack = oc_arena_push_array(&context->checkArena, wa_block, context->controlStackCap);
        OC_ASSERT(context->controlStack, "out of memory");

        if(tmp)
        {
            memcpy(context->controlStack, tmp, context->controlStackLen * sizeof(wa_block));
        }
    }
    context->controlStack[context->controlStackLen] = (wa_block){
        .begin = instr,
        .beginOffset = context->codeLen,
        .scopeBase = context->opdStackLen,
    };

    if(context->controlStackLen && context->controlStack[context->controlStackLen - 1].polymorphic)
    {
        context->controlStack[context->controlStackLen].prevPolymorphic = true;
    }
    context->controlStackLen++;
}

wa_block wa_control_stack_pop(wa_build_context* context)
{
    wa_block block = { 0 };
    if(context->controlStackLen)
    {
        context->controlStackLen--;
        block = context->controlStack[context->controlStackLen];
    }
    return (block);
}

wa_block* wa_control_stack_lookup(wa_build_context* context, u32 label)
{
    wa_block* block = 0;
    if(label < context->controlStackLen)
    {
        block = &context->controlStack[context->controlStackLen - label - 1];
    }
    return (block);
}

wa_block* wa_control_stack_top(wa_build_context* context)
{
    return wa_control_stack_lookup(context, 0);
}

wa_block wa_control_stack_top_value(wa_build_context* context)
{
    wa_block* block = wa_control_stack_top(context);
    if(block)
    {
        return (*block);
    }
    else
    {
        return ((wa_block){ 0 });
    }
}

u32 wa_allocate_register(wa_build_context* context)
{
    u32 index = 0;
    if(context->freeRegLen)
    {
        context->freeRegLen--;
        index = context->freeRegs[context->freeRegLen];
    }
    else
    {
        //TODO: prevent overflow
        index = context->nextRegIndex;
        context->nextRegIndex++;
    }
    return (index);
}

void wa_free_slot(wa_build_context* context, u64 index)
{
    OC_DEBUG_ASSERT(context->freeRegLen >= WA_MAX_REG);
    context->freeRegs[context->freeRegLen] = index;
    context->freeRegLen++;
}

void wa_operand_stack_push(wa_build_context* context, wa_operand_slot s)
{
    if(context->opdStack == 0 || context->opdStackLen >= context->opdStackCap)
    {
        context->opdStackCap = (context->opdStackCap + 8) * 2;
        wa_operand_slot* tmp = context->opdStack;
        context->opdStack = oc_arena_push_array(&context->checkArena, wa_operand_slot, context->opdStackCap);
        OC_ASSERT(context->opdStack, "out of memory");

        if(tmp)
        {
            memcpy(context->opdStack, tmp, context->opdStackLen * sizeof(wa_operand_slot));
        }
    }
    context->opdStack[context->opdStackLen] = s;
    context->opdStackLen++;
}

u32 wa_operand_stack_push_reg(wa_build_context* context, wa_value_type type, wa_instr* instr)
{
    wa_operand_slot s = {
        .kind = WA_OPERAND_SLOT_REG,
        .type = type,
        .index = wa_allocate_register(context),
        .originInstr = instr,
        .originOpd = context->codeLen,
    };
    wa_operand_stack_push(context, s);

    return s.index;
}

wa_operand_slot wa_operand_stack_pop(wa_build_context* context)
{
    wa_operand_slot slot = { 0 };
    u64 scopeBase = 0;
    wa_block* block = wa_control_stack_top(context);
    OC_ASSERT(block);
    scopeBase = block->scopeBase;

    if(context->opdStackLen > scopeBase)
    {
        context->opdStackLen--;
        slot = context->opdStack[context->opdStackLen];

        if(slot.kind == WA_OPERAND_SLOT_REG && slot.count == 0)
        {
            wa_free_slot(context, slot.index);
        }
    }
    else if(block->polymorphic)
    {
        slot = (wa_operand_slot){
            .kind = WA_OPERAND_SLOT_REG,
            .index = 0,
            .type = WA_TYPE_UNKNOWN,
        };
    }
    return (slot);
}

wa_operand_slot wa_operand_stack_lookup(wa_build_context* context, u32 index)
{
    wa_operand_slot slot = { 0 };
    wa_block* block = wa_control_stack_top(context);
    OC_ASSERT(block);
    u64 scopeBase = block->scopeBase;

    if(index < context->opdStackLen - scopeBase)
    {
        slot = context->opdStack[context->opdStackLen - index - 1];
    }
    else if(block->polymorphic)
    {
        slot = (wa_operand_slot){
            .kind = WA_OPERAND_SLOT_REG,
            .index = 0,
            .type = WA_TYPE_UNKNOWN,
        };
    }
    return (slot);
}

u32 wa_operand_stack_scope_size(wa_build_context* context)
{
    wa_block* block = wa_control_stack_top(context);
    return context->opdStackLen - block->scopeBase;
}

void wa_operand_stack_pop_slots(wa_build_context* context, u64 count)
{
    wa_block* block = wa_control_stack_top(context);
    OC_ASSERT(block);
    u64 scopeBase = block->scopeBase;

    u64 slotCount = oc_min(context->opdStackLen - scopeBase, count);

    for(u64 i = 0; i < slotCount; i++)
    {
        wa_operand_slot* slot = &context->opdStack[context->opdStackLen - i - 1];
        if(slot->kind == WA_OPERAND_SLOT_REG && slot->count == 0)
        {
            wa_free_slot(context, slot->index);
        }
    }

    context->opdStackLen -= slotCount;
}

wa_operand_slot wa_operand_stack_top(wa_build_context* context)
{
    wa_operand_slot slot = { 0 };
    u64 scopeBase = 0;
    wa_block* block = wa_control_stack_top(context);
    OC_ASSERT(block);
    scopeBase = block->scopeBase;

    if(context->opdStackLen > scopeBase)
    {
        slot = context->opdStack[context->opdStackLen - 1];
    }
    else if(block->polymorphic)
    {
        slot = (wa_operand_slot){
            .kind = WA_OPERAND_SLOT_REG,
            .index = 0,
            .type = WA_TYPE_UNKNOWN,
        };
    }

    return (slot);
}

void wa_operand_stack_pop_scope(wa_build_context* context, wa_block* block)
{
    OC_ASSERT(context->opdStackLen >= block->scopeBase);

    u64 slotCount = context->opdStackLen - block->scopeBase;
    for(u64 i = 0; i < slotCount; i++)
    {
        wa_operand_stack_pop(context);
    }
    OC_ASSERT(context->opdStackLen == block->scopeBase);
}

bool wa_check_operand_type(wa_value_type t1, wa_value_type t2)
{
    return (t1 == t2
            || t1 == WA_TYPE_UNKNOWN
            || t2 == WA_TYPE_UNKNOWN
            || t1 == WA_TYPE_ANY
            || t2 == WA_TYPE_ANY
            || (t1 == WA_TYPE_REF && (t2 == WA_TYPE_FUNC_REF || t2 == WA_TYPE_EXTERN_REF))
            || (t2 == WA_TYPE_REF && (t1 == WA_TYPE_FUNC_REF || t1 == WA_TYPE_EXTERN_REF))
            || (t1 == WA_TYPE_NUM_OR_VEC && (wa_is_value_type_numeric(t2) || t2 == WA_TYPE_V128))
            || (t2 == WA_TYPE_NUM_OR_VEC && (wa_is_value_type_numeric(t1) || t1 == WA_TYPE_V128)));
}

wa_operand_slot* wa_operand_stack_get_slots(oc_arena* arena,
                                            wa_build_context* context,
                                            wa_instr* instr,
                                            u32 count,
                                            wa_value_type* types,
                                            bool popSlots)
{
    wa_operand_slot* inSlots = oc_arena_push_array(arena, wa_operand_slot, count);
    for(u32 i = 0; i < count; i++)
    {
        u32 opdIndex = count - 1 - i;

        if(popSlots)
        {
            inSlots[opdIndex] = wa_operand_stack_pop(context);
        }
        else
        {
            inSlots[opdIndex] = wa_operand_stack_lookup(context, i);
        }

        if(wa_operand_slot_is_nil(&inSlots[opdIndex])
           || !wa_check_operand_type(inSlots[opdIndex].type, types[opdIndex]))
        {
            wa_compile_error(context, instr->ast, "unbalanced stack\n");
            //TODO: here we can give a better error message since we have all the operands popped so far
            break;
        }
    }
    return (inSlots);
}

void wa_block_set_polymorphic(wa_build_context* context)
{
    wa_block* block = wa_control_stack_top(context);
    block->polymorphic = true;
    wa_operand_stack_pop_scope(context, block);
}

void wa_emit(wa_build_context* context, wa_code* code)
{
    if(context->codeLen >= context->codeCap)
    {
        //TODO: better handle realloc
        oc_arena_scope scratch = oc_scratch_begin();
        wa_code* tmp = oc_arena_push_array(scratch.arena, wa_code, context->codeLen);
        memcpy(tmp, context->code, context->codeLen * sizeof(wa_code));

        context->codeCap = (context->codeCap + 1) * 1.5;

        oc_arena_clear(&context->codeArena);
        context->code = oc_arena_push_array(&context->codeArena, wa_code, context->codeCap);
        memcpy(context->code, tmp, context->codeLen * sizeof(wa_code));

        oc_scratch_end(scratch);
    }
    memcpy(&context->code[context->codeLen], code, sizeof(wa_code));
    context->codeLen++;
}

wa_code* wa_push_code(wa_build_context* context)
{
    if(context->codeLen >= context->codeCap)
    {
        //TODO: better handle realloc
        oc_arena_scope scratch = oc_scratch_begin();
        wa_code* tmp = oc_arena_push_array(scratch.arena, wa_code, context->codeLen);
        memcpy(tmp, context->code, context->codeLen * sizeof(wa_code));

        context->codeCap = (context->codeCap + 1) * 1.5;

        oc_arena_clear(&context->codeArena);
        context->code = oc_arena_push_array(&context->codeArena, wa_code, context->codeCap);
        memcpy(context->code, tmp, context->codeLen * sizeof(wa_code));

        oc_scratch_end(scratch);
    }
    wa_code* code = &context->code[context->codeLen];
    memset(code, 0, sizeof(wa_code));
    context->codeLen++;
    return (code);
}

void wa_emit_opcode(wa_build_context* context, wa_instr_op op)
{
    wa_code* code = wa_push_code(context);
    code->opcode = op;
}

void wa_emit_index(wa_build_context* context, u32 index)
{
    wa_code* code = wa_push_code(context);
    code->index = index;
}

void wa_emit_i64(wa_build_context* context, i64 val)
{
    wa_code* code = wa_push_code(context);
    code->valI64 = val;
}

void wa_move_slot_if_used(wa_build_context* context, u32 slotIndex)
{
    wa_block* block = wa_control_stack_top(context);
    u32 newReg = UINT32_MAX;
    u64 count = 0;

    //NOTE: we check only slots in current scope. This means we never clobber reserved input/return slots
    for(u32 stackIndex = block->scopeBase; stackIndex < context->opdStackLen; stackIndex++)
    {
        wa_operand_slot* slot = &context->opdStack[stackIndex];
        if(slot->index == slotIndex)
        {
            if(newReg == UINT32_MAX)
            {
                newReg = wa_allocate_register(context);
            }
            slot->kind = WA_OPERAND_SLOT_REG;
            slot->index = newReg;
            slot->count = count;
            count++;
        }
    }
    if(newReg != UINT32_MAX)
    {
        wa_emit_opcode(context, WA_INSTR_move);
        wa_emit_index(context, newReg);
        wa_emit_index(context, slotIndex);
    }
}

void wa_move_locals_to_registers(wa_build_context* context)
{
    for(u32 stackIndex = 0; stackIndex < context->opdStackLen; stackIndex++)
    {
        wa_operand_slot* slot = &context->opdStack[stackIndex];
        if(slot->kind == WA_OPERAND_SLOT_LOCAL)
        {
            wa_move_slot_if_used(context, slot->index);
        }
    }
}

void wa_push_block_inputs(wa_build_context* context, wa_func_type* type)
{
    OC_ASSERT(context->controlStackLen > 1);
    wa_block* prevBlock = &context->controlStack[context->controlStackLen - 2];

    u64 paramStart = context->opdStackLen - (type->paramCount + type->returnCount);
    u64 inputEnd = context->opdStackLen - type->returnCount;
    u64 copyStart = type->paramCount - (inputEnd - prevBlock->scopeBase);

    for(u64 inIndex = 0; inIndex < type->paramCount; inIndex++)
    {
        if(inIndex < copyStart)
        {
            //NOTE: if there wasn't enough operands on the stack,
            //      push fake ones with the correct type for that new block.
            //      This means this block is either unreachable, or will already have
            //      triggered a validation error.
            wa_operand_stack_push(context,
                                  (wa_operand_slot){
                                      .kind = WA_OPERAND_SLOT_REG,
                                      .type = type->params[inIndex],
                                  });
        }
        else
        {
            //NOTE copy block inputs on top of the stack
            wa_operand_slot slotCopy = context->opdStack[paramStart + inIndex];
            slotCopy.count++;
            wa_operand_stack_push(context, slotCopy);
        }
    }
}

void wa_block_begin(wa_build_context* context, wa_instr* instr)
{
    oc_arena_scope scratch = oc_scratch_begin();
    wa_func_type* type = instr->blockType;

    //NOTE: check block type input (but we don't pop or use them here)
    wa_operand_stack_get_slots(scratch.arena,
                               context,
                               instr,
                               type->paramCount,
                               type->params,
                               false);

    //NOTE allocate block results and push them on the stack
    for(u64 outIndex = 0; outIndex < type->returnCount; outIndex++)
    {
        wa_operand_stack_push_reg(context, type->returns[outIndex], instr);
        //TODO immediately put them in the freelist so they can be used in the branches (this might complicate copying results a bit...)
        // wa_free_slot(context, index);
    }

    wa_control_stack_push(context, instr);
    wa_push_block_inputs(context, type);

    oc_scratch_end(scratch);
}

void wa_block_move_results_to_input_slots(wa_build_context* context, wa_block* block, wa_instr* instr)
{
    oc_arena_scope scratch = oc_scratch_begin();
    wa_func_type* type = block->begin->blockType;

    wa_operand_slot* slots = wa_operand_stack_get_slots(scratch.arena,
                                                        context,
                                                        instr,
                                                        type->paramCount,
                                                        type->params,
                                                        false);

    if(!block->polymorphic && !block->prevPolymorphic)
    {
        for(u64 paramIndex = 0; paramIndex < type->paramCount; paramIndex++)
        {
            wa_operand_slot* dst = &context->opdStack[block->scopeBase - type->returnCount - type->paramCount + paramIndex];
            wa_emit_opcode(context, WA_INSTR_move);
            wa_emit_index(context, dst->index);
            wa_emit_index(context, slots[paramIndex].index);
        }
    }
    oc_scratch_end(scratch);
}

void wa_block_move_results_to_output_slots(wa_build_context* context, wa_block* block, wa_instr* instr)
{
    oc_arena_scope scratch = oc_scratch_begin();
    wa_func_type* type = block->begin->blockType;

    wa_operand_slot* slots = wa_operand_stack_get_slots(scratch.arena,
                                                        context,
                                                        instr,
                                                        type->returnCount,
                                                        type->returns,
                                                        false);

    if(!block->polymorphic && !block->prevPolymorphic)
    {
        for(u32 returnIndex = 0; returnIndex < type->returnCount; returnIndex++)
        {
            wa_operand_slot* dst = &context->opdStack[block->scopeBase - type->returnCount + returnIndex];
            wa_emit_opcode(context, WA_INSTR_move);
            wa_emit_index(context, dst->index);
            wa_emit_index(context, slots[returnIndex].index);
        }
    }
}

void wa_block_end(wa_build_context* context, wa_block* block, wa_instr* instr)
{
    if(block->begin->op == WA_INSTR_if && !block->begin->elseBranch)
    {
        //TODO: coalesce with case WA_INSTR_else in compile proc

        //NOTE: if there was no else branch, we must still generate a fake else branch to copy the block inputs to
        //      the output.
        wa_func_type* type = block->begin->blockType;

        wa_block_move_results_to_output_slots(context, block, instr);
        wa_operand_stack_pop_scope(context, block);
        wa_push_block_inputs(context, type);

        wa_emit_opcode(context, WA_INSTR_jump);
        wa_emit_i64(context, 0);

        block->polymorphic = false;
        block->begin->elseBranch = instr;
        block->elseOffset = context->codeLen;
    }

    wa_block_move_results_to_output_slots(context, block, instr);

    wa_func_type* type = block->begin->blockType;

    //NOTE - pop slots until scope is empty,
    //     - pop block scope
    //     - pop reserved outputs and saved params,
    //     - push result slots on top of stack

    wa_operand_stack_pop_scope(context, block);
    wa_control_stack_pop(context);

    //NOTE: here we keep return slots allocated by just decrementing stack len,
    //      and then we pop (and recycle) input slots. Then we just copy return
    //      slots to the top of the stack.
    //WARN: If we later decide to reuse reserved return slots inside block, we
    //      must re-mark them as allocated here too...

    context->opdStackLen -= type->returnCount;
    u64 returnSlotStart = context->opdStackLen;

    for(u64 index = 0; index < type->paramCount; index++)
    {
        wa_operand_stack_pop(context);
    }

    memcpy(&context->opdStack[context->opdStackLen],
           &context->opdStack[returnSlotStart],
           type->returnCount * sizeof(wa_operand_slot));

    context->opdStackLen += type->returnCount;

    if(block->begin->op == WA_INSTR_if)
    {
        OC_ASSERT(block->begin->elseBranch);

        //NOTE: patch conditional jump to else branch
        context->code[block->beginOffset + 1].valI64 = block->elseOffset - (block->beginOffset + 1);

        //NOTE: patch jump from end of if branch to end of else branch
        context->code[block->elseOffset - 1].valI64 = context->codeLen - (block->elseOffset - 1);
    }
}

void wa_patch_jump_targets(wa_build_context* context, wa_block* block)
{
    oc_list_for(block->jumpTargets, target, wa_jump_target, listElt)
    {
        context->code[target->offset].valI64 = context->codeLen - target->offset;
    }
}

int wa_compile_return(wa_build_context* context, wa_func_type* type, wa_instr* instr)
{
    /////////////////////////////////////////////////////////////////
    //TODO: return may be used in branches of branch table, so it must
    //      _NOT_ change the stack...
    /////////////////////////////////////////////////////////////////
    oc_arena_scope scratch = oc_scratch_begin();
    wa_operand_slot* returns = wa_operand_stack_get_slots(scratch.arena,
                                                          context,
                                                          instr,
                                                          type->returnCount,
                                                          type->returns,
                                                          false);

    for(u32 retIndex = 0; retIndex < type->returnCount; retIndex++)
    {
        wa_operand_slot slot = returns[retIndex];

        //NOTE store value to return slot
        if(slot.type != WA_TYPE_UNKNOWN && slot.index != retIndex)
        {
            ////////////////////////////////////////////////////////////////////////////////////////////////////
            //TODO: coalesce with move if used _BUT_ preserve the stack for futher returns in other branches
            ////////////////////////////////////////////////////////////////////////////////////////////////////
            u32 newReg = UINT32_MAX;
            u32 count = 0;
            for(u32 i = 0; i < type->returnCount; i++)
            {
                wa_operand_slot* r = &returns[i];
                if(r->index == retIndex)
                {
                    if(newReg == UINT32_MAX)
                    {
                        newReg = wa_allocate_register(context);
                    }
                    r->kind = WA_OPERAND_SLOT_REG;
                    r->index = newReg;
                    r->count = count;
                    count++;
                }
            }
            if(newReg != UINT32_MAX)
            {
                wa_emit_opcode(context, WA_INSTR_move);
                wa_emit_index(context, newReg);
                wa_emit_index(context, retIndex);
            }

            wa_emit_opcode(context, WA_INSTR_move);
            wa_emit_index(context, retIndex);
            wa_emit_index(context, slot.index);
        }
    }
    wa_emit_opcode(context, WA_INSTR_return);

    //WARN: wa_compile_return() is also used by conditional or table branches when they target the top-level scope,
    //      so we don't set the stack polymorphic here. Instead we do it in the callers when return is unconditional
    oc_scratch_end(scratch);
    return 0;
}

void wa_compile_branch(wa_build_context* context, wa_instr* instr, u32 label)
{
    if(label + 1 == context->controlStackLen)
    {
        //Do a return
        wa_compile_return(context, context->exprType, instr);
    }
    else
    {
        wa_block* block = wa_control_stack_lookup(context, label);
        if(!block)
        {
            //TODO here we should pass the ast of the _immediate_, not the instruction
            wa_compile_error(context,
                             instr->ast,
                             "block level %u not found\n",
                             label);
            return;
        }

        if(block->begin->op == WA_INSTR_block
           || block->begin->op == WA_INSTR_if)
        {
            wa_block_move_results_to_output_slots(context, block, instr);

            //jump to end
            wa_emit_opcode(context, WA_INSTR_jump);

            //NOTE: emit a jump target operand for end of block
            wa_jump_target* target = oc_arena_push_type(&context->checkArena, wa_jump_target);
            target->offset = context->codeLen;
            oc_list_push_back(&block->jumpTargets, &target->listElt);

            wa_emit_i64(context, 0);
        }
        else if(block->begin->op == WA_INSTR_loop)
        {
            wa_block_move_results_to_input_slots(context, block, instr);

            //jump to begin
            wa_emit_opcode(context, WA_INSTR_jump);
            wa_emit_i64(context, block->beginOffset - context->codeLen);
        }
        else
        {
            OC_ASSERT(0, "unreachable");
        }
    }
}

void wa_build_context_clear(wa_build_context* context)
{
    oc_arena_clear(&context->checkArena);
    context->codeLen = 0;

    context->opdStackLen = 0;
    context->opdStackCap = 0;

    context->controlStackLen = 0;
    context->controlStackCap = 0;

    context->nextRegIndex = 0;
    context->freeRegLen = 0;
}

bool wa_validate_immediates(wa_build_context* context, wa_func* func, wa_instr* instr, const wa_instr_info* info)
{
    wa_module* module = context->module;

    bool check = true;
    //NOTE: validate immediates
    for(u32 immIndex = 0; immIndex < instr->immCount; immIndex++)
    {
        wa_immediate_type type;

        if(instr->op == WA_INSTR_select_t)
        {
            type = WA_IMM_VALUE_TYPE;
        }
        if(instr->op == WA_INSTR_br_table)
        {
            type = WA_IMM_LABEL;
        }
        else
        {
            type = info->imm[immIndex];
        }

        wa_code* imm = &instr->imm[immIndex];
        switch(type)
        {
            case WA_IMM_VALUE_TYPE:
            {
                if(!wa_is_value_type(imm->valueType))
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid value type 0x%x\n",
                                     (u32)imm->valueType);
                    check = false;
                }
            }
            break;
            case WA_IMM_REF_TYPE:
            {
                if(imm->valueType != WA_TYPE_FUNC_REF && imm->valueType != WA_TYPE_EXTERN_REF)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid reference type %x\n",
                                     (u32)imm->valueType);
                    check = false;
                }
            }
            break;
            case WA_IMM_LOCAL_INDEX:
            {
                if(imm->index >= func->localCount)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid local index %u (localCount: %u)\n",
                                     imm->index,
                                     func->localCount);
                    check = false;
                }
            }
            break;
            case WA_IMM_FUNC_INDEX:
            {
                if(imm->index >= module->functionCount)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid function index %u (function count: %u)\n",
                                     imm->index,
                                     module->functionCount);
                    check = false;
                }
            }
            break;
            case WA_IMM_GLOBAL_INDEX:
            {
                if(imm->index >= module->globalCount)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid global index %u (global count: %u)\n",
                                     imm->index,
                                     module->globalCount);
                    check = false;
                }
            }
            break;
            case WA_IMM_TYPE_INDEX:
            {
                if(imm->index >= module->typeCount)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid type index %u (type count: %u)\n",
                                     imm->index,
                                     module->typeCount);
                    check = false;
                }
            }
            break;

            case WA_IMM_TABLE_INDEX:
            {
                if(imm->index >= module->tableCount)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid table index %u (table count: %u)\n",
                                     imm->index,
                                     module->tableCount);
                    check = false;
                }
            }
            break;
            case WA_IMM_ELEM_INDEX:
            {
                if(imm->index >= module->elementCount)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid element index %u (element count: %u)\n",
                                     imm->index,
                                     module->elementCount);
                    check = false;
                }
            }
            break;
            case WA_IMM_DATA_INDEX:
            {
                if(imm->index >= module->dataCount)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "invalid data index %u (data count: %u)\n",
                                     imm->index,
                                     module->dataCount);
                    check = false;
                }
            }
            break;

            default:
                break;
        }
    }
    return check;
}

void wa_compile_expression(wa_build_context* context, wa_func_type* type, wa_func* func, oc_list instructions)
{
    wa_module* module = context->module;

    context->exprType = type;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //TODO: remove the need to pass instr -- this will break else checks if first instr is an "if"...
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    wa_control_stack_push(context, oc_list_first_entry(instructions, wa_instr, listElt));

    oc_arena_scope scratch = oc_scratch_begin();

    oc_list_for(instructions, instr, wa_instr, listElt)
    {
        oc_scratch_end(scratch);

        const wa_instr_info* info = &wa_instr_infos[instr->op];

        if(!wa_validate_immediates(context, func, instr, info))
        {
            //NOTE: skip validating the rest of the instruction to avoid using
            //      invalid indices.
            continue;
        }

        //NOTE: special case handling of control instructions
        if(instr->op == WA_INSTR_block || instr->op == WA_INSTR_loop)
        {
            wa_move_locals_to_registers(context);
            wa_block_begin(context, instr);
        }
        else if(instr->op == WA_INSTR_if)
        {
            wa_move_locals_to_registers(context);

            wa_operand_slot* slot = wa_operand_stack_get_slots(scratch.arena,
                                                               context,
                                                               instr,
                                                               1,
                                                               (wa_value_type[]){ WA_TYPE_I32 },
                                                               true);
            wa_block_begin(context, instr);

            wa_emit_opcode(context, WA_INSTR_jump_if_zero);
            wa_emit_i64(context, 0);
            wa_emit_index(context, slot->index);
        }
        else if(instr->op == WA_INSTR_else)
        {
            wa_block* ifBlock = wa_control_stack_top(context);
            OC_ASSERT(ifBlock);

            if(ifBlock->begin->op != WA_INSTR_if
               || ifBlock->begin->elseBranch)
            {
                //TODO: should this be validated at parse stage?
                wa_compile_error(context,
                                 instr->ast,
                                 "unexpected else block\n");
            }
            else
            {
                wa_func_type* type = ifBlock->begin->blockType;

                wa_block_move_results_to_output_slots(context, ifBlock, instr);
                wa_operand_stack_pop_scope(context, ifBlock);
                wa_push_block_inputs(context, type);

                wa_emit_opcode(context, WA_INSTR_jump);
                wa_emit_i64(context, 0);

                ifBlock->polymorphic = false;
                ifBlock->begin->elseBranch = instr;
                ifBlock->elseOffset = context->codeLen;
            }
        }
        else if(instr->op == WA_INSTR_br)
        {
            u32 label = instr->imm[0].index;
            wa_compile_branch(context, instr, label);
            wa_block_set_polymorphic(context);
        }
        else if(instr->op == WA_INSTR_br_if)
        {
            wa_operand_slot* slot = wa_operand_stack_get_slots(scratch.arena,
                                                               context,
                                                               instr,
                                                               1,
                                                               (wa_value_type[]){ WA_TYPE_I32 },
                                                               true);

            wa_emit_opcode(context, WA_INSTR_jump_if_zero);
            u64 jumpOffset = context->codeLen;
            wa_emit_i64(context, 0);
            wa_emit_index(context, slot->index);

            u32 label = instr->imm[0].index;
            wa_compile_branch(context, instr, label);

            context->code[jumpOffset].valI64 = context->codeLen - jumpOffset;
        }
        else if(instr->op == WA_INSTR_br_table)
        {
            wa_operand_slot* slot = wa_operand_stack_get_slots(scratch.arena,
                                                               context,
                                                               instr,
                                                               1,
                                                               (wa_value_type[]){ WA_TYPE_I32 },
                                                               true);

            wa_emit_opcode(context, WA_INSTR_jump_table);
            u64 baseOffset = context->codeLen;

            wa_emit_index(context, instr->immCount);
            wa_emit_index(context, slot->index);

            u64* patchOffsets = oc_arena_push_array(scratch.arena, u64, instr->immCount);

            // reserve room for the table entries
            for(u32 i = 0; i < instr->immCount; i++)
            {
                patchOffsets[i] = context->codeLen;
                wa_emit_i64(context, 0);
            }

            // each entry jumps to a block that moves the results to the correct slots
            // and jumps to the actual destination
            //TODO: we can avoid this trampoline for branches that don't need result values
            for(u32 i = 0; i < instr->immCount; i++)
            {
                context->code[patchOffsets[i]].valI64 = context->codeLen - baseOffset;
                u32 label = instr->imm[i].index;
                wa_compile_branch(context, instr, label);
            }
            wa_block_set_polymorphic(context);
        }
        else if(instr->op == WA_INSTR_end)
        {
            wa_block* block = wa_control_stack_top(context);
            OC_ASSERT(block, "Unbalanced control stack.");

            if(context->controlStackLen == 1)
            {
                //TODO: is this sufficient to elide all previous returns?
                wa_instr* prev = oc_list_prev_entry(instructions, instr, wa_instr, listElt);
                if(!prev || prev->op != WA_INSTR_return)
                {
                    wa_compile_return(context, type, instr);
                }
                wa_patch_jump_targets(context, block);
                wa_control_stack_pop(context);

                OC_ASSERT(oc_list_last(instructions) == &instr->listElt);
            }
            else
            {
                wa_block_end(context, block, instr);
                wa_patch_jump_targets(context, block);
            }
        }
        else if(instr->op == WA_INSTR_call || instr->op == WA_INSTR_call_indirect)
        {
            //NOTE: compute max used slot
            //TODO: we could probably be more clever here, eg in some case just move the index operand
            //      past the arguments?
            i64 maxUsedSlot = func->localCount;

            for(u32 stackIndex = 0; stackIndex < context->opdStackLen; stackIndex++)
            {
                wa_operand_slot* slot = &context->opdStack[stackIndex];
                maxUsedSlot = oc_max((i64)slot->index, maxUsedSlot);
            }

            //NOTE: get callee type, and indirect index for call indirect
            wa_operand_slot* indirectSlot = 0;
            wa_func* callee = 0;
            wa_func_type* type = 0;

            if(instr->op == WA_INSTR_call)
            {
                callee = &module->functions[instr->imm[0].index];
                type = callee->type;
            }
            else
            {
                indirectSlot = wa_operand_stack_get_slots(scratch.arena,
                                                          context,
                                                          instr,
                                                          1,
                                                          (wa_value_type[]){ WA_TYPE_I32 },
                                                          true);

                type = &module->types[instr->imm[0].index];
            }
            u32 paramCount = type->paramCount;

            //NOTE: put call args at the end of the stack
            //TODO: first check if args are already in order at the end of the frame?

            wa_operand_slot* argSlots = wa_operand_stack_get_slots(scratch.arena,
                                                                   context,
                                                                   instr,
                                                                   type->paramCount,
                                                                   type->params,
                                                                   true);

            for(u32 argIndex = 0; argIndex < paramCount; argIndex++)
            {
                wa_emit_opcode(context, WA_INSTR_move);
                wa_emit_index(context, maxUsedSlot + 1 + argIndex);
                wa_emit_index(context, argSlots[argIndex].index);
            }

            if(instr->op == WA_INSTR_call)
            {
                wa_emit_opcode(context, WA_INSTR_call);
                wa_emit_index(context, instr->imm[0].index);
                wa_emit_i64(context, maxUsedSlot + 1);
            }
            else
            {
                wa_emit_opcode(context, WA_INSTR_call_indirect);
                wa_emit_index(context, instr->imm[0].index);
                wa_emit_index(context, instr->imm[1].index);
                wa_emit_i64(context, maxUsedSlot + 1);
                wa_emit_index(context, indirectSlot->index);
            }
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //TODO: we need to update next reg index so that next reg can't be allocated in the same slot...
            //      we probably also need to put the remaining args regs not used by returns in the freelist
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////
            context->nextRegIndex = oc_max(maxUsedSlot + 1 + type->returnCount, context->nextRegIndex);

            for(u32 retIndex = 0; retIndex < type->returnCount; retIndex++)
            {
                wa_operand_stack_push(context,
                                      (wa_operand_slot){
                                          .kind = WA_OPERAND_SLOT_REG,
                                          .type = type->returns[retIndex],
                                          .index = maxUsedSlot + 1 + retIndex,
                                      });
            }
        }
        else if(instr->op == WA_INSTR_return)
        {
            wa_compile_return(context, type, instr);

            wa_block* block = wa_control_stack_top(context);
            block->polymorphic = true;
            wa_operand_stack_pop_scope(context, block);
        }
        else
        {
            //NOTE: common codepath for all other instructions

            u32 immCount = instr->immCount;
            wa_code* imm = instr->imm;

            u32 inCount = info->inCount;
            wa_value_type* in = (wa_value_type*)info->in;

            u32 outCount = info->outCount;
            wa_value_type* out = (wa_value_type*)info->out;

            //NOTE: inputs/outputs types derived from immediates
            switch(instr->op)
            {
                case WA_INSTR_select_t:
                {
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
                    in[0] = instr->imm[0].valueType;
                    in[1] = instr->imm[0].valueType;
                    in[2] = WA_TYPE_I32;
                }
                break;

                case WA_INSTR_if:
                {
                    inCount = instr->blockType->paramCount + 1;
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
                    memcpy(in, instr->blockType->params, instr->blockType->paramCount * sizeof(wa_value_type));
                    in[inCount - 1] = WA_TYPE_I32;

                    wa_move_locals_to_registers(context);
                }
                break;

                case WA_INSTR_local_get:
                {
                    out = oc_arena_push_array(scratch.arena, wa_value_type, outCount);
                    u32 localIndex = imm[0].valU32;
                    out[0] = func->locals[localIndex];
                }
                break;
                case WA_INSTR_local_set:
                {
                    u32 localIndex = imm[0].valU32;
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
                    in[0] = func->locals[localIndex];

                    //NOTE: check if the local was used in the stack and if so save it to a slot
                    //      this must be done before popping the stack to avoid saving the local
                    //      to the same reg as the operand (same for local_tee below).
                    wa_move_slot_if_used(context, localIndex);
                }
                break;
                case WA_INSTR_local_tee:
                {
                    u32 localIndex = imm[0].valU32;
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
                    in[0] = func->locals[localIndex];
                    out = oc_arena_push_array(scratch.arena, wa_value_type, outCount);
                    out[0] = func->locals[localIndex];

                    wa_move_slot_if_used(context, localIndex);
                }
                break;
                case WA_INSTR_global_get:
                {
                    out = oc_arena_push_array(scratch.arena, wa_value_type, outCount);
                    u32 globalIndex = imm[0].valU32;
                    out[0] = module->globals[globalIndex].type;
                }
                break;
                case WA_INSTR_global_set:
                {
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
                    u32 globalIndex = imm[0].valU32;
                    in[0] = module->globals[globalIndex].type;
                }
                break;
                case WA_INSTR_ref_null:
                {
                    immCount = 0;
                    out = oc_arena_push_array(scratch.arena, wa_value_type, outCount);
                    out[0] = imm[0].valueType;
                }
                break;
                case WA_INSTR_table_grow:
                {
                    u32 tableIndex = imm[0].valU32;
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
                    in[0] = module->tables[tableIndex].type;
                    in[1] = WA_TYPE_I32;
                }
                break;
                case WA_INSTR_table_get:
                {
                    u32 tableIndex = imm[0].valU32;
                    out = oc_arena_push_array(scratch.arena, wa_value_type, outCount);
                    out[0] = module->tables[tableIndex].type;
                }
                break;
                case WA_INSTR_table_set:
                {
                    u32 tableIndex = imm[0].valU32;
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
                    in[0] = WA_TYPE_I32;
                    in[1] = module->tables[tableIndex].type;
                }
                break;
                case WA_INSTR_table_fill:
                {
                    u32 tableIndex = imm[0].valU32;
                    in = oc_arena_push_array(scratch.arena, wa_value_type, outCount);
                    in[0] = WA_TYPE_I32;
                    in[1] = module->tables[tableIndex].type;
                    in[2] = WA_TYPE_I32;
                }
                break;

                case WA_INSTR_memory_size:
                case WA_INSTR_memory_grow:
                case WA_INSTR_memory_copy:
                case WA_INSTR_memory_fill:
                {
                    immCount = 0;
                }
                break;

                case WA_INSTR_memory_init:
                {
                    immCount = 1;
                }
                break;

                default:
                    break;
            }

            //NOTE: get inputs
            wa_operand_slot* inSlots = wa_operand_stack_get_slots(scratch.arena,
                                                                  context,
                                                                  instr,
                                                                  inCount,
                                                                  in,
                                                                  true);

            //NOTE: additional input checks
            if((instr->op >= WA_INSTR_i32_load && instr->op <= WA_INSTR_memory_grow)
               || instr->op == WA_INSTR_memory_init
               || instr->op == WA_INSTR_memory_copy
               || instr->op == WA_INSTR_memory_fill)
            {
                if(module->memoryCount == 0)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "found memory instruction, but the module has no declared memory.\n");
                }
            }

            //NOTE: custom checks and emit
            if(instr->op == WA_INSTR_unreachable)
            {
                wa_emit_opcode(context, WA_INSTR_unreachable);
                wa_block_set_polymorphic(context);
            }
            else if(instr->op == WA_INSTR_drop
                    || instr->op == WA_INSTR_nop)
            {
                // do nothing
            }
            else if(instr->op == WA_INSTR_select
                    || instr->op == WA_INSTR_select_t)
            {
                if(!wa_check_operand_type(inSlots[0].type, inSlots[1].type))
                {
                    wa_compile_error(context, instr->ast, "select operands must be of same type\n");
                }
                out = oc_arena_push_array(scratch.arena, wa_value_type, outCount);
                out[0] = inSlots[0].type;

                u32 outIndex = wa_operand_stack_push_reg(context, out[0], instr);

                wa_emit_opcode(context, WA_INSTR_select);
                wa_emit_index(context, outIndex);
                wa_emit_index(context, inSlots[0].index);
                wa_emit_index(context, inSlots[1].index);
                wa_emit_index(context, inSlots[2].index);
            }
            else if(instr->op == WA_INSTR_local_get)
            {
                u32 localIndex = instr->imm[0].index;

                wa_operand_stack_push(
                    context,
                    (wa_operand_slot){
                        .kind = WA_OPERAND_SLOT_LOCAL,
                        .type = func->locals[localIndex],
                        .index = localIndex,
                        .originInstr = instr,
                        .originOpd = context->codeLen,
                    });
            }
            else if(instr->op == WA_INSTR_local_set || instr->op == WA_INSTR_local_tee)
            {
                u32 localIndex = instr->imm[0].valU32;

                //TODO: check if the local was written to since the value was pushed, and if not, change
                //      the output operand of the value's origin instruction rather than issuing a move.
                //WARN:  this can't be used after a branch, since the branch might use the original slot index
                //      so we'd need to add a "touched" bit and set it for operands used in a branch?
                wa_emit_opcode(context, WA_INSTR_move);
                wa_emit_index(context, localIndex);
                wa_emit_index(context, inSlots[0].index);

                if(instr->op == WA_INSTR_local_tee)
                {
                    wa_operand_stack_push(
                        context,
                        (wa_operand_slot){
                            .kind = WA_OPERAND_SLOT_LOCAL,
                            .type = func->locals[localIndex],
                            .index = localIndex,
                            .originInstr = instr,
                            .originOpd = context->codeLen,
                        });
                }
            }
            else if(instr->op == WA_INSTR_global_get)
            {
                u32 globalIndex = instr->imm[0].valU32;

                u32 regIndex = wa_operand_stack_push_reg(context,
                                                         module->globals[globalIndex].type,
                                                         instr);

                wa_emit_opcode(context, WA_INSTR_global_get);
                wa_emit_index(context, regIndex);
                wa_emit_index(context, globalIndex);
            }
            else if(instr->op == WA_INSTR_global_set)
            {
                u32 globalIndex = instr->imm[0].valU32;
                wa_emit_opcode(context, WA_INSTR_global_set);
                wa_emit_index(context, globalIndex);
                wa_emit_index(context, inSlots[0].index);
            }
            else
            {
                //NOTE generic emit code
                wa_emit_opcode(context, instr->op);

                for(int opdIndex = 0; opdIndex < outCount; opdIndex++)
                {
                    u32 outIndex = wa_operand_stack_push_reg(context, out[opdIndex], instr);
                    wa_emit_index(context, outIndex);
                }

                for(int immIndex = 0; immIndex < immCount; immIndex++)
                {
                    wa_emit(context, &instr->imm[immIndex]);
                }

                for(u32 i = 0; i < inCount; i++)
                {
                    wa_emit_index(context, inSlots[i].index);
                }
            }
        }
    }
    //NOTE: clear remaining tmp data if we exited the loop early
    oc_scratch_end(scratch);
}

void wa_compile_code(oc_arena* arena, wa_module* module)
{
    wa_build_context context = {
        .arena = arena,
        .module = module,
    };
    oc_arena_init(&context.codeArena);
    oc_arena_init(&context.checkArena);

    context.codeCap = 4;
    context.code = oc_arena_push_array(&context.codeArena, wa_code, 4);

    for(u32 funcIndex = module->functionImportCount; funcIndex < module->functionCount; funcIndex++)
    {
        wa_func* func = &module->functions[funcIndex];

        wa_build_context_clear(&context);
        context.nextRegIndex = func->localCount;

        wa_compile_expression(&context, func->type, func, func->instructions);

        func->codeLen = context.codeLen;
        func->code = oc_arena_push_array(arena, wa_code, context.codeLen);
        memcpy(func->code, context.code, context.codeLen * sizeof(wa_code));
    }

    for(u32 globalIndex = module->globalImportCount; globalIndex < module->globalCount; globalIndex++)
    {
        wa_global* global = &module->globals[globalIndex];

        wa_build_context_clear(&context);
        context.nextRegIndex = 0;

        i64 t = 0x7f - (i64)global->type + 1;
        wa_func_type* exprType = (wa_func_type*)&WA_BLOCK_VALUE_TYPES[t];

        wa_compile_expression(&context, exprType, 0, global->init);

        global->codeLen = context.codeLen;
        global->code = oc_arena_push_array(arena, wa_code, context.codeLen);
        memcpy(global->code, context.code, context.codeLen * sizeof(wa_code));
    }

    for(u32 eltIndex = 0; eltIndex < module->elementCount; eltIndex++)
    {
        wa_element* element = &module->elements[eltIndex];

        if(!oc_list_empty(element->tableOffset))
        {
            ///////////////////////////////////////////////////////////////////////////
            //TODO: this should go in wa_compile_expression to avoid forgetting it?
            ///////////////////////////////////////////////////////////////////////////
            wa_build_context_clear(&context);
            context.nextRegIndex = 0;

            wa_compile_expression(&context, (wa_func_type*)&WA_BLOCK_VALUE_TYPES[1], 0, element->tableOffset);
            element->tableOffsetCode = oc_arena_push_array(arena, wa_code, context.codeLen);
            memcpy(element->tableOffsetCode, context.code, context.codeLen * sizeof(wa_code));
        }

        if(element->initCount)
        {
            element->code = oc_arena_push_array(arena, wa_code*, element->initCount);
            for(u32 exprIndex = 0; exprIndex < element->initCount; exprIndex++)
            {
                wa_build_context_clear(&context);
                context.nextRegIndex = 0;

                i64 t = 0x7f - (i64)element->type + 1;
                wa_func_type* exprType = (wa_func_type*)&WA_BLOCK_VALUE_TYPES[t];

                OC_ASSERT(!oc_list_empty(element->initInstr[exprIndex]));

                wa_compile_expression(&context, exprType, 0, element->initInstr[exprIndex]);

                element->code[exprIndex] = oc_arena_push_array(arena, wa_code, context.codeLen);
                memcpy(element->code[exprIndex], context.code, context.codeLen * sizeof(wa_code));
            }
        }
    }

    for(u32 dataIndex = 0; dataIndex < module->dataCount; dataIndex++)
    {
        wa_data_segment* seg = &module->data[dataIndex];

        if(!oc_list_empty(seg->memoryOffset))
        {
            wa_build_context_clear(&context);
            context.nextRegIndex = 0;

            wa_compile_expression(&context, (wa_func_type*)&WA_BLOCK_VALUE_TYPES[1], 0, seg->memoryOffset);
            seg->memoryOffsetCode = oc_arena_push_array(arena, wa_code, context.codeLen);
            memcpy(seg->memoryOffsetCode, context.code, context.codeLen * sizeof(wa_code));
        }
    }

    oc_arena_cleanup(&context.codeArena);
    oc_arena_cleanup(&context.checkArena);
}

wa_module* wa_module_create(oc_arena* arena, oc_str8 contents)
{
    wa_module* module = oc_arena_push_type(arena, wa_module);
    memset(module, 0, sizeof(wa_module));

    wa_parser parser = {
        .module = module,
        .arena = arena,
        .len = contents.len,
        .contents = contents.ptr,
    };

    module->root = wa_ast_alloc(&parser, WA_AST_ROOT);

    wa_ast* magic = wa_read_raw_u32(&parser, module->root, OC_STR8("wasm magic number"));
    if(magic->kind == WA_AST_U32
       && magic->valU32 != 0x6d736100)
    {
        wa_parse_error(&parser, magic, "wrong wasm magic number");
        return module;
    }
    magic->kind = WA_AST_MAGIC;

    wa_ast* version = wa_read_raw_u32(&parser, module->root, OC_STR8("wasm version"));
    if(version->kind == WA_AST_U32
       && version->valU32 != 1)
    {
        wa_parse_error(&parser, version, "wrong wasm version");
        return module;
    }

    wa_parse_sections(&parser, module);
    wa_parse_types(&parser, module);
    wa_parse_imports(&parser, module);
    wa_parse_functions(&parser, module);
    wa_parse_globals(&parser, module);
    wa_parse_tables(&parser, module);
    wa_parse_memories(&parser, module);
    wa_parse_exports(&parser, module);
    wa_parse_start(&parser, module);
    wa_parse_elements(&parser, module);
    wa_parse_code(&parser, module);
    wa_parse_data(&parser, module);

    if(oc_list_empty(module->errors))
    {
        wa_compile_code(arena, module);
    }

    return (module);
}

//-------------------------------------------------------------------------
// debug print
//-------------------------------------------------------------------------
void wa_ast_print_indent(oc_arena* arena, oc_str8_list* line, u32 indent)
{
    for(int i = 0; i < indent; i++)
    {
        oc_str8_list_pushf(arena, line, "  ");
    }
}

void wa_ast_print_listing(oc_arena* arena, oc_str8_list* listing, oc_str8_list* raw, wa_ast* ast, oc_str8 contents, u32 indent)
{
    oc_str8_list line = { 0 };
    oc_str8_list_pushf(arena, &line, "0x%08llx  ", ast->loc.start);

    wa_ast_print_indent(arena, &line, indent);

    if(ast->label.len)
    {
        oc_str8_list_pushf(arena,
                           &line,
                           "[%s] %.*s",
                           wa_ast_kind_strings[ast->kind],
                           oc_str8_ip(ast->label));
    }
    else
    {
        oc_str8_list_pushf(arena, &line, "[%s]", wa_ast_kind_strings[ast->kind]);
    }

    switch(ast->kind)
    {
        case WA_AST_U8:
            oc_str8_list_pushf(arena, &line, ": 0x%.2hhx", ast->valU8);
            break;
        case WA_AST_U32:
            oc_str8_list_pushf(arena, &line, ": %u", ast->valU32);
            break;
        case WA_AST_I32:
            oc_str8_list_pushf(arena, &line, ": %i", ast->valI32);
            break;
        case WA_AST_U64:
            oc_str8_list_pushf(arena, &line, ": %llu", ast->valU64);
            break;
        case WA_AST_I64:
            oc_str8_list_pushf(arena, &line, ": %lli", ast->valI64);
            break;
        case WA_AST_F32:
            oc_str8_list_pushf(arena, &line, ": %f", ast->valF32);
            break;
        case WA_AST_F64:
            oc_str8_list_pushf(arena, &line, ": %f", ast->valF64);
            break;
        case WA_AST_NAME:
            oc_str8_list_pushf(arena, &line, ": %.*s", oc_str8_ip(ast->str8));
            break;

        case WA_AST_VALUE_TYPE:
            oc_str8_list_pushf(arena, &line, ": %s", wa_value_type_string(ast->valU32));
            break;

        case WA_AST_INSTR:
        {
            oc_str8_list_pushf(arena, &line, ": %s", wa_instr_strings[ast->instr->op]);
        }
        break;

        default:
            break;
    }

    if(oc_list_empty(ast->children))
    {
        oc_str8_list rawLine = { 0 };
        oc_str8_list_pushf(arena, &rawLine, "0x");
        for(u64 i = 0; i < ast->loc.len; i++)
        {
            oc_str8_list_pushf(arena, &rawLine, "%02x", contents.ptr[ast->loc.start + i]);
        }

        oc_str8 r = oc_str8_list_join(arena, rawLine);
        oc_str8_list_push(arena, raw, r);
    }
    else
    {
        oc_str8_list_push(arena, raw, OC_STR8(""));
    }

    oc_str8 str = oc_str8_list_join(arena, line);
    oc_str8_list_push(arena, listing, str);

    oc_list_for(ast->children, child, wa_ast, parentElt)
    {
        wa_ast_print_listing(arena, listing, raw, child, contents, indent + 1);
    }
}

void wa_ast_print(wa_ast* ast, oc_str8 contents)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8_list listing = { 0 };
    oc_str8_list raw = { 0 };
    wa_ast_print_listing(scratch.arena, &listing, &raw, ast, contents, 0);

    u32 maxWidth = 0;
    oc_str8_list_for(listing, line)
    {
        maxWidth = oc_max(line->string.len, maxWidth);
    }

    oc_str8_list result = { 0 };
    oc_str8_elt* eltA = oc_list_first_entry(listing.list, oc_str8_elt, listElt);
    oc_str8_elt* eltB = oc_list_first_entry(raw.list, oc_str8_elt, listElt);

    for(;
        eltA != 0 && eltB != 0;
        eltA = oc_list_next_entry(listing.list, eltA, oc_str8_elt, listElt),
        eltB = oc_list_next_entry(raw.list, eltB, oc_str8_elt, listElt))
    {
        oc_str8_list line = { 0 };
        oc_str8_list_push(scratch.arena, &line, eltA->string);
        for(int i = 0; i < maxWidth - eltA->string.len; i++)
        {
            oc_str8_list_push(scratch.arena, &line, OC_STR8(" "));
        }
        oc_str8_list_push(scratch.arena, &line, OC_STR8(" | "));
        oc_str8_list_push(scratch.arena, &line, eltB->string);

        oc_str8 r = oc_str8_list_join(scratch.arena, line);
        oc_str8_list_push(scratch.arena, &result, r);
    }

    oc_str8_list_for(result, line)
    {
        printf("%.*s\n", oc_str8_ip(line->string));
    }

    oc_scratch_end(scratch);
}

void wa_print_bytecode(u64 len, wa_code* bytecode)
{
    for(u64 codeIndex = 0; codeIndex < len; codeIndex++)
    {
        u64 startIndex = codeIndex;

        wa_code* c = &bytecode[codeIndex];
        printf("0x%08llx ", codeIndex);
        printf("%-16s0x%02x ", wa_instr_strings[c->opcode], c->opcode);

        const wa_instr_info* info = &wa_instr_infos[c->opcode];

        for(u64 i = 0; i < info->opdCount; i++)
        {
            codeIndex++;
            if(codeIndex >= len)
            {
                break;
            }
            printf("0x%02llx ", bytecode[codeIndex].valI64);
        }

        if(c->opcode == WA_INSTR_jump_table)
        {
            printf("\n\t");
            u64 brCount = bytecode[startIndex + 1].valI32;
            for(u64 i = 0; i < brCount; i++)
            {
                codeIndex++;
                printf("0x%02llx ", bytecode[codeIndex].valI64);
            }
        }

        printf("\n");
    }
    printf("0x%08llx eof\n", len);
}

void wa_print_code(wa_module* module)
{
    printf("\n\nCompiled Code:\n\n");
    for(u64 funcIndex = 0; funcIndex < module->functionCount; funcIndex++)
    {
        wa_func* func = &module->functions[funcIndex];

        for(u64 exportIndex = 0; exportIndex < module->exportCount; exportIndex++)
        {
            wa_export* export = &module->exports[exportIndex];
            if(export->kind == WA_EXPORT_FUNCTION && export->index == funcIndex)
            {
                printf("%.*s:\n", oc_str8_ip(export->name));
                break;
            }
        }
        wa_print_bytecode(func->codeLen, func->code);
    }
    printf("\n");
}

//-------------------------------------------------------------------------
// instance
//-------------------------------------------------------------------------

typedef struct wa_instance
{
    oc_arena* arena;
    wa_module* module;

    wa_func* functions;
    wa_value** globals;
    wa_table** tables;
    wa_memory** memories;

    wa_data_segment* data;
    wa_element* elements;
} wa_instance;

wa_instance* wa_instance_create(oc_arena* arena, wa_module* module)
{
    wa_instance* instance = oc_arena_push_type(arena, wa_instance);
    memset(instance, 0, sizeof(wa_instance));

    instance->arena = arena;

    instance->module = module;
    instance->functions = oc_arena_push_array(arena, wa_func, module->functionCount);
    memcpy(instance->functions, module->functions, module->functionCount * sizeof(wa_func));

    instance->globals = oc_arena_push_array(arena, wa_value*, module->globalCount);
    memset(instance->globals, 0, module->globalImportCount * sizeof(wa_value*));

    for(u32 globalIndex = module->globalImportCount; globalIndex < module->globalCount; globalIndex++)
    {
        instance->globals[globalIndex] = oc_arena_push_type(arena, wa_value);
        memset(instance->globals[globalIndex], 0, sizeof(wa_value));
    }

    // tables
    instance->tables = oc_arena_push_array(arena, wa_table*, module->tableCount);
    memset(instance->tables, 0, module->tableImportCount * sizeof(wa_table*));

    for(u32 tableIndex = module->tableImportCount; tableIndex < module->tableCount; tableIndex++)
    {
        wa_table_type* desc = &module->tables[tableIndex];
        wa_table* table = oc_arena_push_type(arena, wa_table);

        table->size = desc->limits.min;
        table->contents = oc_arena_push_array(arena, wa_value, desc->limits.min);
        memset(table->contents, 0, desc->limits.min * sizeof(wa_value));

        instance->tables[tableIndex] = table;
    }

    // elements
    instance->elements = oc_arena_push_array(arena, wa_element, module->elementCount);
    memcpy(instance->elements, module->elements, module->elementCount * sizeof(wa_element));
    for(u32 eltIndex = 0; eltIndex < module->elementCount; eltIndex++)
    {
        wa_element* elt = &instance->elements[eltIndex];
        elt->refs = oc_arena_push_array(arena, wa_value, elt->initCount);
        memset(elt->refs, 0, elt->initCount * sizeof(wa_element));
    }

    // memories
    oc_base_allocator* allocator = oc_base_allocator_default();

    instance->memories = oc_arena_push_array(arena, wa_memory*, module->memoryCount);
    memset(instance->memories, 0, module->memoryImportCount * sizeof(wa_memory*));

    for(u32 memIndex = module->memoryImportCount; memIndex < module->memoryCount; memIndex++)
    {
        wa_limits* limits = &module->memories[memIndex];
        wa_memory* mem = oc_arena_push_type(arena, wa_memory);

        mem->limits = *limits;
        mem->cap = (limits->kind == WA_LIMIT_MIN) ? 4LLU << 30 : limits->max * WA_PAGE_SIZE;
        mem->size = limits->min * WA_PAGE_SIZE;
        mem->ptr = oc_base_reserve(allocator, mem->cap);
        oc_base_commit(allocator, mem->ptr, mem->size);

        instance->memories[memIndex] = mem;
    }

    instance->data = oc_arena_push_array(arena, wa_data_segment, module->dataCount);
    memcpy(instance->data, module->data, module->dataCount * sizeof(wa_data_segment));

    return (instance);
}

bool wa_check_function_type(wa_func_type* t1, wa_func_type* t2)
{
    if(t1->paramCount != t2->paramCount || t1->returnCount != t2->returnCount)
    {
        return (false);
    }
    for(u32 i = 0; i < t1->paramCount; i++)
    {
        if(t1->params[i] != t2->params[i])
        {
            return (false);
        }
    }
    for(u32 i = 0; i < t1->returnCount; i++)
    {
        if(t1->returns[i] != t2->returns[i])
        {
            return (false);
        }
    }
    return true;
}

bool wa_check_limits_match(wa_limits* l1, wa_limits* l2)
{
    return ((l1->min >= l2->min)
            && (l2->kind == WA_LIMIT_MIN
                || (l1->kind == WA_LIMIT_MIN_MAX && l1->max <= l2->max)));
}

//TODO: temporary for tests, coalesce this with normal function binding
wa_status wa_instance_bind_function_wasm(wa_instance* instance, oc_str8 name, wa_func_type* type, wa_instance* extInstance, u32 extIndex)
{
    wa_module* module = instance->module;
    u32 funcIndex = 0;

    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];

        if(!oc_str8_cmp(name, import->importName))
        {
            if(import->kind != WA_IMPORT_FUNCTION)
            {
                return WA_FAIL_IMPORT_TYPE_MISMATCH;
            }

            wa_func* func = &instance->functions[funcIndex];
            func->extInstance = extInstance;
            func->extIndex = extIndex;

            if(!wa_check_function_type(type, func->type))
            {
                return WA_FAIL_IMPORT_TYPE_MISMATCH;
            }
        }
        funcIndex++;
    }
    return (WA_OK);
}

wa_status wa_instance_bind_function(wa_instance* instance, oc_str8 name, wa_func_type* type, wa_host_proc proc)
{
    wa_module* module = instance->module;
    u32 funcIndex = 0;

    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];

        if(!oc_str8_cmp(name, import->importName))
        {
            if(import->kind != WA_IMPORT_FUNCTION)
            {
                return WA_FAIL_IMPORT_TYPE_MISMATCH;
            }
            wa_func* func = &instance->functions[funcIndex];
            func->proc = proc;

            if(type->paramCount != func->type->paramCount
               || type->returnCount != func->type->returnCount)
            {
                //log error to module
                return WA_FAIL_IMPORT_TYPE_MISMATCH;
            }
            for(u32 paramIndex = 0; paramIndex < type->paramCount; paramIndex++)
            {
                if(type->params[paramIndex] != func->type->params[paramIndex])
                {
                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                }
            }
            for(u32 returnIndex = 0; returnIndex < type->returnCount; returnIndex++)
            {
                if(type->returns[returnIndex] != func->type->returns[returnIndex])
                {
                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                }
            }
        }
        if(import->kind == WA_IMPORT_FUNCTION)
        {
            funcIndex++;
        }
    }
    return (WA_OK);
}

wa_status wa_instance_bind_global(wa_instance* instance, oc_str8 name, wa_value_type type, bool mutable, wa_value* value)
{
    wa_module* module = instance->module;
    u32 globalIndex = 0;

    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];

        if(!oc_str8_cmp(name, import->importName))
        {
            if(import->kind != WA_IMPORT_GLOBAL)
            {
                return WA_FAIL_IMPORT_TYPE_MISMATCH;
            }

            wa_global* globalDesc = &module->globals[globalIndex];
            if(globalDesc->type != type || globalDesc->mut != mutable)
            {
                return WA_FAIL_IMPORT_TYPE_MISMATCH;
            }
            instance->globals[globalIndex] = value;
        }
        if(import->kind == WA_IMPORT_GLOBAL)
        {
            globalIndex++;
        }
    }
    return (WA_OK);
}

wa_status wa_instance_bind_memory(wa_instance* instance, oc_str8 moduleName, oc_str8 name, wa_memory* memory)
{
    wa_module* module = instance->module;
    u32 memIndex = 0;

    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];

        if(!oc_str8_cmp(name, import->importName) && !oc_str8_cmp(moduleName, import->moduleName))
        {
            if(import->kind != WA_IMPORT_MEMORY)
            {
                return WA_FAIL_IMPORT_TYPE_MISMATCH;
            }
            if(!wa_check_limits_match(&memory->limits, &module->memories[memIndex]))
            {
                return WA_FAIL_IMPORT_TYPE_MISMATCH;
            }

            instance->memories[memIndex] = memory;
        }
        if(import->kind == WA_IMPORT_MEMORY)
        {
            memIndex++;
        }
    }
    return (WA_OK);
}

wa_status wa_instance_bind_table(wa_instance* instance, oc_str8 moduleName, oc_str8 name, wa_table_type desc, wa_table* table)
{
    wa_module* module = instance->module;
    u32 tableIndex = 0;

    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];

        if(!oc_str8_cmp(name, import->importName) && !oc_str8_cmp(moduleName, import->moduleName))
        {
            if(import->kind != WA_IMPORT_TABLE)
            {
                return WA_FAIL_IMPORT_TYPE_MISMATCH;
            }

            if(module->tables[tableIndex].type != desc.type
               || !wa_check_limits_match(&desc.limits, &module->tables[tableIndex].limits))
            {
                return WA_FAIL_IMPORT_TYPE_MISMATCH;
            }

            instance->tables[tableIndex] = table;
        }
        if(import->kind == WA_IMPORT_TABLE)
        {
            tableIndex++;
        }
    }
    return (WA_OK);
}

wa_status wa_instance_interpret_expr(wa_instance* instance,
                                     wa_func_type* type,
                                     wa_code* code,
                                     u32 argCount,
                                     wa_value* args,
                                     u32 retCount,
                                     wa_value* returns);

wa_status wa_instance_invoke(wa_instance* instance,
                             wa_func* func,
                             u32 argCount,
                             wa_value* args,
                             u32 retCount,
                             wa_value* returns);

wa_status wa_instance_link(wa_instance* instance)
{
    wa_module* module = instance->module;

    for(u32 funcIndex = 0; funcIndex < module->functionImportCount; funcIndex++)
    {
        wa_func* func = &instance->functions[funcIndex];
        if(!func->proc && !func->extInstance)
        {
            //oc_log_error("Couldn't link instance: import %.*s not satisfied.\n", oc_str8_ip(func->import->importName));
            return WA_FAIL_MISSING_IMPORT;
        }
    }

    for(u32 globalIndex = 0; globalIndex < module->globalImportCount; globalIndex++)
    {
        if(instance->globals[globalIndex] == 0)
        {
            return WA_FAIL_MISSING_IMPORT;
        }
    }

    for(u32 tableIndex = 0; tableIndex < module->tableImportCount; tableIndex++)
    {
        if(instance->tables[tableIndex] == 0)
        {
            //oc_log_error("Coulnd't link instance: table import is not satisfied\n");
            return WA_FAIL_MISSING_IMPORT;
        }
    }

    for(u32 memIndex = 0; memIndex < module->memoryImportCount; memIndex++)
    {
        if(instance->memories[memIndex] == 0)
        {
            //oc_log_error("Coulnd't link instance: memory import is not satisfied\n");
            return WA_FAIL_MISSING_IMPORT;
        }
    }

    for(u64 globalIndex = module->globalImportCount; globalIndex < module->globalCount; globalIndex++)
    {
        wa_global* global = &module->globals[globalIndex];
        if(global->code)
        {
            i64 t = 0x7f - (i64)global->type + 1;
            wa_func_type* exprType = (wa_func_type*)&WA_BLOCK_VALUE_TYPES[t];

            wa_status status = wa_instance_interpret_expr(instance, exprType, global->code, 0, 0, 1, instance->globals[globalIndex]);
            if(status != WA_OK)
            {
                //oc_log_error("Couldn't link instance: couldn't execute global initialization expression.\n");
                return status;
            }
        }
    }

    for(u64 eltIndex = 0; eltIndex < module->elementCount; eltIndex++)
    {
        wa_element* element = &instance->elements[eltIndex];

        i64 t = 0x7f - (i64)element->type + 1;
        wa_func_type* exprType = (wa_func_type*)&WA_BLOCK_VALUE_TYPES[t];

        for(u32 exprIndex = 0; exprIndex < element->initCount; exprIndex++)
        {
            wa_value* result = &element->refs[exprIndex];
            wa_status status = wa_instance_interpret_expr(instance, exprType, element->code[exprIndex], 0, 0, 1, result);
            if(status != WA_OK)
            {
                //oc_log_error("Couldn't link instance: couldn't execute element initialization expression.\n");
                return status;
            }
        }

        if(element->mode == WA_ELEMENT_ACTIVE)
        {
            //TODO: check table size?
            wa_table_type* desc = &module->tables[element->tableIndex];
            wa_table* table = instance->tables[element->tableIndex];

            wa_value offset = { 0 };
            wa_status status = wa_instance_interpret_expr(instance,
                                                          (wa_func_type*)&WA_BLOCK_VALUE_TYPES[1],
                                                          element->tableOffsetCode,
                                                          0, 0,
                                                          1, &offset);
            if(status != WA_OK)
            {
                //oc_log_error("Couldn't link instance: couldn't execute element offset expression.\n");
                return status;
            }
            //TODO: check oob
            if(offset.valI32 + element->initCount > table->size || offset.valI32 + element->initCount < offset.valI32)
            {
                return WA_TRAP_TABLE_OUT_OF_BOUNDS;
            }
            memcpy(&table->contents[offset.valI32], element->refs, element->initCount * sizeof(wa_value));
        }
        if(element->mode == WA_ELEMENT_ACTIVE || element->mode == WA_ELEMENT_DECLARATIVE)
        {
            //NOTE: drop the element
            element->initCount = 0;
        }
    }

    for(u32 dataIndex = 0; dataIndex < module->dataCount; dataIndex++)
    {
        wa_data_segment* seg = &module->data[dataIndex];

        if(seg->mode == WA_DATA_ACTIVE)
        {
            wa_limits* limits = &module->memories[seg->memoryIndex];
            wa_memory* mem = instance->memories[seg->memoryIndex];

            wa_value offsetVal = { 0 };
            wa_status status = wa_instance_interpret_expr(instance,
                                                          (wa_func_type*)&WA_BLOCK_VALUE_TYPES[1],
                                                          seg->memoryOffsetCode,
                                                          0, 0,
                                                          1, &offsetVal);
            if(status != WA_OK)
            {
                //oc_log_error("Couldn't link instance: couldn't execute data offset expression.\n");
                return status;
            }
            u32 offset = *(u32*)&offsetVal.valI32;

            if(offset + seg->init.len > mem->size)
            {
                //oc_log_error("Couldn't link instance: data offset out of bounds.\n");
                return WA_TRAP_MEMORY_OUT_OF_BOUNDS;
            }
            memcpy(mem->ptr + offset, seg->init.ptr, seg->init.len);
        }
    }

    if(module->hasStart)
    {
        if(module->startIndex >= module->functionCount)
        {
            oc_log_error("Couldn't link instance: invalid start function index.\n");
            return WA_FAIL_MISSING_IMPORT; //TODO: change this
        }
        wa_func* func = &instance->functions[module->startIndex];

        if(func->type->paramCount || func->type->returnCount)
        {
            oc_log_error("Couldn't link instance: invalid start function type.\n");
            return WA_FAIL_MISSING_IMPORT; //TODO: change this
        }

        wa_status status = wa_instance_invoke(instance, func, 0, 0, 0, 0);
        if(status != WA_OK)
        {
            return status;
        }
    }

    return WA_OK;
}

wa_status wa_instance_invoke(wa_instance* instance,
                             wa_func* func,
                             u32 argCount,
                             wa_value* args,
                             u32 retCount,
                             wa_value* returns);

wa_func* wa_instance_find_function(wa_instance* instance, oc_str8 name)
{
    wa_module* module = instance->module;

    wa_func* func = 0;
    for(u32 exportIndex = 0; exportIndex < module->exportCount; exportIndex++)
    {
        wa_export* export = &module->exports[exportIndex];
        if(export->kind == WA_EXPORT_FUNCTION && !oc_str8_cmp(export->name, name))
        {
            func = &instance->functions[export->index];
            break;
        }
    }
    return (func);
}

typedef struct wa_control
{
    wa_code* returnPC;
    wa_value* returnFrame;
} wa_control;

wa_status wa_instance_interpret_expr(wa_instance* instance,
                                     wa_func_type* type,
                                     wa_code* code,
                                     u32 argCount,
                                     wa_value* args,
                                     u32 retCount,
                                     wa_value* returns)
{
    wa_control controlStack[1024] = { 0 };
    u32 controlStackTop = 0;

    wa_value localsBuffer[1024] = { 0 };
    wa_value* locals = localsBuffer;
    wa_code* pc = code;

    memcpy(locals, args, argCount * sizeof(wa_value));

    char* memPtr = 0;
    wa_memory* memory = 0;
    if(instance->module->memoryCount)
    {
        memory = instance->memories[0];
        memPtr = memory->ptr;
    }

    while(1)
    {
        wa_instr_op opcode = pc->opcode;
        pc++;

        switch(opcode)
        {

#define I0 pc[0]
#define I1 pc[1]
#define I2 pc[2]
#define I3 pc[3]

#define L0 locals[pc[0].valI32]
#define L1 locals[pc[1].valI32]
#define L2 locals[pc[2].valI32]
#define L3 locals[pc[3].valI32]
#define L4 locals[pc[4].valI32]

#define G0 instance->globals[pc[0].valI32]
#define G1 instance->globals[pc[1].valI32]

#define RES L0
#define OPD1 L1
#define OPD2 L2

            case WA_INSTR_unreachable:
            {
                return WA_TRAP_UNREACHABLE;
            }
            break;

            case WA_INSTR_i32_const:
            {
                L0.valI32 = I1.valI32;
                pc += 2;
            }
            break;

            case WA_INSTR_i64_const:
            {
                L0.valI64 = I1.valI64;
                pc += 2;
            }
            break;

            case WA_INSTR_f32_const:
            {
                L0.valF32 = I1.valF32;
                pc += 2;
            }
            break;

            case WA_INSTR_f64_const:
            {
                L0.valF64 = I1.valF64;
                pc += 2;
            }
            break;

            case WA_INSTR_move:
            {
                memcpy(&L0, &L1, sizeof(wa_value));
                pc += 2;
            }
            break;

            case WA_INSTR_global_get:
            {
                memcpy(&L0, G1, sizeof(wa_value));
                pc += 2;
            }
            break;

            case WA_INSTR_global_set:
            {
                memcpy(G0, &L1, sizeof(wa_value));
                pc += 2;
            }
            break;

            case WA_INSTR_select:
            {
                if(L3.valI32)
                {
                    memcpy(&L0, &L1, sizeof(u64));
                }
                else
                {
                    memcpy(&L0, &L2, sizeof(u64));
                }
                pc += 4;
            }
            break;

#define WA_CHECK_READ_ACCESS(t)                                             \
    u32 offset = I1.memArg.offset + (u32)L2.valI32;                         \
    if(offset < I1.memArg.offset                                            \
       || offset + sizeof(t) > memory->size || offset + sizeof(t) < offset) \
    {                                                                       \
        return WA_TRAP_MEMORY_OUT_OF_BOUNDS;                                \
    }

            case WA_INSTR_i32_load:
            {
                WA_CHECK_READ_ACCESS(i32);
                L0.valI32 = *(i32*)&memPtr[I1.memArg.offset + (u32)L2.valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i64_load:
            {
                WA_CHECK_READ_ACCESS(i64);
                L0.valI64 = *(i64*)&memPtr[I1.memArg.offset + (u32)L2.valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_f32_load:
            {
                WA_CHECK_READ_ACCESS(f32);
                L0.valF32 = *(f32*)&memPtr[I1.memArg.offset + (u32)L2.valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_f64_load:
            {
                WA_CHECK_READ_ACCESS(f64);
                L0.valF64 = *(f64*)&memPtr[I1.memArg.offset + (u32)L2.valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i32_load8_s:
            {
                WA_CHECK_READ_ACCESS(u8);
                L0.valI32 = (i32) * (i8*)&memPtr[I1.memArg.offset + (u32)L2.valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i32_load8_u:
            {
                WA_CHECK_READ_ACCESS(u8);
                *(u32*)&L0.valI32 = (u32) * (u8*)&memPtr[I1.memArg.offset + (u32)L2.valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i32_load16_s:
            {
                WA_CHECK_READ_ACCESS(u16);
                L0.valI32 = (i32) * (i16*)&memPtr[I1.memArg.offset + (u32)L2.valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i32_load16_u:
            {
                WA_CHECK_READ_ACCESS(u16);
                *(u32*)&L0.valI32 = (u32) * (u16*)&memPtr[I1.memArg.offset + (u32)L2.valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i64_load8_s:
            {
                WA_CHECK_READ_ACCESS(u8);
                L0.valI64 = (i64) * (i8*)&memPtr[I1.memArg.offset + (u32)L2.valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i64_load8_u:
            {
                WA_CHECK_READ_ACCESS(u8);
                *(u32*)&L0.valI64 = (u64) * (u8*)&memPtr[I1.memArg.offset + (u32)L2.valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i64_load16_s:
            {
                WA_CHECK_READ_ACCESS(u16);
                L0.valI64 = (i64) * (i16*)&memPtr[I1.memArg.offset + (u32)L2.valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i64_load16_u:
            {
                WA_CHECK_READ_ACCESS(u16);
                *(u32*)&L0.valI64 = (u64) * (u16*)&memPtr[I1.memArg.offset + (u32)L2.valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i64_load32_s:
            {
                WA_CHECK_READ_ACCESS(u32);
                L0.valI64 = (i64) * (i32*)&memPtr[I1.memArg.offset + (u32)L2.valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i64_load32_u:
            {
                WA_CHECK_READ_ACCESS(u32);
                *(u32*)&L0.valI64 = (u64) * (u32*)&memPtr[I1.memArg.offset + (u32)L2.valI32];
                pc += 3;
            }
            break;

#define WA_CHECK_WRITE_ACCESS(t)                                            \
    u32 offset = I0.memArg.offset + (u32)L1.valI32;                         \
    if(offset < I0.memArg.offset                                            \
       || offset + sizeof(t) > memory->size || offset + sizeof(t) < offset) \
    {                                                                       \
        return WA_TRAP_MEMORY_OUT_OF_BOUNDS;                                \
    }

            case WA_INSTR_i32_store:
            {
                WA_CHECK_WRITE_ACCESS(u32);
                *(i32*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = L2.valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_store:
            {
                WA_CHECK_WRITE_ACCESS(u64);
                *(i64*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = L2.valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_f32_store:
            {
                WA_CHECK_WRITE_ACCESS(f32);
                *(f32*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = L2.valF32;
                pc += 3;
            }
            break;

            case WA_INSTR_f64_store:
            {
                WA_CHECK_WRITE_ACCESS(f64);
                *(f64*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = L2.valF64;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_store8:
            {
                WA_CHECK_WRITE_ACCESS(u8);
                *(u8*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = *(u8*)&L2.valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_store16:
            {
                WA_CHECK_WRITE_ACCESS(u16);
                *(u16*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = *(u16*)&L2.valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_store8:
            {
                WA_CHECK_WRITE_ACCESS(u8);
                *(u8*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = *(u8*)&L2.valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_store16:
            {
                WA_CHECK_WRITE_ACCESS(u16);
                *(u16*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = *(u16*)&L2.valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_store32:
            {
                WA_CHECK_WRITE_ACCESS(u32);
                *(u32*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = *(u32*)&L2.valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_jump:
            {
                pc += I0.valI64;
            }
            break;

            case WA_INSTR_jump_if_zero:
            {
                if(L1.valI32 == 0)
                {
                    pc += I0.valI64;
                }
                else
                {
                    pc += 2;
                }
            }
            break;

            case WA_INSTR_jump_table:
            {
                u32 count = I0.valU32;
                u32 index = L1.valI32;

                if(index >= count)
                {
                    index = count - 1;
                }

                pc += pc[2 + index].valI64;
            }
            break;

            case WA_INSTR_call:
            {
                wa_func* callee = &instance->functions[I0.valI64];

                if(callee->code)
                {
                    controlStack[controlStackTop] = (wa_control){
                        .returnPC = pc + 2,
                        .returnFrame = locals,
                    };
                    controlStackTop++;

                    if(controlStackTop >= 1024)
                    {
                        return (WA_TRAP_STACK_OVERFLOW);
                    }

                    locals += I1.valI64;
                    pc = callee->code;
                }
                else if(callee->extInstance)
                {
                    //TODO temporary for test, do it better

                    wa_func* extFunc = &callee->extInstance->functions[callee->extIndex];

                    wa_status status = wa_instance_invoke(callee->extInstance,
                                                          extFunc,
                                                          callee->type->paramCount,
                                                          locals + I1.valI64,
                                                          callee->type->returnCount,
                                                          locals + I1.valI64);

                    if(status != WA_OK)
                    {
                        return status;
                    }
                    pc += 2;
                }
                else
                {
                    wa_value* saveLocals = locals;
                    locals += I1.valI64;
                    callee->proc(locals, locals);
                    pc += 2;
                    locals = saveLocals;
                }
            }
            break;

            case WA_INSTR_call_indirect:
            {
                u32 typeIndex = *(u32*)&I0.valI32;
                u32 tableIndex = *(u32*)&I1.valI32;
                i64 maxUsedSlot = I2.valI64;
                u32 index = *(u32*)&(L3.valI32);

                wa_table* table = instance->tables[tableIndex];

                if(index >= table->size)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }

                wa_instance* refInstance = table->contents[index].refInstance;
                u32 funcIndex = table->contents[index].refIndex;

                if(refInstance == 0)
                {
                    return WA_TRAP_REF_NULL;
                }

                wa_func* callee = &refInstance->functions[funcIndex];
                wa_func_type* t1 = callee->type;
                wa_func_type* t2 = &instance->module->types[typeIndex];

                if(!wa_check_function_type(t1, t2))
                {
                    return (WA_TRAP_INDIRECT_CALL_TYPE_MISMATCH);
                }

                //////////////////////////////
                //TODO runtime check type
                //////////////////////////////

                if(refInstance == instance)
                {
                    if(callee->code)
                    {
                        controlStack[controlStackTop] = (wa_control){
                            .returnPC = pc + 4,
                            .returnFrame = locals,
                        };
                        controlStackTop++;
                        if(controlStackTop >= 1024)
                        {
                            return (WA_TRAP_STACK_OVERFLOW);
                        }

                        locals += maxUsedSlot;
                        pc = callee->code;
                    }
                    else if(callee->extInstance)
                    {
                        //TODO temporary for test, do it better

                        wa_func* extFunc = &callee->extInstance->functions[callee->extIndex];

                        wa_status status = wa_instance_invoke(callee->extInstance,
                                                              extFunc,
                                                              callee->type->paramCount,
                                                              locals + maxUsedSlot,
                                                              callee->type->returnCount,
                                                              locals + maxUsedSlot);

                        if(status != WA_OK)
                        {
                            return status;
                        }
                        pc += 4;
                    }
                    else
                    {
                        wa_value* saveLocals = locals;
                        locals += maxUsedSlot;
                        callee->proc(locals, locals);
                        pc += 4;
                        locals = saveLocals;
                    }
                }
                else
                {
                    wa_status status = wa_instance_invoke(refInstance,
                                                          callee,
                                                          callee->type->paramCount,
                                                          locals + maxUsedSlot,
                                                          callee->type->returnCount,
                                                          locals + maxUsedSlot);

                    if(status != WA_OK)
                    {
                        return status;
                    }
                    pc += 4;
                }
            }
            break;

            case WA_INSTR_return:
            {
                if(!controlStackTop)
                {
                    goto end;
                }
                controlStackTop--;
                wa_control control = controlStack[controlStackTop];
                locals = control.returnFrame;
                pc = control.returnPC;
            }
            break;

            case WA_INSTR_ref_null:
            {
                L0.refInstance = 0;
                L0.refIndex = 0;
                pc += 1;
            }
            break;

            case WA_INSTR_ref_is_null:
            {
                L0.valI32 = (L1.refInstance == 0) ? 1 : 0;
                pc += 2;
            }
            break;

            case WA_INSTR_ref_func:
            {
                L0.refInstance = instance,
                L0.refIndex = I1.valI64;
                pc += 2;
            }
            break;

            case WA_INSTR_i32_add:
            {
                RES.valI32 = OPD1.valI32 + OPD2.valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_sub:
            {
                RES.valI32 = OPD1.valI32 - OPD2.valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_mul:
            {
                RES.valI32 = OPD1.valI32 * OPD2.valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_div_s:
            {
                if(OPD2.valI32 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else if(OPD1.valI32 == INT32_MIN && OPD2.valI32 == -1)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }
                else
                {
                    RES.valI32 = OPD1.valI32 / OPD2.valI32;
                }
                pc += 3;
            }
            break;

            case WA_INSTR_i32_div_u:
            {
                if(OPD2.valI32 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else
                {
                    *(u32*)&RES.valI32 = *(u32*)&OPD1.valI32 / *(u32*)&OPD2.valI32;
                }
                pc += 3;
            }
            break;

            case WA_INSTR_i32_rem_s:
            {
                if(OPD2.valI32 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else if(OPD1.valI32 == INT32_MIN && OPD2.valI32 == -1)
                {
                    RES.valI32 = 0;
                }
                else
                {
                    RES.valI32 = OPD1.valI32 % OPD2.valI32;
                }
                pc += 3;
            }
            break;

            case WA_INSTR_i32_rem_u:
            {
                if(OPD2.valI32 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else
                {
                    *(u32*)&RES.valI32 = *(u32*)&OPD1.valI32 % *(u32*)&OPD2.valI32;
                }
                pc += 3;
            }
            break;

            case WA_INSTR_i32_and:
            {
                RES.valI32 = OPD1.valI32 & OPD2.valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_or:
            {
                RES.valI32 = OPD1.valI32 | OPD2.valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_xor:
            {
                RES.valI32 = OPD1.valI32 ^ OPD2.valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_shl:
            {
                RES.valI32 = OPD1.valI32 << OPD2.valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_shr_s:
            {
                RES.valI32 = OPD1.valI32 >> OPD2.valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_shr_u:
            {
                *(u32*)&RES.valI32 = *(u32*)&OPD1.valI32 >> *(u32*)&OPD2.valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_rotr:
            {
                u32 n = *(u32*)&OPD1.valI32;
                u32 r = *(u32*)&OPD2.valI32;
                *(u32*)&RES.valI32 = (n >> r) | (n << (32 - r));
                pc += 3;
            }
            break;

            case WA_INSTR_i32_rotl:
            {
                u32 n = *(u32*)&OPD1.valI32;
                u32 r = *(u32*)&OPD2.valI32;
                *(u32*)&RES.valI32 = (n << r) | (n >> (32 - r));
                pc += 3;
            }
            break;

            case WA_INSTR_i32_clz:
            {
                if(OPD1.valI32 == 0)
                {
                    RES.valI32 = 32;
                }
                else
                {
                    RES.valI32 = __builtin_clz(*(u32*)&OPD1.valI32);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i32_ctz:
            {
                if(OPD1.valI32 == 0)
                {
                    RES.valI32 = 32;
                }
                else
                {
                    RES.valI32 = __builtin_ctz(*(u32*)&OPD1.valI32);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i32_popcnt:
            {
                RES.valI32 = __builtin_popcount(*(u32*)&OPD1.valI32);
                pc += 2;
            }
            break;

            case WA_INSTR_i32_extend8_s:
            {
                RES.valI32 = (i32)(i8)(OPD1.valI32 & 0xff);
                pc += 2;
            }
            break;

            case WA_INSTR_i32_extend16_s:
            {
                RES.valI32 = (i32)(i16)(OPD1.valI32 & 0xffff);
                pc += 2;
            }
            break;

            case WA_INSTR_i32_eqz:
            {
                RES.valI32 = (OPD1.valI32 == 0) ? 1 : 0;
                pc += 2;
            }
            break;

            case WA_INSTR_i32_eq:
            {
                RES.valI32 = (OPD1.valI32 == OPD2.valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_ne:
            {
                RES.valI32 = (OPD1.valI32 != OPD2.valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_lt_s:
            {
                RES.valI32 = (OPD1.valI32 < OPD2.valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_lt_u:
            {
                RES.valI32 = (*(u32*)&OPD1.valI32 < *(u32*)&OPD2.valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_le_s:
            {
                RES.valI32 = (OPD1.valI32 <= OPD2.valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_le_u:
            {
                RES.valI32 = (*(u32*)&OPD1.valI32 <= *(u32*)&OPD2.valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_gt_s:
            {
                RES.valI32 = (OPD1.valI32 > OPD2.valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_gt_u:
            {
                RES.valI32 = (*(u32*)&OPD1.valI32 > *(u32*)&OPD2.valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_ge_s:
            {
                RES.valI32 = (OPD1.valI32 >= OPD2.valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_ge_u:
            {
                RES.valI32 = (*(u32*)&OPD1.valI32 >= *(u32*)&OPD2.valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_add:
            {
                RES.valI64 = OPD1.valI64 + OPD2.valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_sub:
            {
                RES.valI64 = OPD1.valI64 - OPD2.valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_mul:
            {
                RES.valI64 = OPD1.valI64 * OPD2.valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_div_s:
            {
                if(OPD2.valI64 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else if(OPD1.valI64 == INT64_MIN && OPD2.valI64 == -1)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }
                else
                {
                    RES.valI64 = OPD1.valI64 / OPD2.valI64;
                }
                pc += 3;
            }
            break;

            case WA_INSTR_i64_div_u:
            {
                if(OPD2.valI64 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else
                {
                    *(u64*)&RES.valI64 = *(u64*)&OPD1.valI64 / *(u64*)&OPD2.valI64;
                }
                pc += 3;
            }
            break;

            case WA_INSTR_i64_rem_s:
            {
                if(OPD2.valI64 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else if(OPD1.valI64 == INT64_MIN && OPD2.valI64 == -1)
                {
                    RES.valI64 = 0;
                }
                else
                {
                    RES.valI64 = OPD1.valI64 % OPD2.valI64;
                }
                pc += 3;
            }
            break;

            case WA_INSTR_i64_rem_u:
            {
                if(OPD2.valI64 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else
                {
                    *(u64*)&RES.valI64 = *(u64*)&OPD1.valI64 % *(u64*)&OPD2.valI64;
                }
                pc += 3;
            }
            break;

            case WA_INSTR_i64_and:
            {
                RES.valI64 = OPD1.valI64 & OPD2.valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_or:
            {
                RES.valI64 = OPD1.valI64 | OPD2.valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_xor:
            {
                RES.valI64 = OPD1.valI64 ^ OPD2.valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_shl:
            {
                RES.valI64 = OPD1.valI64 << OPD2.valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_shr_s:
            {
                RES.valI64 = OPD1.valI64 >> OPD2.valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_shr_u:
            {
                *(u64*)&RES.valI64 = *(u64*)&OPD1.valI64 >> *(u64*)&OPD2.valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_rotr:
            {
                u64 n = *(u64*)&OPD1.valI64;
                u64 r = *(u64*)&OPD2.valI64;
                *(u64*)&RES.valI64 = (n >> r) | (n << (64 - r));
                pc += 3;
            }
            break;

            case WA_INSTR_i64_rotl:
            {
                u64 n = *(u64*)&OPD1.valI64;
                u64 r = *(u64*)&OPD2.valI64;
                *(u64*)&RES.valI64 = (n << r) | (n >> (64 - r));
                pc += 3;
            }
            break;

            case WA_INSTR_i64_clz:
            {
                if(OPD1.valI64 == 0)
                {
                    RES.valI64 = 64;
                }
                else
                {
                    RES.valI64 = __builtin_clzl(*(u64*)&OPD1.valI64);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i64_ctz:
            {
                if(OPD1.valI64 == 0)
                {
                    RES.valI64 = 64;
                }
                else
                {
                    RES.valI64 = __builtin_ctzl(*(u64*)&OPD1.valI64);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i64_popcnt:
            {
                RES.valI64 = __builtin_popcountl(*(u64*)&OPD1.valI64);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_extend8_s:
            {
                RES.valI64 = (i64)(i8)(OPD1.valI64 & 0xff);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_extend16_s:
            {
                RES.valI64 = (i64)(i16)(OPD1.valI64 & 0xffff);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_extend32_s:
            {
                RES.valI64 = (i64)(i32)(OPD1.valI64 & 0xffffffff);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_eqz:
            {
                RES.valI32 = (OPD1.valI64 == 0) ? 1 : 0;
                pc += 2;
            }
            break;

            case WA_INSTR_i64_eq:
            {
                RES.valI32 = (OPD1.valI64 == OPD2.valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_ne:
            {
                RES.valI32 = (OPD1.valI64 != OPD2.valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_lt_s:
            {
                RES.valI32 = (OPD1.valI64 < OPD2.valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_lt_u:
            {
                RES.valI32 = (*(u64*)&OPD1.valI64 < *(u64*)&OPD2.valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_le_s:
            {
                RES.valI32 = (OPD1.valI64 <= OPD2.valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_le_u:
            {
                RES.valI32 = (*(u64*)&OPD1.valI64 <= *(u64*)&OPD2.valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_gt_s:
            {
                RES.valI32 = (OPD1.valI64 > OPD2.valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_gt_u:
            {
                RES.valI32 = (*(u64*)&OPD1.valI64 > *(u64*)&OPD2.valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_ge_s:
            {
                RES.valI32 = (OPD1.valI64 >= OPD2.valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_ge_u:
            {
                RES.valI32 = (*(u64*)&OPD1.valI64 >= *(u64*)&OPD2.valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_f32_eq:
            {
                RES.valI32 = (OPD1.valF32 == OPD2.valF32) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f32_ne:
            {
                RES.valI32 = (OPD1.valF32 != OPD2.valF32) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f32_lt:
            {
                RES.valI32 = (OPD1.valF32 < OPD2.valF32) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f32_gt:
            {
                RES.valI32 = (OPD1.valF32 > OPD2.valF32) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f32_le:
            {
                RES.valI32 = (OPD1.valF32 <= OPD2.valF32) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f32_ge:
            {
                RES.valI32 = (OPD1.valF32 >= OPD2.valF32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_f64_eq:
            {
                RES.valI32 = (OPD1.valF64 == OPD2.valF64) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f64_ne:
            {
                RES.valI32 = (OPD1.valF64 != OPD2.valF64) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f64_lt:
            {
                RES.valI32 = (OPD1.valF64 < OPD2.valF64) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f64_gt:
            {
                RES.valI32 = (OPD1.valF64 > OPD2.valF64) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f64_le:
            {
                RES.valI32 = (OPD1.valF64 <= OPD2.valF64) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f64_ge:
            {
                RES.valI32 = (OPD1.valF64 >= OPD2.valF64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_f32_abs:
            {
                RES.valF32 = fabsf(OPD1.valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_f32_neg:
            {
                RES.valF32 = -OPD1.valF32;
                pc += 2;
            }
            break;

            case WA_INSTR_f32_ceil:
            {
                RES.valF32 = ceilf(OPD1.valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_f32_floor:
            {
                RES.valF32 = floorf(OPD1.valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_f32_trunc:
            {
                RES.valF32 = truncf(OPD1.valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_f32_nearest:
            {
                RES.valF32 = rintf(OPD1.valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_f32_sqrt:
            {
                RES.valF32 = sqrtf(OPD1.valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_f32_add:
            {
                RES.valF32 = OPD1.valF32 + OPD2.valF32;
                pc += 3;
            }
            break;

            case WA_INSTR_f32_sub:
            {
                RES.valF32 = OPD1.valF32 - OPD2.valF32;
                pc += 3;
            }
            break;

            case WA_INSTR_f32_mul:
            {
                RES.valF32 = OPD1.valF32 * OPD2.valF32;
                pc += 3;
            }
            break;

            case WA_INSTR_f32_div:
            {
                RES.valF32 = OPD1.valF32 / OPD2.valF32;
                pc += 3;
            }
            break;

            case WA_INSTR_f32_min:
            {
                f32 a = OPD1.valF32;
                f32 b = OPD2.valF32;
                if(isnan(a) || isnan(b))
                {
                    u32 u = 0x7fc00000;
                    memcpy(&RES.valF32, &u, sizeof(f32));
                }
                else if(a == 0 && b == 0)
                {
                    RES.valF32 = signbit(a) ? a : b;
                }
                else
                {
                    RES.valF32 = oc_min(a, b);
                }
                pc += 3;
            }
            break;

            case WA_INSTR_f32_max:
            {
                f32 a = OPD1.valF32;
                f32 b = OPD2.valF32;
                if(isnan(a) || isnan(b))
                {
                    u32 u = 0x7fc00000;
                    memcpy(&RES.valF32, &u, sizeof(f32));
                }
                else if(a == 0 && b == 0)
                {
                    RES.valF32 = signbit(a) ? b : a;
                }

                else
                {
                    RES.valF32 = oc_max(a, b);
                }
                pc += 3;
            }
            break;

            case WA_INSTR_f32_copysign:
            {
                RES.valF32 = copysignf(OPD1.valF32, OPD2.valF32);
                pc += 3;
            }
            break;

            case WA_INSTR_f64_abs:
            {
                RES.valF64 = fabs(OPD1.valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_f64_neg:
            {
                RES.valF64 = -OPD1.valF64;
                pc += 2;
            }
            break;

            case WA_INSTR_f64_ceil:
            {
                RES.valF64 = ceil(OPD1.valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_f64_floor:
            {
                RES.valF64 = floor(OPD1.valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_f64_trunc:
            {
                RES.valF64 = trunc(OPD1.valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_f64_nearest:
            {
                RES.valF64 = rint(OPD1.valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_f64_sqrt:
            {
                RES.valF64 = sqrt(OPD1.valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_f64_add:
            {
                RES.valF64 = OPD1.valF64 + OPD2.valF64;
                pc += 3;
            }
            break;

            case WA_INSTR_f64_sub:
            {
                RES.valF64 = OPD1.valF64 - OPD2.valF64;
                pc += 3;
            }
            break;

            case WA_INSTR_f64_mul:
            {
                RES.valF64 = OPD1.valF64 * OPD2.valF64;
                pc += 3;
            }
            break;

            case WA_INSTR_f64_div:
            {
                RES.valF64 = OPD1.valF64 / OPD2.valF64;
                pc += 3;
            }
            break;

            case WA_INSTR_f64_min:
            {
                f64 a = OPD1.valF64;
                f64 b = OPD2.valF64;

                if(isnan(a) || isnan(b))
                {
                    u64 u = 0x7ff8000000000000;
                    memcpy(&RES.valF64, &u, sizeof(f64));
                }
                else if(a == 0 && b == 0)
                {
                    RES.valF64 = signbit(a) ? a : b;
                }
                else
                {
                    RES.valF64 = oc_min(a, b);
                }
                pc += 3;
            }
            break;

            case WA_INSTR_f64_max:
            {
                f64 a = OPD1.valF64;
                f64 b = OPD2.valF64;

                if(isnan(a) || isnan(b))
                {
                    u64 u = 0x7ff8000000000000;
                    memcpy(&RES.valF64, &u, sizeof(f64));
                }
                else if(a == 0 && b == 0)
                {
                    RES.valF64 = signbit(a) ? b : a;
                }
                else
                {
                    RES.valF64 = oc_max(a, b);
                }
                pc += 3;
            }
            break;

            case WA_INSTR_f64_copysign:
            {
                RES.valF64 = copysign(OPD1.valF64, OPD2.valF64);
                pc += 3;
            }
            break;

            case WA_INSTR_i32_wrap_i64:
            {
                RES.valI32 = (OPD1.valI64 & 0x00000000ffffffff);
                pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_f32_s:
            {
                if(isnan(OPD1.valF32))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }
                else if(OPD1.valF32 >= 2147483648.0f || OPD1.valF32 < -2147483648.0f)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                RES.valI32 = (i32)truncf(OPD1.valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_f32_u:
            {
                if(isnan(OPD1.valF32))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(OPD1.valF32 >= 4294967296.0f || OPD1.valF32 <= -1.0f)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                *(u32*)&RES.valI32 = (u32)truncf(OPD1.valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_f64_s:
            {
                if(isnan(OPD1.valF64))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(OPD1.valF64 >= 2147483648.0 || OPD1.valF64 <= -2147483649.0)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                RES.valI32 = (i32)trunc(OPD1.valF64);
                pc += 2;
            }
            break;
            case WA_INSTR_i32_trunc_f64_u:
            {
                if(isnan(OPD1.valF64))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(OPD1.valF64 >= 4294967296.0 || OPD1.valF64 <= -1.0)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                *(u32*)&RES.valI32 = (u32)trunc(OPD1.valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_f32_s:
            {
                if(isnan(OPD1.valF32))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(OPD1.valF32 >= 9223372036854775808.0f || OPD1.valF32 < -9223372036854775808.0f)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                RES.valI64 = (i64)truncf(OPD1.valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_f32_u:
            {
                if(isnan(OPD1.valF32))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(OPD1.valF32 >= 18446744073709551616.0f || OPD1.valF32 <= -1.0f)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                *(u64*)&RES.valI64 = (u64)truncf(OPD1.valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_f64_s:
            {
                if(isnan(OPD1.valF64))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(OPD1.valF64 >= 9223372036854775808.0 || OPD1.valF64 < -9223372036854775808.0)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                RES.valI64 = (i64)trunc(OPD1.valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_f64_u:
            {
                if(isnan(OPD1.valF64))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(OPD1.valF64 >= 18446744073709551616.0 || OPD1.valF64 <= -1.0)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                *(u64*)&RES.valI64 = (u64)trunc(OPD1.valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_f32_convert_i32_s:
            {
                RES.valF32 = (f32)OPD1.valI32;
                pc += 2;
            }
            break;

            case WA_INSTR_f32_convert_i32_u:
            {
                RES.valF32 = (f32) * (u32*)&OPD1.valI32;
                pc += 2;
            }
            break;

            case WA_INSTR_f32_convert_i64_s:
            {
                RES.valF32 = (f32)OPD1.valI64;
                pc += 2;
            }
            break;

            case WA_INSTR_f32_convert_i64_u:
            {
                RES.valF32 = (f32) * (u64*)&OPD1.valI64;
                pc += 2;
            }
            break;

            case WA_INSTR_f32_demote_f64:
            {
                RES.valF32 = (f32)OPD1.valF64;
                pc += 2;
            }
            break;

            case WA_INSTR_f64_convert_i32_s:
            {
                RES.valF64 = (f64)OPD1.valI32;
                pc += 2;
            }
            break;
            case WA_INSTR_f64_convert_i32_u:
            {
                RES.valF64 = (f64) * (u32*)&OPD1.valI32;
                pc += 2;
            }
            break;
            case WA_INSTR_f64_convert_i64_s:
            {
                RES.valF64 = (f64)OPD1.valI64;
                pc += 2;
            }
            break;
            case WA_INSTR_f64_convert_i64_u:
            {
                RES.valF64 = (f64) * (u64*)&OPD1.valI64;
                pc += 2;
            }
            break;

            case WA_INSTR_f64_promote_f32:
            {
                RES.valF64 = (f64)OPD1.valF32;
                pc += 2;
            }
            break;

            case WA_INSTR_i32_reinterpret_f32:
            {
                RES.valI32 = *(i32*)&OPD1.valF32;
                pc += 2;
            }
            break;
            case WA_INSTR_i64_reinterpret_f64:
            {
                RES.valI64 = *(i64*)&OPD1.valF64;
                pc += 2;
            }
            break;
            case WA_INSTR_f32_reinterpret_i32:
            {
                RES.valF32 = *(f32*)&OPD1.valI32;
                pc += 2;
            }
            break;
            case WA_INSTR_f64_reinterpret_i64:
            {
                RES.valF64 = *(f64*)&OPD1.valI64;
                pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_sat_f32_s:
            {
                if(isnan(OPD1.valF32))
                {
                    RES.valI32 = 0;
                }
                else if(OPD1.valF32 >= 2147483648.0f)
                {
                    RES.valI32 = INT32_MAX;
                }
                else if(OPD1.valF32 < -2147483648.0f)
                {
                    RES.valI32 = INT32_MIN;
                }
                else
                {
                    RES.valI32 = (i32)truncf(OPD1.valF32);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_sat_f32_u:
            {
                if(isnan(OPD1.valF32))
                {
                    RES.valI32 = 0;
                }
                else if(OPD1.valF32 >= 4294967296.0f)
                {
                    *(u32*)&RES.valI32 = 0xffffffff;
                }
                else if(OPD1.valF32 <= -1.0f)
                {
                    RES.valI32 = 0;
                }
                else
                {
                    *(u32*)&RES.valI32 = (u32)truncf(OPD1.valF32);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_sat_f64_s:
            {
                if(isnan(OPD1.valF64))
                {
                    RES.valI32 = 0;
                }
                else if(OPD1.valF64 >= 2147483648.0)
                {
                    RES.valI32 = INT32_MAX;
                }
                else if(OPD1.valF64 <= -2147483649.0)
                {
                    RES.valI32 = INT32_MIN;
                }
                else
                {
                    RES.valI32 = (i32)trunc(OPD1.valF64);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_sat_f64_u:
            {
                if(isnan(OPD1.valF64))
                {
                    RES.valI32 = 0;
                }
                else if(OPD1.valF64 >= 4294967296.0)
                {
                    *(u32*)&RES.valI32 = 0xffffffff;
                }
                else if(OPD1.valF64 <= -1.0)
                {
                    RES.valI32 = 0;
                }
                else
                {
                    *(u32*)&RES.valI32 = (u32)trunc(OPD1.valF64);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_sat_f32_s:
            {
                if(isnan(OPD1.valF32))
                {
                    RES.valI64 = 0;
                }
                else if(OPD1.valF32 >= 9223372036854775808.0f)
                {
                    RES.valI64 = INT64_MAX;
                }
                else if(OPD1.valF32 < -9223372036854775808.0f)
                {
                    RES.valI64 = INT64_MIN;
                }
                else
                {
                    RES.valI64 = (i64)truncf(OPD1.valF32);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_sat_f32_u:
            {
                if(isnan(OPD1.valF32))
                {
                    RES.valI64 = 0;
                }
                else if(OPD1.valF32 >= 18446744073709551616.0f)
                {
                    *(u64*)&RES.valI64 = 0xffffffffffffffffLLU;
                }
                else if(OPD1.valF32 <= -1.0f)
                {
                    RES.valI64 = 0;
                }
                else
                {
                    *(u64*)&RES.valI64 = (u64)truncf(OPD1.valF32);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_sat_f64_s:
            {
                if(isnan(OPD1.valF64))
                {
                    RES.valI64 = 0;
                }
                else if(OPD1.valF64 >= 9223372036854775808.0)
                {
                    RES.valI64 = INT64_MAX;
                }
                else if(OPD1.valF64 < -9223372036854775808.0)
                {
                    RES.valI64 = INT64_MIN;
                }
                else
                {
                    RES.valI64 = (i64)trunc(OPD1.valF64);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_sat_f64_u:
            {
                if(isnan(OPD1.valF64))
                {
                    RES.valI64 = 0;
                }
                else if(OPD1.valF64 >= 18446744073709551616.0)
                {
                    *(u64*)&RES.valI64 = 0xffffffffffffffffLLU;
                }
                else if(OPD1.valF64 <= -1.0)
                {
                    RES.valI64 = 0;
                }
                else
                {
                    *(u64*)&RES.valI64 = (u64)trunc(OPD1.valF64);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i64_extend_i32_s:
            {
                RES.valI64 = (i64)(i32)(OPD1.valI32);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_extend_i32_u:
            {
                RES.valI64 = *(u32*)&(OPD1.valI32);
                pc += 2;
            }
            break;

            case WA_INSTR_memory_size:
            {
                wa_memory* mem = instance->memories[0];
                RES.valI32 = (i32)(mem->size / WA_PAGE_SIZE);
                pc += 1;
            }
            break;

            case WA_INSTR_memory_grow:
            {
                wa_memory* mem = instance->memories[0];

                i32 res = -1;
                u32 n = *(u32*)&(L1.valI32);
                oc_base_allocator* allocator = oc_base_allocator_default();
                u64 size = (u64)n * WA_PAGE_SIZE;

                if(mem->size + size <= mem->cap
                   && ((mem->size + size) / WA_PAGE_SIZE <= mem->cap))
                {
                    res = mem->size / WA_PAGE_SIZE;
                    oc_base_commit(allocator, mem->ptr + mem->size, n * WA_PAGE_SIZE);
                    mem->size += size;
                    mem->limits.min = mem->size / WA_PAGE_SIZE;
                }

                L0.valI32 = res;

                pc += 2;
            }
            break;

            case WA_INSTR_memory_fill:
            {
                wa_memory* mem = instance->memories[0];

                u32 d = *(u32*)&L0.valI32;
                i32 val = L1.valI32;
                u32 n = *(u32*)&L2.valI32;

                if(d + n > mem->size || d + n < d)
                {
                    return WA_TRAP_MEMORY_OUT_OF_BOUNDS;
                }
                else
                {
                    memset(mem->ptr + d, val, n);
                }
                pc += 3;
            }
            break;

            case WA_INSTR_memory_copy:
            {
                wa_memory* mem = instance->memories[0];
                u32 d = *(u32*)&L0.valI32;
                u32 s = *(u32*)&L1.valI32;
                u32 n = *(u32*)&L2.valI32;

                if(s + n > mem->size || s + n < s
                   || d + n > mem->size || d + n < d)
                {
                    return WA_TRAP_MEMORY_OUT_OF_BOUNDS;
                }
                memmove(mem->ptr + d, mem->ptr + s, n);
                pc += 3;
            }
            break;

            case WA_INSTR_memory_init:
            {
                wa_memory* mem = instance->memories[0];
                wa_data_segment* seg = &instance->data[I0.valI32];

                u32 d = *(u32*)&L1.valI32;
                u32 s = *(u32*)&L2.valI32;
                u32 n = *(u32*)&L3.valI32;

                if(s + n > seg->init.len || s + n < s
                   || d + n > mem->size || d + n < d)
                {
                    return WA_TRAP_MEMORY_OUT_OF_BOUNDS;
                }
                memmove(mem->ptr + d, seg->init.ptr + s, n);
                pc += 4;
            }
            break;

            case WA_INSTR_data_drop:
            {
                wa_data_segment* seg = &instance->data[I0.valI32];
                seg->init.len = 0;
                pc += 1;
            }
            break;

            case WA_INSTR_table_init:
            {
                wa_element* elt = &instance->elements[I0.valI32];
                wa_table* table = instance->tables[I1.valI32];

                u32 d = *(u32*)&L2.valI32;
                u32 s = *(u32*)&L3.valI32;
                u32 n = *(u32*)&L4.valI32;

                if(n + s > elt->initCount || n + s < n
                   || d + n > table->size || d + n < d)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }
                memmove(table->contents + d, elt->refs + s, n * sizeof(wa_value));
                pc += 5;
            }
            break;

            case WA_INSTR_table_fill:
            {
                wa_table* table = instance->tables[I0.valI32];

                u32 d = *(u32*)&L1.valI32;
                wa_value val = L2;
                u32 n = *(u32*)&L3.valI32;

                if(d + n > table->size || d + n < d)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }
                for(u32 i = 0; i < n; i++)
                {
                    table->contents[d + i] = val;
                }

                pc += 4;
            }
            break;

            case WA_INSTR_table_copy:
            {
                wa_table* tx = instance->tables[I0.valI32];
                wa_table* ty = instance->tables[I1.valI32];

                u32 d = *(u32*)&L2.valI32;
                u32 s = *(u32*)&L3.valI32;
                u32 n = *(u32*)&L4.valI32;

                if(s + n > ty->size || s + n < s
                   || d + n > tx->size || d + n < d)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }
                memmove(tx->contents + d, ty->contents + s, n * sizeof(wa_value));
                pc += 5;
            }
            break;

            case WA_INSTR_table_size:
            {
                wa_table* table = instance->tables[I1.valI32];
                L0.valI32 = table->size;
                pc += 2;
            }
            break;

            case WA_INSTR_table_grow:
            {
                wa_limits* limits = &instance->module->tables[I1.valI32].limits;
                wa_table* table = instance->tables[I1.valI32];
                wa_value val = L2;
                u32 size = L3.valI32;

                i32 ret = -1;
                if((u64)table->size + (u64)size <= UINT32_MAX && (limits->kind != WA_LIMIT_MIN_MAX || table->size + size <= limits->max))
                {
                    wa_value* contents = oc_arena_push_array(instance->arena, wa_value, table->size + size);
                    memcpy(contents, table->contents, table->size * sizeof(wa_value));
                    for(u32 i = 0; i < size; i++)
                    {
                        contents[table->size + i] = val;
                    }
                    ret = table->size;
                    table->size += size;
                    table->contents = contents;
                }
                L0.valI32 = ret;
                pc += 4;
            }
            break;

            case WA_INSTR_table_get:
            {
                wa_table* table = instance->tables[I1.valI32];
                u32 eltIndex = L2.valI32;

                if(eltIndex >= table->size)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }
                L0 = table->contents[eltIndex];
                pc += 3;
            }
            break;

            case WA_INSTR_table_set:
            {
                wa_table* table = instance->tables[I0.valI32];
                u32 eltIndex = L1.valI32;
                wa_value val = L2;

                if(eltIndex >= table->size)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }
                table->contents[eltIndex] = val;
                pc += 3;
            }
            break;

            case WA_INSTR_elem_drop:
            {
                wa_element* elt = &instance->elements[I0.valI32];
                elt->initCount = 0;
                pc += 1;
            }
            break;

            default:
                oc_log_error("invalid opcode %s\n", wa_instr_strings[opcode]);
                return WA_TRAP_INVALID_OP;
        }
    }

end:
    for(u32 retIndex = 0; retIndex < retCount; retIndex++)
    {
        returns[retIndex] = locals[retIndex];
    }

    return WA_OK;
}

wa_status wa_instance_invoke(wa_instance* instance,
                             wa_func* func,
                             u32 argCount,
                             wa_value* args,
                             u32 retCount,
                             wa_value* returns)
{
    if(argCount != func->type->paramCount || retCount != func->type->returnCount)
    {
        return WA_ERROR_INVALID_ARGS;
    }

    if(func->code)
    {
        return (wa_instance_interpret_expr(instance, func->type, func->code, argCount, args, retCount, returns));
    }
    else if(func->extInstance)
    {
        wa_func* extFunc = &func->extInstance->functions[func->extIndex];
        return wa_instance_invoke(func->extInstance, extFunc, argCount, args, retCount, returns);
    }
    else
    {
        //TODO: host proc should return a status
        func->proc(args, returns);
        return WA_OK;
    }
}

//-------------------------------------------------------------------------
// test
//-------------------------------------------------------------------------
#include "json.c"

wa_typed_value parse_value_64(oc_str8 string)
{
    wa_typed_value res = { 0 };

    u64 offset = 0;
    u64 numberU64 = 0;
    bool minus = false;

    if(string.len && string.ptr[0] == '-')
    {
        minus = true;
        offset++;
    }

    while(offset < string.len)
    {
        char c = string.ptr[offset];
        if(c >= '0' && c <= '9')
        {
            numberU64 *= 10;
            numberU64 += c - '0';
            offset += 1;
        }
        else
        {
            break;
        }
    }

    f64 numberF64;
    if(offset < string.len
       && string.ptr[offset] == '.')
    {
        offset += 1;

        u64 decimals = 0;
        u64 decimalCount = 0;

        while(offset < string.len)
        {
            char c = string.ptr[offset];
            if(c >= '0' && c <= '9')
            {
                decimals *= 10;
                decimals += c - '0';
                offset += 1;
                decimalCount += 1;
            }
            else
            {
                break;
            }
        }
        res.type = WA_TYPE_F64;
        res.value.valF64 = (f64)numberU64 + (f64)decimals / pow(10, decimalCount);
        if(minus)
        {
            res.value.valF64 = -res.value.valF64;
        }
    }
    else
    {
        res.type = WA_TYPE_I64;
        res.value.valI64 = numberU64;
        if(minus)
        {
            res.value.valI64 = -res.value.valI64;
        }
    }
    return (res);
}

wa_typed_value parse_value_32(oc_str8 string)
{
    wa_typed_value val = parse_value_64(string);
    if(val.type == WA_TYPE_I64)
    {
        if(val.value.valI64 > INT32_MAX)
        {
            val.value.valI64 = INT32_MIN + (val.value.valI64 - INT32_MAX - 1LLU);
        }
        val.type = WA_TYPE_I32;
        val.value.valI32 = (i32)val.value.valI64;
    }
    else if(val.type == WA_TYPE_F64)
    {
        val.type = WA_TYPE_F32;
        val.value.valF32 = (i32)val.value.valF64;
    }
    return (val);
}

json_node* json_find_assert(json_node* node, const char* name, json_node_kind kind)
{
    json_node* res = json_find(node, OC_STR8(name));
    OC_ASSERT(res && res->kind == kind);
    return (res);
}

bool wa_is_nan_canonical_f32(f32 f)
{
    u32 u = 0;
    memcpy(&u, &f, sizeof(u32));
    return (u == 0x7fc00000 || u == 0xffc00000);
}

bool wa_is_nan_arithmetic_f32(f32 f)
{
    u32 u = 0;
    memcpy(&u, &f, sizeof(u32));
    return ((u & 0x7fc00000) == 0x7fc00000);
}

bool wa_is_nan_canonical_f64(f64 f)
{
    u64 u = 0;
    memcpy(&u, &f, sizeof(u64));
    return (u == 0x7ff8000000000000 || u == 0xfff8000000000000);
}

bool wa_is_nan_arithmetic_f64(f64 f)
{
    u64 u = 0;
    memcpy(&u, &f, sizeof(u64));
    return ((u & 0x7ff8000000000000) == 0x7ff8000000000000);
}

wa_typed_value test_parse_value(json_node* arg)
{
    wa_typed_value value = { 0 };

    json_node* argType = json_find_assert(arg, "type", JSON_STRING);
    json_node* argVal = json_find_assert(arg, "value", JSON_STRING);

    if(!oc_str8_cmp(argType->string, OC_STR8("i32")))
    {
        value = parse_value_32(argVal->string);
        OC_ASSERT(value.type == WA_TYPE_I32);
    }
    else if(!oc_str8_cmp(argType->string, OC_STR8("i64")))
    {
        value = parse_value_64(argVal->string);
        OC_ASSERT(value.type == WA_TYPE_I64);
    }
    else if(!oc_str8_cmp(argType->string, OC_STR8("f32")))
    {
        if(!oc_str8_cmp(argVal->string, OC_STR8("nan:canonical")))
        {
            value.type = WA_TYPE_F32;
            u32 val = 0x7fc00000;
            memcpy(&value.value.valF32, &val, sizeof(f32));
        }
        else if(!oc_str8_cmp(argVal->string, OC_STR8("nan:arithmetic")))
        {
            value.type = WA_TYPE_F32;
            u32 val = 0x7fc00001;
            memcpy(&value.value.valF32, &val, sizeof(f32));
        }
        else
        {
            value = parse_value_32(argVal->string);
            OC_ASSERT(value.type == WA_TYPE_I32);
            value.type = WA_TYPE_F32;
            memcpy(&value.value.valF32, &value.value.valI32, sizeof(f32));
        }
    }
    else if(!oc_str8_cmp(argType->string, OC_STR8("f64")))
    {
        if(!oc_str8_cmp(argVal->string, OC_STR8("nan:canonical")))
        {
            value.type = WA_TYPE_F64;
            u64 val = 0x7ff8000000000000;
            memcpy(&value.value.valF64, &val, sizeof(f64));
        }
        else if(!oc_str8_cmp(argVal->string, OC_STR8("nan:arithmetic")))
        {
            value.type = WA_TYPE_F64;
            u64 val = 0x7ff8000000000001;
            memcpy(&value.value.valF64, &val, sizeof(f64));
        }
        else
        {
            value = parse_value_64(argVal->string);
            OC_ASSERT(value.type == WA_TYPE_I64);
            value.type = WA_TYPE_F64;
            memcpy(&value.value.valF64, &value.value.valI64, sizeof(f64));
        }
    }
    else if(!oc_str8_cmp(argType->string, OC_STR8("externref")))
    {
        value.type = WA_TYPE_EXTERN_REF;
        if(!oc_str8_cmp(argVal->string, OC_STR8("null")))
        {
            value.value.valI64 = 0;
        }
        else
        {
            //NOTE: tests use 0 as first non null value. We use 0 as null, so add 1 here
            value = parse_value_64(argVal->string);
            OC_ASSERT(value.type == WA_TYPE_I64);
            value.value.valI64 += 1;
        }
    }
    else if(!oc_str8_cmp(argType->string, OC_STR8("funcref")))
    {
        value.type = WA_TYPE_FUNC_REF;
        if(!oc_str8_cmp(argVal->string, OC_STR8("null")))
        {
            value.value.valI64 = 0;
        }
        else
        {
            //NOTE: tests use 0 as first non null value. We use 0 as null, so add 1 here
            value = parse_value_64(argVal->string);
            OC_ASSERT(value.type == WA_TYPE_I64);
            value.value.valI64 += 1;
        }
    }
    else
    {
        OC_ASSERT(0, "unsupported test value type");
    }
    return (value);
}

typedef struct wa_test_instance
{
    oc_list_elt listElt;
    oc_str8 name;
    oc_str8 registeredName;
    wa_instance* instance;
} wa_test_instance;

typedef struct wa_test_env
{
    oc_arena* arena;

    oc_list instances;

    oc_str8 fileName;
    json_node* command;

    u32 passed;
    u32 failed;
    u32 skipped;

    u32 totalPassed;
    u32 totalFailed;
    u32 totalSkipped;

    bool verbose;

    wa_memory testspecMemory;
    wa_table testspecTable;
    wa_value testspecGlobal_i32;
    wa_value testspecGlobal_i64;
    wa_value testspecGlobal_f32;
    wa_value testspecGlobal_f64;

} wa_test_env;

typedef enum wa_test_status
{
    WA_TEST_FAIL,
    WA_TEST_SKIP,
    WA_TEST_PASS,
} wa_test_status;

typedef struct wa_test_result
{
    wa_test_status status;
    wa_status trap;
    u32 count;
    wa_value* values;

} wa_test_result;

wa_test_result wa_test_invoke(wa_test_env* env, wa_instance* instance, json_node* action)
{
    json_node* funcName = json_find_assert(action, "field", JSON_STRING);
    json_node* args = json_find_assert(action, "args", JSON_LIST);

    wa_func* func = wa_instance_find_function(instance, funcName->string);
    if(!func)
    {
        return (wa_test_result){ .status = WA_TEST_FAIL };
    }

    u32 argCount = args->childCount;
    OC_ASSERT(argCount == func->type->paramCount);

    wa_value* argVals = oc_arena_push_array(env->arena, wa_value, argCount);

    u32 argIndex = 0;
    oc_list_for(args->children, arg, json_node, listElt)
    {
        wa_typed_value val = test_parse_value(arg);
        argVals[argIndex] = val.value;
        argIndex++;
    }

    wa_test_result res = { 0 };

    res.count = func->type->returnCount;
    res.values = oc_arena_push_array(env->arena, wa_value, res.count);

    res.trap = wa_instance_invoke(instance, func, argCount, argVals, res.count, res.values);
    res.status = (res.trap == WA_OK) ? WA_TEST_PASS : WA_TEST_FAIL;
    return res;
}

static const char* wa_test_status_string[] = {
    "[FAIL]",
    "[SKIP]",
    "[PASS]",
};

static const char* wa_test_status_color_start[] = {
    "\033[38;5;9m\033[1m",
    "\033[38;5;13m\033[1m",
    "\033[38;5;10m\033[1m",
};

static const char* wa_test_status_color_stop = "\033[m";

void wa_test_mark(wa_test_env* env, wa_test_status status, oc_str8 fileName, json_node* command)
{
    switch(status)
    {
        case WA_TEST_FAIL:
            env->failed++;
            break;
        case WA_TEST_SKIP:
            env->skipped++;
            break;
        case WA_TEST_PASS:
            env->passed++;
            break;
    }

    if(env->verbose)
    {
        printf("%s", wa_test_status_color_start[status]);
        printf("%s", wa_test_status_string[status]);
        printf("%s", wa_test_status_color_stop);

        printf(" %.*s", oc_str8_ip(fileName));

        json_node* line = json_find(command, OC_STR8("line"));
        if(line)
        {
            printf(":%lli", line->numI64);
        }

        json_node* type = json_find_assert(command, "type", JSON_STRING);
        printf(" (%.*s)\n", oc_str8_ip(type->string));
    }
}

#define wa_test_pass(env, fileName, command) wa_test_mark(env, WA_TEST_PASS, fileName, command)
#define wa_test_fail(env, fileName, command) wa_test_mark(env, WA_TEST_FAIL, fileName, command)
#define wa_test_skip(env, fileName, command) wa_test_mark(env, WA_TEST_SKIP, fileName, command)

wa_test_result wa_test_action(wa_test_env* env, wa_instance* instance, json_node* action)
{
    wa_test_result result = { 0 };

    json_node* actionType = json_find_assert(action, "type", JSON_STRING);

    if(!oc_str8_cmp(actionType->string, OC_STR8("invoke")))
    {
        result = wa_test_invoke(env, instance, action);
    }
    /*
    else if(!oc_str8_cmp(actionType->string, OC_STR8("get"))
    {
        //TODO
    }
    */
    else
    {
        wa_test_skip(env, env->fileName, env->command);
        result.status = WA_TEST_SKIP;
    }
    return (result);
}

wa_module* wa_test_module_load(oc_arena* arena, oc_str8 filename)
{
    oc_arena_scope scratch = oc_scratch_begin_next(arena);

    oc_str8 contents = { 0 };

    oc_file file = oc_file_open(filename, OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);
    if(oc_file_is_nil(file))
    {
        oc_log_error("Couldn't open file %.*s\n", oc_str8_ip(filename));
        return (0);
    }

    contents.len = oc_file_size(file);
    contents.ptr = oc_arena_push(scratch.arena, contents.len);

    oc_file_read(file, contents.len, contents.ptr);
    oc_file_close(file);

    wa_module* module = wa_module_create(arena, contents);

    oc_scratch_end(scratch);

    return (module);
}

wa_instance* wa_test_get_instance(wa_test_env* env, json_node* action)
{
    wa_instance* instance = 0;
    json_node* name = json_find(action, OC_STR8("module"));
    if(name)
    {
        OC_ASSERT(name->kind == JSON_STRING);
        oc_list_for(env->instances, item, wa_test_instance, listElt)
        {
            if(!oc_str8_cmp(item->name, name->string))
            {
                instance = item->instance;
                break;
            }
        }
    }
    else
    {
        wa_test_instance* item = oc_list_last_entry(env->instances, wa_test_instance, listElt);
        if(item)
        {
            instance = item->instance;
        }
    }
    return (instance);
}

void test_print(wa_value* args, wa_value* rets)
{
}

void test_print_i32(wa_value* args, wa_value* rets)
{
}

void test_print_i64(wa_value* args, wa_value* rets)
{
}

void test_print_f32(wa_value* args, wa_value* rets)
{
}

void test_print_f64(wa_value* args, wa_value* rets)
{
}

void test_print_i32_f32(wa_value* args, wa_value* rets)
{
}

void test_print_f64_f64(wa_value* args, wa_value* rets)
{
}

wa_status wa_test_instantiate(wa_test_env* env, wa_test_instance* testInstance, wa_module* module)
{
    testInstance->instance = wa_instance_create(env->arena, module);

    wa_status status = wa_instance_bind_memory(testInstance->instance,
                                               OC_STR8("spectest"),
                                               OC_STR8("memory"),
                                               &env->testspecMemory);
    if(status != WA_OK)
    {
        return status;
    }

    status = wa_instance_bind_table(testInstance->instance,
                                    OC_STR8("spectest"),
                                    OC_STR8("table"), (wa_table_type){
                                                          .type = WA_TYPE_FUNC_REF,
                                                          .limits = {
                                                              .kind = WA_LIMIT_MIN_MAX,
                                                              .min = 10,
                                                              .max = 20,
                                                          },
                                                      },
                                    &env->testspecTable);
    if(status != WA_OK)
    {
        return status;
    }

    status = wa_instance_bind_global(testInstance->instance, OC_STR8("global_i32"), WA_TYPE_I32, false, &env->testspecGlobal_i32);
    if(status != WA_OK)
    {
        return status;
    }

    status = wa_instance_bind_global(testInstance->instance, OC_STR8("global_i64"), WA_TYPE_I64, false, &env->testspecGlobal_i64);
    if(status != WA_OK)
    {
        return status;
    }

    status = wa_instance_bind_global(testInstance->instance, OC_STR8("global_f32"), WA_TYPE_F32, false, &env->testspecGlobal_i32);
    if(status != WA_OK)
    {
        return status;
    }

    status = wa_instance_bind_global(testInstance->instance, OC_STR8("global_f64"), WA_TYPE_F64, false, &env->testspecGlobal_i64);
    if(status != WA_OK)
    {
        return status;
    }

    status = wa_instance_bind_function(testInstance->instance,
                                       OC_STR8("print"),
                                       &(wa_func_type){ 0 },
                                       test_print);
    if(status != WA_OK)
    {
        return status;
    }

    status = wa_instance_bind_function(testInstance->instance,
                                       OC_STR8("print_i32"),
                                       &(wa_func_type){
                                           .paramCount = 1,
                                           .params = (wa_value_type[]){
                                               WA_TYPE_I32,
                                           },
                                       },
                                       test_print_i32);
    if(status != WA_OK)
    {
        return status;
    }

    status = wa_instance_bind_function(testInstance->instance,
                                       OC_STR8("print_i64"),
                                       &(wa_func_type){
                                           .paramCount = 1,
                                           .params = (wa_value_type[]){
                                               WA_TYPE_I64,
                                           },
                                       },
                                       test_print_i64);
    if(status != WA_OK)
    {
        return status;
    }

    status = wa_instance_bind_function(testInstance->instance,
                                       OC_STR8("print_f32"),
                                       &(wa_func_type){
                                           .paramCount = 1,
                                           .params = (wa_value_type[]){
                                               WA_TYPE_F32,
                                           },
                                       },
                                       test_print_f32);
    if(status != WA_OK)
    {
        return status;
    }

    status = wa_instance_bind_function(testInstance->instance,
                                       OC_STR8("print_f64"),
                                       &(wa_func_type){
                                           .paramCount = 1,
                                           .params = (wa_value_type[]){
                                               WA_TYPE_F64,
                                           },
                                       },
                                       test_print_f64);
    if(status != WA_OK)
    {
        return status;
    }

    status = wa_instance_bind_function(testInstance->instance,
                                       OC_STR8("print_i32_f32"),
                                       &(wa_func_type){
                                           .paramCount = 2,
                                           .params = (wa_value_type[]){
                                               WA_TYPE_I32,
                                               WA_TYPE_F32,
                                           },
                                       },
                                       test_print_i32_f32);
    if(status != WA_OK)
    {
        return status;
    }

    status = wa_instance_bind_function(testInstance->instance,
                                       OC_STR8("print_f64_f64"),
                                       &(wa_func_type){
                                           .paramCount = 2,
                                           .params = (wa_value_type[]){
                                               WA_TYPE_F64,
                                               WA_TYPE_F64,
                                           },
                                       },
                                       test_print_f64_f64);
    if(status != WA_OK)
    {
        return status;
    }

    //TODO link registered modules...
    u32 funcIndex = 0;
    u32 memIndex = 0;
    u32 tableIndex = 0;
    u32 globalIndex = 0;

    for(u32 i = 0; i < module->importCount; i++)
    {
        wa_import* import = &module->imports[i];

        oc_list_for(env->instances, registeredInstance, wa_test_instance, listElt)
        {
            wa_module* registeredModule = registeredInstance->instance->module;

            if(!oc_str8_cmp(registeredInstance->registeredName, import->moduleName))
            {
                for(u32 exportIndex = 0; exportIndex < registeredModule->exportCount; exportIndex++)
                {
                    wa_export* export = &registeredModule->exports[exportIndex];
                    if(!oc_str8_cmp(export->name, import->importName))
                    {
                        if((u32)import->kind != (u32) export->kind)
                        {
                            return WA_FAIL_IMPORT_TYPE_MISMATCH;
                        }
                        if(import->kind == WA_IMPORT_FUNCTION)
                        {
                            wa_func* func = &testInstance->instance->functions[funcIndex];
                            wa_func* extFunc = &registeredInstance->instance->functions[export->index];

                            if(!wa_check_function_type(func->type, extFunc->type))
                            {
                                return WA_FAIL_IMPORT_TYPE_MISMATCH;
                            }

                            func->extInstance = registeredInstance->instance;
                            func->extIndex = export->index;
                        }
                        else if(import->kind == WA_IMPORT_MEMORY)
                        {
                            if(!wa_check_limits_match(&registeredInstance->instance->memories[export->index]->limits, &module->memories[memIndex]))
                            {
                                return WA_FAIL_IMPORT_TYPE_MISMATCH;
                            }
                            testInstance->instance->memories[memIndex] = registeredInstance->instance->memories[export->index];
                        }
                        else if(import->kind == WA_IMPORT_TABLE)
                        {
                            if(module->tables[tableIndex].type != registeredModule->tables[export->index].type
                               || !wa_check_limits_match(&registeredModule->tables[export->index].limits, &module->tables[tableIndex].limits))
                            {
                                return WA_FAIL_IMPORT_TYPE_MISMATCH;
                            }

                            testInstance->instance->tables[tableIndex] = registeredInstance->instance->tables[export->index];
                        }
                        else if(import->kind == WA_IMPORT_GLOBAL)
                        {
                            if(module->globals[globalIndex].type != registeredModule->globals[export->index].type
                               || module->globals[globalIndex].mut != registeredModule->globals[export->index].mut)
                            {
                                return WA_FAIL_IMPORT_TYPE_MISMATCH;
                            }

                            testInstance->instance->globals[globalIndex] = registeredInstance->instance->globals[export->index];
                        }
                        break;
                    }
                }
                break;
            }
        }
        if(import->kind == WA_IMPORT_FUNCTION)
        {
            funcIndex++;
        }
        else if(import->kind == WA_IMPORT_MEMORY)
        {
            memIndex++;
        }
        else if(import->kind == WA_IMPORT_TABLE)
        {
            tableIndex++;
        }
        else if(import->kind == WA_IMPORT_GLOBAL)
        {
            globalIndex++;
        }
    }
    return wa_instance_link(testInstance->instance);
}

wa_status wa_test_failure_string_to_status(oc_str8 failure)
{
    wa_status expected = WA_OK;
    if(!oc_str8_cmp(failure, OC_STR8("integer divide by zero")))
    {
        expected = WA_TRAP_DIVIDE_BY_ZERO;
    }
    else if(!oc_str8_cmp(failure, OC_STR8("integer overflow")))
    {
        expected = WA_TRAP_INTEGER_OVERFLOW;
    }
    else if(!oc_str8_cmp(failure, OC_STR8("invalid conversion to integer")))
    {
        expected = WA_TRAP_INVALID_INTEGER_CONVERSION;
    }
    else if(!oc_str8_cmp(failure, OC_STR8("out of bounds memory access")))
    {
        expected = WA_TRAP_MEMORY_OUT_OF_BOUNDS;
    }
    else if(!oc_str8_cmp(failure, OC_STR8("out of bounds table access")))
    {
        expected = WA_TRAP_TABLE_OUT_OF_BOUNDS;
    }
    else if(!oc_str8_cmp(oc_str8_slice(failure, 0, oc_min(failure.len, OC_STR8("uninitialized element").len)), OC_STR8("uninitialized element")))
    {
        expected = WA_TRAP_REF_NULL;
    }
    else if(!oc_str8_cmp(failure, OC_STR8("undefined element")))
    {
        expected = WA_TRAP_TABLE_OUT_OF_BOUNDS;
    }
    else if(!oc_str8_cmp(failure, OC_STR8("unreachable")))
    {
        expected = WA_TRAP_UNREACHABLE;
    }
    else if(!oc_str8_cmp(failure, OC_STR8("indirect call type mismatch")))
    {
        expected = WA_TRAP_INDIRECT_CALL_TYPE_MISMATCH;
    }
    else if(!oc_str8_cmp(failure, OC_STR8("unknown import")))
    {
        expected = WA_FAIL_MISSING_IMPORT;
    }
    else if(!oc_str8_cmp(failure, OC_STR8("incompatible import type")))
    {
        expected = WA_FAIL_IMPORT_TYPE_MISMATCH;
    }
    else
    {
        oc_log_error("unhandled assert_trap failure string %.*s\n", oc_str8_ip(failure));
    }
    return expected;
}

int test_file(oc_str8 testPath, oc_str8 testName, oc_str8 testDir, i32 filterLine, wa_test_env* env)
{
    oc_str8 contents = { 0 };
    {
        oc_file file = oc_file_open(testPath, OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);
        if(oc_file_last_error(file) != OC_IO_OK)
        {
            oc_log_error("Couldn't open file %.*s\n", oc_str8_ip(testPath));
            return (-1);
        }

        contents.len = oc_file_size(file);
        contents.ptr = oc_arena_push(env->arena, contents.len);

        oc_file_read(file, contents.len, contents.ptr);
        oc_file_close(file);
    }

    //spec test memory
    oc_base_allocator* allocator = oc_base_allocator_default();
    env->testspecMemory = (wa_memory){
        .limits = {
            .kind = WA_LIMIT_MIN_MAX,
            .min = 1,
            .max = 2,
        },
        .cap = 2 * WA_PAGE_SIZE,
        .size = WA_PAGE_SIZE,
        .ptr = oc_base_reserve(allocator, 2 * WA_PAGE_SIZE),
    };
    oc_base_commit(allocator, env->testspecMemory.ptr, env->testspecMemory.size);

    //spec test table
    env->testspecTable = (wa_table){
        .size = 10,
        .contents = oc_arena_push_array(env->arena, wa_value, 10),
    };
    memset(env->testspecTable.contents, 0, 10 * sizeof(wa_value));

    env->testspecGlobal_i32 = (wa_value){ .valI32 = 666 };
    env->testspecGlobal_i64 = (wa_value){ .valI64 = 666 };

    env->fileName = testName;

    json_node* json = json_parse_str8(env->arena, contents);
    json_node* commands = json_find_assert(json, "commands", JSON_LIST);

    oc_list_for(commands->children, command, json_node, listElt)
    {
        env->command = command;

        json_node* line = json_find(command, OC_STR8("line"));

        json_node* type = json_find_assert(command, "type", JSON_STRING);

        if(!oc_str8_cmp(type->string, OC_STR8("module")))
        {
            wa_test_instance* testInstance = oc_arena_push_type(env->arena, wa_test_instance);
            memset(testInstance, 0, sizeof(wa_test_instance));

            json_node* name = json_find(command, OC_STR8("name"));
            if(name)
            {
                OC_ASSERT(name->kind == JSON_STRING);
                testInstance->name = oc_str8_push_copy(env->arena, name->string);
            }

            json_node* filename = json_find_assert(command, "filename", JSON_STRING);

            oc_str8_list list = { 0 };
            oc_str8_list_push(env->arena, &list, testDir);
            oc_str8_list_push(env->arena, &list, filename->string);

            oc_str8 filePath = oc_path_join(env->arena, list);

            wa_module* module = wa_test_module_load(env->arena, filePath);

            if(!oc_list_empty(module->errors))
            {
                if(env->verbose)
                {
                    wa_module_print_errors(module);
                }
                wa_test_fail(env, testName, command);
            }
            else
            {
                wa_status status = wa_test_instantiate(env, testInstance, module);
                if(status != WA_OK)
                {
                    wa_test_fail(env, testName, command);
                }
                else
                {
                    wa_test_pass(env, testName, command);
                    oc_list_push_back(&env->instances, &testInstance->listElt);
                }
            }
        }
        else if(!oc_str8_cmp(type->string, OC_STR8("register")))
        {
            json_node* as = json_find_assert(command, "as", JSON_STRING);
            wa_test_instance* mod = oc_list_last_entry(env->instances, wa_test_instance, listElt);
            if(!mod || !mod->instance)
            {
                wa_test_fail(env, testName, command);
                continue;
            }
            mod->registeredName = oc_str8_push_copy(env->arena, as->string);
            wa_test_pass(env, testName, command);
        }
        else
        {
            if(filterLine >= 0)
            {
                if(!line || line->numI64 != filterLine)
                {
                    continue;
                }
            }

            if(!oc_str8_cmp(type->string, OC_STR8("action")))
            {
                json_node* action = json_find_assert(command, "action", JSON_OBJECT);

                wa_instance* instance = wa_test_get_instance(env, action);
                if(!instance)
                {
                    wa_test_fail(env, testName, command);
                    continue;
                }

                wa_test_action(env, instance, action);
                wa_test_pass(env, testName, command);
            }
            else if(!oc_str8_cmp(type->string, OC_STR8("assert_return")))
            {
                json_node* expected = json_find_assert(command, "expected", JSON_LIST);
                json_node* action = json_find_assert(command, "action", JSON_OBJECT);

                wa_instance* instance = wa_test_get_instance(env, action);
                if(!instance)
                {
                    wa_test_fail(env, testName, command);
                    continue;
                }

                wa_test_result result = wa_test_action(env, instance, action);

                if(result.status != WA_TEST_SKIP)
                {
                    bool check = (result.status == WA_TEST_PASS)
                              && (expected->childCount == result.count);
                    if(check)
                    {
                        u32 retIndex = 0;
                        oc_list_for(expected->children, expectRet, json_node, listElt)
                        {
                            //TODO: handle expected NaN

                            wa_typed_value expectVal = test_parse_value(expectRet);
                            switch(expectVal.type)
                            {
                                case WA_TYPE_I32:
                                    check = check && (result.values[retIndex].valI32 == expectVal.value.valI32);
                                    break;
                                case WA_TYPE_I64:
                                    check = check && (result.values[retIndex].valI64 == expectVal.value.valI64);
                                    break;
                                case WA_TYPE_F32:
                                {
                                    if(wa_is_nan_canonical_f32(expectVal.value.valF32))
                                    {
                                        check = check && wa_is_nan_canonical_f32(result.values[retIndex].valF32);
                                    }
                                    else if(wa_is_nan_arithmetic_f32(expectVal.value.valF32))
                                    {
                                        check = check && wa_is_nan_arithmetic_f32(result.values[retIndex].valF32);
                                    }
                                    else
                                    {
                                        //NOTE(martin): here we have to do a memcmp because we could be comparing non-canonical, non-arithmetic NaNs.
                                        check = check && !memcmp(&result.values[retIndex].valF32, &expectVal.value.valF32, sizeof(f32));
                                    }
                                }
                                break;
                                case WA_TYPE_F64:
                                {
                                    if(wa_is_nan_canonical_f64(expectVal.value.valF64))
                                    {
                                        check = check && wa_is_nan_canonical_f64(result.values[retIndex].valF64);
                                    }
                                    else if(wa_is_nan_arithmetic_f64(expectVal.value.valF64))
                                    {
                                        check = check && wa_is_nan_arithmetic_f64(result.values[retIndex].valF64);
                                    }
                                    else
                                    {
                                        //NOTE(martin): here we have to do a memcmp because we could be comparing non-canonical, non-arithmetic NaNs.
                                        check = check && !memcmp(&result.values[retIndex].valF64, &expectVal.value.valF64, sizeof(f32));
                                    }
                                }
                                break;

                                case WA_TYPE_EXTERN_REF:
                                case WA_TYPE_FUNC_REF:
                                {
                                    check = check && (result.values[retIndex].valI64 == expectVal.value.valI64);
                                }
                                break;

                                default:
                                    oc_log_error("unexpected type %s\n", wa_value_type_string(expectVal.type));
                                    OC_ASSERT(0, "unreachable");
                                    break;
                            }
                            if(!check)
                            {
                                break;
                            }
                            retIndex++;
                        }
                    }

                    if(!check)
                    {
                        wa_test_fail(env, testName, command);
                    }
                    else
                    {
                        wa_test_pass(env, testName, command);
                    }
                }
            }
            else if(!oc_str8_cmp(type->string, OC_STR8("assert_uninstantiable"))
                    || !oc_str8_cmp(type->string, OC_STR8("assert_unlinkable")))
            {
                json_node* failure = json_find_assert(command, "text", JSON_STRING);
                json_node* filename = json_find_assert(command, "filename", JSON_STRING);

                wa_status expected = wa_test_failure_string_to_status(failure->string);
                if(expected == WA_OK)
                {
                    wa_test_fail(env, testName, command);
                    continue;
                }

                wa_test_instance testInstance = { 0 };

                oc_str8_list list = { 0 };
                oc_str8_list_push(env->arena, &list, testDir);
                oc_str8_list_push(env->arena, &list, filename->string);

                oc_str8 filePath = oc_path_join(env->arena, list);

                wa_module* module = wa_test_module_load(env->arena, filePath);

                if(!oc_list_empty(module->errors))
                {
                    if(env->verbose)
                    {
                        wa_module_print_errors(module);
                    }
                    wa_test_fail(env, testName, command);
                }

                wa_status status = wa_test_instantiate(env, &testInstance, module);
                if(status != expected)
                {
                    wa_test_fail(env, testName, command);
                }
                else
                {
                    wa_test_pass(env, testName, command);
                }
            }
            else if(!oc_str8_cmp(type->string, OC_STR8("assert_trap")))
            {
                json_node* failure = json_find_assert(command, "text", JSON_STRING);
                json_node* action = json_find_assert(command, "action", JSON_OBJECT);

                wa_instance* instance = wa_test_get_instance(env, action);
                if(!instance)
                {
                    wa_test_fail(env, testName, command);
                    continue;
                }

                wa_status expected = wa_test_failure_string_to_status(failure->string);
                if(expected == WA_OK)
                {
                    wa_test_fail(env, testName, command);
                    continue;
                }

                wa_test_result result = wa_test_action(env, instance, action);

                if(result.status == WA_TEST_PASS || result.trap != expected)
                {
                    wa_test_fail(env, testName, command);
                }
                else
                {
                    wa_test_pass(env, testName, command);
                }
            }
            else if(!oc_str8_cmp(type->string, OC_STR8("assert_invalid")))
            {
                json_node* filename = json_find_assert(command, "filename", JSON_STRING);
                wa_module* module = wa_test_module_load(env->arena, filename->string);
                OC_ASSERT(module);

                //TODO: check the failure reason
                if(oc_list_empty(module->errors))
                {
                    wa_test_fail(env, testName, command);
                }
                else
                {
                    wa_test_pass(env, testName, command);
                }
            }
            else if(!oc_str8_cmp(type->string, OC_STR8("assert_malformed")))
            {
                wa_test_skip(env, testName, command);
            }
            else
            {
                oc_log_error("unsupported command %.*s\n", oc_str8_ip(type->string));
                wa_test_skip(env, testName, command);
            }
        }
    }

    env->totalPassed += env->passed;
    env->totalSkipped += env->skipped;
    env->totalFailed += env->failed;

    oc_base_release(allocator, env->testspecMemory.ptr, env->testspecMemory.size);

    return (0);
}

#include <sys/stat.h>
#include <dirent.h>

int test_main(int argc, char** argv)
{
    if(argc < 3)
    {
        printf("usage: warm test jsonfile [line]");
        return (-1);
    }

    oc_str8 testPath = OC_STR8(argv[2]);
    oc_str8 testName = oc_path_slice_filename(testPath);
    oc_str8 testDir = oc_path_slice_directory(testPath);

    i32 filterLine = -1;
    if(argc > 3)
    {
        filterLine = atoi(argv[3]);
    }

    oc_arena arena = { 0 };
    oc_arena_init(&arena);

    wa_test_env env = {
        .arena = &arena,
    };

    struct stat stbuf;
    stat(testPath.ptr, &stbuf);
    if(stbuf.st_mode & S_IFDIR)
    {
        testDir = testPath;

        DIR* dir = opendir(testPath.ptr);
        struct dirent* entry = 0;

        while((entry = readdir(dir)) != NULL)
        {
            oc_arena_clear(env.arena);
            env.instances = (oc_list){ 0 };
            env.passed = 0;
            env.skipped = 0;
            env.failed = 0;

            oc_str8 name = oc_str8_from_buffer(entry->d_namlen, entry->d_name);
            if(name.len > 5 && !oc_str8_cmp(oc_str8_slice(name, name.len - 5, name.len), OC_STR8(".json")))
            {
                oc_str8_list list = { 0 };
                oc_str8_list_push(&arena, &list, testDir);
                oc_str8_list_push(&arena, &list, name);

                oc_str8 path = oc_path_join(&arena, list);
                test_file(path, name, testDir, -1, &env);

                wa_test_status status = env.failed ? WA_TEST_FAIL : WA_TEST_PASS;

                printf("%s", wa_test_status_color_start[status]);
                printf("%s", wa_test_status_string[status]);
                printf("%s", wa_test_status_color_stop);

                printf(" %.*s: passed: %i, skipped: %i, failed: %i, total: %i\n",
                       oc_str8_ip(name),
                       env.passed,
                       env.skipped,
                       env.failed,
                       env.passed + env.skipped + env.failed);
            }
        }
        closedir(dir);
    }
    else
    {
        env.verbose = true;
        test_file(testPath, testName, testDir, filterLine, &env);
    }

    printf("\n--------------------------------------------------------------\n"
           "passed: %i, skipped: %i, failed: %i, total: %i\n"
           "--------------------------------------------------------------\n",
           env.totalPassed,
           env.totalSkipped,
           env.totalFailed,
           env.totalPassed + env.totalSkipped + env.totalFailed);

    return (0);
}

//-------------------------------------------------------------------------
// main
//-------------------------------------------------------------------------

void test_proc(wa_value* args, wa_value* returns)
{
    printf("hello! %i\n", args[0].valI32);
    returns[0].valI32 = 55;
}

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        printf("usage: warm module funcName [args...]\n");
        exit(-1);
    }
    if(!strcmp(argv[1], "test"))
    {
        return test_main(argc, argv);
    }

    oc_str8 modulePath = OC_STR8(argv[1]);
    oc_str8 funcName = OC_STR8(argv[2]);

    oc_arena arena = { 0 };
    oc_arena_init(&arena);

    //NOTE: load module
    oc_str8 contents = { 0 };

    oc_file file = oc_file_open(modulePath, OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE);

    contents.len = oc_file_size(file);
    contents.ptr = oc_arena_push(&arena, contents.len);

    oc_file_read(file, contents.len, contents.ptr);
    oc_file_close(file);

    wa_module* module = wa_module_create(&arena, contents);

    wa_ast_print(module->root, contents);
    wa_print_code(module);

    if(!oc_list_empty(module->errors))
    {
        wa_module_print_errors(module);
        exit(-1);
    }

    wa_instance* instance = wa_instance_create(&arena, module);

    wa_status status = wa_instance_bind_function(instance,
                                                 OC_STR8("test"),
                                                 &(wa_func_type){
                                                     .paramCount = 1,
                                                     .params = (wa_value_type[]){
                                                         WA_TYPE_I32,
                                                     },
                                                     .returnCount = 1,
                                                     .returns = (wa_value_type[]){
                                                         WA_TYPE_I32,
                                                     },
                                                 },
                                                 test_proc);
    if(status != WA_OK)
    {
        oc_log_error("%s", wa_status_strings[status]);
        exit(-1);
    }

    if(wa_instance_link(instance) != WA_OK)
    {
        exit(-1);
    }
    else
    {
        printf("Run:\n");
        wa_func* func = wa_instance_find_function(instance, funcName);

        if(!func)
        {
            oc_log_error("Couldn't find function %.*s.\n", oc_str8_ip(funcName));
            exit(-1);
        }

        u32 argCount = 0;
        wa_value args[32] = {};

        if(argc - 3 < func->type->paramCount)
        {
            oc_log_error("not enough arguments for function %.*s (expected %u, got %u)\n",
                         oc_str8_ip(funcName),
                         func->type->paramCount,
                         argc - 3);
            exit(-1);
        }
        else if(argc - 3 > func->type->paramCount)
        {
            oc_log_error("too many arguments for function %.*s (expected %u, got %u)\n",
                         oc_str8_ip(funcName),
                         func->type->paramCount,
                         argc - 3);
            exit(-1);
        }

        for(int i = 0; i < oc_min(argc - 3, 32); i++)
        {
            wa_typed_value val = parse_value_32(OC_STR8(argv[i + 3]));

            if(val.type != func->type->params[i])
            {
                oc_log_error("wrong type for argument %i of function %.*s (expected %.*s, got %.*s)\n",
                             i,
                             oc_str8_ip(funcName),
                             wa_value_type_string(func->type->params[i]),
                             wa_value_type_string(val.type));
                exit(-1);
            }
            args[i] = val.value;
            argCount++;
        }

        u32 retCount = 1;
        wa_value returns[32];

        wa_instance_invoke(instance, func, argCount, args, retCount, returns);

        printf("results: ");
        for(u32 retIndex = 0; retIndex < retCount; retIndex++)
        {
            printf("%lli ", returns[retIndex].valI64);
        }
        printf("\n");
    }
    return (0);
}
