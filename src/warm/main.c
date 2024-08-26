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

    WA_TYPE_UNKNOWN = 0xff,
}

wa_value_type;

typedef union wa_code
{
    wa_instr_op opcode;
    i32 valU32;
    i32 valI32;
    u64 valU64;
    i64 valI64;
    f32 valF32;
    f64 valF64;
    u32 index;
    wa_value_type valueType;

    struct
    {
        u32 align;
        u32 offset;
    } memArg;

    u8 laneIndex;

} wa_code;

typedef union wa_value
{
    i32 valI32;
    i64 valI64;
    f32 valF32;
    f64 valF64;
    //TODO v128, funcref, externref...
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

typedef void (*wa_host_proc)(wa_value* args, wa_value* returns); //TODO: complete with memory / etc

typedef struct wa_import wa_import;

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

} wa_element;

typedef enum wa_import_kind
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

typedef enum wa_export_kind
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

} wa_module;

typedef struct wa_instance
{
    wa_module* module;

    wa_func* functions;
    wa_value* globals;
    wa_table* tables;

    u64 memoryCap;
    u64 memorySize;
    char* memory;

    //TODO: tables
} wa_instance;

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
    WA_ERROR_BIND_TYPE,
    WA_FAIL_MISSING_IMPORT,

    WA_TRAP_UNREACHABLE,
    WA_TRAP_INVALID_OP,
    WA_TRAP_DIVIDE_BY_ZERO,
    WA_TRAP_INTEGER_OVERFLOW,
    WA_TRAP_INVALID_INTEGER_CONVERSION,
    WA_TRAP_STACK_OVERFLOW,
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
    wa_ast* ast = wa_ast_alloc(parser, WA_AST_U8);
    ast->loc.start = parser->offset;
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

    ast->loc.len = parser->offset - ast->loc.start;
    ast->parent = parent;
    oc_list_push_back(&parent->children, &ast->parentElt);

    return (ast);
}

wa_ast* wa_read_raw_u32(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_alloc(parser, WA_AST_U32);
    ast->loc.start = parser->offset;
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

    ast->loc.len = parser->offset - ast->loc.start;
    ast->parent = parent;
    oc_list_push_back(&parent->children, &ast->parentElt);

    return (ast);
}

wa_ast* wa_read_leb128_u64(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_alloc(parser, WA_AST_U64);
    ast->loc.start = parser->offset;
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
    ast->loc.len = parser->offset - ast->loc.start;

    ast->parent = parent;
    oc_list_push_back(&parent->children, &ast->parentElt);

    return (ast);
}

wa_ast* wa_read_leb128_i64(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_alloc(parser, WA_AST_I64);
    ast->loc.start = parser->offset;
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

    ast->loc.len = parser->offset - ast->loc.start;
    ast->parent = parent;
    oc_list_push_back(&parent->children, &ast->parentElt);

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
    wa_ast* ast = wa_ast_alloc(parser, WA_AST_F32);
    ast->loc.start = parser->offset;
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

    ast->loc.len = parser->offset - ast->loc.start;
    ast->parent = parent;
    oc_list_push_back(&parent->children, &ast->parentElt);

    return ast;
}

wa_ast* wa_read_f64(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_alloc(parser, WA_AST_F64);
    ast->loc.start = parser->offset;
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

    ast->loc.len = parser->offset - ast->loc.start;
    ast->parent = parent;
    oc_list_push_back(&parent->children, &ast->parentElt);

    return ast;
}

wa_ast* wa_read_name(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_alloc(parser, WA_AST_NAME);
    ast->loc.start = parser->offset;
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
    ast->loc.len = parser->offset - ast->loc.start;
    ast->parent = parent;
    oc_list_push_back(&parent->children, &ast->parentElt);

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
        wa_ast* section = wa_ast_alloc(parser, WA_AST_SECTION);
        section->loc.start = parser->offset;

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

        section->loc.len = parser->offset - section->loc.start;
        section->parent = module->root;
        oc_list_push_back(&module->root->children, &section->parentElt);
    }
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

    wa_ast* vector = wa_ast_alloc(parser, WA_AST_VECTOR);
    vector->loc.start = parser->offset;
    vector->parent = section;
    oc_list_push_back(&section->children, &vector->parentElt);

    wa_ast* typeCountAst = wa_read_leb128_u32(parser, vector, OC_STR8("count"));
    if(wa_ast_has_errors(typeCountAst))
    {
        return;
    }

    module->typeCount = typeCountAst->valU32;
    module->types = oc_arena_push_array(parser->arena, wa_func_type, module->typeCount);

    for(u32 typeIndex = 0; typeIndex < module->typeCount; typeIndex++)
    {
        wa_func_type* type = &module->types[typeIndex];

        wa_ast* typeAst = wa_ast_alloc(parser, WA_AST_TYPE);
        typeAst->loc.start = parser->offset;
        typeAst->parent = vector;
        oc_list_push_back(&vector->children, &typeAst->parentElt);

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

        typeAst->loc.len = parser->offset - typeAst->loc.start;
    }

    vector->loc.len = parser->offset - vector->loc.start;
    section->loc.len = parser->offset - section->loc.start;

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
    wa_ast* limitsAst = wa_ast_alloc(parser, WA_AST_LIMITS);
    limitsAst->loc.start = parser->offset;
    limitsAst->parent = parent;
    oc_list_push_back(&parent->children, &limitsAst->parentElt);

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

    limitsAst->loc.len = parser->offset - limitsAst->loc.start;
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

    wa_ast* vector = wa_ast_alloc(parser, WA_AST_VECTOR);
    vector->loc.start = parser->offset;
    vector->parent = section;
    oc_list_push_back(&section->children, &vector->parentElt);

    wa_ast* importCountAst = wa_read_leb128_u32(parser, vector, OC_STR8("count"));

    module->importCount = importCountAst->valU32;
    module->imports = oc_arena_push_array(parser->arena, wa_import, module->importCount);

    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];

        wa_ast* importAst = wa_ast_alloc(parser, WA_AST_IMPORT);
        importAst->loc.start = parser->offset;
        importAst->parent = vector;
        oc_list_push_back(&vector->children, &importAst->parentElt);

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
        importAst->loc.len = parser->offset - importAst->loc.start;
    }

    vector->loc.len = parser->offset - vector->loc.start;
    section->loc.len = parser->offset - section->loc.start;

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
        vector = wa_ast_alloc(parser, WA_AST_VECTOR);
        vector->loc.start = parser->offset;
        vector->parent = section;
        oc_list_push_back(&section->children, &vector->parentElt);

        wa_ast* functionCountAst = wa_read_leb128_u32(parser, vector, OC_STR8("count"));

        module->functionCount = functionCountAst->valU32;
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
        vector->loc.len = parser->offset - vector->loc.start;
        section->loc.len = parser->offset - section->loc.start;
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
        vector = wa_ast_alloc(parser, WA_AST_VECTOR);
        vector->loc.start = parser->offset;
        vector->parent = section;
        oc_list_push_back(&section->children, &vector->parentElt);

        wa_ast* globalCountAst = wa_read_leb128_u32(parser, vector, OC_STR8("count"));
        module->globalCount = globalCountAst->valU32;
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

            wa_ast* globalAst = wa_ast_alloc(parser, WA_AST_GLOBAL);
            globalAst->loc.start = parser->offset;
            globalAst->parent = vector;
            oc_list_push_back(&vector->children, &globalAst->parentElt);

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

            globalAst->loc.len = parser->offset - globalAst->loc.start;
        }
        vector->loc.len = parser->offset - vector->loc.start;
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
        vector = wa_ast_alloc(parser, WA_AST_VECTOR);
        vector->loc.start = parser->offset;
        vector->parent = section;
        oc_list_push_back(&section->children, &vector->parentElt);

        wa_ast* tableCountAst = wa_read_leb128_u32(parser, vector, OC_STR8("count"));
        module->tableCount = tableCountAst->valU32;
    }

    module->tables = oc_arena_push_array(parser->arena, wa_table_type, module->tableImportCount + module->tableCount);
    memset(module->tables, 0, (module->tableImportCount + module->tableCount) * sizeof(wa_func));

    //NOTE: re-read imports, because the format is kinda stupid -- they should have included imports in the tables section
    u32 tableImportIndex = 0;
    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];
        if(import->kind == WA_IMPORT_TABLE)
        {
            wa_table_type* table = &module->tables[tableImportIndex];
            table->type = import->index;
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

            wa_ast* tableAst = wa_ast_alloc(parser, WA_AST_TABLE_TYPE);
            tableAst->loc.start = parser->offset;
            tableAst->parent = vector;
            oc_list_push_back(&vector->children, &tableAst->parentElt);

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

            tableAst->loc.len = parser->offset - tableAst->loc.start;
        }
        vector->loc.len = parser->offset - vector->loc.start;
        section->loc.len = parser->offset - section->loc.start;
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

    wa_ast* vector = wa_ast_alloc(parser, WA_AST_VECTOR);
    vector->loc.start = parser->offset;
    vector->parent = section;
    oc_list_push_back(&section->children, &vector->parentElt);

    wa_ast* exportCountAst = wa_read_leb128_u32(parser, vector, OC_STR8("count"));
    module->exportCount = exportCountAst->valU32;
    module->exports = oc_arena_push_array(parser->arena, wa_export, module->exportCount);

    for(u32 exportIndex = 0; exportIndex < module->exportCount; exportIndex++)
    {
        wa_export* export = &module->exports[exportIndex];

        wa_ast* exportAst = wa_ast_alloc(parser, WA_AST_EXPORT);
        exportAst->loc.start = parser->offset;
        exportAst->parent = vector;
        oc_list_push_back(&vector->children, &exportAst->parentElt);

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
        exportAst->loc.len = parser->offset - exportAst->loc.start;
    }
    vector->loc.len = parser->offset - vector->loc.start;
    section->loc.len = parser->offset - section->loc.start;

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

wa_ast* wa_parse_expression(wa_parser* parser, wa_ast* parent, u32 localCount, oc_list* list, bool constant)
{
    wa_module* module = parser->module;

    wa_ast* exprAst = wa_ast_alloc(parser, WA_AST_FUNC_BODY);
    exprAst->loc.start = parser->offset;
    exprAst->parent = parent;
    oc_list_push_back(&parent->children, &exprAst->parentElt);

    //TODO: we should validate block nesting here?

    i64 blockDepth = 0;

    while(!wa_parser_end(parser) && blockDepth >= 0)
    {
        wa_instr* instr = oc_arena_push_type(parser->arena, wa_instr);
        memset(instr, 0, sizeof(wa_instr));
        oc_list_push_back(list, &instr->listElt);

        wa_ast* instrAst = wa_ast_alloc(parser, WA_AST_INSTR);
        instrAst->loc.start = parser->offset;
        instrAst->parent = exprAst;
        oc_list_push_back(&exprAst->children, &instrAst->parentElt);
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
            wa_ast* vector = wa_ast_alloc(parser, WA_AST_VECTOR);
            vector->loc.start = parser->offset;
            vector->parent = instrAst;
            oc_list_push_back(&instrAst->children, &vector->parentElt);

            wa_ast* count = wa_read_leb128_u32(parser, vector, OC_STR8("count"));
            if(count->valU32 != 1)
            {
                wa_parse_error(parser,
                               count,
                               "select instruction can have at most one immediate\n");
                break;
            }

            wa_ast* immAst = wa_parse_value_type(parser, vector, OC_STR8("type"));

            instr->immCount = 1;
            instr->imm = oc_arena_push_type(parser->arena, wa_code);
            instr->imm[0].valueType = immAst->valU32;

            vector->loc.len = parser->offset - vector->loc.start;
        }
        else if(instr->op == WA_INSTR_br_table)
        {
            wa_ast* vector = wa_ast_alloc(parser, WA_AST_VECTOR);
            vector->loc.start = parser->offset;
            vector->parent = instrAst;
            oc_list_push_back(&instrAst->children, &vector->parentElt);

            wa_ast* count = wa_read_leb128_u32(parser, vector, OC_STR8("count"));

            instr->immCount = count->valU32 + 1;
            instr->imm = oc_arena_push_array(parser->arena, wa_code, instr->immCount);

            for(u32 i = 0; i < instr->immCount - 1; i++)
            {
                wa_ast* immAst = wa_read_leb128_u32(parser, vector, OC_STR8("label"));
                instr->imm[i].index = immAst->valU32;
            }
            wa_ast* immAst = wa_read_leb128_u32(parser, vector, OC_STR8("label"));
            instr->imm[instr->immCount - 1].index = immAst->valU32;

            vector->loc.len = parser->offset - vector->loc.start;
        }
        else
        {
            //generic case
            ////////////////////////////////////////////////////////////////////
            //TODO validate all immediates
            ////////////////////////////////////////////////////////////////////
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

                        if(immAst->valU32 >= localCount)
                        {
                            wa_parse_error(parser,
                                           immAst,
                                           "invalid local index %u (localCount: %u)\n",
                                           immAst->valU32,
                                           localCount);
                        }
                    }
                    break;

                    case WA_IMM_FUNC_INDEX:
                    {
                        wa_ast* immAst = wa_read_leb128_u32(parser, instrAst, OC_STR8("function index"));
                        instr->imm[immIndex].index = immAst->valU32;
                        immAst->kind = WA_AST_FUNC_INDEX;

                        if(immAst->valU32 >= module->functionCount)
                        {
                            wa_parse_error(parser,
                                           immAst,
                                           "invalid function index %u (function count: %u)\n",
                                           immAst->valU32,
                                           module->functionCount);
                        }
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
                        wa_ast* memArgAst = wa_ast_alloc(parser, WA_AST_MEM_ARG);
                        memArgAst->loc.start = parser->offset;
                        memArgAst->parent = instrAst;
                        oc_list_push_back(&instrAst->children, &memArgAst->parentElt);

                        wa_ast* alignAst = wa_read_leb128_u32(parser, memArgAst, OC_STR8("mem arg"));
                        wa_ast* offsetAst = wa_read_leb128_u32(parser, memArgAst, OC_STR8("mem arg"));

                        instr->imm[immIndex].memArg.align = alignAst->valU32;
                        instr->imm[immIndex].memArg.offset = offsetAst->valU32;

                        memArgAst->loc.len = parser->offset - memArgAst->loc.start;
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

        instrAst->loc.len = parser->offset - instrAst->loc.start;
    }
    //TODO check that we exited from an end instruction

    exprAst->loc.len = parser->offset - exprAst->loc.start;
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

    wa_ast* vector = wa_ast_alloc(parser, WA_AST_VECTOR);
    vector->loc.start = parser->offset;
    vector->parent = section;
    oc_list_push_back(&section->children, &vector->parentElt);

    wa_ast* elementCountAst = wa_read_leb128_u32(parser, vector, OC_STR8("count"));
    module->elementCount = elementCountAst->valU32;
    module->elements = oc_arena_push_array(parser->arena, wa_element, module->elementCount);
    memset(module->elements, 0, module->elementCount * sizeof(wa_element));

    for(u32 eltIndex = 0; eltIndex < module->elementCount; eltIndex++)
    {
        wa_element* element = &module->elements[eltIndex];

        wa_ast* elementAst = wa_ast_alloc(parser, WA_AST_ELEMENT);
        elementAst->loc.start = parser->offset;
        elementAst->parent = vector;
        oc_list_push_back(&vector->children, &elementAst->parentElt);

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

            wa_ast* exprVec = wa_ast_alloc(parser, WA_AST_VECTOR);
            exprVec->loc.start = parser->offset;
            exprVec->parent = elementAst;
            oc_list_push_back(&elementAst->children, &exprVec->parentElt);

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

            wa_ast* funcVec = wa_ast_alloc(parser, WA_AST_VECTOR);
            funcVec->loc.start = parser->offset;
            funcVec->parent = elementAst;
            oc_list_push_back(&elementAst->children, &funcVec->parentElt);

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
                init[0].op = WA_INSTR_ref_func;
                init[0].immCount = 1;
                init[0].imm = oc_arena_push_type(parser->arena, wa_code);
                init[0].imm[0].index = funcIndexAst->valU32;
                oc_list_push_back(&element->initInstr[i], &init[0].listElt);

                init[1].op = WA_INSTR_end;
                oc_list_push_back(&element->initInstr[i], &init[1].listElt);
            }
        }

        elementAst->loc.len = parser->offset - elementAst->loc.start;
    }

    vector->loc.len = parser->offset - vector->loc.start;

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

void wa_parse_code(wa_parser* parser, wa_module* module)
{
    wa_ast* section = module->toc.code.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.code.offset, OC_STR8("code section"));
    u64 startOffset = parser->offset;

    wa_ast* vector = wa_ast_alloc(parser, WA_AST_VECTOR);
    vector->loc.start = parser->offset;
    vector->parent = section;
    oc_list_push_back(&section->children, &vector->parentElt);

    wa_ast* functionCountAst = wa_read_leb128_u32(parser, vector, OC_STR8("count"));
    u32 functionCount = functionCountAst->valU32;

    if(functionCount != module->functionCount - module->functionImportCount)
    {
        wa_parse_error(parser,
                       functionCountAst,
                       "Function count mismatch (function section: %i, code section: %i\n",
                       module->functionCount - module->functionImportCount,
                       functionCount);
    }
    functionCount = oc_min(functionCount + module->functionImportCount, module->functionCount);

    for(u32 funcIndex = module->functionImportCount; funcIndex < functionCount; funcIndex++)
    {
        wa_func* func = &module->functions[funcIndex];

        wa_ast* funcAst = wa_ast_alloc(parser, WA_AST_FUNC);
        funcAst->loc.start = parser->offset;
        funcAst->parent = vector;
        oc_list_push_back(&vector->children, &funcAst->parentElt);

        funcAst->func = func;

        oc_arena_scope scratch = oc_scratch_begin();

        wa_ast* lenAst = wa_read_leb128_u32(parser, funcAst, OC_STR8("function length"));

        u32 funcLen = lenAst->valU32;
        u32 funcStartOffset = parser->offset;

        //NOTE: parse locals
        wa_ast* localsVector = wa_ast_alloc(parser, WA_AST_VECTOR);
        localsVector->loc.start = parser->offset;
        localsVector->parent = funcAst;
        oc_list_push_back(&funcAst->children, &localsVector->parentElt);

        wa_ast* localEntryCountAst = wa_read_leb128_u32(parser, localsVector, OC_STR8("local type entries count"));

        u32 localEntryCount = localEntryCountAst->valU32;

        u32* counts = oc_arena_push_array(scratch.arena, u32, localEntryCount);
        wa_value_type* types = oc_arena_push_array(scratch.arena, wa_value_type, localEntryCount);

        func->localCount = func->type->paramCount;
        for(u32 localEntryIndex = 0; localEntryIndex < localEntryCount; localEntryIndex++)
        {
            wa_ast* localEntryAst = wa_ast_alloc(parser, WA_AST_LOCAL_ENTRY);
            localEntryAst->loc.start = parser->offset;
            localEntryAst->parent = localsVector;
            oc_list_push_back(&localsVector->children, &localEntryAst->parentElt);

            wa_ast* countAst = wa_read_leb128_u32(parser, localEntryAst, OC_STR8("count"));
            wa_ast* typeAst = wa_read_byte(parser, localEntryAst, OC_STR8("index"));
            typeAst->kind = WA_AST_TYPE_INDEX;

            counts[localEntryIndex] = countAst->valU32;
            types[localEntryIndex] = typeAst->valU8;
            func->localCount += counts[localEntryIndex];

            localEntryAst->loc.len = parser->offset - localEntryAst->loc.start;
            //TODO: validate types? or validate later?
        }
        localsVector->loc.len = parser->offset - localsVector->loc.start;

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

        funcAst->loc.len = parser->offset - funcAst->loc.start;

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

void wa_free_slot(wa_build_context* context, u64 index)
{
    OC_DEBUG_ASSERT(context->freeRegLen >= WA_MAX_REG);
    context->freeRegs[context->freeRegLen] = index;
    context->freeRegLen++;
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

void wa_block_set_polymorphic(wa_build_context* context, wa_block* block)
{
    block->polymorphic = true;
    wa_operand_stack_pop_scope(context, block);
}

void wa_emit(wa_build_context* context, wa_code code)
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
    context->code[context->codeLen] = code;
    context->codeLen++;
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
        wa_emit(context, (wa_code){ .opcode = WA_INSTR_move });
        wa_emit(context, (wa_code){ .valI32 = slotIndex });
        wa_emit(context, (wa_code){ .valI32 = newReg });
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

bool wa_check_operand_type(wa_value_type t1, wa_value_type t2)
{
    return (t1 == t2 || t1 == WA_TYPE_UNKNOWN || t2 == WA_TYPE_UNKNOWN);
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
    //NOTE: check block type input
    wa_func_type* type = instr->blockType;

    for(u64 inIndex = 0; inIndex < type->paramCount; inIndex++)
    {
        wa_operand_slot slot = wa_operand_stack_lookup(context, type->paramCount - inIndex - 1);
        if(wa_operand_slot_is_nil(&slot))
        {
            wa_compile_error(context,
                             instr->ast,
                             "not enough operands on stack (expected %i, got %i)\n",
                             type->paramCount,
                             wa_operand_stack_scope_size(context));
        }
        else
        {
            wa_value_type expected = type->params[inIndex];
            if(!wa_check_operand_type(slot.type, expected))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "operand type mismatch for param %i (expected %s, got %s)\n",
                                 inIndex,
                                 wa_value_type_string(expected),
                                 wa_value_type_string(slot.type));
            }
        }
    }

    //NOTE allocate block results and push them on the stack
    for(u64 outIndex = 0; outIndex < type->returnCount; outIndex++)
    {
        u32 index = wa_allocate_register(context);

        wa_operand_slot s = {
            .kind = WA_OPERAND_SLOT_REG,
            .index = index,
            .type = type->returns[outIndex],
        };
        wa_operand_stack_push(context, s);

        //TODO immediately put them in the freelist so they can be used in the branches (this might complicate copying results a bit...)
        // wa_free_slot(context, index);
    }

    wa_control_stack_push(context, instr);
    wa_push_block_inputs(context, type);
}

void wa_block_move_results_to_input_slots(wa_build_context* context, wa_block* block, wa_instr* instr)
{
    wa_func_type* type = block->begin->blockType;

    for(u64 paramIndex = 0; paramIndex < type->paramCount; paramIndex++)
    {
        wa_operand_slot slot = wa_operand_stack_lookup(context, type->paramCount - paramIndex - 1);
        if(wa_operand_slot_is_nil(&slot))
        {
            wa_compile_error(context,
                             instr->ast,
                             "not enough operands on stack (expected %i, got %i)\n",
                             type->paramCount,
                             wa_operand_stack_scope_size(context));
            return;
        }
        else
        {
            wa_value_type expected = type->params[paramIndex];
            if(!wa_check_operand_type(expected, slot.type))
            {
                //TODO: here we'd like to point to the instruction that triggered the end of the block,
                //      _but also_ the ast of the param type...
                wa_compile_error(context,
                                 instr->ast,
                                 "operand type mismatch for block input %i (expected %s, got %s)\n",
                                 paramIndex,
                                 wa_value_type_string(expected),
                                 wa_value_type_string(slot.type));
                return;
            }

            if(!block->polymorphic && !block->prevPolymorphic)
            {
                wa_operand_slot* dst = &context->opdStack[block->scopeBase - type->returnCount - type->paramCount + paramIndex];
                wa_emit(context, (wa_code){ .opcode = WA_INSTR_move });
                wa_emit(context, (wa_code){ .valI64 = slot.index });
                wa_emit(context, (wa_code){ .valI64 = dst->index });
            }
        }
    }
}

void wa_block_move_results_to_output_slots(wa_build_context* context, wa_block* block, wa_instr* instr)
{
    //NOTE validate block type output
    wa_func_type* type = block->begin->blockType;

    for(u32 returnIndex = 0; returnIndex < type->returnCount; returnIndex++)
    {
        wa_operand_slot slot = wa_operand_stack_lookup(context, type->returnCount - returnIndex - 1);
        if(wa_operand_slot_is_nil(&slot))
        {
            wa_compile_error(context,
                             instr->ast,
                             "not enough operands on stack (expected %i, got %i)\n",
                             type->returnCount,
                             wa_operand_stack_scope_size(context));
        }
        else
        {
            wa_value_type expected = type->returns[returnIndex];
            if(!wa_check_operand_type(expected, slot.type))
            {
                //TODO: here we'd like to point to the instruction that triggered the end of the block,
                //      _but also_ the ast of the param type...
                wa_compile_error(context,
                                 instr->ast,
                                 "operand type mismatch for block output %i (expected %s, got %s)\n",
                                 returnIndex,
                                 wa_value_type_string(expected),
                                 wa_value_type_string(slot.type));
                return;
            }

            if(!block->polymorphic && !block->prevPolymorphic)
            {
                wa_operand_slot* dst = &context->opdStack[block->scopeBase - type->returnCount + returnIndex];
                wa_emit(context, (wa_code){ .opcode = WA_INSTR_move });
                wa_emit(context, (wa_code){ .valI64 = slot.index });
                wa_emit(context, (wa_code){ .valI64 = dst->index });
            }
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

        wa_emit(context, (wa_code){ .opcode = WA_INSTR_jump });
        wa_emit(context, (wa_code){ 0 });

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
    for(u32 retIndex = 0; retIndex < type->returnCount; retIndex++)
    {
        wa_operand_slot slot = wa_operand_stack_lookup(context, type->returnCount - retIndex - 1);
        if(wa_operand_slot_is_nil(&slot))
        {
            //TODO: we should also point to the ast of the return type
            wa_compile_error(context,
                             instr->ast,
                             "Not enough return values (expected: %i, stack depth: %i)\n",
                             type->returnCount,
                             wa_operand_stack_scope_size(context));
            return -1;
        }

        wa_value_type expectedType = type->returns[retIndex];
        wa_value_type returnType = slot.type;

        if(!wa_check_operand_type(returnType, expectedType))
        {
            //TODO: we should also point to the ast of the return type
            wa_compile_error(context,
                             instr->ast,
                             "Return type doesn't match function signature (expected %s, got %s)\n",
                             wa_value_type_string(expectedType),
                             wa_value_type_string(returnType));
            return -1;
        }

        //NOTE store value to return slot
        if(slot.type != WA_TYPE_UNKNOWN && slot.index != retIndex)
        {
            wa_move_slot_if_used(context, retIndex);

            wa_emit(context, (wa_code){ .opcode = WA_INSTR_move });
            wa_emit(context, (wa_code){ .valI64 = slot.index });
            wa_emit(context, (wa_code){ .valI64 = retIndex });
        }
    }
    wa_emit(context, (wa_code){ .opcode = WA_INSTR_return });

    //WARN: wa_compile_return() is also used by conditional or table branches when they target the top-level scope,
    //      so we don't set the stack polymorphic here. Instead we do it in the callers when return is unconditional

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
            wa_emit(context, (wa_code){ .opcode = WA_INSTR_jump });

            //NOTE: emit a jump target operand for end of block
            wa_jump_target* target = oc_arena_push_type(&context->checkArena, wa_jump_target);
            target->offset = context->codeLen;
            oc_list_push_back(&block->jumpTargets, &target->listElt);

            wa_emit(context, (wa_code){ .valI64 = 0 });
        }
        else if(block->begin->op == WA_INSTR_loop)
        {
            wa_block_move_results_to_input_slots(context, block, instr);

            //jump to begin
            wa_emit(context, (wa_code){ .opcode = WA_INSTR_jump });
            wa_emit(context, (wa_code){ .valI64 = block->beginOffset - context->codeLen });
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

void wa_compile_expression(wa_build_context* context, wa_func_type* type, wa_func* func, oc_list instructions)
{
    wa_module* module = context->module;

    context->exprType = type;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    //TODO: remove the need to pass instr -- this will break else checks if first instr is an "if"...
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    wa_control_stack_push(context, oc_list_first_entry(instructions, wa_instr, listElt));

    oc_list_for(instructions, instr, wa_instr, listElt)
    {
        const wa_instr_info* info = &wa_instr_infos[instr->op];

        //NOTE: immediates should have been validated when parsing instructions

        if(instr->op == WA_INSTR_unreachable)
        {
            wa_emit(context, (wa_code){ .opcode = WA_INSTR_unreachable });
            wa_block* block = wa_control_stack_top(context);
            wa_block_set_polymorphic(context, block);
        }
        if(instr->op == WA_INSTR_nop)
        {
            // do nothing
        }
        else if(instr->op == WA_INSTR_drop)
        {
            wa_operand_slot slot = wa_operand_stack_pop(context);
            if(wa_operand_slot_is_nil(&slot))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "unbalanced stack\n");
                break;
            }
            //TODO: possibly remove instruction that pushed this operand?
        }
        else if(instr->op == WA_INSTR_select || instr->op == WA_INSTR_select_t)
        {
            wa_operand_slot slot2 = wa_operand_stack_pop(context);
            wa_operand_slot slot1 = wa_operand_stack_pop(context);
            wa_operand_slot slot0 = wa_operand_stack_pop(context);

            if(wa_operand_slot_is_nil(&slot2))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "unbalanced stack\n");
                break;
            }

            if(!wa_check_operand_type(slot0.type, slot1.type))
            {
                //TODO: is this true?
                wa_compile_error(context,
                                 instr->ast,
                                 "select first two operands should be of the same type\n");
                break;
            }
            if(!wa_check_operand_type(slot2.type, WA_TYPE_I32))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "select last operands should be type i32\n");
                break;
            }

            if(instr->op == WA_INSTR_select_t)
            {
                if(slot0.type != instr->imm[0].valueType)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "select operands should be numeric\n");
                    break;
                }
            }
            else if(!wa_is_value_type_numeric(slot0.type))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "select operands should be numeric\n");
                break;
            }

            u32 index = wa_allocate_register(context);

            wa_operand_slot out = {
                .kind = WA_OPERAND_SLOT_REG,
                .type = slot0.type,
                .index = index,
                .originInstr = instr,
                .originOpd = context->codeLen,
            };
            wa_operand_stack_push(context, out);

            wa_emit(context, (wa_code){ .opcode = WA_INSTR_select });
            wa_emit(context, (wa_code){ .valI32 = slot0.index });
            wa_emit(context, (wa_code){ .valI32 = slot1.index });
            wa_emit(context, (wa_code){ .valI32 = slot2.index });
            wa_emit(context, (wa_code){ .valI32 = out.index });
        }
        else if(instr->op == WA_INSTR_block || instr->op == WA_INSTR_loop)
        {
            wa_move_locals_to_registers(context);
            wa_block_begin(context, instr);
        }
        else if(instr->op == WA_INSTR_br)
        {
            u32 label = instr->imm[0].index;
            wa_compile_branch(context, instr, label);

            wa_block* block = wa_control_stack_top(context);
            wa_block_set_polymorphic(context, block);
        }
        else if(instr->op == WA_INSTR_br_if)
        {
            wa_operand_slot slot = wa_operand_stack_pop(context);
            if(wa_operand_slot_is_nil(&slot))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "unbalanced stack\n");
                break;
            }

            if(!wa_check_operand_type(slot.type, WA_TYPE_I32))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "operand type mismatch (expected %s, got %s)\n",
                                 wa_value_type_string(WA_TYPE_I32),
                                 wa_value_type_string(slot.type));
                break;
            }

            wa_emit(context, (wa_code){ .opcode = WA_INSTR_jump_if_zero });
            u64 jumpOffset = context->codeLen;
            wa_emit(context, (wa_code){ .valI64 = 0 });
            wa_emit(context, (wa_code){ .valI64 = slot.index });

            u32 label = instr->imm[0].index;
            wa_compile_branch(context, instr, label);

            context->code[jumpOffset].valI64 = context->codeLen - jumpOffset;
        }
        else if(instr->op == WA_INSTR_br_table)
        {
            wa_operand_slot slot = wa_operand_stack_pop(context);
            if(wa_operand_slot_is_nil(&slot))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "unbalanced stack\n");
                break;
            }

            if(!wa_check_operand_type(slot.type, WA_TYPE_I32))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "operand type mismatch (expected %s, got %s)\n",
                                 wa_value_type_string(WA_TYPE_I32),
                                 wa_value_type_string(slot.type));
                break;
            }

            wa_emit(context, (wa_code){ .opcode = WA_INSTR_jump_table });
            u64 baseOffset = context->codeLen;

            wa_emit(context, (wa_code){ .valU32 = instr->immCount });
            wa_emit(context, (wa_code){ .valU32 = slot.index });

            oc_arena_scope scratch = oc_scratch_begin();
            u64* patchOffsets = oc_arena_push_array(scratch.arena, u64, instr->immCount);

            // reserve room for the table entries
            for(u32 i = 0; i < instr->immCount; i++)
            {
                patchOffsets[i] = context->codeLen;
                wa_emit(context, (wa_code){ 0 });
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

            oc_scratch_end(scratch);

            wa_block* block = wa_control_stack_top(context);
            wa_block_set_polymorphic(context, block);
        }
        else if(instr->op == WA_INSTR_if)
        {
            wa_operand_slot slot = wa_operand_stack_pop(context);
            if(wa_operand_slot_is_nil(&slot))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "unbalanced stack\n");
                break;
            }

            if(!wa_check_operand_type(slot.type, WA_TYPE_I32))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "operand type mismatch (expected %s, got %s)\n",
                                 wa_value_type_string(WA_TYPE_I32),
                                 wa_value_type_string(slot.type));
                break;
            }

            wa_move_locals_to_registers(context);

            wa_block_begin(context, instr);

            wa_emit(context, (wa_code){ .opcode = WA_INSTR_jump_if_zero });
            wa_emit(context, (wa_code){ 0 });
            wa_emit(context, (wa_code){ .valI64 = slot.index });
        }
        else if(instr->op == WA_INSTR_else)
        {
            wa_block* ifBlock = wa_control_stack_top(context);
            if(!ifBlock
               || ifBlock->begin->op != WA_INSTR_if
               || ifBlock->begin->elseBranch)
            {
                //TODO: should this be validated at parse stage?
                wa_compile_error(context,
                                 instr->ast,
                                 "unexpected else block\n");
                break;
            }
            wa_func_type* type = ifBlock->begin->blockType;

            wa_block_move_results_to_output_slots(context, ifBlock, instr);
            wa_operand_stack_pop_scope(context, ifBlock);
            wa_push_block_inputs(context, type);

            wa_emit(context, (wa_code){ .opcode = WA_INSTR_jump });
            wa_emit(context, (wa_code){ 0 });

            ifBlock->polymorphic = false;
            ifBlock->begin->elseBranch = instr;
            ifBlock->elseOffset = context->codeLen;
        }
        else if(instr->op == WA_INSTR_end)
        {
            wa_block* block = wa_control_stack_top(context);
            if(!block)
            {
                wa_compile_error(context, instr->ast, "Unbalanced control stack\n");
                break;
            }
            else if(context->controlStackLen == 1)
            {
                //TODO: is this sufficient to elide all previous returns?

                wa_instr* prev = oc_list_prev_entry(func->instructions, instr, wa_instr, listElt);
                if(!prev || prev->op != WA_INSTR_return)
                {
                    if(wa_compile_return(context, type, instr))
                    {
                        break;
                    }
                }
                wa_patch_jump_targets(context, block);
                wa_control_stack_pop(context);
                break;
            }
            else
            {
                wa_block_end(context, block, instr);
                wa_patch_jump_targets(context, block);
            }
        }
        else if(instr->op == WA_INSTR_call)
        {
            wa_func* callee = &module->functions[instr->imm[0].index];
            u32 paramCount = callee->type->paramCount;

            i64 maxUsedSlot = -1;
            for(u32 stackIndex = 0; stackIndex < context->opdStackLen; stackIndex++)
            {
                wa_operand_slot* slot = &context->opdStack[stackIndex];
                maxUsedSlot = oc_max((i64)slot->index, maxUsedSlot);
            }

            //TODO: first check if args are already in order at the end of the frame?
            for(u32 argIndex = 0; argIndex < paramCount; argIndex++)
            {
                wa_operand_slot slot = wa_operand_stack_lookup(context, paramCount - argIndex - 1);
                if(wa_operand_slot_is_nil(&slot))
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "Not enough arguments on stack (expected: %u, got: %u)\n",
                                     paramCount,
                                     wa_operand_stack_scope_size(context));
                    break;
                }

                wa_value_type paramType = callee->type->params[argIndex];

                if(!wa_check_operand_type(slot.type, paramType))
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "Type mismatch for argument %u (expected %s, got %s)\n",
                                     argIndex,
                                     wa_value_type_string(paramType),
                                     wa_value_type_string(slot.type));
                    break;
                }
                wa_emit(context, (wa_code){ .opcode = WA_INSTR_move });
                wa_emit(context, (wa_code){ .valI32 = slot.index });
                wa_emit(context, (wa_code){ .valI32 = maxUsedSlot + 1 + argIndex });
            }

            wa_operand_stack_pop_slots(context, paramCount);

            wa_emit(context, (wa_code){ .opcode = WA_INSTR_call });
            wa_emit(context, (wa_code){ .valI64 = instr->imm[0].index });
            wa_emit(context, (wa_code){ .valI64 = maxUsedSlot + 1 });

            /////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //TODO: we need to update next reg index so that next reg can't be allocated in the same slot...
            //      we probably also need to put the remaining args regs not used by returns in the freelist
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////
            context->nextRegIndex = oc_max(maxUsedSlot + 1 + callee->type->returnCount, context->nextRegIndex);

            for(u32 retIndex = 0; retIndex < callee->type->returnCount; retIndex++)
            {
                wa_operand_stack_push(context,
                                      (wa_operand_slot){
                                          .kind = WA_OPERAND_SLOT_REG,
                                          .type = callee->type->returns[retIndex],
                                          .index = maxUsedSlot + 1 + retIndex,
                                      });
            }
        }
        else if(instr->op == WA_INSTR_call_indirect)
        {
            ////////////////////////////////////////
            //TODO: coalesce with regular call
            ////////////////////////////////////////

            wa_operand_slot slot = wa_operand_stack_pop(context);
            if(wa_operand_slot_is_nil(&slot))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "unbalanced stack\n");
                break;
            }

            if(!wa_check_operand_type(slot.type, WA_TYPE_I32))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "operand type mismatch (expected %s, got %s)\n",
                                 wa_value_type_string(WA_TYPE_I32),
                                 wa_value_type_string(slot.type));
                break;
            }

            wa_func_type* type = &module->types[instr->imm[0].index];
            u32 paramCount = type->paramCount;

            //TODO: we should probably be more clever here, eg in some case just move the index operand
            //      past the arguments?
            i64 maxUsedSlot = slot.index;
            for(u32 stackIndex = 0; stackIndex < context->opdStackLen; stackIndex++)
            {
                wa_operand_slot* slot = &context->opdStack[stackIndex];
                maxUsedSlot = oc_max((i64)slot->index, maxUsedSlot);
            }

            //TODO: first check if args are already in order at the end of the frame?
            for(u32 argIndex = 0; argIndex < paramCount; argIndex++)
            {
                wa_operand_slot slot = wa_operand_stack_lookup(context, paramCount - argIndex - 1);
                if(wa_operand_slot_is_nil(&slot))
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "Not enough arguments on stack (expected: %u, got: %u)\n",
                                     paramCount,
                                     wa_operand_stack_scope_size(context));
                    break;
                }

                wa_value_type paramType = type->params[argIndex];

                if(!wa_check_operand_type(slot.type, paramType))
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "Type mismatch for argument %u (expected %s, got %s)\n",
                                     argIndex,
                                     wa_value_type_string(paramType),
                                     wa_value_type_string(slot.type));
                    break;
                }
                wa_emit(context, (wa_code){ .opcode = WA_INSTR_move });
                wa_emit(context, (wa_code){ .valI32 = slot.index });
                wa_emit(context, (wa_code){ .valI32 = maxUsedSlot + 1 + argIndex });
            }

            wa_operand_stack_pop_slots(context, paramCount);

            wa_emit(context, (wa_code){ .opcode = WA_INSTR_call_indirect });
            wa_emit(context, (wa_code){ .valI32 = instr->imm[0].index });
            wa_emit(context, (wa_code){ .valI32 = instr->imm[1].index });
            wa_emit(context, (wa_code){ .valI64 = maxUsedSlot + 1 });
            wa_emit(context, (wa_code){ .valI32 = slot.index });

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
            if(wa_compile_return(context, type, instr))
            {
                break;
            }
            wa_block* block = wa_control_stack_top(context);
            block->polymorphic = true;
            wa_operand_stack_pop_scope(context, block);
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
        else if(instr->op == WA_INSTR_local_set)
        {
            u32 localIndex = instr->imm[0].index;

            wa_operand_slot slot = wa_operand_stack_pop(context);
            if(wa_operand_slot_is_nil(&slot))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "unbalanced stack\n");
                break;
            }

            if(!wa_check_operand_type(slot.type, func->locals[localIndex]))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "type mismatch for local.set instruction (expected %s, got %s)\n",
                                 wa_value_type_string(func->locals[localIndex]),
                                 wa_value_type_string(slot.type));
                break;
            }

            // check if the local was used in the stack and if so save it to a slot
            wa_move_slot_if_used(context, localIndex);

            //TODO: check if the local was written to since the value was pushed, and if not, change
            //      the output operand of the value's origin instruction rather than issuing a move
            bool shortcutSet = false;
            /*NOTE: this can't be used after a branch, since the branch might use the original slot index
                //      so for now, disable this optimization
                // later we can add a "touched" bit and set it for operands used in a branch?
                if(slot.kind == WA_OPERAND_SLOT_REG && slot.originOpd)
                {
                    shortcutSet = true;
                    for(u32 instrIt = slot.originInstr; instrIt < instrIndex; instrIt++)
                    {
                        if(func->instructions[instrIt].op == WA_INSTR_local_set && func->instructions[instrIt].val[0].valI32 == localIndex)
                        {
                            shortcutSet = false;
                            break;
                        }
                    }
                }
                */
            if(shortcutSet)
            {
                OC_DEBUG_ASSERT(context->code[slot.originOpd].valU64 == slot.index);
                context->code[slot.originOpd].valI64 = localIndex;
            }
            else
            {
                wa_emit(context, (wa_code){ .opcode = WA_INSTR_move });
                wa_emit(context, (wa_code){ .valI32 = slot.index });
                wa_emit(context, (wa_code){ .valI32 = localIndex });
            }
        }
        else if(instr->op == WA_INSTR_local_tee)
        {
            u32 localIndex = instr->imm[0].index;

            wa_operand_slot slot = wa_operand_stack_top(context);
            if(wa_operand_slot_is_nil(&slot))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "unbalanced stack\n");
                break;
            }

            if(!wa_check_operand_type(slot.type, func->locals[localIndex]))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "type mismatch for local.set instruction (expected %s, got %s)\n",
                                 wa_value_type_string(func->locals[localIndex]),
                                 wa_value_type_string(slot.type));
                break;
            }
            wa_move_slot_if_used(context, localIndex);

            wa_emit(context, (wa_code){ .opcode = WA_INSTR_move });
            wa_emit(context, (wa_code){ .valI32 = slot.index });
            wa_emit(context, (wa_code){ .valI32 = localIndex });
        }
        else if(instr->op == WA_INSTR_global_get)
        {
            u32 globalIndex = instr->imm[0].index;

            u32 regIndex = wa_allocate_register(context);

            wa_operand_stack_push(
                context,
                (wa_operand_slot){
                    .kind = WA_OPERAND_SLOT_REG,
                    .type = module->globals[globalIndex].type,
                    .index = regIndex,
                    .originInstr = instr,
                    .originOpd = context->codeLen,
                });

            wa_emit(context, (wa_code){ .opcode = WA_INSTR_global_get });
            wa_emit(context, (wa_code){ .valI32 = globalIndex });
            wa_emit(context, (wa_code){ .valI32 = regIndex });
        }
        else if(instr->op == WA_INSTR_global_set)
        {
            u32 globalIndex = instr->imm[0].index;

            wa_operand_slot slot = wa_operand_stack_top(context);
            if(wa_operand_slot_is_nil(&slot))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "unbalanced stack\n");
                break;
            }

            if(!wa_check_operand_type(slot.type, module->globals[globalIndex].type))
            {
                wa_compile_error(context,
                                 instr->ast,
                                 "type mismatch for global.set instruction (expected %s, got %s)\n",
                                 wa_value_type_string(module->globals[globalIndex].type),
                                 wa_value_type_string(slot.type));
                break;
            }
            wa_emit(context, (wa_code){ .opcode = WA_INSTR_global_set });
            wa_emit(context, (wa_code){ .valI32 = slot.index });
            wa_emit(context, (wa_code){ .valI32 = globalIndex });
        }
        else
        {
            //TODO validate instruction
            //NOTE emit opcode
            wa_emit(context, (wa_code){ .opcode = instr->op });

            //emit immediates
            for(int immIndex = 0; immIndex < info->immCount; immIndex++)
            {
                wa_emit(context, instr->imm[immIndex]);
            }

            // validate stack, allocate slots and emit operands
            //TODO: could validate in order (from deeper to upper in the stack, and have the operands in the right order for binops...)
            for(int opdIndex = 0; opdIndex < info->inCount; opdIndex++)
            {
                wa_operand_slot slot = wa_operand_stack_pop(context);
                if(wa_operand_slot_is_nil(&slot))
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "unbalanced stack\n");
                    break;
                }

                if(!wa_check_operand_type(slot.type, info->in[opdIndex]))
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "operand type mismatch\n");
                    break;
                }

                wa_emit(context, (wa_code){ .index = slot.index });
            }

            for(int opdIndex = 0; opdIndex < info->outCount; opdIndex++)
            {
                u32 index = wa_allocate_register(context);

                wa_operand_slot s = {
                    .kind = WA_OPERAND_SLOT_REG,
                    .type = info->out[opdIndex],
                    .index = index,
                    .originInstr = instr,
                    .originOpd = context->codeLen,
                };
                wa_operand_stack_push(context, s);

                wa_emit(context, (wa_code){ .index = s.index });
            }
        }
    }
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
    wa_parse_exports(&parser, module);
    wa_parse_elements(&parser, module);
    wa_parse_code(&parser, module);

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

wa_instance* wa_instance_create(oc_arena* arena, wa_module* module)
{
    wa_instance* instance = oc_arena_push_type(arena, wa_instance);
    memset(instance, 0, sizeof(wa_instance));

    instance->module = module;
    instance->functions = oc_arena_push_array(arena, wa_func, module->functionCount);
    memcpy(instance->functions, module->functions, module->functionCount * sizeof(wa_func));

    instance->globals = oc_arena_push_array(arena, wa_value, module->globalCount);

    instance->tables = oc_arena_push_array(arena, wa_table, module->tableCount);
    for(u32 tableIndex = 0; tableIndex < module->tableCount; tableIndex++)
    {
        wa_table_type* desc = &module->tables[tableIndex];

        instance->tables[tableIndex].size = desc->limits.min;
        instance->tables[tableIndex].contents = oc_arena_push_array(arena, wa_value, desc->limits.min);
        memset(instance->tables[tableIndex].contents, 0, desc->limits.min * sizeof(wa_value));
    }

    return (instance);
}

wa_status wa_instance_bind_function(wa_instance* instance, oc_str8 name, wa_func_type* type, wa_host_proc proc)
{
    wa_module* module = instance->module;
    u32 funcIndex = 0;

    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];

        if(import->kind == WA_IMPORT_FUNCTION)
        {
            if(!oc_str8_cmp(name, import->importName))
            {
                wa_func* func = &instance->functions[funcIndex];
                func->proc = proc;

                if(type->paramCount != func->type->paramCount
                   || type->returnCount != func->type->returnCount)
                {
                    //log error to module
                    return WA_ERROR_BIND_TYPE;
                }
                for(u32 paramIndex = 0; paramIndex < type->paramCount; paramIndex++)
                {
                    if(type->params[paramIndex] != func->type->params[paramIndex])
                    {
                        return WA_ERROR_BIND_TYPE;
                    }
                }
                for(u32 returnIndex = 0; returnIndex < type->returnCount; returnIndex++)
                {
                    if(type->returns[returnIndex] != func->type->returns[returnIndex])
                    {
                        return WA_ERROR_BIND_TYPE;
                    }
                }
                break;
            }
            funcIndex++;
        }
    }
    return (WA_OK);
}

wa_status wa_instance_bind_global(wa_instance* instance, oc_str8 name, wa_typed_value value)
{
    wa_module* module = instance->module;
    u32 globalIndex = 0;

    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];

        if(import->kind == WA_IMPORT_GLOBAL)
        {
            if(!oc_str8_cmp(name, import->importName))
            {
                wa_global* globalDesc = &module->globals[globalIndex];

                if(globalDesc->type != value.type)
                {
                    //log error to module
                    return WA_ERROR_BIND_TYPE;
                }
                instance->globals[globalIndex] = value.value;
                break;
            }
            globalIndex++;
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

wa_status wa_instance_link(wa_instance* instance)
{
    wa_module* module = instance->module;

    for(u32 funcIndex = 0; funcIndex < module->functionImportCount; funcIndex++)
    {
        wa_func* func = &instance->functions[funcIndex];
        if(!func->proc)
        {
            oc_log_error("Couldn't link instance: import %.*s not satisfied.\n",
                         oc_str8_ip(func->import->importName));
            return WA_FAIL_MISSING_IMPORT;
        }
    }

    for(u64 globalIndex = 0; globalIndex < module->globalCount; globalIndex++)
    {
        wa_global* global = &module->globals[globalIndex];
        if(global->code)
        {
            i64 t = 0x7f - (i64)global->type + 1;
            wa_func_type* exprType = (wa_func_type*)&WA_BLOCK_VALUE_TYPES[t];

            wa_status status = wa_instance_interpret_expr(instance, exprType, global->code, 0, 0, 1, &instance->globals[globalIndex]);
            if(status != WA_OK)
            {
                return status;
            }
        }
    }

    for(u64 eltIndex = 0; eltIndex < module->elementCount; eltIndex++)
    {
        wa_element* element = &module->elements[eltIndex];
        if(element->mode == WA_ELEMENT_ACTIVE)
        {
            //TODO: check table size?

            wa_table_type* desc = &module->tables[element->tableIndex];
            wa_table* table = &instance->tables[element->tableIndex];

            wa_value offset = { 0 };
            wa_status status = wa_instance_interpret_expr(instance,
                                                          (wa_func_type*)&WA_BLOCK_VALUE_TYPES[1],
                                                          element->tableOffsetCode,
                                                          0, 0,
                                                          1, &offset);
            if(status != WA_OK)
            {
                return status;
            }

            i64 t = 0x7f - (i64)element->type + 1;
            wa_func_type* exprType = (wa_func_type*)&WA_BLOCK_VALUE_TYPES[t];

            for(u32 exprIndex = 0; exprIndex < element->initCount; exprIndex++)
            {
                wa_value* result = &table->contents[offset.valI32 + exprIndex];
                wa_status status = wa_instance_interpret_expr(instance, exprType, element->code[exprIndex], 0, 0, 1, result);
                if(status != WA_OK)
                {
                    return status;
                }
            }
        }
    }

    //TODO honor memory limits
    oc_base_allocator* allocator = oc_base_allocator_default();
    instance->memoryCap = 4 << 20;
    instance->memorySize = 4096;
    instance->memory = oc_base_reserve(allocator, instance->memoryCap);
    oc_base_commit(allocator, instance->memory, instance->memorySize);

    return WA_OK;
}

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
    wa_control controlStack[1024] = {};
    u32 controlStackTop = 0;

    wa_value localsBuffer[1024];
    wa_value* locals = localsBuffer;
    wa_code* pc = code;

    memcpy(locals, args, argCount * sizeof(wa_value));

    char* memory = instance->memory;

    while(1)
    {
        wa_instr_op opcode = pc->opcode;
        pc++;

        switch(opcode)
        {
            case WA_INSTR_unreachable:
            {
                return WA_TRAP_UNREACHABLE;
            }
            break;

            case WA_INSTR_i32_const:
            {
                locals[pc[1].valI32].valI32 = pc[0].valI32;
                pc += 2;
            }
            break;

            case WA_INSTR_i64_const:
            {
                locals[pc[1].valI32].valI64 = pc[0].valI64;
                pc += 2;
            }
            break;

            case WA_INSTR_f32_const:
            {
                locals[pc[1].valI32].valF32 = pc[0].valF32;
                pc += 2;
            }
            break;

            case WA_INSTR_f64_const:
            {
                locals[pc[1].valI32].valF64 = pc[0].valF64;
                pc += 2;
            }
            break;

            case WA_INSTR_move:
            {
                locals[pc[1].valI32].valI64 = locals[pc[0].valI32].valI64;
                pc += 2;
            }
            break;

            case WA_INSTR_global_get:
            {
                memcpy(&locals[pc[1].valI32].valI64, &instance->globals[pc[0].valI32], sizeof(u64));
                pc += 2;
            }
            break;

            case WA_INSTR_global_set:
            {
                memcpy(&instance->globals[pc[1].valI32], &locals[pc[0].valI32].valI64, sizeof(u64));
                pc += 2;
            }
            break;

            case WA_INSTR_select:
            {
                if(locals[pc[2].valI32].valI32)
                {
                    memcpy(&locals[pc[3].valI32], &locals[pc[0].valI32], sizeof(u64));
                }
                else
                {
                    memcpy(&locals[pc[3].valI32], &locals[pc[1].valI32], sizeof(u64));
                }
                pc += 4;
            }
            break;

            case WA_INSTR_i32_load:
            {
                locals[pc[2].valI32].valI32 = *(i32*)&memory[pc[0].memArg.offset + locals[pc[1].valI32].valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i64_load:
            {
                locals[pc[2].valI32].valI64 = *(i64*)&memory[pc[0].memArg.offset + locals[pc[1].valI32].valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_f32_load:
            {
                locals[pc[2].valI32].valF32 = *(f32*)&memory[pc[0].memArg.offset + locals[pc[1].valI32].valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_f64_load:
            {
                locals[pc[2].valI32].valF64 = *(f64*)&memory[pc[0].memArg.offset + locals[pc[1].valI32].valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i32_load8_s:
            {
                locals[pc[2].valI32].valI32 = (i32) * (i8*)&memory[pc[0].memArg.offset + locals[pc[1].valI32].valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i32_load8_u:
            {
                *(u32*)&locals[pc[2].valI32].valI32 = (u32) * (u8*)&memory[pc[0].memArg.offset + locals[pc[1].valI32].valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i32_load16_s:
            {
                locals[pc[2].valI32].valI32 = (i32) * (i16*)&memory[pc[0].memArg.offset + locals[pc[1].valI32].valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i32_load16_u:
            {
                *(u32*)&locals[pc[2].valI32].valI32 = (u32) * (u16*)&memory[pc[0].memArg.offset + locals[pc[1].valI32].valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i64_load8_s:
            {
                locals[pc[2].valI32].valI64 = (i64) * (i8*)&memory[pc[0].memArg.offset + locals[pc[1].valI32].valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i64_load8_u:
            {
                *(u32*)&locals[pc[2].valI32].valI64 = (u64) * (u8*)&memory[pc[0].memArg.offset + locals[pc[1].valI32].valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i64_load16_s:
            {
                locals[pc[2].valI32].valI64 = (i64) * (i16*)&memory[pc[0].memArg.offset + locals[pc[1].valI32].valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i64_load16_u:
            {
                *(u32*)&locals[pc[2].valI32].valI64 = (u64) * (u16*)&memory[pc[0].memArg.offset + locals[pc[1].valI32].valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i64_load32_s:
            {
                locals[pc[2].valI32].valI64 = (i64) * (i32*)&memory[pc[0].memArg.offset + locals[pc[1].valI32].valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i64_load32_u:
            {
                *(u32*)&locals[pc[2].valI32].valI64 = (u64) * (u32*)&memory[pc[0].memArg.offset + locals[pc[1].valI32].valI32];
                pc += 3;
            }
            break;

            case WA_INSTR_i32_store:
            {
                *(i32*)&memory[pc[0].memArg.offset + locals[pc[2].valI32].valI32] = locals[pc[1].valI32].valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_store:
            {
                *(i64*)&memory[pc[0].memArg.offset + locals[pc[2].valI32].valI32] = locals[pc[1].valI32].valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_f32_store:
            {
                *(f32*)&memory[pc[0].memArg.offset + locals[pc[2].valI32].valI32] = locals[pc[1].valI32].valF32;
                pc += 3;
            }
            break;

            case WA_INSTR_f64_store:
            {
                *(f64*)&memory[pc[0].memArg.offset + locals[pc[2].valI32].valI32] = locals[pc[1].valI32].valF64;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_store8:
            {
                *(u8*)&memory[pc[0].memArg.offset + locals[pc[2].valI32].valI32] = *(u8*)&locals[pc[1].valI32].valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_store16:
            {
                *(u16*)&memory[pc[0].memArg.offset + locals[pc[2].valI32].valI32] = *(u16*)&locals[pc[1].valI32].valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_store8:
            {
                *(u8*)&memory[pc[0].memArg.offset + locals[pc[2].valI32].valI32] = *(u8*)&locals[pc[1].valI32].valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_store16:
            {
                *(u16*)&memory[pc[0].memArg.offset + locals[pc[2].valI32].valI32] = *(u16*)&locals[pc[1].valI32].valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_store32:
            {
                *(u32*)&memory[pc[0].memArg.offset + locals[pc[2].valI32].valI32] = *(u32*)&locals[pc[1].valI32].valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_jump:
            {
                pc += pc[0].valI64;
            }
            break;

            case WA_INSTR_jump_if_zero:
            {
                if(locals[pc[1].valI32].valI32 == 0)
                {
                    pc += pc[0].valI64;
                }
                else
                {
                    pc += 2;
                }
            }
            break;

            case WA_INSTR_jump_table:
            {
                u32 count = pc[0].valU32;
                u32 index = locals[pc[1].valI32].valI32;

                if(index >= count)
                {
                    index = count - 1;
                }

                pc += pc[2 + index].valI64;
            }
            break;

            case WA_INSTR_call:
            {
                wa_func* callee = &instance->functions[pc[0].valI64];

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

                    locals += pc[1].valI64;
                    pc = callee->code;
                }
                else
                {
                    wa_value* saveLocals = locals;
                    locals += pc[1].valI64;
                    callee->proc(locals, locals);
                    pc += 2;
                    locals = saveLocals;
                }
            }
            break;

            case WA_INSTR_call_indirect:
            {
                u32 typeIndex = *(u32*)&pc[0].valI32;
                u32 tableIndex = *(u32*)&pc[1].valI32;
                i64 maxUsedSlot = pc[2].valI64;
                u32 index = *(u32*)&(locals[pc[3].valI32].valI32);

                wa_table* table = &instance->tables[tableIndex];
                u32 funcIndex = *(u32*)&table->contents[index].valI32;

                wa_func* callee = &instance->functions[funcIndex];

                //TODO check type

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
                else
                {
                    wa_value* saveLocals = locals;
                    locals += maxUsedSlot;
                    callee->proc(locals, locals);
                    pc += 4;
                    locals = saveLocals;
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
                locals[pc[1].valI32].valI64 = 0; //???
                pc += 2;
            }
            break;

            case WA_INSTR_ref_func:
            {
                locals[pc[1].valI32].valI64 = pc[0].valI64; //???
                pc += 2;
            }
            break;

            case WA_INSTR_i32_add:
            {
                locals[pc[2].valI32].valI32 = locals[pc[1].valI32].valI32 + locals[pc[0].valI32].valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_sub:
            {
                locals[pc[2].valI32].valI32 = locals[pc[1].valI32].valI32 - locals[pc[0].valI32].valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_mul:
            {
                locals[pc[2].valI32].valI32 = locals[pc[1].valI32].valI32 * locals[pc[0].valI32].valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_div_s:
            {
                if(locals[pc[0].valI32].valI32 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else if(locals[pc[0].valI32].valI32 == -1 && locals[pc[1].valI32].valI32 == INT32_MIN)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }
                else
                {
                    locals[pc[2].valI32].valI32 = locals[pc[1].valI32].valI32 / locals[pc[0].valI32].valI32;
                }
                pc += 3;
            }
            break;

            case WA_INSTR_i32_div_u:
            {
                if(locals[pc[0].valI32].valI32 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else
                {
                    *(u32*)&locals[pc[2].valI32].valI32 = *(u32*)&locals[pc[1].valI32].valI32 / *(u32*)&locals[pc[0].valI32].valI32;
                }
                pc += 3;
            }
            break;

            case WA_INSTR_i32_rem_s:
            {
                if(locals[pc[0].valI32].valI32 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else if(locals[pc[0].valI32].valI32 == -1 && locals[pc[1].valI32].valI32 == INT32_MIN)
                {
                    locals[pc[2].valI32].valI32 = 0;
                }
                else
                {
                    locals[pc[2].valI32].valI32 = locals[pc[1].valI32].valI32 % locals[pc[0].valI32].valI32;
                }
                pc += 3;
            }
            break;

            case WA_INSTR_i32_rem_u:
            {
                if(locals[pc[0].valI32].valI32 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else
                {
                    *(u32*)&locals[pc[2].valI32].valI32 = *(u32*)&locals[pc[1].valI32].valI32 % *(u32*)&locals[pc[0].valI32].valI32;
                }
                pc += 3;
            }
            break;

            case WA_INSTR_i32_and:
            {
                locals[pc[2].valI32].valI32 = locals[pc[1].valI32].valI32 & locals[pc[0].valI32].valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_or:
            {
                locals[pc[2].valI32].valI32 = locals[pc[1].valI32].valI32 | locals[pc[0].valI32].valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_xor:
            {
                locals[pc[2].valI32].valI32 = locals[pc[1].valI32].valI32 ^ locals[pc[0].valI32].valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_shl:
            {
                locals[pc[2].valI32].valI32 = locals[pc[1].valI32].valI32 << locals[pc[0].valI32].valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_shr_s:
            {
                locals[pc[2].valI32].valI32 = locals[pc[1].valI32].valI32 >> locals[pc[0].valI32].valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_shr_u:
            {
                *(u32*)&locals[pc[2].valI32].valI32 = *(u32*)&locals[pc[1].valI32].valI32 >> *(u32*)&locals[pc[0].valI32].valI32;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_rotr:
            {
                u32 n = *(u32*)&locals[pc[1].valI32].valI32;
                u32 r = *(u32*)&locals[pc[0].valI32].valI32;
                *(u32*)&locals[pc[2].valI32].valI32 = (n >> r) | (n << (32 - r));
                pc += 3;
            }
            break;

            case WA_INSTR_i32_rotl:
            {
                u32 n = *(u32*)&locals[pc[1].valI32].valI32;
                u32 r = *(u32*)&locals[pc[0].valI32].valI32;
                *(u32*)&locals[pc[2].valI32].valI32 = (n << r) | (n >> (32 - r));
                pc += 3;
            }
            break;

            case WA_INSTR_i32_clz:
            {
                if(locals[pc[0].valI32].valI32 == 0)
                {
                    locals[pc[1].valI32].valI32 = 32;
                }
                else
                {
                    locals[pc[1].valI32].valI32 = __builtin_clz(*(u32*)&locals[pc[0].valI32].valI32);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i32_ctz:
            {
                if(locals[pc[0].valI32].valI32 == 0)
                {
                    locals[pc[1].valI32].valI32 = 32;
                }
                else
                {
                    locals[pc[1].valI32].valI32 = __builtin_ctz(*(u32*)&locals[pc[0].valI32].valI32);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i32_popcnt:
            {
                locals[pc[1].valI32].valI32 = __builtin_popcount(*(u32*)&locals[pc[0].valI32].valI32);
                pc += 2;
            }
            break;

            case WA_INSTR_i32_extend8_s:
            {
                locals[pc[1].valI32].valI32 = (i32)(i8)(locals[pc[0].valI32].valI32 & 0xff);
                pc += 2;
            }
            break;

            case WA_INSTR_i32_extend16_s:
            {
                locals[pc[1].valI32].valI32 = (i32)(i16)(locals[pc[0].valI32].valI32 & 0xffff);
                pc += 2;
            }
            break;

            case WA_INSTR_i32_eqz:
            {
                locals[pc[1].valI32].valI32 = (locals[pc[0].valI32].valI32 == 0) ? 1 : 0;
                pc += 2;
            }
            break;

            case WA_INSTR_i32_eq:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valI32 == locals[pc[0].valI32].valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_ne:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valI32 != locals[pc[0].valI32].valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_lt_s:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valI32 < locals[pc[0].valI32].valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_lt_u:
            {
                locals[pc[2].valI32].valI32 = (*(u32*)&locals[pc[1].valI32].valI32 < *(u32*)&locals[pc[0].valI32].valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_le_s:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valI32 <= locals[pc[0].valI32].valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_le_u:
            {
                locals[pc[2].valI32].valI32 = (*(u32*)&locals[pc[1].valI32].valI32 <= *(u32*)&locals[pc[0].valI32].valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_gt_s:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valI32 > locals[pc[0].valI32].valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_gt_u:
            {
                locals[pc[2].valI32].valI32 = (*(u32*)&locals[pc[1].valI32].valI32 > *(u32*)&locals[pc[0].valI32].valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_ge_s:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valI32 >= locals[pc[0].valI32].valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i32_ge_u:
            {
                locals[pc[2].valI32].valI32 = (*(u32*)&locals[pc[1].valI32].valI32 >= *(u32*)&locals[pc[0].valI32].valI32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_add:
            {
                locals[pc[2].valI32].valI64 = locals[pc[1].valI32].valI64 + locals[pc[0].valI32].valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_sub:
            {
                locals[pc[2].valI32].valI64 = locals[pc[1].valI32].valI64 - locals[pc[0].valI32].valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_mul:
            {
                locals[pc[2].valI32].valI64 = locals[pc[1].valI32].valI64 * locals[pc[0].valI32].valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_div_s:
            {
                if(locals[pc[0].valI32].valI64 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else if(locals[pc[0].valI32].valI64 == -1 && locals[pc[1].valI32].valI64 == INT64_MIN)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }
                else
                {
                    locals[pc[2].valI32].valI64 = locals[pc[1].valI32].valI64 / locals[pc[0].valI32].valI64;
                }
                pc += 3;
            }
            break;

            case WA_INSTR_i64_div_u:
            {
                if(locals[pc[0].valI32].valI64 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else
                {
                    *(u64*)&locals[pc[2].valI32].valI64 = *(u64*)&locals[pc[1].valI32].valI64 / *(u64*)&locals[pc[0].valI32].valI64;
                }
                pc += 3;
            }
            break;

            case WA_INSTR_i64_rem_s:
            {
                if(locals[pc[0].valI32].valI64 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else if(locals[pc[0].valI32].valI64 == -1 && locals[pc[1].valI32].valI64 == INT64_MIN)
                {
                    locals[pc[2].valI32].valI64 = 0;
                }
                else
                {
                    locals[pc[2].valI32].valI64 = locals[pc[1].valI32].valI64 % locals[pc[0].valI32].valI64;
                }
                pc += 3;
            }
            break;

            case WA_INSTR_i64_rem_u:
            {
                if(locals[pc[0].valI32].valI64 == 0)
                {
                    return WA_TRAP_DIVIDE_BY_ZERO;
                }
                else
                {
                    *(u64*)&locals[pc[2].valI32].valI64 = *(u64*)&locals[pc[1].valI32].valI64 % *(u64*)&locals[pc[0].valI32].valI64;
                }
                pc += 3;
            }
            break;

            case WA_INSTR_i64_and:
            {
                locals[pc[2].valI32].valI64 = locals[pc[1].valI32].valI64 & locals[pc[0].valI32].valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_or:
            {
                locals[pc[2].valI32].valI64 = locals[pc[1].valI32].valI64 | locals[pc[0].valI32].valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_xor:
            {
                locals[pc[2].valI32].valI64 = locals[pc[1].valI32].valI64 ^ locals[pc[0].valI32].valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_shl:
            {
                locals[pc[2].valI32].valI64 = locals[pc[1].valI32].valI64 << locals[pc[0].valI32].valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_shr_s:
            {
                locals[pc[2].valI32].valI64 = locals[pc[1].valI32].valI64 >> locals[pc[0].valI32].valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_shr_u:
            {
                *(u64*)&locals[pc[2].valI32].valI64 = *(u64*)&locals[pc[1].valI32].valI64 >> *(u64*)&locals[pc[0].valI32].valI64;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_rotr:
            {
                u64 n = *(u64*)&locals[pc[1].valI32].valI64;
                u64 r = *(u64*)&locals[pc[0].valI32].valI64;
                *(u64*)&locals[pc[2].valI32].valI64 = (n >> r) | (n << (64 - r));
                pc += 3;
            }
            break;

            case WA_INSTR_i64_rotl:
            {
                u64 n = *(u64*)&locals[pc[1].valI32].valI64;
                u64 r = *(u64*)&locals[pc[0].valI32].valI64;
                *(u64*)&locals[pc[2].valI32].valI64 = (n << r) | (n >> (64 - r));
                pc += 3;
            }
            break;

            case WA_INSTR_i64_clz:
            {
                if(locals[pc[0].valI32].valI64 == 0)
                {
                    locals[pc[1].valI32].valI64 = 64;
                }
                else
                {
                    locals[pc[1].valI32].valI64 = __builtin_clzl(*(u64*)&locals[pc[0].valI32].valI64);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i64_ctz:
            {
                if(locals[pc[0].valI32].valI64 == 0)
                {
                    locals[pc[1].valI32].valI64 = 64;
                }
                else
                {
                    locals[pc[1].valI32].valI64 = __builtin_ctzl(*(u64*)&locals[pc[0].valI32].valI64);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i64_popcnt:
            {
                locals[pc[1].valI32].valI64 = __builtin_popcountl(*(u64*)&locals[pc[0].valI32].valI64);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_extend8_s:
            {
                locals[pc[1].valI32].valI64 = (i64)(i8)(locals[pc[0].valI32].valI64 & 0xff);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_extend16_s:
            {
                locals[pc[1].valI32].valI64 = (i64)(i16)(locals[pc[0].valI32].valI64 & 0xffff);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_extend32_s:
            {
                locals[pc[1].valI32].valI64 = (i64)(i32)(locals[pc[0].valI32].valI64 & 0xffffffff);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_eqz:
            {
                locals[pc[1].valI32].valI32 = (locals[pc[0].valI32].valI64 == 0) ? 1 : 0;
                pc += 2;
            }
            break;

            case WA_INSTR_i64_eq:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valI64 == locals[pc[0].valI32].valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_ne:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valI64 != locals[pc[0].valI32].valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_lt_s:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valI64 < locals[pc[0].valI32].valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_lt_u:
            {
                locals[pc[2].valI32].valI32 = (*(u64*)&locals[pc[1].valI32].valI64 < *(u64*)&locals[pc[0].valI32].valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_le_s:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valI64 <= locals[pc[0].valI32].valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_le_u:
            {
                locals[pc[2].valI32].valI32 = (*(u64*)&locals[pc[1].valI32].valI64 <= *(u64*)&locals[pc[0].valI32].valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_gt_s:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valI64 > locals[pc[0].valI32].valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_gt_u:
            {
                locals[pc[2].valI32].valI32 = (*(u64*)&locals[pc[1].valI32].valI64 > *(u64*)&locals[pc[0].valI32].valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_ge_s:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valI64 >= locals[pc[0].valI32].valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_i64_ge_u:
            {
                locals[pc[2].valI32].valI32 = (*(u64*)&locals[pc[1].valI32].valI64 >= *(u64*)&locals[pc[0].valI32].valI64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_f32_eq:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valF32 == locals[pc[0].valI32].valF32) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f32_ne:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valF32 != locals[pc[0].valI32].valF32) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f32_lt:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valF32 < locals[pc[0].valI32].valF32) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f32_gt:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valF32 > locals[pc[0].valI32].valF32) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f32_le:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valF32 <= locals[pc[0].valI32].valF32) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f32_ge:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valF32 >= locals[pc[0].valI32].valF32) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_f64_eq:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valF64 == locals[pc[0].valI32].valF64) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f64_ne:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valF64 != locals[pc[0].valI32].valF64) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f64_lt:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valF64 < locals[pc[0].valI32].valF64) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f64_gt:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valF64 > locals[pc[0].valI32].valF64) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f64_le:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valF64 <= locals[pc[0].valI32].valF64) ? 1 : 0;
                pc += 3;
            }
            break;
            case WA_INSTR_f64_ge:
            {
                locals[pc[2].valI32].valI32 = (locals[pc[1].valI32].valF64 >= locals[pc[0].valI32].valF64) ? 1 : 0;
                pc += 3;
            }
            break;

            case WA_INSTR_f32_abs:
            {
                locals[pc[1].valI32].valF32 = fabsf(locals[pc[0].valI32].valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_f32_neg:
            {
                locals[pc[1].valI32].valF32 = -locals[pc[0].valI32].valF32;
                pc += 2;
            }
            break;

            case WA_INSTR_f32_ceil:
            {
                locals[pc[1].valI32].valF32 = ceilf(locals[pc[0].valI32].valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_f32_floor:
            {
                locals[pc[1].valI32].valF32 = floorf(locals[pc[0].valI32].valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_f32_trunc:
            {
                locals[pc[1].valI32].valF32 = truncf(locals[pc[0].valI32].valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_f32_nearest:
            {
                locals[pc[1].valI32].valF32 = rintf(locals[pc[0].valI32].valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_f32_sqrt:
            {
                locals[pc[1].valI32].valF32 = sqrtf(locals[pc[0].valI32].valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_f32_add:
            {
                locals[pc[2].valI32].valF32 = locals[pc[1].valI32].valF32 + locals[pc[0].valI32].valF32;
                pc += 3;
            }
            break;

            case WA_INSTR_f32_sub:
            {
                locals[pc[2].valI32].valF32 = locals[pc[1].valI32].valF32 - locals[pc[0].valI32].valF32;
                pc += 3;
            }
            break;

            case WA_INSTR_f32_mul:
            {
                locals[pc[2].valI32].valF32 = locals[pc[1].valI32].valF32 * locals[pc[0].valI32].valF32;
                pc += 3;
            }
            break;

            case WA_INSTR_f32_div:
            {
                locals[pc[2].valI32].valF32 = locals[pc[1].valI32].valF32 / locals[pc[0].valI32].valF32;
                pc += 3;
            }
            break;

            case WA_INSTR_f32_min:
            {
                if(isnan(locals[pc[1].valI32].valF32) || isnan(locals[pc[0].valI32].valF32))
                {
                    u32 u = 0x7fc00000;
                    memcpy(&locals[pc[2].valI32].valF32, &u, sizeof(f32));
                }
                else
                {
                    locals[pc[2].valI32].valF32 = oc_min(locals[pc[1].valI32].valF32, locals[pc[0].valI32].valF32);
                }
                pc += 3;
            }
            break;

            case WA_INSTR_f32_max:
            {
                if(isnan(locals[pc[1].valI32].valF32) || isnan(locals[pc[0].valI32].valF32))
                {
                    u32 u = 0x7fc00000;
                    memcpy(&locals[pc[2].valI32].valF32, &u, sizeof(f32));
                }
                else
                {
                    locals[pc[2].valI32].valF32 = oc_max(locals[pc[1].valI32].valF32, locals[pc[0].valI32].valF32);
                }
                pc += 3;
            }
            break;

            case WA_INSTR_f32_copysign:
            {
                locals[pc[2].valI32].valF32 = copysignf(locals[pc[1].valI32].valF32, locals[pc[0].valI32].valF32);
                pc += 3;
            }
            break;

            case WA_INSTR_f64_abs:
            {
                locals[pc[1].valI32].valF64 = fabs(locals[pc[0].valI32].valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_f64_neg:
            {
                locals[pc[1].valI32].valF64 = -locals[pc[0].valI32].valF64;
                pc += 2;
            }
            break;

            case WA_INSTR_f64_ceil:
            {
                locals[pc[1].valI32].valF64 = ceil(locals[pc[0].valI32].valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_f64_floor:
            {
                locals[pc[1].valI32].valF64 = floor(locals[pc[0].valI32].valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_f64_trunc:
            {
                locals[pc[1].valI32].valF64 = trunc(locals[pc[0].valI32].valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_f64_nearest:
            {
                locals[pc[1].valI32].valF64 = rint(locals[pc[0].valI32].valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_f64_sqrt:
            {
                locals[pc[1].valI32].valF64 = sqrt(locals[pc[0].valI32].valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_f64_add:
            {
                locals[pc[2].valI32].valF64 = locals[pc[1].valI32].valF64 + locals[pc[0].valI32].valF64;
                pc += 3;
            }
            break;

            case WA_INSTR_f64_sub:
            {
                locals[pc[2].valI32].valF64 = locals[pc[1].valI32].valF64 - locals[pc[0].valI32].valF64;
                pc += 3;
            }
            break;

            case WA_INSTR_f64_mul:
            {
                locals[pc[2].valI32].valF64 = locals[pc[1].valI32].valF64 * locals[pc[0].valI32].valF64;
                pc += 3;
            }
            break;

            case WA_INSTR_f64_div:
            {
                locals[pc[2].valI32].valF64 = locals[pc[1].valI32].valF64 / locals[pc[0].valI32].valF64;
                pc += 3;
            }
            break;

            case WA_INSTR_f64_min:
            {
                if(isnan(locals[pc[1].valI32].valF64) || isnan(locals[pc[0].valI32].valF64))
                {
                    u64 u = 0x7ff8000000000000;
                    memcpy(&locals[pc[2].valI32].valF64, &u, sizeof(f64));
                }
                else
                {
                    locals[pc[2].valI32].valF64 = oc_min(locals[pc[1].valI32].valF64, locals[pc[0].valI32].valF64);
                }
                pc += 3;
            }
            break;

            case WA_INSTR_f64_max:
            {
                if(isnan(locals[pc[1].valI32].valF64) || isnan(locals[pc[0].valI32].valF64))
                {
                    u64 u = 0x7ff8000000000000;
                    memcpy(&locals[pc[2].valI32].valF64, &u, sizeof(f64));
                }
                else
                {
                    locals[pc[2].valI32].valF64 = oc_max(locals[pc[1].valI32].valF64, locals[pc[0].valI32].valF64);
                }
                pc += 3;
            }
            break;

            case WA_INSTR_f64_copysign:
            {
                locals[pc[2].valI32].valF64 = copysign(locals[pc[1].valI32].valF64, locals[pc[0].valI32].valF64);
                pc += 3;
            }
            break;

            case WA_INSTR_i32_wrap_i64:
            {
                locals[pc[1].valI32].valI32 = (locals[pc[0].valI32].valI64 & 0x00000000ffffffff);
                pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_f32_s:
            {
                if(isnan(locals[pc[0].valI32].valF32))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }
                else if(locals[pc[0].valI32].valF32 >= 2147483648.0f || locals[pc[0].valI32].valF32 < -2147483648.0f)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                locals[pc[1].valI32].valI32 = (i32)truncf(locals[pc[0].valI32].valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_f32_u:
            {
                if(isnan(locals[pc[0].valI32].valF32))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(locals[pc[0].valI32].valF32 >= 4294967296.0f || locals[pc[0].valI32].valF32 <= -1.0f)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                *(u32*)&locals[pc[1].valI32].valI32 = (u32)truncf(locals[pc[0].valI32].valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_f64_s:
            {
                if(isnan(locals[pc[0].valI32].valF64))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(locals[pc[0].valI32].valF64 >= 2147483648.0 || locals[pc[0].valI32].valF64 <= -2147483649.0)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                locals[pc[1].valI32].valI32 = (i32)trunc(locals[pc[0].valI32].valF64);
                pc += 2;
            }
            break;
            case WA_INSTR_i32_trunc_f64_u:
            {
                if(isnan(locals[pc[0].valI32].valF64))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(locals[pc[0].valI32].valF64 >= 4294967296.0 || locals[pc[0].valI32].valF64 <= -1.0)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                *(u32*)&locals[pc[1].valI32].valI32 = (u32)trunc(locals[pc[0].valI32].valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_f32_s:
            {
                if(isnan(locals[pc[0].valI32].valF32))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(locals[pc[0].valI32].valF32 >= 9223372036854775808.0f || locals[pc[0].valI32].valF32 < -9223372036854775808.0f)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                locals[pc[1].valI32].valI64 = (i64)truncf(locals[pc[0].valI32].valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_f32_u:
            {
                if(isnan(locals[pc[0].valI32].valF32))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(locals[pc[0].valI32].valF32 >= 18446744073709551616.0f || locals[pc[0].valI32].valF32 <= -1.0f)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                *(u64*)&locals[pc[1].valI32].valI64 = (u64)truncf(locals[pc[0].valI32].valF32);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_f64_s:
            {
                if(isnan(locals[pc[0].valI32].valF64))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(locals[pc[0].valI32].valF64 >= 9223372036854775808.0 || locals[pc[0].valI32].valF64 < -9223372036854775808.0)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                locals[pc[1].valI32].valI64 = (i64)trunc(locals[pc[0].valI32].valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_f64_u:
            {
                if(isnan(locals[pc[0].valI32].valF64))
                {
                    return WA_TRAP_INVALID_INTEGER_CONVERSION;
                }

                if(locals[pc[0].valI32].valF64 >= 18446744073709551616.0 || locals[pc[0].valI32].valF64 <= -1.0)
                {
                    return WA_TRAP_INTEGER_OVERFLOW;
                }

                *(u64*)&locals[pc[1].valI32].valI64 = (u64)trunc(locals[pc[0].valI32].valF64);
                pc += 2;
            }
            break;

            case WA_INSTR_f32_convert_i32_s:
            {
                locals[pc[1].valI32].valF32 = (f32)locals[pc[0].valI32].valI32;
                pc += 2;
            }
            break;

            case WA_INSTR_f32_convert_i32_u:
            {
                locals[pc[1].valI32].valF32 = (f32) * (u32*)&locals[pc[0].valI32].valI32;
                pc += 2;
            }
            break;

            case WA_INSTR_f32_convert_i64_s:
            {
                locals[pc[1].valI32].valF32 = (f32)locals[pc[0].valI32].valI64;
                pc += 2;
            }
            break;

            case WA_INSTR_f32_convert_i64_u:
            {
                locals[pc[1].valI32].valF32 = (f32) * (u64*)&locals[pc[0].valI32].valI64;
                pc += 2;
            }
            break;

            case WA_INSTR_f32_demote_f64:
            {
                locals[pc[1].valI32].valF32 = (f32)locals[pc[0].valI32].valF64;
                pc += 2;
            }
            break;

            case WA_INSTR_f64_convert_i32_s:
            {
                locals[pc[1].valI32].valF64 = (f64)locals[pc[0].valI32].valI32;
                pc += 2;
            }
            break;
            case WA_INSTR_f64_convert_i32_u:
            {
                locals[pc[1].valI32].valF64 = (f64) * (u32*)&locals[pc[0].valI32].valI32;
                pc += 2;
            }
            break;
            case WA_INSTR_f64_convert_i64_s:
            {
                locals[pc[1].valI32].valF64 = (f64)locals[pc[0].valI32].valI64;
                pc += 2;
            }
            break;
            case WA_INSTR_f64_convert_i64_u:
            {
                locals[pc[1].valI32].valF64 = (f64) * (u64*)&locals[pc[0].valI32].valI64;
                pc += 2;
            }
            break;

            case WA_INSTR_f64_promote_f32:
            {
                locals[pc[1].valI32].valF64 = (f64)locals[pc[0].valI32].valF32;
                pc += 2;
            }
            break;

            case WA_INSTR_i32_reinterpret_f32:
            {
                locals[pc[1].valI32].valI32 = *(i32*)&locals[pc[0].valI32].valF32;
                pc += 2;
            }
            break;
            case WA_INSTR_i64_reinterpret_f64:
            {
                locals[pc[1].valI32].valI64 = *(i64*)&locals[pc[0].valI32].valF64;
                pc += 2;
            }
            break;
            case WA_INSTR_f32_reinterpret_i32:
            {
                locals[pc[1].valI32].valF32 = *(f32*)&locals[pc[0].valI32].valI32;
                pc += 2;
            }
            break;
            case WA_INSTR_f64_reinterpret_i64:
            {
                locals[pc[1].valI32].valF64 = *(f64*)&locals[pc[0].valI32].valI64;
                pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_sat_f32_s:
            {
                if(isnan(locals[pc[0].valI32].valF32))
                {
                    locals[pc[1].valI32].valI32 = 0;
                }
                else if(locals[pc[0].valI32].valF32 >= 2147483648.0f)
                {
                    locals[pc[1].valI32].valI32 = INT32_MAX;
                }
                else if(locals[pc[0].valI32].valF32 < -2147483648.0f)
                {
                    locals[pc[1].valI32].valI32 = INT32_MIN;
                }
                else
                {
                    locals[pc[1].valI32].valI32 = (i32)truncf(locals[pc[0].valI32].valF32);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_sat_f32_u:
            {
                if(isnan(locals[pc[0].valI32].valF32))
                {
                    locals[pc[1].valI32].valI32 = 0;
                }
                else if(locals[pc[0].valI32].valF32 >= 4294967296.0f)
                {
                    *(u32*)&locals[pc[1].valI32].valI32 = 0xffffffff;
                }
                else if(locals[pc[0].valI32].valF32 <= -1.0f)
                {
                    locals[pc[1].valI32].valI32 = 0;
                }
                else
                {
                    *(u32*)&locals[pc[1].valI32].valI32 = (u32)truncf(locals[pc[0].valI32].valF32);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_sat_f64_s:
            {
                if(isnan(locals[pc[0].valI32].valF64))
                {
                    locals[pc[1].valI32].valI32 = 0;
                }
                else if(locals[pc[0].valI32].valF64 >= 2147483648.0)
                {
                    locals[pc[1].valI32].valI32 = INT32_MAX;
                }
                else if(locals[pc[0].valI32].valF64 <= -2147483649.0)
                {
                    locals[pc[1].valI32].valI32 = INT32_MIN;
                }
                else
                {
                    locals[pc[1].valI32].valI32 = (i32)trunc(locals[pc[0].valI32].valF64);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_sat_f64_u:
            {
                if(isnan(locals[pc[0].valI32].valF64))
                {
                    locals[pc[1].valI32].valI32 = 0;
                }
                else if(locals[pc[0].valI32].valF64 >= 4294967296.0)
                {
                    *(u32*)&locals[pc[1].valI32].valI32 = 0xffffffff;
                }
                else if(locals[pc[0].valI32].valF64 <= -1.0)
                {
                    locals[pc[1].valI32].valI32 = 0;
                }
                else
                {
                    *(u32*)&locals[pc[1].valI32].valI32 = (u32)trunc(locals[pc[0].valI32].valF64);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_sat_f32_s:
            {
                if(isnan(locals[pc[0].valI32].valF32))
                {
                    locals[pc[1].valI32].valI64 = 0;
                }
                else if(locals[pc[0].valI32].valF32 >= 9223372036854775808.0f)
                {
                    locals[pc[1].valI32].valI64 = INT64_MAX;
                }
                else if(locals[pc[0].valI32].valF32 < -9223372036854775808.0f)
                {
                    locals[pc[1].valI32].valI64 = INT64_MIN;
                }
                else
                {
                    locals[pc[1].valI32].valI64 = (i64)truncf(locals[pc[0].valI32].valF32);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_sat_f32_u:
            {
                if(isnan(locals[pc[0].valI32].valF32))
                {
                    locals[pc[1].valI32].valI64 = 0;
                }
                else if(locals[pc[0].valI32].valF32 >= 18446744073709551616.0f)
                {
                    *(u64*)&locals[pc[1].valI32].valI64 = 0xffffffffffffffffLLU;
                }
                else if(locals[pc[0].valI32].valF32 <= -1.0f)
                {
                    locals[pc[1].valI32].valI64 = 0;
                }
                else
                {
                    *(u64*)&locals[pc[1].valI32].valI64 = (u64)truncf(locals[pc[0].valI32].valF32);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_sat_f64_s:
            {
                if(isnan(locals[pc[0].valI32].valF64))
                {
                    locals[pc[1].valI32].valI64 = 0;
                }
                else if(locals[pc[0].valI32].valF64 >= 9223372036854775808.0)
                {
                    locals[pc[1].valI32].valI64 = INT64_MAX;
                }
                else if(locals[pc[0].valI32].valF64 < -9223372036854775808.0)
                {
                    locals[pc[1].valI32].valI64 = INT64_MIN;
                }
                else
                {
                    locals[pc[1].valI32].valI64 = (i64)trunc(locals[pc[0].valI32].valF64);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_sat_f64_u:
            {
                if(isnan(locals[pc[0].valI32].valF64))
                {
                    locals[pc[1].valI32].valI64 = 0;
                }
                else if(locals[pc[0].valI32].valF64 >= 18446744073709551616.0)
                {
                    *(u64*)&locals[pc[1].valI32].valI64 = 0xffffffffffffffffLLU;
                }
                else if(locals[pc[0].valI32].valF64 <= -1.0)
                {
                    locals[pc[1].valI32].valI64 = 0;
                }
                else
                {
                    *(u64*)&locals[pc[1].valI32].valI64 = (u64)trunc(locals[pc[0].valI32].valF64);
                }
                pc += 2;
            }
            break;

            case WA_INSTR_i64_extend_i32_s:
            {
                locals[pc[1].valI32].valI64 = (i64)(i32)(locals[pc[0].valI32].valI32);
                pc += 2;
            }
            break;

            case WA_INSTR_i64_extend_i32_u:
            {
                locals[pc[1].valI32].valI64 = *(u32*)&(locals[pc[0].valI32].valI32);
                pc += 2;
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
    return (wa_instance_interpret_expr(instance, func->type, func->code, argCount, args, retCount, returns));
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
            value = parse_value_64(argVal->string);
            OC_ASSERT(value.type == WA_TYPE_I64);
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
            value = parse_value_64(argVal->string);
            OC_ASSERT(value.type == WA_TYPE_I64);
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

} wa_test_env;

typedef struct wa_test_result
{
    wa_status status;
    u32 count;
    wa_value* values;

} wa_test_result;

wa_test_result wa_test_invoke(wa_test_env* env, wa_instance* instance, json_node* action)
{
    json_node* funcName = json_find_assert(action, "field", JSON_STRING);
    json_node* args = json_find_assert(action, "args", JSON_LIST);

    wa_func* func = wa_instance_find_function(instance, funcName->string);

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

    res.status = wa_instance_invoke(instance, func, argCount, argVals, res.count, res.values);

    return res;
}

typedef enum wa_test_status
{
    WA_TEST_FAIL,
    WA_TEST_SKIP,
    WA_TEST_PASS,
} wa_test_status;

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

    if(env->verbose || status == WA_TEST_FAIL)
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

    env->fileName = testName;

    json_node* json = json_parse_str8(env->arena, contents);
    json_node* commands = json_find_assert(json, "commands", JSON_LIST);

    oc_list_for(commands->children, command, json_node, listElt)
    {
        env->command = command;

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
            oc_list_push_back(&env->instances, &testInstance->listElt);

            json_node* filename = json_find_assert(command, "filename", JSON_STRING);

            oc_str8_list list = { 0 };
            oc_str8_list_push(env->arena, &list, testDir);
            oc_str8_list_push(env->arena, &list, filename->string);

            oc_str8 filePath = oc_path_join(env->arena, list);

            wa_module* module = wa_test_module_load(env->arena, filePath);
            OC_ASSERT(module);

            if(!oc_list_empty(module->errors))
            {
                wa_module_print_errors(module);
                wa_module_print_errors(module);

                wa_test_fail(env, testName, command);
                continue;
            }
            else
            {
                testInstance->instance = wa_instance_create(env->arena, module);

                wa_instance_bind_global(testInstance->instance, OC_STR8("global_i32"), (wa_typed_value){ .type = WA_TYPE_I32, .value.valI32 = 666 });
                wa_instance_bind_global(testInstance->instance, OC_STR8("global_i64"), (wa_typed_value){ .type = WA_TYPE_I64, .value.valI64 = 666 });

                if(wa_instance_link(testInstance->instance) != WA_OK)
                {
                    testInstance->instance = 0;
                    wa_test_fail(env, testName, command);
                    continue;
                }

                wa_test_pass(env, testName, command);
            }
        }
        else
        {
            if(filterLine >= 0)
            {
                json_node* line = json_find(command, OC_STR8("line"));
                if(!line || line->numI64 != filterLine)
                {
                    continue;
                }
            }

            if(!oc_str8_cmp(type->string, OC_STR8("assert_return")))
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

                bool check = (result.status == WA_OK)
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

                wa_status expected = WA_OK;
                if(!oc_str8_cmp(failure->string, OC_STR8("integer divide by zero")))
                {
                    expected = WA_TRAP_DIVIDE_BY_ZERO;
                }
                else if(!oc_str8_cmp(failure->string, OC_STR8("integer overflow")))
                {
                    expected = WA_TRAP_INTEGER_OVERFLOW;
                }
                else if(!oc_str8_cmp(failure->string, OC_STR8("invalid conversion to integer")))
                {
                    expected = WA_TRAP_INVALID_INTEGER_CONVERSION;
                }
                else
                {
                    wa_test_skip(env, testName, command);
                    continue;
                }

                wa_test_result result = wa_test_action(env, instance, action);

                if(result.status == WA_OK || result.status != expected)
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
            else
            {
                wa_test_skip(env, testName, command);
            }
        }
    }

    env->totalPassed += env->passed;
    env->totalSkipped += env->skipped;
    env->totalFailed += env->failed;

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
