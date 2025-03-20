#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "warm.h"

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

wa_func_type wa_func_get_type(oc_arena* arena, wa_instance* instance, wa_func* func)
{
    wa_func_type* type = func->type;

    wa_func_type res = (wa_func_type){
        .params = oc_arena_push_array(arena, wa_value_type, type->paramCount),
        .returns = oc_arena_push_array(arena, wa_value_type, type->returnCount),
        .paramCount = type->paramCount,
        .returnCount = type->returnCount,
    };
    memcpy(res.params, type->params, type->paramCount * sizeof(wa_value_type));
    memcpy(res.returns, type->returns, type->returnCount * sizeof(wa_value_type));

    return (res);
}

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

typedef struct wa_module
{
    oc_arena* arena;
    oc_list errors;
    wa_ast* root;

    struct
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
    } toc;

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

    //Debug info
    //TODO reorganize

    u32 warmToWasmMapLen;
    oc_list* warmToWasmMap;

    u32 wasmToWarmMapLen;
    oc_list* wasmToWarmMap;

    dw_info* debugInfo;

    wa_source_info sourceInfo;

    u64 wasmToLineCount;
    wa_wasm_to_line_entry* wasmToLine;

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

#include "wasm_tables.c"

//-------------------------------------------------------------------------
// errors
//-------------------------------------------------------------------------

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

bool wa_module_has_errors(wa_module* module)
{
    return (!oc_list_empty(module->errors));
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

wa_ast* wa_read_leb128(wa_parser* parser, wa_ast* parent, oc_str8 label, u32 bitWidth, bool isSigned)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_U64);
    ast->label = label;

    char byte = 0;
    u64 shift = 0;
    u64 res = 0;
    u32 count = 0;
    u32 maxCount = (u32)ceil(bitWidth / 7.);

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

        if(count >= maxCount)
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: leb128 u64 representation too long.\n",
                           oc_str8_ip(label));
            res = 0;
            break;
        }

        byte = parser->contents[parser->offset];
        parser->offset++;

        res |= ((u64)byte & 0x7f) << shift;
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
                wa_parse_error(parser,
                               ast,
                               "Couldn't read %.*s: leb128 overflow.\n",
                               oc_str8_ip(label));
                res = 0;
            }
        }

        if(shift < 64 && (byte & 0x40))
        {
            res |= (~0ULL << shift);
        }
    }
    else
    {
        if(count == maxCount)
        {
            //NOTE: for signed the spec mandates that unused bits must be zero,
            // so we construct a mask to select only unused bits,
            // and we check that they're all 0
            u8 bitsInLastByte = (bitWidth - (maxCount - 1) * 7);
            u8 lastByteMask = (0xff << (bitsInLastByte)) & 0x7f;
            u8 bits = byte & lastByteMask;

            if(bits != 0)
            {
                wa_parse_error(parser,
                               ast,
                               "Couldn't read %.*s: leb128 overflow.\n",
                               oc_str8_ip(label));
                res = 0;
            }
        }
    }

    ast->valU64 = res;
    wa_ast_end(parser, ast);

    return (ast);
}

wa_ast* wa_read_leb128_u64(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    return (wa_read_leb128(parser, parent, label, 64, false));
}

/*
wa_ast* wa_read_sleb128(wa_parser* parser, wa_ast* parent, oc_str8 label, u32 bitWidth)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_I64);
    ast->label = label;

    char byte = 0;
    u64 shift = 0;
    i64 res = 0;

    u32 maxShift = 7 * (u32)ceil(bitWidth / 7.);
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

        if(shift >= maxShift)
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: leb128 i64 representation too long.\n",
                           oc_str8_ip(label));
            res = 0;
            break;
        }

        byte = parser->contents[parser->offset];
        parser->offset++;

        if(shift == maxShift && (byte & 0x7e))
        {
            wa_parse_error(parser,
                           ast,
                           "Couldn't read %.*s: leb128 overflow.\n",
                           oc_str8_ip(label));
            res = 0;
            byte = 0;
            break;
        }

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
*/

wa_ast* wa_read_leb128_i64(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    return (wa_read_leb128(parser, parent, label, 64, true));
}

wa_ast* wa_read_leb128_u32(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_read_leb128(parser, parent, label, 32, false);
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
    wa_ast* ast = wa_read_leb128(parser, parent, label, 32, true);
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

            oc_utf8_dec dec = {};
            for(u64 i = 0; i < ast->str8.len; i += dec.size)
            {
                dec = oc_utf8_decode_at(ast->str8, i);
                if(dec.status != OC_UTF8_OK)
                {
                    break;
                }
            }

            switch(dec.status)
            {
                case OC_UTF8_OK:
                    break;
                case OC_UTF8_OUT_OF_BOUNDS:
                    wa_parse_error(parser,
                                   ast,
                                   "Invalid UTF8 encoding: out of bounds.\n");
                    break;
                case OC_UTF8_UNEXPECTED_CONTINUATION_BYTE:
                    wa_parse_error(parser,
                                   ast,
                                   "Invalid UTF8 encoding: unexpected continuation byte.\n");
                    break;
                case OC_UTF8_UNEXPECTED_LEADING_BYTE:
                    wa_parse_error(parser,
                                   ast,
                                   "Invalid UTF8 encoding: unexpected leading byte.\n");
                    break;
                case OC_UTF8_INVALID_BYTE:
                    wa_parse_error(parser,
                                   ast,
                                   "Invalid UTF8 encoding: invalid byte.\n");
                    break;
                case OC_UTF8_INVALID_CODEPOINT:
                    wa_parse_error(parser,
                                   ast,
                                   "Invalid UTF8 encoding: invalid codepoint.\n");
                    break;
                case OC_UTF8_OVERLONG_ENCODING:
                    wa_parse_error(parser,
                                   ast,
                                   "Invalid UTF8 encoding: overlong encoding.\n");
                    break;
            }
        }
    }
    wa_ast_end(parser, ast);
    return (ast);
}

wa_ast* wa_read_bytes_vector(wa_parser* parser, wa_ast* parent, oc_str8 label)
{
    wa_ast* ast = wa_ast_begin(parser, parent, WA_AST_VECTOR);
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
        wa_ast* ast = wa_ast_begin(parser, module->root, WA_AST_SECTION);
        wa_ast* sectionID = wa_read_byte(parser, ast, OC_STR8("section ID"));
        if(wa_ast_has_errors(sectionID))
        {
            return;
        }

        wa_ast* sectionLen = wa_read_leb128_u32(parser, ast, OC_STR8("section length"));
        if(wa_ast_has_errors(sectionLen))
        {
            return;
        }

        u64 contentOffset = parser->offset;

        //TODO: check if section was already defined...

        wa_section* entry = 0;
        switch(sectionID->valU8)
        {
            case 0:
            {
                wa_ast* name = wa_read_name(parser, ast, OC_STR8("section name"));

                if(!oc_str8_cmp(name->str8, OC_STR8("name")))
                {
                    entry = &module->toc.names;
                    ast->label = OC_STR8("Names section");
                }
                else
                {
                    entry = oc_arena_push_type(parser->arena, wa_section);
                    memset(entry, 0, sizeof(wa_section));
                    ast->label = OC_STR8("Custom section");
                }
                entry->name = name->str8;

                if(parser->offset - contentOffset > sectionLen->valU32)
                {
                    wa_parse_error(parser,
                                   ast,
                                   "Unexpected end of custom section.\n",
                                   sectionID);
                }
            }
            break;

            case 1:
                entry = &module->toc.types;
                ast->label = OC_STR8("Types section");
                break;

            case 2:
                entry = &module->toc.imports;
                ast->label = OC_STR8("Imports section");
                break;

            case 3:
                entry = &module->toc.functions;
                ast->label = OC_STR8("Functions section");
                break;

            case 4:
                entry = &module->toc.tables;
                ast->label = OC_STR8("Tables section");
                break;

            case 5:
                entry = &module->toc.memory;
                ast->label = OC_STR8("Memory section");
                break;

            case 6:
                entry = &module->toc.globals;
                ast->label = OC_STR8("Globals section");
                break;

            case 7:
                entry = &module->toc.exports;
                ast->label = OC_STR8("Exports section");
                break;

            case 8:
                entry = &module->toc.start;
                ast->label = OC_STR8("Start section");
                break;

            case 9:
                entry = &module->toc.elements;
                ast->label = OC_STR8("Elements section");
                break;

            case 10:
                entry = &module->toc.code;
                ast->label = OC_STR8("Code section");
                break;

            case 11:
                entry = &module->toc.data;
                ast->label = OC_STR8("Data section");
                break;

            case 12:
                entry = &module->toc.dataCount;
                ast->label = OC_STR8("Data count section");
                break;

            default:
            {
                wa_parse_error(parser,
                               ast,
                               "Unknown section identifier %i.\n",
                               sectionID);
            }
            break;
        }

        if(entry)
        {
            if(entry->ast)
            {
                wa_parse_error(parser, ast, "Redeclaration of %.*s.\n", oc_str8_ip(ast->label));
            }
            entry->id = sectionID->valU8;
            entry->offset = parser->offset;
            entry->len = sectionLen->valU32;
            entry->ast = ast;

            if(entry->id == 0)
            {
                entry->len = sectionLen->valU32 - (parser->offset - contentOffset);
                oc_list_push_back(&module->toc.customSections, &entry->listElt);
            }
        }
        if(contentOffset + sectionLen->valU32 > parser->len || contentOffset + sectionLen->valU32 < contentOffset)
        {
            wa_parse_error(parser,
                           ast,
                           "Length of section out of bounds.\n",
                           sectionID);
        }
        wa_parser_seek(parser, contentOffset + sectionLen->valU32, OC_STR8("next section"));
        wa_ast_end(parser, ast);
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

void wa_parse_names(wa_parser* parser, wa_module* module)
{
    wa_ast* section = module->toc.names.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.names.offset, OC_STR8("names section"));
    u64 startOffset = parser->offset;

    while(parser->offset - startOffset < module->toc.names.len)
    {
        wa_ast* subsection = wa_ast_begin(parser, section, WA_AST_NAME_SUBSECTION);
        wa_ast* id = wa_read_byte(parser, subsection, OC_STR8("subsection id"));
        wa_ast* size = wa_read_leb128_u32(parser, subsection, OC_STR8("subsection size"));
        u32 subStartOffset = parser->offset;

        switch(id->valU8)
        {
            case 0:
            {
                //NOTE: module name
            }
            break;
            case 1:
            {
                //NOTE: function names
                wa_ast* vector = wa_ast_begin_vector(parser, subsection, &module->functionNameCount);
                module->functionNames = oc_arena_push_array(module->arena, wa_name_entry, module->functionNameCount);

                for(u32 entryIndex = 0; entryIndex < module->functionNameCount; entryIndex++)
                {
                    wa_ast* entry = wa_ast_begin(parser, vector, WA_AST_NAME_ENTRY);
                    wa_ast* index = wa_read_leb128_u32(parser, entry, OC_STR8("index"));
                    wa_ast* name = wa_read_name(parser, entry, OC_STR8("name"));

                    module->functionNames[entryIndex] = (wa_name_entry){
                        .index = index->valU32,
                        .name = name->str8,
                    };

                    wa_ast_end(parser, entry);
                }
                wa_ast_end(parser, vector);
            }
            break;
            case 2:
            {
                //NOTE: local names
            }
            break;
            case 0x07:
            {
                //NOTE: unstandardized global names
            }
            break;
            default:
            {
                oc_log_warning("Unexpected subsection id %hhi at offset 0x%02x.\n", id->valU8, parser->offset);
            }
            break;
        }
        wa_ast_end(parser, subsection);
        wa_parser_seek(parser, subStartOffset + size->valU32, OC_STR8("next subsection"));
    }
    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.names.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.names.len,
                       parser->offset - startOffset);
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
                import->index = indexAst->valU32;

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

    module->globals = oc_arena_push_array(parser->arena, wa_global_desc, module->globalCount + module->globalImportCount);
    memset(module->globals, 0, (module->globalCount + module->globalImportCount) * sizeof(wa_global_desc));

    //NOTE: re-read imports, because the format is kinda stupid -- they should have included imports in the global section
    u32 globalImportIndex = 0;
    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];
        if(import->kind == WA_IMPORT_GLOBAL)
        {
            wa_global_desc* global = &module->globals[globalImportIndex];
            global->type = import->type;
            global->mut = import->mut;
            globalImportIndex++;
        }
    }

    if(section)
    {
        for(u32 globalIndex = 0; globalIndex < module->globalCount; globalIndex++)
        {
            wa_global_desc* global = &module->globals[globalIndex + module->globalImportCount];

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
    wa_instr* instr = 0;

    while(!wa_parser_end(parser) && blockDepth >= 0)
    {
        instr = oc_arena_push_type(parser->arena, wa_instr);
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

        //NOTE: memory.init and data.drop need a data count section
        if((instr->op == WA_INSTR_memory_init || instr->op == WA_INSTR_data_drop)
           && !module->toc.dataCount.ast)
        {
            wa_parse_error(parser, instrAst, "%s requires a data count section.\n", wa_instr_strings[instr->op]);
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
                        wa_ast* immAst = wa_read_byte(parser, instrAst, OC_STR8("zero"));
                        instr->imm[immIndex].valI32 = immAst->valU8;
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

    if(!instr || instr->op != WA_INSTR_end)
    {
        wa_parse_error(parser, exprAst, "unexpected end of expression\n");
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

void wa_parse_data_count(wa_parser* parser, wa_module* module)
{
    wa_ast* section = module->toc.dataCount.ast;
    if(!section)
    {
        return;
    }

    wa_parser_seek(parser, module->toc.dataCount.offset, OC_STR8("data count section"));
    u64 startOffset = parser->offset;

    wa_ast* dataCount = wa_read_leb128_u32(parser, section, OC_STR8("data count"));
    module->dataCount = dataCount->valU32;

    //NOTE: check section size
    if(parser->offset - startOffset != module->toc.dataCount.len)
    {
        wa_parse_error(parser,
                       section,
                       "Size of section does not match declared size (declared = %llu, actual = %llu)\n",
                       module->toc.dataCount.len,
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

    u32 dataCount = 0;
    wa_ast* vector = wa_ast_begin_vector(parser, section, &dataCount);

    if(module->toc.dataCount.ast && dataCount != module->dataCount)
    {
        wa_parse_error(parser,
                       vector,
                       "Number of data segments does not match data count section (expected %u, got %u).\n",
                       module->dataCount,
                       dataCount);
    }
    module->dataCount = dataCount;

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
        wa_ast* initVec = wa_read_bytes_vector(parser, segmentAst, OC_STR8("init"));
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
        if(module->functionCount - module->functionImportCount)
        {
            wa_parse_error(parser,
                           module->toc.functions.ast,
                           "Function section declares %i functions, but code section is absent",
                           module->functionCount - module->functionImportCount);
        }
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
            wa_ast* typeAst = wa_read_byte(parser, localEntryAst, OC_STR8("type"));
            typeAst->kind = WA_AST_VALUE_TYPE;

            counts[localEntryIndex] = countAst->valU32;
            types[localEntryIndex] = typeAst->valU8;

            if(func->localCount + counts[localEntryIndex] < func->localCount)
            {
                //NOTE: overflow
                wa_parse_error(parser,
                               funcAst,
                               "Too many locals for function %i\n",
                               funcIndex);
                goto parse_function_end;
            }

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

#include "dwarf.c"

wa_source_node* wa_source_node_alloc(wa_module* module)
{
    wa_source_node* node = oc_arena_push_type(module->arena, wa_source_node);
    memset(node, 0, sizeof(wa_source_node));
    return node;
}

typedef struct wa_source_file_elt
{
    oc_list_elt listElt;
    wa_source_file file;
    u64 index;
} wa_source_file_elt;

wa_source_file_elt* wa_find_or_add_source_file(oc_arena* scratchArena, oc_arena* pathArena, oc_list* files, u64* fileCount, oc_str8 rootPath, oc_str8 fullPath)
{
    wa_source_file_elt* file = 0;

    oc_list_for(*files, elt, wa_source_file_elt, listElt)
    {
        if(!oc_str8_cmp(elt->file.rootPath, rootPath) && !oc_str8_cmp(elt->file.fullPath, fullPath))
        {
            file = elt;
        }
    }

    if(!file)
    {
        file = oc_arena_push_type(scratchArena, wa_source_file_elt);
        memset(file, 0, sizeof(wa_source_file_elt));
        file->file.fullPath = oc_str8_push_copy(pathArena, fullPath);
        file->file.rootPath = oc_str8_slice(file->file.fullPath, 0, rootPath.len);
        file->index = *fileCount;
        oc_list_push_back(files, &file->listElt);
        (*fileCount)++;
    }
    return file;
}

void wa_parse_dwarf(wa_parser* parser, wa_module* module)
{
    /////////////////////////////////////////////////////////////////////////
    //WIP: Dwarf stuff

    //NOTE: load dwarf sections
    //TODO: - put all that in its own wa_load_debug_info function
    //      ? split dw_xxx struct (for parsing dwarf specific stuff) from wa_debug_xxx (for internal representation)

    oc_arena_scope scratch = oc_scratch_begin();

    dw_sections dwarfSections = { 0 };

    oc_list_for(module->toc.customSections, section, wa_section, listElt)
    {
        oc_str8 sectionContents = (oc_str8){
            .ptr = parser->contents + section->offset,
            .len = section->len,
        };

        if(!oc_str8_cmp(section->name, OC_STR8(".debug_abbrev")))
        {
            dwarfSections.abbrev = sectionContents;
        }
        else if(!oc_str8_cmp(section->name, OC_STR8(".debug_info")))
        {
            dwarfSections.info = sectionContents;
        }
        else if(!oc_str8_cmp(section->name, OC_STR8(".debug_str_offsets")))
        {
            dwarfSections.strOffsets = sectionContents;
        }
        else if(!oc_str8_cmp(section->name, OC_STR8(".debug_str")))
        {
            dwarfSections.str = sectionContents;
        }
        else if(!oc_str8_cmp(section->name, OC_STR8(".debug_addr")))
        {
            dwarfSections.addr = sectionContents;
        }
        else if(!oc_str8_cmp(section->name, OC_STR8(".debug_line")))
        {
            dwarfSections.line = sectionContents;
        }
        else if(!oc_str8_cmp(section->name, OC_STR8(".debug_line_str")))
        {
            dwarfSections.lineStr = sectionContents;
        }
    }

    module->debugInfo = oc_arena_push_type(module->arena, dw_info);
    memset(module->debugInfo, 0, sizeof(dw_info));

    /*
    //NOTE: dump dwarf abbrev table if it exists
    if(dwarfSections.abbrev.len)
    {
        dw_dump_abbrev_table(dwarfSections.abbrev);
    }
    */

    //NOTE: load line info if it exists
    //TODO: put that in dw_info
    if(dwarfSections.line.len)
    {
        module->debugInfo->line = oc_arena_push_type(module->arena, dw_line_info);
        *module->debugInfo->line = dw_load_line_info(module->arena, &dwarfSections);

        dw_parse_info(module->arena, &dwarfSections, module->debugInfo);
        dw_print_debug_info(module->debugInfo);

        //dw_dump_info(dwarfSections);
        //dw_print_line_info(module->debugInfo->line);

        dw_line_info* lineInfo = module->debugInfo->line;

        //NOTE: alloc wasm to line map
        for(u64 tableIndex = 0; tableIndex < lineInfo->tableCount; tableIndex++)
        {
            module->wasmToLineCount += lineInfo->tables[tableIndex].entryCount;
        }
        module->wasmToLine = oc_arena_push_array(module->arena, wa_wasm_to_line_entry, module->wasmToLineCount);
        u64 wasmToLineIndex = 0;

        //NOTE: build a global file table and build wasm line map
        wa_source_info* sourceInfo = &module->sourceInfo;
        oc_arena_scope scratch = oc_scratch_begin_next(module->arena);

        oc_list files = { 0 };
        sourceInfo->fileCount = 1; // index 0 is reserved for a "nil" wa_source_file entry

        for(u64 tableIndex = 0; tableIndex < lineInfo->tableCount; tableIndex++)
        {
            dw_line_table* table = &lineInfo->tables[tableIndex];

            //NOTE: determine the paths of compile unit current directory, current file, and current file dir,
            //      which varies between dwarf 4 and 5.
            //WARN: currentFileDir can be different from currentDir if currentFile is an absolute path, or if it
            //      explicitly refers to another directory than the compile unit current directory.
            oc_str8 currentDir = { 0 };
            oc_str8 currentFile = { 0 };
            oc_str8 currentFileDir = { 0 };
            oc_str8 currentFileAbs = { 0 };

            bool currentFileInTable = false;

            if(table->header.version == 4)
            {
                //NOTE: set current dir current file from CU info
                OC_ASSERT(tableIndex < module->debugInfo->unitCount);
                dw_unit* unit = &module->debugInfo->units[tableIndex];
                dw_die* die = unit->rootDie;

                OC_ASSERT(die->abbrev->tag == DW_TAG_compile_unit);

                //TODO: make sure the die DW_AT_stmt_list corresponds to the current table
                dw_attr* dirAttr = dw_die_get_attr(die, DW_AT_comp_dir);
                if(dirAttr)
                {
                    currentDir = dirAttr->string;
                }

                dw_attr* fileAttr = dw_die_get_attr(die, DW_AT_name);
                if(fileAttr)
                {
                    currentFile = fileAttr->string;
                }
            }
            else
            {
                currentDir = table->header.dirEntries[0].path;
                currentFile = table->header.fileEntries[0].path;
            }

            if(currentFile.len && currentFile.ptr[0] == '/')
            {
                currentFileDir = currentFile;
                currentFileAbs = currentFile;
            }
            else
            {
                currentFileDir = currentDir;
                currentFileAbs = oc_path_append(scratch.arena, currentFileDir, currentFile);
            }
            OC_DEBUG_ASSERT(!oc_str8_cmp(currentFileDir, oc_str8_slice(currentFileAbs, 0, currentFileDir.len)),
                            "currentFileDir should be a prefix of currentFileAbs");

            //NOTE: allocate a table to map dwarf file indices to our global file table indices
            u64* fileIndices = oc_arena_push_array(scratch.arena, u64, table->header.fileEntryCount + 1);

            //NOTE: add the current table file entries to our global table
            for(u64 fileIndex = 0; fileIndex < table->header.fileEntryCount; fileIndex++)
            {
                //NOTE: first find the root path and full path of the file
                dw_file_entry* fileEntry = &table->header.fileEntries[fileIndex];
                oc_str8 filePath = fileEntry->path;
                oc_str8 rootPath = { 0 };
                oc_str8 fullPath = { 0 };

                if(filePath.len && filePath.ptr[0] == '/')
                {
                    //NOTE: absolute file path. This will be shown as a single file name at the root of the tree
                    rootPath = filePath;
                    fullPath = filePath;
                }
                else
                {
                    //NOTE: relative file path. This will be shown in a subtree whose root it either the name of the
                    // dir path (in case of an absolute dir path), or the name of the CU path (in case of a relative dir path)

                    oc_str8 dirPath = { 0 };
                    if(table->header.version == 5)
                    {
                        dw_file_entry* dirEntry = &table->header.dirEntries[fileEntry->dirIndex];
                        dirPath = dirEntry->path;
                    }
                    else if(fileEntry->dirIndex)
                    {
                        dw_file_entry* dirEntry = &table->header.dirEntries[fileEntry->dirIndex - 1];
                        dirPath = dirEntry->path;
                    }
                    else
                    {
                        dirPath = currentDir;
                    }

                    if(dirPath.len && dirPath.ptr[0] == '/')
                    {
                        // absolute dir path
                        rootPath = dirPath;
                    }
                    else
                    {
                        // dir path relative to current directory of compilation
                        rootPath = currentDir;
                        filePath = oc_path_append(scratch.arena, dirPath, filePath);
                    }
                    fullPath = oc_path_append(scratch.arena, rootPath, filePath);

                    OC_DEBUG_ASSERT(!oc_str8_cmp(rootPath, oc_str8_slice(fullPath, 0, rootPath.len)),
                                    "rootPath should be a prefix of fullPath");
                }

                //NOTE: now we create a wa_source_file entry with the given root path and file path, if it doesn't exist
                wa_source_file_elt* file = wa_find_or_add_source_file(scratch.arena,
                                                                      module->arena,
                                                                      &files,
                                                                      &sourceInfo->fileCount,
                                                                      rootPath,
                                                                      fullPath);

                //NOTE: if the found/created wa_source_file represents the current file, we mark it as found.
                //      this avoids duplicates when toolchain explicity put the current file in dwarf version 4
                //      file entries rather than using index 0 as the implicit current file.
                if(!oc_str8_cmp(currentFileAbs, file->file.fullPath))
                {
                    currentFileInTable = true;
                    if(table->header.version == 4)
                    {
                        fileIndices[0] = file->index;
                    }
                }

                if(table->header.version == 4)
                {
                    fileIndices[fileIndex + 1] = file->index;
                }
                else
                {
                    fileIndices[fileIndex] = file->index;
                }
            }

            if(table->header.version == 4 && currentFile.len && !currentFileInTable)
            {
                //NOTE: if the compile unit "current file" is only referenced implicitly by index 0,
                //      we add it to our global table here

                wa_source_file_elt* file = wa_find_or_add_source_file(scratch.arena,
                                                                      module->arena,
                                                                      &files,
                                                                      &sourceInfo->fileCount,
                                                                      currentFileDir,
                                                                      currentFileAbs);

                fileIndices[0] = file->index;
            }

            //NOTE: add the table line entries to our module's wasmToLineEntry table
            for(u64 entryIndex = 0; entryIndex < table->entryCount; entryIndex++)
            {
                dw_line_entry* lineEntry = &table->entries[entryIndex];
                wa_wasm_to_line_entry* wasmToLineEntry = &module->wasmToLine[wasmToLineIndex];

                wasmToLineEntry->wasmOffset = lineEntry->address;
                wasmToLineEntry->loc.fileIndex = fileIndices[lineEntry->file];
                wasmToLineEntry->loc.line = lineEntry->line;

                wasmToLineIndex++;
            }
        }

        //NOTE: now copy all wa_source_file entries to the module's global file array, and finally clear the scratch
        //      the first element of the array is a zeroed wa_source_file (index 0 is used as an invalid fileIndex)
        sourceInfo->files = oc_arena_push_array(module->arena, wa_source_file, sourceInfo->fileCount);
        memset(&sourceInfo->files[0], 0, sizeof(wa_source_file));
        oc_list_for_indexed(files, it, wa_source_file_elt, listElt)
        {
            sourceInfo->files[it.index + 1] = it.elt->file;
        }

        oc_scratch_end(scratch);
    }

    oc_scratch_end(scratch);
}

wa_source_info* wa_module_get_source_info(wa_module* module)
{
    return &module->sourceInfo;
}

typedef struct wa_bytecode_mapping
{
    oc_list_elt listElt;

    u32 funcIndex;
    u32 codeIndex;
    wa_instr* instr;

} wa_bytecode_mapping;

void wa_warm_to_wasm_loc_push(wa_module* module, u32 funcIndex, u32 codeIndex, wa_instr* instr)
{
    wa_bytecode_mapping* mapping = oc_arena_push_type(module->arena, wa_bytecode_mapping);
    mapping->funcIndex = funcIndex;
    mapping->codeIndex = codeIndex;
    mapping->instr = instr;

    u64 id = (u64)funcIndex << 32 | (u64)codeIndex;
    u64 hash = oc_hash_xx64_string((oc_str8){ .ptr = (char*)&id, .len = 8 });
    u64 index = hash % module->warmToWasmMapLen;

    oc_list_push_back(&module->warmToWasmMap[index], &mapping->listElt);
}

void wa_wasm_to_warm_loc_push(wa_module* module, u32 funcIndex, u32 codeIndex, wa_instr* instr)
{
    wa_bytecode_mapping* mapping = oc_arena_push_type(module->arena, wa_bytecode_mapping);
    mapping->funcIndex = funcIndex;
    mapping->codeIndex = codeIndex;
    mapping->instr = instr;

    u64 id = mapping->instr->ast->loc.start - module->toc.code.offset;
    u64 hash = oc_hash_xx64_string((oc_str8){ .ptr = (char*)&id, .len = 8 });
    u64 index = hash % module->wasmToWarmMapLen;

    oc_list_push_back(&module->wasmToWarmMap[index], &mapping->listElt);
}

//-------------------------------------------------------------------------
// Compile
//-------------------------------------------------------------------------

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

enum
{
    WA_TYPE_STACK_MAX_LEN = 512,
    WA_CONTROL_STACK_MAX_LEN = 512,
    WA_MAX_REG = 4096,
};

typedef struct wa_operand_slot
{
    u32 index;
    wa_instr* originInstr;
    u64 originOpd;
} wa_operand_slot;

typedef struct wa_register_slot
{
    u32 refCount;
    i32 nextFree;
    wa_value_type type;

} wa_register_slot;

typedef struct wa_operand
{
    wa_value_type type;
    u32 index;

} wa_operand;

typedef struct wa_build_context
{
    oc_arena* arena;     // the module's arena
    oc_arena checkArena; // temp arena for checking
    oc_arena codeArena;  // temp arena for building code
    wa_module* module;

    u32 regCount;
    wa_register_slot regs[WA_MAX_REG];
    i32 firstFreeReg;

    u64 opdStackLen;
    u64 opdStackCap;
    wa_operand_slot* opdStack;

    u64 controlStackLen;
    u64 controlStackCap;
    wa_block* controlStack;

    wa_func* currentFunction;
    wa_instr* currentInstr;
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
    if(ast)
    {
        oc_list_push_back(&ast->errors, &error->astElt);
    }

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

bool wa_operand_is_nil(wa_operand* opd)
{
    return (opd->type == WA_TYPE_INVALID);
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

enum
{
    WA_MAX_SLOT_COUNT = 4096,
};

bool wa_operand_slot_is_unknown(wa_operand_slot* opd)
{
    return opd->index == UINT32_MAX;
}

u32 wa_allocate_register(wa_build_context* context, wa_value_type type)
{
    u32 index = 0;
    wa_register_slot* reg = 0;

    if(context->firstFreeReg >= 0)
    {
        reg = &context->regs[context->firstFreeReg];
        context->firstFreeReg = reg->nextFree;
    }
    else
    {
        if(context->regCount >= WA_MAX_REG)
        {
            wa_compile_error(context, context->currentInstr->ast, "register allocation overflow\n");
        }
        else
        {
            reg = &context->regs[context->regCount];
            context->regCount++;
        }
    }

    if(reg)
    {
        reg->type = type;
        reg->refCount = 0;
        index = reg - context->regs;
    }
    return (index);
}

void wa_retain_register(wa_build_context* context, u32 index)
{
    OC_DEBUG_ASSERT(index < WA_MAX_REG);
    wa_register_slot* reg = &context->regs[index];
    reg->refCount++;
}

void wa_release_register(wa_build_context* context, u32 index)
{
    OC_DEBUG_ASSERT(index < WA_MAX_REG);
    wa_register_slot* reg = &context->regs[index];

    OC_DEBUG_ASSERT(reg->refCount);
    reg->refCount--;

    u32 localCount = context->currentFunction ? context->currentFunction->localCount : 0;
    if(index >= localCount)
    {
        if(reg->refCount == 0)
        {
            reg->nextFree = context->firstFreeReg;
            context->firstFreeReg = index;
        }
    }
}

void wa_operand_stack_push(wa_build_context* context, wa_operand_slot opd)
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
    context->opdStack[context->opdStackLen] = opd;
    context->opdStackLen++;

    if(!wa_operand_slot_is_unknown(&opd))
    {
        wa_retain_register(context, opd.index);
    }
}

void wa_operand_stack_push_return_slots(wa_build_context* context, u32 maxUsedSlot, u32 returnCount, wa_value_type* returns)
{
    //NOTE: return slots will start at maxUsedSlot, so we must reserve returnCount slots after that.
    context->regCount = oc_max(maxUsedSlot + 1 + returnCount, context->regCount);

    for(u32 retIndex = 0; retIndex < returnCount; retIndex++)
    {
        u64 slotIndex = maxUsedSlot + 1 + retIndex;

        //NOTE: if that slot was in the free list, we remove it.
        //      otherwise it is a fresh slot
        {
            i32 freeRegIndex = context->firstFreeReg;
            wa_register_slot* prev = 0;
            while(freeRegIndex >= 0)
            {
                wa_register_slot* reg = &context->regs[freeRegIndex];

                if(freeRegIndex == slotIndex)
                {
                    if(prev)
                    {
                        prev->nextFree = reg->nextFree;
                    }
                    else
                    {
                        context->firstFreeReg = reg->nextFree;
                    }
                    break;
                }

                prev = reg;
                freeRegIndex = reg->nextFree;
            }
        }

        context->regs[slotIndex].refCount = 0;
        context->regs[slotIndex].type = returns[retIndex];

        //NOTE: push the slot
        wa_operand_stack_push(context,
                              (wa_operand_slot){
                                  .index = slotIndex,
                              });
    }
}

u32 wa_operand_stack_push_reg(wa_build_context* context, wa_value_type type, wa_instr* instr)
{
    wa_operand_slot opd = {
        .index = wa_allocate_register(context, type),
        .originInstr = instr,
        .originOpd = context->codeLen,
    };
    wa_operand_stack_push(context, opd);

    return opd.index;
}

u32 wa_operand_stack_push_local(wa_build_context* context, u32 index)
{
    wa_operand_slot opd = {
        .index = index,
        //        .originInstr = instr,
        //        .originOpd = context->codeLen,
    };
    wa_operand_stack_push(context, opd);
    return opd.index;
}

u32 wa_operand_stack_push_unknown(wa_build_context* context)
{
    wa_operand_slot opd = {
        .index = UINT32_MAX,
    };
    wa_operand_stack_push(context, opd);
    return opd.index;
}

void wa_operand_stack_push_copy(wa_build_context* context, u32 index)
{
    wa_operand_slot opdCopy = context->opdStack[index];
    wa_operand_stack_push(context, opdCopy);
}

wa_operand wa_operand_stack_lookup(wa_build_context* context, u32 index)
{
    wa_operand opd = { 0 };
    wa_block* block = wa_control_stack_top(context);
    OC_ASSERT(block);

    if(index < context->opdStackLen - block->scopeBase)
    {
        wa_operand_slot* slot = &context->opdStack[context->opdStackLen - index - 1];

        if(wa_operand_slot_is_unknown(slot))
        {
            opd.type = WA_TYPE_UNKNOWN;
        }
        else
        {
            opd.index = slot->index;
            opd.type = context->regs[slot->index].type;
        }
    }
    else if(block->polymorphic)
    {
        opd = (wa_operand){
            .type = WA_TYPE_UNKNOWN,
        };
    }
    return (opd);
}

wa_operand wa_operand_stack_pop(wa_build_context* context)
{
    wa_operand opd = wa_operand_stack_lookup(context, 0);

    wa_block* block = wa_control_stack_top(context);
    OC_ASSERT(block);

    if(context->opdStackLen > block->scopeBase)
    {
        context->opdStackLen--;
        if(opd.type != WA_TYPE_UNKNOWN)
        {
            wa_release_register(context, opd.index);
        }
    }
    return opd;
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

    u64 opdCount = oc_min(context->opdStackLen - scopeBase, count);

    for(u64 i = 0; i < opdCount; i++)
    {
        wa_operand_slot* opd = &context->opdStack[context->opdStackLen - i - 1];
        wa_release_register(context, opd->index);
    }

    context->opdStackLen -= opdCount;
}

void wa_operand_stack_pop_scope(wa_build_context* context, wa_block* block)
{
    OC_ASSERT(context->opdStackLen >= block->scopeBase);

    u64 opdCount = context->opdStackLen - block->scopeBase;
    for(u64 i = 0; i < opdCount; i++)
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

wa_operand* wa_operand_stack_get_operands(oc_arena* arena,
                                          wa_build_context* context,
                                          wa_instr* instr,
                                          u32 count,
                                          wa_value_type* types,
                                          bool popOpds)
{
    wa_operand* opds = oc_arena_push_array(arena, wa_operand, count);
    for(u32 i = 0; i < count; i++)
    {
        u32 opdIndex = count - 1 - i;

        if(popOpds)
        {
            opds[opdIndex] = wa_operand_stack_pop(context);
        }
        else
        {
            opds[opdIndex] = wa_operand_stack_lookup(context, i);
        }

        if(wa_operand_is_nil(&opds[opdIndex]))
        {
            wa_compile_error(context, instr->ast, "unbalanced stack\n");
            break;
        }
        else
        {
            if(!wa_check_operand_type(opds[opdIndex].type, types[opdIndex]))
            {
                wa_compile_error(context, instr->ast, "operand types mismatch\n");
                //TODO: here we can give a better error message since we have all the operands popped so far
                break;
            }
        }
    }
    return (opds);
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
    u32 index = context->codeLen;
    wa_code* code = wa_push_code(context);
    code->opcode = op;

    if(context->currentFunction)
    {
        wa_module* module = context->module;
        wa_warm_to_wasm_loc_push(module, context->currentFunction - module->functions, index, context->currentInstr);
    }
    context->currentInstr->codeIndex = index;
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

bool wa_operand_slot_is_local(wa_build_context* context, wa_operand_slot* opd)
{
    u32 localCount = context->currentFunction ? context->currentFunction->localCount : 0;
    return (opd->index < localCount);
}

void wa_move_register_if_used_in_operands(wa_build_context* context,
                                          u32 slotIndex,
                                          u32 opdCount,
                                          wa_operand* operands,
                                          bool rewriteStack)
{
    u32 newReg = UINT32_MAX;
    for(u32 opdIndex = 0; opdIndex < opdCount; opdIndex++)
    {
        wa_operand* opd = &operands[opdIndex];

        if(opd->index == slotIndex)
        {
            if(newReg == UINT32_MAX)
            {
                newReg = wa_allocate_register(context, opd->type);
            }
            else
            {
                wa_retain_register(context, newReg);
            }

            //TODO: move that below with count
            wa_release_register(context, slotIndex);

            opd->index = newReg;

            if(rewriteStack)
            {
                context->opdStack[context->opdStackLen - opdCount + opdIndex].index = newReg;
            }
        }
    }
    if(newReg != UINT32_MAX)
    {
        wa_emit_opcode(context, WA_INSTR_move);
        wa_emit_index(context, slotIndex);
        wa_emit_index(context, newReg);
    }
}

void wa_move_local_if_used(wa_build_context* context, u32 slotIndex)
{
    OC_DEBUG_ASSERT(slotIndex < context->currentFunction.localCount);

    wa_block* block = wa_control_stack_top(context);
    u32 opdCount = context->opdStackLen - block->scopeBase;

    oc_arena_scope scratch = oc_scratch_begin();
    wa_operand* operands = oc_arena_push_array(scratch.arena, wa_operand, opdCount);

    for(u32 i = 0; i < opdCount; i++)
    {
        u32 opdIndex = opdCount - 1 - i;
        operands[opdIndex] = wa_operand_stack_lookup(context, i);
    }

    wa_move_register_if_used_in_operands(context, slotIndex, opdCount, operands, true);

    oc_scratch_end(scratch);
}

/*
void wa_move_local_if_used(wa_build_context* context, u32 slotIndex)
{
    OC_DEBUG_ASSERT(slotIndex < context->currentFunction.localCount);

    wa_block* block = wa_control_stack_top(context);
    u32 newReg = UINT32_MAX;
    u64 count = 0;

    //NOTE: we check only slots in current scope. This means we never clobber reserved input/return slots
    for(u32 stackIndex = block->scopeBase; stackIndex < context->opdStackLen; stackIndex++)
    {
        wa_operand_slot* opd = &context->opdStack[stackIndex];
        if(opd->index == slotIndex)
        {
            if(newReg == UINT32_MAX)
            {
                newReg = wa_allocate_register(context, context->regs[slotIndex].type);
            }
            else
            {
                wa_retain_register(context, newReg);
            }
            opd->index = newReg;

            //NOTE: we don't need to release previous slot because it is a local
        }
    }
    if(newReg != UINT32_MAX)
    {
        wa_emit_opcode(context, WA_INSTR_move);
        wa_emit_index(context, slotIndex);
        wa_emit_index(context, newReg);
    }
}
*/

void wa_move_locals_to_registers(wa_build_context* context)
{
    for(u32 stackIndex = 0; stackIndex < context->opdStackLen; stackIndex++)
    {
        wa_operand_slot* opd = &context->opdStack[stackIndex];
        if(wa_operand_slot_is_local(context, opd))
        {
            wa_move_local_if_used(context, opd->index);
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
            wa_operand_stack_push_unknown(context);
        }
        else
        {
            //NOTE copy block inputs on top of the stack
            wa_operand_stack_push_copy(context, paramStart + inIndex);
        }
    }
}

void wa_block_begin(wa_build_context* context, wa_instr* instr)
{
    oc_arena_scope scratch = oc_scratch_begin();
    wa_func_type* type = instr->blockType;

    //NOTE: check block type input (but we don't pop or use them here)
    wa_operand_stack_get_operands(scratch.arena,
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

    wa_operand* opds = wa_operand_stack_get_operands(scratch.arena,
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
            wa_emit_index(context, opds[paramIndex].index);
            wa_emit_index(context, dst->index);
        }
    }
    oc_scratch_end(scratch);
}

void wa_block_move_results_to_output_slots(wa_build_context* context, wa_block* block, wa_instr* instr)
{
    oc_arena_scope scratch = oc_scratch_begin();
    wa_func_type* type = block->begin->blockType;

    wa_operand* opds = wa_operand_stack_get_operands(scratch.arena,
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
            wa_emit_index(context, opds[returnIndex].index);
            wa_emit_index(context, dst->index);
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
    wa_operand* returns = wa_operand_stack_get_operands(scratch.arena,
                                                        context,
                                                        instr,
                                                        type->returnCount,
                                                        type->returns,
                                                        false);

    for(u32 retIndex = 0; retIndex < type->returnCount; retIndex++)
    {
        wa_operand opd = returns[retIndex];

        //NOTE store value to return slot
        if(opd.index != retIndex)
        {
            //NOTE:
            //  if operand isn't already stored at retIndex, we will move it there
            //  so we need to move register retIndex to a new reg if it's used in other return operands

            wa_move_register_if_used_in_operands(context, retIndex, type->returnCount, returns, false);
            /*
            ////////////////////////////////////////////////////////////////////////////////////////////////////
            //TODO: coalesce with move if used _BUT_ preserve the stack for futher returns in other branches
            ////////////////////////////////////////////////////////////////////////////////////////////////////

            u32 newReg = UINT32_MAX;
            u32 count = 0;
            for(u32 i = 0; i < type->returnCount; i++)
            {
                wa_operand* r = &returns[i];
                if(r->index == retIndex)
                {
                    if(newReg == UINT32_MAX)
                    {
                        newReg = wa_allocate_register(context, r->type);
                    }
                    else
                    {
                        wa_retain_register(context, newReg);
                    }

                    wa_release_register(context, retIndex);

                    r->index = newReg;
                }
            }
            if(newReg != UINT32_MAX)
            {
                wa_emit_opcode(context, WA_INSTR_move);
                wa_emit_index(context, retIndex);
                wa_emit_index(context, newReg);
            }
*/
            wa_emit_opcode(context, WA_INSTR_move);
            wa_emit_index(context, opd.index);
            wa_emit_index(context, retIndex);
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

    context->regCount = 0;
    context->firstFreeReg = -1;

    context->currentFunction = 0;
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
            case WA_IMM_ZERO:
            {
                if(imm->valI32 != 0)
                {
                    wa_compile_error(context,
                                     instr->ast,
                                     "non zero value in 0x%x in zero immediate\n",
                                     (u32)imm->valI32);
                    check = false;
                }
            }
            break;

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
        context->currentInstr = instr;

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

            wa_operand* opd = wa_operand_stack_get_operands(scratch.arena,
                                                            context,
                                                            instr,
                                                            1,
                                                            (wa_value_type[]){ WA_TYPE_I32 },
                                                            true);
            wa_block_begin(context, instr);

            wa_emit_opcode(context, WA_INSTR_jump_if_zero);
            wa_emit_i64(context, 0);
            wa_emit_index(context, opd->index);
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
            wa_operand* opd = wa_operand_stack_get_operands(scratch.arena,
                                                            context,
                                                            instr,
                                                            1,
                                                            (wa_value_type[]){ WA_TYPE_I32 },
                                                            true);

            wa_emit_opcode(context, WA_INSTR_jump_if_zero);
            u64 jumpOffset = context->codeLen;
            wa_emit_i64(context, 0);
            wa_emit_index(context, opd->index);

            u32 label = instr->imm[0].index;
            wa_compile_branch(context, instr, label);

            context->code[jumpOffset].valI64 = context->codeLen - jumpOffset;
        }
        else if(instr->op == WA_INSTR_br_table)
        {
            wa_operand* opd = wa_operand_stack_get_operands(scratch.arena,
                                                            context,
                                                            instr,
                                                            1,
                                                            (wa_value_type[]){ WA_TYPE_I32 },
                                                            true);

            wa_emit_opcode(context, WA_INSTR_jump_table);
            u64 baseOffset = context->codeLen;

            wa_emit_index(context, instr->immCount);
            wa_emit_index(context, opd->index);

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
                wa_instr* prev = oc_list_prev_entry(instr, wa_instr, listElt);
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

            instr->codeIndex = context->codeLen;
        }
        else if(instr->op == WA_INSTR_call || instr->op == WA_INSTR_call_indirect)
        {
            //NOTE: compute max used slot
            //TODO: we could probably be more clever here, eg in some case just move the index operand
            //      past the arguments?
            i64 maxUsedSlot = func->localCount;

            for(u32 stackIndex = 0; stackIndex < context->opdStackLen; stackIndex++)
            {
                wa_operand_slot* opd = &context->opdStack[stackIndex];
                maxUsedSlot = oc_max((i64)opd->index, maxUsedSlot);
            }

            //NOTE: get callee type, and indirect index for call indirect
            wa_operand* indirectOpd = 0;
            wa_func* callee = 0;
            wa_func_type* type = 0;

            if(instr->op == WA_INSTR_call)
            {
                callee = &module->functions[instr->imm[0].index];
                type = callee->type;
            }
            else
            {
                indirectOpd = wa_operand_stack_get_operands(scratch.arena,
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

            wa_operand* argOpds = wa_operand_stack_get_operands(scratch.arena,
                                                                context,
                                                                instr,
                                                                type->paramCount,
                                                                type->params,
                                                                true);

            for(u32 argIndex = 0; argIndex < paramCount; argIndex++)
            {
                wa_emit_opcode(context, WA_INSTR_move);
                wa_emit_index(context, argOpds[argIndex].index);
                wa_emit_index(context, maxUsedSlot + 1 + argIndex);
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
                wa_emit_index(context, indirectOpd->index);
            }

            wa_operand_stack_push_return_slots(context, maxUsedSlot, type->returnCount, type->returns);
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
                    wa_move_local_if_used(context, localIndex);
                }
                break;
                case WA_INSTR_local_tee:
                {
                    u32 localIndex = imm[0].valU32;
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
                    in[0] = func->locals[localIndex];
                    out = oc_arena_push_array(scratch.arena, wa_value_type, outCount);
                    out[0] = func->locals[localIndex];

                    wa_move_local_if_used(context, localIndex);
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
                    in = oc_arena_push_array(scratch.arena, wa_value_type, inCount);
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
            wa_operand* inOpds = wa_operand_stack_get_operands(scratch.arena,
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
                instr->codeIndex = context->codeLen;
                // do nothing
            }
            else if(instr->op == WA_INSTR_select
                    || instr->op == WA_INSTR_select_t)
            {
                if(!wa_check_operand_type(inOpds[0].type, inOpds[1].type))
                {
                    wa_compile_error(context, instr->ast, "select operands must be of same type\n");
                }
                out = oc_arena_push_array(scratch.arena, wa_value_type, outCount);
                out[0] = inOpds[0].type;

                u32 outIndex = wa_operand_stack_push_reg(context, out[0], instr);

                wa_emit_opcode(context, WA_INSTR_select);
                wa_emit_index(context, inOpds[0].index);
                wa_emit_index(context, inOpds[1].index);
                wa_emit_index(context, inOpds[2].index);
                wa_emit_index(context, outIndex);
            }
            else if(instr->op == WA_INSTR_local_get)
            {
                u32 localIndex = instr->imm[0].index;

                wa_operand_stack_push_local(context, localIndex);

                instr->codeIndex = context->codeLen;
            }
            else if(instr->op == WA_INSTR_local_set || instr->op == WA_INSTR_local_tee)
            {
                u32 localIndex = instr->imm[0].valU32;

                //TODO: check if the local was written to since the value was pushed, and if not, change
                //      the output operand of the value's origin instruction rather than issuing a move.
                //WARN:  this can't be used after a branch, since the branch might use the original slot index
                //      so we'd need to add a "touched" bit and set it for operands used in a branch?
                wa_emit_opcode(context, WA_INSTR_move);
                wa_emit_index(context, inOpds[0].index);
                wa_emit_index(context, localIndex);

                if(instr->op == WA_INSTR_local_tee)
                {
                    wa_operand_stack_push_local(context, localIndex);
                }
            }
            else if(instr->op == WA_INSTR_global_get)
            {
                u32 globalIndex = instr->imm[0].valU32;

                u32 regIndex = wa_operand_stack_push_reg(context,
                                                         module->globals[globalIndex].type,
                                                         instr);

                wa_emit_opcode(context, WA_INSTR_global_get);
                wa_emit_index(context, globalIndex);
                wa_emit_index(context, regIndex);
            }
            else if(instr->op == WA_INSTR_global_set)
            {
                u32 globalIndex = instr->imm[0].valU32;
                wa_emit_opcode(context, WA_INSTR_global_set);
                wa_emit_index(context, globalIndex);
                wa_emit_index(context, inOpds[0].index);
            }
            else
            {
                //NOTE generic emit code
                wa_emit_opcode(context, instr->op);

                for(int immIndex = 0; immIndex < immCount; immIndex++)
                {
                    wa_emit(context, &instr->imm[immIndex]);
                }

                for(u32 i = 0; i < inCount; i++)
                {
                    wa_emit_index(context, inOpds[i].index);
                }

                for(int opdIndex = 0; opdIndex < outCount; opdIndex++)
                {
                    u32 outIndex = wa_operand_stack_push_reg(context, out[opdIndex], instr);
                    wa_emit_index(context, outIndex);
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

        context.regCount = func->localCount;
        for(u32 localIndex = 0; localIndex < func->localCount; localIndex++)
        {
            context.regs[localIndex].refCount = 0;
            context.regs[localIndex].type = func->locals[localIndex];
        }

        context.currentFunction = func;

        wa_compile_expression(&context, func->type, func, func->instructions);

        func->codeLen = context.codeLen;
        func->code = oc_arena_push_array(arena, wa_code, context.codeLen);
        memcpy(func->code, context.code, context.codeLen * sizeof(wa_code));

        if(context.regCount >= WA_MAX_SLOT_COUNT)
        {
            wa_compile_error(&context, 0, "too many register slots (%i, max is %i).", context.regCount, WA_MAX_SLOT_COUNT);
        }
        func->maxRegCount = context.regCount;

        //NOTE: emit wasm to warm mappings
        {
            u64 codeIndex = func->codeLen - 1;
            oc_list_for_reverse(func->instructions, instr, wa_instr, listElt)
            {
                if(instr->codeIndex)
                {
                    codeIndex = instr->codeIndex;
                }
                wa_wasm_to_warm_loc_push(module, funcIndex, codeIndex, instr);
            }
        }
    }

    for(u32 globalIndex = module->globalImportCount; globalIndex < module->globalCount; globalIndex++)
    {
        wa_global_desc* global = &module->globals[globalIndex];

        wa_build_context_clear(&context);
        context.regCount = 0;

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
            context.regCount = 0;

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
                context.regCount = 0;

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
            context.regCount = 0;

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

    module->arena = arena;

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
    wa_parse_dwarf(&parser, module);
    wa_parse_names(&parser, module);

    wa_parse_types(&parser, module);
    wa_parse_imports(&parser, module);
    wa_parse_functions(&parser, module);
    wa_parse_globals(&parser, module);
    wa_parse_tables(&parser, module);
    wa_parse_memories(&parser, module);
    wa_parse_exports(&parser, module);
    wa_parse_start(&parser, module);
    wa_parse_elements(&parser, module);
    wa_parse_data_count(&parser, module);
    wa_parse_code(&parser, module);
    wa_parse_data(&parser, module);

    if(oc_list_empty(module->errors))
    {
        //TODO: tune this
        module->warmToWasmMapLen = 4096;
        module->warmToWasmMap = oc_arena_push_array(arena, oc_list, 4096);
        memset(module->warmToWasmMap, 0, 4096 * sizeof(oc_list));

        module->wasmToWarmMapLen = 4096;
        module->wasmToWarmMap = oc_arena_push_array(arena, oc_list, 4096);
        memset(module->wasmToWarmMap, 0, 4096 * sizeof(oc_list));

        wa_compile_code(arena, module);
    }

    return (module);
}

wa_status wa_module_status(wa_module* module)
{
    if(oc_list_empty(module->errors))
    {
        return WA_OK;
    }
    else
    {
        wa_module_error* error = oc_list_last_entry(module->errors, wa_module_error, moduleElt);
        return error->status;
    }
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

void wa_ast_print_listing(oc_arena* arena, wa_module* module, oc_str8_list* listing, oc_str8_list* raw, wa_ast* ast, oc_str8 contents, u32 indent)
{
    oc_str8_list line = { 0 };
    oc_str8_list_pushf(arena, &line, "0x%08llx  ", ast->loc.start - module->toc.code.offset);

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
        wa_ast_print_listing(arena, module, listing, raw, child, contents, indent + 1);
    }
}

void wa_ast_print(wa_module* module, wa_ast* ast, oc_str8 contents)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8_list listing = { 0 };
    oc_str8_list raw = { 0 };
    wa_ast_print_listing(scratch.arena, module, &listing, &raw, ast, contents, 0);

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
        eltA = oc_list_next_entry(eltA, oc_str8_elt, listElt),
        eltB = oc_list_next_entry(eltB, oc_str8_elt, listElt))
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
        OC_ASSERT(c->opcode < WA_INSTR_COUNT);

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

wa_status wa_instance_status(wa_instance* instance)
{
    return instance->status;
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

wa_status wa_instance_link_imports(wa_instance* instance, wa_instance_options* options)
{
    wa_module* module = instance->module;
    //NOTE: link imports
    u32 funcIndex = 0;
    u32 globalIndex = 0;
    u32 tableIndex = 0;
    u32 memIndex = 0;

    for(u32 importIndex = 0; importIndex < module->importCount; importIndex++)
    {
        wa_import* import = &module->imports[importIndex];

        for(u32 packageIndex = 0; packageIndex < options->packageCount; packageIndex++)
        {
            wa_import_package* package = &options->importPackages[packageIndex];
            if(!oc_str8_cmp(package->name, import->moduleName))
            {
                oc_list_for(package->bindings, elt, wa_import_package_elt, listElt)
                {
                    wa_import_binding* binding = &elt->binding;
                    if(!oc_str8_cmp(binding->name, import->importName))
                    {
                        switch(import->kind)
                        {
                            case WA_IMPORT_GLOBAL:
                            {
                                if(binding->kind != WA_BINDING_WASM_GLOBAL
                                   && binding->kind != WA_BINDING_HOST_GLOBAL)
                                {
                                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                }

                                wa_global_desc* globalDesc = &module->globals[globalIndex];
                                wa_global* global = 0;

                                if(binding->kind == WA_BINDING_WASM_GLOBAL)
                                {
                                    global = binding->instance->globals[binding->wasmGlobal];
                                }
                                else
                                {
                                    global = binding->hostGlobal;
                                }

                                if(globalDesc->type != global->type || globalDesc->mut != global->mut)
                                {
                                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                }
                                instance->globals[globalIndex] = global;
                            }
                            break;

                            case WA_IMPORT_FUNCTION:
                            {
                                if(binding->kind != WA_BINDING_WASM_FUNCTION
                                   && binding->kind != WA_BINDING_HOST_FUNCTION)
                                {
                                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                }

                                wa_func* importFunc = &instance->functions[funcIndex];
                                wa_func_type* importType = importFunc->type;

                                wa_func_type* boundType = 0;
                                if(binding->kind == WA_BINDING_WASM_FUNCTION)
                                {
                                    boundType = binding->instance->functions[binding->wasmFunction].type;
                                }
                                else
                                {
                                    boundType = &binding->hostFunction.type;
                                }

                                if(importType->paramCount != boundType->paramCount
                                   || importType->returnCount != boundType->returnCount)
                                {
                                    //log error to module
                                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                }
                                for(u32 paramIndex = 0; paramIndex < importType->paramCount; paramIndex++)
                                {
                                    if(importType->params[paramIndex] != boundType->params[paramIndex])
                                    {
                                        return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                    }
                                }
                                for(u32 returnIndex = 0; returnIndex < importType->returnCount; returnIndex++)
                                {
                                    if(importType->returns[returnIndex] != boundType->returns[returnIndex])
                                    {
                                        return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                    }
                                }

                                if(binding->kind == WA_BINDING_WASM_FUNCTION)
                                {
                                    importFunc->extInstance = binding->instance;
                                    importFunc->extIndex = binding->wasmFunction;
                                }
                                else
                                {
                                    importFunc->proc = binding->hostFunction.proc;
                                    importFunc->user = binding->hostFunction.userData;
                                }
                            }
                            break;

                            case WA_IMPORT_MEMORY:
                            {
                                wa_memory* memory = 0;
                                if(binding->kind == WA_BINDING_WASM_MEMORY)
                                {
                                    memory = binding->instance->memories[binding->wasmMemory];
                                }
                                else if(binding->kind == WA_BINDING_HOST_MEMORY)
                                {
                                    memory = binding->hostMemory;
                                }
                                else
                                {
                                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                }

                                if(!wa_check_limits_match(&memory->limits, &module->memories[memIndex]))
                                {
                                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                }

                                instance->memories[memIndex] = memory;
                            }
                            break;

                            case WA_IMPORT_TABLE:
                            {
                                wa_table* table = 0;
                                if(binding->kind == WA_BINDING_WASM_TABLE)
                                {
                                    table = binding->instance->tables[binding->wasmTable];
                                }
                                else if(binding->kind == WA_BINDING_HOST_TABLE)
                                {
                                    table = binding->hostTable;
                                }
                                else
                                {
                                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                }

                                if(table->type != module->tables[tableIndex].type
                                   || !wa_check_limits_match(&table->limits, &module->tables[tableIndex].limits))
                                {
                                    return WA_FAIL_IMPORT_TYPE_MISMATCH;
                                }

                                instance->tables[tableIndex] = table;
                            }
                            break;
                        }
                    }
                }
            }
        }

        switch(import->kind)
        {
            case WA_IMPORT_GLOBAL:
                globalIndex++;
                break;
            case WA_IMPORT_FUNCTION:
                funcIndex++;
                break;
            case WA_IMPORT_TABLE:
                tableIndex++;
                break;
            case WA_IMPORT_MEMORY:
                memIndex++;
                break;
        }
    }

    //NOTE: check that all imports are satisfied
    //TODO: hoist in in loop
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

    return WA_OK;
}

wa_status wa_instance_interpret_expr(wa_instance* instance,
                                     wa_func* func,
                                     wa_func_type* type,
                                     wa_code* code,
                                     u32 argCount,
                                     wa_value* args,
                                     u32 retCount,
                                     wa_value* returns);

/*
wa_status wa_instance_invoke(wa_instance* instance,
                             wa_func* func,
                             u32 argCount,
                             wa_value* args,
                             u32 retCount,
                             wa_value* returns);
*/

wa_status wa_instance_initialize(wa_instance* instance)
{
    wa_module* module = instance->module;

    for(u64 globalIndex = module->globalImportCount; globalIndex < module->globalCount; globalIndex++)
    {
        wa_global_desc* global = &module->globals[globalIndex];
        if(global->code)
        {
            i64 t = 0x7f - (i64)global->type + 1;
            wa_func_type* exprType = (wa_func_type*)&WA_BLOCK_VALUE_TYPES[t];

            wa_status status = wa_instance_interpret_expr(instance, 0, exprType, global->code, 0, 0, 1, &instance->globals[globalIndex]->value);
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
            wa_status status = wa_instance_interpret_expr(instance, 0, exprType, element->code[exprIndex], 0, 0, 1, result);
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
                                                          0,
                                                          (wa_func_type*)&WA_BLOCK_VALUE_TYPES[1],
                                                          element->tableOffsetCode,
                                                          0, 0,
                                                          1, &offset);
            if(status != WA_OK)
            {
                //oc_log_error("Couldn't link instance: couldn't execute element offset expression.\n");
                return status;
            }
            if(offset.valI32 + element->initCount > table->limits.min || offset.valI32 + element->initCount < offset.valI32)
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
                                                          0,
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

            if(offset + seg->init.len > mem->limits.min * WA_PAGE_SIZE || offset + seg->init.len < offset)
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

        //TODO: later take an interpreter as input of instantiate?
        oc_arena_scope scratch = oc_scratch_begin();
        wa_interpreter* interpreter = wa_interpreter_create(scratch.arena);
        wa_status status = wa_interpreter_invoke(interpreter, instance, func, 0, 0, 0, 0);
        oc_scratch_end(scratch);

        if(status != WA_OK)
        {
            return status;
        }
    }
    return WA_OK;
}

wa_instance* wa_instance_create(oc_arena* arena, wa_module* module, wa_instance_options* options)
{
    wa_instance* instance = oc_arena_push_type(arena, wa_instance);
    memset(instance, 0, sizeof(wa_instance));

    instance->arena = arena;
    instance->module = module;

    //NOTE: allocate functions
    instance->functions = oc_arena_push_array(arena, wa_func, module->functionCount);
    memcpy(instance->functions, module->functions, module->functionCount * sizeof(wa_func));

    //NOTE: allocate globals
    instance->globals = oc_arena_push_array(arena, wa_global*, module->globalCount);
    memset(instance->globals, 0, module->globalImportCount * sizeof(wa_global*));

    for(u32 globalIndex = module->globalImportCount; globalIndex < module->globalCount; globalIndex++)
    {
        instance->globals[globalIndex] = oc_arena_push_type(arena, wa_global);
        instance->globals[globalIndex]->type = module->globals[globalIndex].type;
        instance->globals[globalIndex]->mut = module->globals[globalIndex].mut;
        memset(&instance->globals[globalIndex]->value, 0, sizeof(wa_value));
    }

    //NOTE: allocate tables
    instance->tables = oc_arena_push_array(arena, wa_table*, module->tableCount);
    memset(instance->tables, 0, module->tableImportCount * sizeof(wa_table*));

    for(u32 tableIndex = module->tableImportCount; tableIndex < module->tableCount; tableIndex++)
    {
        wa_table_type* desc = &module->tables[tableIndex];
        wa_table* table = oc_arena_push_type(arena, wa_table);

        table->type = desc->type;
        table->limits = desc->limits;
        table->contents = oc_arena_push_array(arena, wa_value, table->limits.min);
        memset(table->contents, 0, table->limits.min * sizeof(wa_value));

        instance->tables[tableIndex] = table;
    }

    //NOTE: allocate elements
    //TODO: we could refer to passive elements directly in the module?
    instance->elements = oc_arena_push_array(arena, wa_element, module->elementCount);
    memcpy(instance->elements, module->elements, module->elementCount * sizeof(wa_element));
    for(u32 eltIndex = 0; eltIndex < module->elementCount; eltIndex++)
    {
        wa_element* elt = &instance->elements[eltIndex];
        elt->refs = oc_arena_push_array(arena, wa_value, elt->initCount);
        memset(elt->refs, 0, elt->initCount * sizeof(wa_element));
    }

    //NOTE: allocate memories
    oc_base_allocator* allocator = oc_base_allocator_default();

    instance->memories = oc_arena_push_array(arena, wa_memory*, module->memoryCount);
    memset(instance->memories, 0, module->memoryImportCount * sizeof(wa_memory*));

    for(u32 memIndex = module->memoryImportCount; memIndex < module->memoryCount; memIndex++)
    {
        wa_limits* limits = &module->memories[memIndex];
        wa_memory* mem = oc_arena_push_type(arena, wa_memory);

        ////////////////////////////////////////////////////////
        //TODO: validate limit before that
        ////////////////////////////////////////////////////////
        mem->limits.kind = limits->kind;
        mem->limits.min = limits->min;
        mem->limits.max = (limits->kind == WA_LIMIT_MIN)
                            ? UINT32_MAX / WA_PAGE_SIZE
                            : limits->max;

        mem->ptr = oc_base_reserve(allocator, mem->limits.max * WA_PAGE_SIZE);
        oc_base_commit(allocator, mem->ptr, mem->limits.min * WA_PAGE_SIZE);

        instance->memories[memIndex] = mem;
    }

    //NOTE: allocate data
    instance->data = oc_arena_push_array(arena, wa_data_segment, module->dataCount);
    memcpy(instance->data, module->data, module->dataCount * sizeof(wa_data_segment));

    //NOTE: link
    instance->status = wa_instance_link_imports(instance, options);
    if(instance->status != WA_OK)
    {
        return instance;
    }

    //NOTE: initialize
    instance->status = wa_instance_initialize(instance);

    return (instance);
}

wa_import_package wa_instance_exports(oc_arena* arena, wa_instance* instance, oc_str8 name)
{
    wa_module* module = instance->module;

    wa_import_package package = {
        .name = oc_str8_push_copy(arena, name),
        .bindingCount = module->exportCount,
    };

    for(u32 exportIndex = 0; exportIndex < module->exportCount; exportIndex++)
    {
        wa_export* export = &module->exports[exportIndex];
        wa_import_binding binding = { 0 };

        binding.name = export->name;
        binding.instance = instance;

        switch(export->kind)
        {
            case WA_EXPORT_GLOBAL:
            {
                binding.kind = WA_BINDING_WASM_GLOBAL;
                binding.wasmGlobal = export->index;
            }
            break;
            case WA_EXPORT_FUNCTION:
            {
                binding.kind = WA_BINDING_WASM_FUNCTION;
                binding.wasmFunction = export->index;
            }
            break;
            case WA_EXPORT_TABLE:
            {
                binding.kind = WA_BINDING_WASM_TABLE;
                binding.wasmTable = export->index;
            }
            break;
            case WA_EXPORT_MEMORY:
            {
                binding.kind = WA_BINDING_WASM_MEMORY;
                binding.wasmMemory = export->index;
            }
            break;
        }

        wa_import_package_push_binding(arena, &package, &binding);
    }

    return (package);
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

wa_global* wa_instance_find_global(wa_instance* instance, oc_str8 name)
{
    wa_module* module = instance->module;

    wa_global* global = 0;
    for(u32 exportIndex = 0; exportIndex < module->exportCount; exportIndex++)
    {
        wa_export* export = &module->exports[exportIndex];
        if(export->kind == WA_EXPORT_GLOBAL && !oc_str8_cmp(export->name, name))
        {
            global = instance->globals[export->index];
            break;
        }
    }
    return (global);
}

typedef struct wa_control
{
    wa_instance* instance;
    wa_func* func;
    wa_code* returnPC;
    wa_value* returnFrame;
} wa_control;

enum
{
    WA_CONTROL_STACK_SIZE = 256,
    WA_LOCALS_BUFFER_SIZE = WA_MAX_SLOT_COUNT * 256,
};

typedef struct wa_interpreter
{
    wa_instance* instance;
    wa_code* code;

    u32 argCount;
    wa_value* args;
    u32 retCount;
    wa_value* returns;

    wa_control controlStack[WA_CONTROL_STACK_SIZE];
    u32 controlStackTop;

    wa_value* localsBuffer;
    wa_value* locals;
    wa_code* pc;

    _Atomic(bool) suspend;
    bool terminated;

    oc_arena arena;
    oc_list breakpoints;
    oc_list breakpointFreeList;

} wa_interpreter;

wa_status wa_interpreter_init(wa_interpreter* interpreter,
                              wa_instance* instance,
                              wa_func* func,
                              wa_func_type* type,
                              wa_code* code,
                              u32 argCount,
                              wa_value* args,
                              u32 retCount,
                              wa_value* returns)
{
    interpreter->instance = instance;
    interpreter->code = code;
    interpreter->argCount = argCount;
    interpreter->args = args;
    interpreter->retCount = retCount;
    interpreter->returns = returns;
    interpreter->pc = code;
    interpreter->controlStack[0] = (wa_control){
        .instance = instance,
        .func = func,
    };
    interpreter->controlStackTop = 0;

    interpreter->terminated = false;

    interpreter->locals = interpreter->localsBuffer;
    memcpy(interpreter->locals, args, argCount * sizeof(wa_value));

    return WA_OK;
}

wa_status wa_interpreter_run(wa_interpreter* interpreter, bool step)
{
    if(interpreter->terminated)
    {
        return WA_TRAP_TERMINATED;
    }
    interpreter->suspend = false;

    wa_instance* instance = interpreter->instance;
    wa_memory* memory = 0;
    char* memPtr = 0;
    if(instance->memories)
    {
        memory = instance->memories[0];
        if(memory)
        {
            memPtr = memory->ptr;
        }
    }

    u32 localsIndex = interpreter->locals - interpreter->localsBuffer;
    if(localsIndex + WA_MAX_SLOT_COUNT >= WA_LOCALS_BUFFER_SIZE)
    {
        return (WA_TRAP_STACK_OVERFLOW);
    }

    while(!interpreter->suspend)
    {
        wa_instr_op opcode = interpreter->pc->opcode;
        interpreter->pc++;

        switch(opcode)
        {

#define I0 interpreter->pc[0]
#define I1 interpreter->pc[1]
#define I2 interpreter->pc[2]
#define I3 interpreter->pc[3]

#define L0 interpreter->locals[interpreter->pc[0].valI32]
#define L1 interpreter->locals[interpreter->pc[1].valI32]
#define L2 interpreter->locals[interpreter->pc[2].valI32]
#define L3 interpreter->locals[interpreter->pc[3].valI32]
#define L4 interpreter->locals[interpreter->pc[4].valI32]

#define G0 interpreter->instance->globals[interpreter->pc[0].valI32]->value
#define G1 interpreter->instance->globals[interpreter->pc[1].valI32]->value

            case WA_INSTR_breakpoint:
            {
                interpreter->pc--;
                return WA_TRAP_BREAKPOINT;
            }
            break;

            case WA_INSTR_unreachable:
            {
                return WA_TRAP_UNREACHABLE;
            }
            break;

            case WA_INSTR_i32_const:
            {
                L1.valI32 = I0.valI32;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_const:
            {
                L1.valI64 = I0.valI64;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_const:
            {
                L1.valF32 = I0.valF32;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_const:
            {
                L1.valF64 = I0.valF64;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_move:
            {
                memcpy(&L1, &L0, sizeof(wa_value));
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_global_get:
            {
                memcpy(&L1, &G0, sizeof(wa_value));
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_global_set:
            {
                memcpy(&G0, &L1, sizeof(wa_value));
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_select:
            {
                if(L2.valI32)
                {
                    memcpy(&L3, &L0, sizeof(u64));
                }
                else
                {
                    memcpy(&L3, &L1, sizeof(u64));
                }
                interpreter->pc += 4;
            }
            break;

#define WA_CHECK_READ_ACCESS(t)                                                                  \
    u32 offset = I0.memArg.offset + (u32)L1.valI32;                                              \
    if(offset < I0.memArg.offset                                                                 \
       || offset + sizeof(t) > memory->limits.min * WA_PAGE_SIZE || offset + sizeof(t) < offset) \
    {                                                                                            \
        /*OC_ASSERT(0, "read out of bounds");*/                                                  \
        return WA_TRAP_MEMORY_OUT_OF_BOUNDS;                                                     \
    }

            case WA_INSTR_i32_load:
            {
                WA_CHECK_READ_ACCESS(i32);
                L2.valI32 = *(i32*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_load:
            {
                WA_CHECK_READ_ACCESS(i64);
                L2.valI64 = *(i64*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_load:
            {
                WA_CHECK_READ_ACCESS(f32);
                L2.valF32 = *(f32*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_load:
            {
                WA_CHECK_READ_ACCESS(f64);
                L2.valF64 = *(f64*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_load8_s:
            {
                WA_CHECK_READ_ACCESS(u8);
                L2.valI32 = (i32) * (i8*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_load8_u:
            {
                WA_CHECK_READ_ACCESS(u8);
                *(u32*)&L2.valI32 = (u32) * (u8*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_load16_s:
            {
                WA_CHECK_READ_ACCESS(u16);
                L2.valI32 = (i32) * (i16*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_load16_u:
            {
                WA_CHECK_READ_ACCESS(u16);
                *(u32*)&L2.valI32 = (u32) * (u16*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_load8_s:
            {
                WA_CHECK_READ_ACCESS(u8);
                L2.valI64 = (i64) * (i8*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_load8_u:
            {
                WA_CHECK_READ_ACCESS(u8);
                *(u32*)&L2.valI64 = (u64) * (u8*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_load16_s:
            {
                WA_CHECK_READ_ACCESS(u16);
                L2.valI64 = (i64) * (i16*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_load16_u:
            {
                WA_CHECK_READ_ACCESS(u16);
                *(u32*)&L2.valI64 = (u64) * (u16*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_load32_s:
            {
                WA_CHECK_READ_ACCESS(u32);
                L2.valI64 = (i64) * (i32*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_load32_u:
            {
                WA_CHECK_READ_ACCESS(u32);
                *(u32*)&L2.valI64 = (u64) * (u32*)&memPtr[I0.memArg.offset + (u32)L1.valI32];
                interpreter->pc += 3;
            }
            break;

#define WA_CHECK_WRITE_ACCESS(t)                                                                 \
    u32 offset = I0.memArg.offset + (u32)L1.valI32;                                              \
    if(offset < I0.memArg.offset                                                                 \
       || offset + sizeof(t) > memory->limits.min * WA_PAGE_SIZE || offset + sizeof(t) < offset) \
    {                                                                                            \
        /*OC_ASSERT(0, "write out of bounds");*/                                                 \
        return WA_TRAP_MEMORY_OUT_OF_BOUNDS;                                                     \
    }

            case WA_INSTR_i32_store:
            {
                WA_CHECK_WRITE_ACCESS(u32);
                *(i32*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = L2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_store:
            {
                WA_CHECK_WRITE_ACCESS(u64);
                *(i64*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = L2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_store:
            {
                WA_CHECK_WRITE_ACCESS(f32);
                *(f32*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = L2.valF32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_store:
            {
                WA_CHECK_WRITE_ACCESS(f64);
                *(f64*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = L2.valF64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_store8:
            {
                WA_CHECK_WRITE_ACCESS(u8);
                *(u8*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = *(u8*)&L2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_store16:
            {
                WA_CHECK_WRITE_ACCESS(u16);
                *(u16*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = *(u16*)&L2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_store8:
            {
                WA_CHECK_WRITE_ACCESS(u8);
                *(u8*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = *(u8*)&L2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_store16:
            {
                WA_CHECK_WRITE_ACCESS(u16);
                *(u16*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = *(u16*)&L2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_store32:
            {
                WA_CHECK_WRITE_ACCESS(u32);
                *(u32*)&memPtr[I0.memArg.offset + (u32)L1.valI32] = *(u32*)&L2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_jump:
            {
                interpreter->pc += I0.valI64;
            }
            break;

            case WA_INSTR_jump_if_zero:
            {
                if(L1.valI32 == 0)
                {
                    interpreter->pc += I0.valI64;
                }
                else
                {
                    interpreter->pc += 2;
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

                interpreter->pc += interpreter->pc[2 + index].valI64;
            }
            break;

            case WA_INSTR_call:
            {
                wa_func* callee = &instance->functions[I0.valI64];
                i64 maxUsedSlot = I1.valI64;

                wa_instance* calleeInstance = instance;

                while(callee->extInstance)
                {
                    calleeInstance = callee->extInstance;
                    callee = &calleeInstance->functions[callee->extIndex];
                }

                if(callee->code)
                {
                    interpreter->controlStackTop++;
                    if(interpreter->controlStackTop >= WA_CONTROL_STACK_SIZE)
                    {
                        return (WA_TRAP_STACK_OVERFLOW);
                    }

                    interpreter->controlStack[interpreter->controlStackTop] = (wa_control){
                        .instance = calleeInstance,
                        .func = callee,
                        .returnPC = interpreter->pc + 2,
                        .returnFrame = interpreter->locals,
                    };

                    interpreter->locals += maxUsedSlot;
                    interpreter->pc = callee->code;
                    instance = calleeInstance;

                    if(interpreter->locals - interpreter->localsBuffer + WA_MAX_SLOT_COUNT >= WA_LOCALS_BUFFER_SIZE)
                    {
                        return (WA_TRAP_STACK_OVERFLOW);
                    }

                    if(instance->memories)
                    {
                        memory = instance->memories[0];
                        if(memory)
                        {
                            memPtr = memory->ptr;
                        }
                    }
                }
                else
                {
                    wa_value* saveLocals = interpreter->locals;
                    interpreter->locals += I1.valI64;
                    callee->proc(interpreter, interpreter->locals, interpreter->locals, callee->user);
                    interpreter->pc += 2;
                    interpreter->locals = saveLocals;
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

                if(index >= table->limits.min)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }

                wa_instance* calleeInstance = table->contents[index].refInstance;
                u32 funcIndex = table->contents[index].refIndex;

                if(calleeInstance == 0)
                {
                    return WA_TRAP_REF_NULL;
                }

                wa_func* callee = &calleeInstance->functions[funcIndex];
                wa_func_type* t1 = callee->type;
                wa_func_type* t2 = &instance->module->types[typeIndex];

                if(!wa_check_function_type(t1, t2))
                {
                    return (WA_TRAP_INDIRECT_CALL_TYPE_MISMATCH);
                }

                while(callee->extInstance)
                {
                    calleeInstance = callee->extInstance;
                    callee = &calleeInstance->functions[callee->extIndex];
                }

                if(callee->code)
                {
                    interpreter->controlStackTop++;
                    if(interpreter->controlStackTop >= WA_CONTROL_STACK_SIZE)
                    {
                        return (WA_TRAP_STACK_OVERFLOW);
                    }

                    interpreter->controlStack[interpreter->controlStackTop] = (wa_control){
                        .instance = calleeInstance,
                        .func = callee,
                        .returnPC = interpreter->pc + 4,
                        .returnFrame = interpreter->locals,
                    };

                    interpreter->locals += maxUsedSlot;
                    interpreter->pc = callee->code;
                    instance = calleeInstance;

                    if(interpreter->locals - interpreter->localsBuffer + WA_MAX_SLOT_COUNT >= WA_LOCALS_BUFFER_SIZE)
                    {
                        return (WA_TRAP_STACK_OVERFLOW);
                    }

                    if(instance->memories)
                    {
                        memory = instance->memories[0];
                        if(memory)
                        {
                            memPtr = memory->ptr;
                        }
                    }
                }
                else
                {
                    wa_value* saveLocals = interpreter->locals;
                    interpreter->locals += maxUsedSlot;
                    callee->proc(interpreter, interpreter->locals, interpreter->locals, callee->user);
                    interpreter->pc += 4;
                    interpreter->locals = saveLocals;
                }
            }
            break;

            case WA_INSTR_return:
            {
                if(!interpreter->controlStackTop)
                {
                    goto end;
                }

                wa_control control = interpreter->controlStack[interpreter->controlStackTop];

                interpreter->locals = control.returnFrame;
                interpreter->pc = control.returnPC;

                interpreter->controlStackTop--;

                instance = interpreter->controlStack[interpreter->controlStackTop].instance;
                if(instance->memories)
                {
                    memory = instance->memories[0];
                    if(memory)
                    {
                        memPtr = memory->ptr;
                    }
                }
            }
            break;

            case WA_INSTR_ref_null:
            {
                L0.refInstance = 0;
                L0.refIndex = 0;
                interpreter->pc += 1;
            }
            break;

            case WA_INSTR_ref_is_null:
            {
                L1.valI32 = (L0.refInstance == 0) ? 1 : 0;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_ref_func:
            {
                L1.refInstance = instance,
                L1.refIndex = I0.valI64;
                interpreter->pc += 2;
            }
            break;

#define OPD1 L0
#define OPD2 L1
#define BRES L2
#define URES L1

            case WA_INSTR_i32_add:
            {
                BRES.valI32 = OPD1.valI32 + OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_sub:
            {
                BRES.valI32 = OPD1.valI32 - OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_mul:
            {
                BRES.valI32 = OPD1.valI32 * OPD2.valI32;
                interpreter->pc += 3;
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
                    BRES.valI32 = OPD1.valI32 / OPD2.valI32;
                }
                interpreter->pc += 3;
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
                    *(u32*)&BRES.valI32 = *(u32*)&OPD1.valI32 / *(u32*)&OPD2.valI32;
                }
                interpreter->pc += 3;
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
                    BRES.valI32 = 0;
                }
                else
                {
                    BRES.valI32 = OPD1.valI32 % OPD2.valI32;
                }
                interpreter->pc += 3;
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
                    *(u32*)&BRES.valI32 = *(u32*)&OPD1.valI32 % *(u32*)&OPD2.valI32;
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_and:
            {
                BRES.valI32 = OPD1.valI32 & OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_or:
            {
                BRES.valI32 = OPD1.valI32 | OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_xor:
            {
                BRES.valI32 = OPD1.valI32 ^ OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_shl:
            {
                BRES.valI32 = OPD1.valI32 << OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_shr_s:
            {
                BRES.valI32 = OPD1.valI32 >> OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_shr_u:
            {
                *(u32*)&BRES.valI32 = *(u32*)&OPD1.valI32 >> *(u32*)&OPD2.valI32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_rotr:
            {
                u32 n = *(u32*)&OPD1.valI32;
                u32 r = *(u32*)&OPD2.valI32;
                *(u32*)&BRES.valI32 = (n >> r) | (n << (32 - r));
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_rotl:
            {
                u32 n = *(u32*)&OPD1.valI32;
                u32 r = *(u32*)&OPD2.valI32;
                *(u32*)&BRES.valI32 = (n << r) | (n >> (32 - r));
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_clz:
            {
                if(OPD1.valI32 == 0)
                {
                    URES.valI32 = 32;
                }
                else
                {
                    URES.valI32 = __builtin_clz(*(u32*)&OPD1.valI32);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_ctz:
            {
                if(OPD1.valI32 == 0)
                {
                    URES.valI32 = 32;
                }
                else
                {
                    URES.valI32 = __builtin_ctz(*(u32*)&OPD1.valI32);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_popcnt:
            {
                URES.valI32 = __builtin_popcount(*(u32*)&OPD1.valI32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_extend8_s:
            {
                URES.valI32 = (i32)(i8)(OPD1.valI32 & 0xff);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_extend16_s:
            {
                URES.valI32 = (i32)(i16)(OPD1.valI32 & 0xffff);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_eqz:
            {
                URES.valI32 = (OPD1.valI32 == 0) ? 1 : 0;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_eq:
            {
                BRES.valI32 = (OPD1.valI32 == OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_ne:
            {
                BRES.valI32 = (OPD1.valI32 != OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_lt_s:
            {
                BRES.valI32 = (OPD1.valI32 < OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_lt_u:
            {
                BRES.valI32 = (*(u32*)&OPD1.valI32 < *(u32*)&OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_le_s:
            {
                BRES.valI32 = (OPD1.valI32 <= OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_le_u:
            {
                BRES.valI32 = (*(u32*)&OPD1.valI32 <= *(u32*)&OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_gt_s:
            {
                BRES.valI32 = (OPD1.valI32 > OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_gt_u:
            {
                BRES.valI32 = (*(u32*)&OPD1.valI32 > *(u32*)&OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_ge_s:
            {
                BRES.valI32 = (OPD1.valI32 >= OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_ge_u:
            {
                BRES.valI32 = (*(u32*)&OPD1.valI32 >= *(u32*)&OPD2.valI32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_add:
            {
                BRES.valI64 = OPD1.valI64 + OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_sub:
            {
                BRES.valI64 = OPD1.valI64 - OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_mul:
            {
                BRES.valI64 = OPD1.valI64 * OPD2.valI64;
                interpreter->pc += 3;
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
                    BRES.valI64 = OPD1.valI64 / OPD2.valI64;
                }
                interpreter->pc += 3;
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
                    *(u64*)&BRES.valI64 = *(u64*)&OPD1.valI64 / *(u64*)&OPD2.valI64;
                }
                interpreter->pc += 3;
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
                    BRES.valI64 = 0;
                }
                else
                {
                    BRES.valI64 = OPD1.valI64 % OPD2.valI64;
                }
                interpreter->pc += 3;
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
                    *(u64*)&BRES.valI64 = *(u64*)&OPD1.valI64 % *(u64*)&OPD2.valI64;
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_and:
            {
                BRES.valI64 = OPD1.valI64 & OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_or:
            {
                BRES.valI64 = OPD1.valI64 | OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_xor:
            {
                BRES.valI64 = OPD1.valI64 ^ OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_shl:
            {
                BRES.valI64 = OPD1.valI64 << OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_shr_s:
            {
                BRES.valI64 = OPD1.valI64 >> OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_shr_u:
            {
                *(u64*)&BRES.valI64 = *(u64*)&OPD1.valI64 >> *(u64*)&OPD2.valI64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_rotr:
            {
                u64 n = *(u64*)&OPD1.valI64;
                u64 r = *(u64*)&OPD2.valI64;
                *(u64*)&BRES.valI64 = (n >> r) | (n << (64 - r));
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_rotl:
            {
                u64 n = *(u64*)&OPD1.valI64;
                u64 r = *(u64*)&OPD2.valI64;
                *(u64*)&BRES.valI64 = (n << r) | (n >> (64 - r));
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_clz:
            {
                if(OPD1.valI64 == 0)
                {
                    URES.valI64 = 64;
                }
                else
                {
                    URES.valI64 = __builtin_clzl(*(u64*)&OPD1.valI64);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_ctz:
            {
                if(OPD1.valI64 == 0)
                {
                    URES.valI64 = 64;
                }
                else
                {
                    URES.valI64 = __builtin_ctzl(*(u64*)&OPD1.valI64);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_popcnt:
            {
                URES.valI64 = __builtin_popcountl(*(u64*)&OPD1.valI64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_extend8_s:
            {
                URES.valI64 = (i64)(i8)(OPD1.valI64 & 0xff);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_extend16_s:
            {
                URES.valI64 = (i64)(i16)(OPD1.valI64 & 0xffff);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_extend32_s:
            {
                URES.valI64 = (i64)(i32)(OPD1.valI64 & 0xffffffff);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_eqz:
            {
                URES.valI32 = (OPD1.valI64 == 0) ? 1 : 0;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_eq:
            {
                BRES.valI32 = (OPD1.valI64 == OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_ne:
            {
                BRES.valI32 = (OPD1.valI64 != OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_lt_s:
            {
                BRES.valI32 = (OPD1.valI64 < OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_lt_u:
            {
                BRES.valI32 = (*(u64*)&OPD1.valI64 < *(u64*)&OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_le_s:
            {
                BRES.valI32 = (OPD1.valI64 <= OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_le_u:
            {
                BRES.valI32 = (*(u64*)&OPD1.valI64 <= *(u64*)&OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_gt_s:
            {
                BRES.valI32 = (OPD1.valI64 > OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_gt_u:
            {
                BRES.valI32 = (*(u64*)&OPD1.valI64 > *(u64*)&OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_ge_s:
            {
                BRES.valI32 = (OPD1.valI64 >= OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i64_ge_u:
            {
                BRES.valI32 = (*(u64*)&OPD1.valI64 >= *(u64*)&OPD2.valI64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_eq:
            {
                BRES.valI32 = (OPD1.valF32 == OPD2.valF32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f32_ne:
            {
                BRES.valI32 = (OPD1.valF32 != OPD2.valF32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f32_lt:
            {
                BRES.valI32 = (OPD1.valF32 < OPD2.valF32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f32_gt:
            {
                BRES.valI32 = (OPD1.valF32 > OPD2.valF32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f32_le:
            {
                BRES.valI32 = (OPD1.valF32 <= OPD2.valF32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f32_ge:
            {
                BRES.valI32 = (OPD1.valF32 >= OPD2.valF32) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_eq:
            {
                BRES.valI32 = (OPD1.valF64 == OPD2.valF64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f64_ne:
            {
                BRES.valI32 = (OPD1.valF64 != OPD2.valF64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f64_lt:
            {
                BRES.valI32 = (OPD1.valF64 < OPD2.valF64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f64_gt:
            {
                BRES.valI32 = (OPD1.valF64 > OPD2.valF64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f64_le:
            {
                BRES.valI32 = (OPD1.valF64 <= OPD2.valF64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;
            case WA_INSTR_f64_ge:
            {
                BRES.valI32 = (OPD1.valF64 >= OPD2.valF64) ? 1 : 0;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_abs:
            {
                URES.valF32 = fabsf(OPD1.valF32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_neg:
            {
                URES.valF32 = -OPD1.valF32;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_ceil:
            {
                URES.valF32 = ceilf(OPD1.valF32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_floor:
            {
                URES.valF32 = floorf(OPD1.valF32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_trunc:
            {
                URES.valF32 = truncf(OPD1.valF32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_nearest:
            {
                URES.valF32 = rintf(OPD1.valF32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_sqrt:
            {
                URES.valF32 = sqrtf(OPD1.valF32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_add:
            {
                BRES.valF32 = OPD1.valF32 + OPD2.valF32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_sub:
            {
                BRES.valF32 = OPD1.valF32 - OPD2.valF32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_mul:
            {
                BRES.valF32 = OPD1.valF32 * OPD2.valF32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_div:
            {
                BRES.valF32 = OPD1.valF32 / OPD2.valF32;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_min:
            {
                f32 a = OPD1.valF32;
                f32 b = OPD2.valF32;
                if(isnan(a) || isnan(b))
                {
                    u32 u = 0x7fc00000;
                    memcpy(&BRES.valF32, &u, sizeof(f32));
                }
                else if(a == 0 && b == 0)
                {
                    BRES.valF32 = signbit(a) ? a : b;
                }
                else
                {
                    BRES.valF32 = oc_min(a, b);
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_max:
            {
                f32 a = OPD1.valF32;
                f32 b = OPD2.valF32;
                if(isnan(a) || isnan(b))
                {
                    u32 u = 0x7fc00000;
                    memcpy(&BRES.valF32, &u, sizeof(f32));
                }
                else if(a == 0 && b == 0)
                {
                    BRES.valF32 = signbit(a) ? b : a;
                }

                else
                {
                    BRES.valF32 = oc_max(a, b);
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f32_copysign:
            {
                BRES.valF32 = copysignf(OPD1.valF32, OPD2.valF32);
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_abs:
            {
                URES.valF64 = fabs(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_neg:
            {
                URES.valF64 = -OPD1.valF64;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_ceil:
            {
                URES.valF64 = ceil(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_floor:
            {
                URES.valF64 = floor(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_trunc:
            {
                URES.valF64 = trunc(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_nearest:
            {
                URES.valF64 = rint(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_sqrt:
            {
                URES.valF64 = sqrt(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_add:
            {
                BRES.valF64 = OPD1.valF64 + OPD2.valF64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_sub:
            {
                BRES.valF64 = OPD1.valF64 - OPD2.valF64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_mul:
            {
                BRES.valF64 = OPD1.valF64 * OPD2.valF64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_div:
            {
                BRES.valF64 = OPD1.valF64 / OPD2.valF64;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_min:
            {
                f64 a = OPD1.valF64;
                f64 b = OPD2.valF64;

                if(isnan(a) || isnan(b))
                {
                    u64 u = 0x7ff8000000000000;
                    memcpy(&BRES.valF64, &u, sizeof(f64));
                }
                else if(a == 0 && b == 0)
                {
                    BRES.valF64 = signbit(a) ? a : b;
                }
                else
                {
                    BRES.valF64 = oc_min(a, b);
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_max:
            {
                f64 a = OPD1.valF64;
                f64 b = OPD2.valF64;

                if(isnan(a) || isnan(b))
                {
                    u64 u = 0x7ff8000000000000;
                    memcpy(&BRES.valF64, &u, sizeof(f64));
                }
                else if(a == 0 && b == 0)
                {
                    BRES.valF64 = signbit(a) ? b : a;
                }
                else
                {
                    BRES.valF64 = oc_max(a, b);
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_f64_copysign:
            {
                BRES.valF64 = copysign(OPD1.valF64, OPD2.valF64);
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_i32_wrap_i64:
            {
                URES.valI32 = (OPD1.valI64 & 0x00000000ffffffff);
                interpreter->pc += 2;
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

                URES.valI32 = (i32)truncf(OPD1.valF32);
                interpreter->pc += 2;
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

                *(u32*)&URES.valI32 = (u32)truncf(OPD1.valF32);
                interpreter->pc += 2;
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

                URES.valI32 = (i32)trunc(OPD1.valF64);
                interpreter->pc += 2;
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

                *(u32*)&URES.valI32 = (u32)trunc(OPD1.valF64);
                interpreter->pc += 2;
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

                URES.valI64 = (i64)truncf(OPD1.valF32);
                interpreter->pc += 2;
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

                *(u64*)&URES.valI64 = (u64)truncf(OPD1.valF32);
                interpreter->pc += 2;
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

                URES.valI64 = (i64)trunc(OPD1.valF64);
                interpreter->pc += 2;
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

                *(u64*)&URES.valI64 = (u64)trunc(OPD1.valF64);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_convert_i32_s:
            {
                URES.valF32 = (f32)OPD1.valI32;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_convert_i32_u:
            {
                URES.valF32 = (f32) * (u32*)&OPD1.valI32;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_convert_i64_s:
            {
                URES.valF32 = (f32)OPD1.valI64;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_convert_i64_u:
            {
                URES.valF32 = (f32) * (u64*)&OPD1.valI64;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f32_demote_f64:
            {
                URES.valF32 = (f32)OPD1.valF64;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_convert_i32_s:
            {
                URES.valF64 = (f64)OPD1.valI32;
                interpreter->pc += 2;
            }
            break;
            case WA_INSTR_f64_convert_i32_u:
            {
                URES.valF64 = (f64) * (u32*)&OPD1.valI32;
                interpreter->pc += 2;
            }
            break;
            case WA_INSTR_f64_convert_i64_s:
            {
                URES.valF64 = (f64)OPD1.valI64;
                interpreter->pc += 2;
            }
            break;
            case WA_INSTR_f64_convert_i64_u:
            {
                URES.valF64 = (f64) * (u64*)&OPD1.valI64;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_f64_promote_f32:
            {
                URES.valF64 = (f64)OPD1.valF32;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_reinterpret_f32:
            {
                URES.valI32 = *(i32*)&OPD1.valF32;
                interpreter->pc += 2;
            }
            break;
            case WA_INSTR_i64_reinterpret_f64:
            {
                URES.valI64 = *(i64*)&OPD1.valF64;
                interpreter->pc += 2;
            }
            break;
            case WA_INSTR_f32_reinterpret_i32:
            {
                URES.valF32 = *(f32*)&OPD1.valI32;
                interpreter->pc += 2;
            }
            break;
            case WA_INSTR_f64_reinterpret_i64:
            {
                URES.valF64 = *(f64*)&OPD1.valI64;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_sat_f32_s:
            {
                if(isnan(OPD1.valF32))
                {
                    URES.valI32 = 0;
                }
                else if(OPD1.valF32 >= 2147483648.0f)
                {
                    URES.valI32 = INT32_MAX;
                }
                else if(OPD1.valF32 < -2147483648.0f)
                {
                    URES.valI32 = INT32_MIN;
                }
                else
                {
                    URES.valI32 = (i32)truncf(OPD1.valF32);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_sat_f32_u:
            {
                if(isnan(OPD1.valF32))
                {
                    URES.valI32 = 0;
                }
                else if(OPD1.valF32 >= 4294967296.0f)
                {
                    *(u32*)&URES.valI32 = 0xffffffff;
                }
                else if(OPD1.valF32 <= -1.0f)
                {
                    URES.valI32 = 0;
                }
                else
                {
                    *(u32*)&URES.valI32 = (u32)truncf(OPD1.valF32);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_sat_f64_s:
            {
                if(isnan(OPD1.valF64))
                {
                    URES.valI32 = 0;
                }
                else if(OPD1.valF64 >= 2147483648.0)
                {
                    URES.valI32 = INT32_MAX;
                }
                else if(OPD1.valF64 <= -2147483649.0)
                {
                    URES.valI32 = INT32_MIN;
                }
                else
                {
                    URES.valI32 = (i32)trunc(OPD1.valF64);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i32_trunc_sat_f64_u:
            {
                if(isnan(OPD1.valF64))
                {
                    URES.valI32 = 0;
                }
                else if(OPD1.valF64 >= 4294967296.0)
                {
                    *(u32*)&URES.valI32 = 0xffffffff;
                }
                else if(OPD1.valF64 <= -1.0)
                {
                    URES.valI32 = 0;
                }
                else
                {
                    *(u32*)&URES.valI32 = (u32)trunc(OPD1.valF64);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_sat_f32_s:
            {
                if(isnan(OPD1.valF32))
                {
                    URES.valI64 = 0;
                }
                else if(OPD1.valF32 >= 9223372036854775808.0f)
                {
                    URES.valI64 = INT64_MAX;
                }
                else if(OPD1.valF32 < -9223372036854775808.0f)
                {
                    URES.valI64 = INT64_MIN;
                }
                else
                {
                    URES.valI64 = (i64)truncf(OPD1.valF32);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_sat_f32_u:
            {
                if(isnan(OPD1.valF32))
                {
                    URES.valI64 = 0;
                }
                else if(OPD1.valF32 >= 18446744073709551616.0f)
                {
                    *(u64*)&URES.valI64 = 0xffffffffffffffffLLU;
                }
                else if(OPD1.valF32 <= -1.0f)
                {
                    URES.valI64 = 0;
                }
                else
                {
                    *(u64*)&URES.valI64 = (u64)truncf(OPD1.valF32);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_sat_f64_s:
            {
                if(isnan(OPD1.valF64))
                {
                    URES.valI64 = 0;
                }
                else if(OPD1.valF64 >= 9223372036854775808.0)
                {
                    URES.valI64 = INT64_MAX;
                }
                else if(OPD1.valF64 < -9223372036854775808.0)
                {
                    URES.valI64 = INT64_MIN;
                }
                else
                {
                    URES.valI64 = (i64)trunc(OPD1.valF64);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_trunc_sat_f64_u:
            {
                if(isnan(OPD1.valF64))
                {
                    URES.valI64 = 0;
                }
                else if(OPD1.valF64 >= 18446744073709551616.0)
                {
                    *(u64*)&URES.valI64 = 0xffffffffffffffffLLU;
                }
                else if(OPD1.valF64 <= -1.0)
                {
                    URES.valI64 = 0;
                }
                else
                {
                    *(u64*)&URES.valI64 = (u64)trunc(OPD1.valF64);
                }
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_extend_i32_s:
            {
                URES.valI64 = (i64)(i32)(OPD1.valI32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_i64_extend_i32_u:
            {
                URES.valI64 = *(u32*)&(OPD1.valI32);
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_memory_size:
            {
                wa_memory* mem = instance->memories[0];
                L0.valI32 = (i32)(mem->limits.min);
                interpreter->pc += 1;
            }
            break;

            case WA_INSTR_memory_grow:
            {
                wa_memory* mem = instance->memories[0];

                i32 res = -1;
                u32 n = *(u32*)&(L0.valI32);
                oc_base_allocator* allocator = oc_base_allocator_default();

                if(mem->limits.min + n <= mem->limits.max
                   && (mem->limits.min + n >= mem->limits.min))
                {
                    res = mem->limits.min;
                    oc_base_commit(allocator, mem->ptr + mem->limits.min * WA_PAGE_SIZE, n * WA_PAGE_SIZE);
                    mem->limits.min += n;
                }

                L1.valI32 = res;

                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_memory_fill:
            {
                wa_memory* mem = instance->memories[0];

                u32 d = *(u32*)&L0.valI32;
                i32 val = L1.valI32;
                u32 n = *(u32*)&L2.valI32;

                if(d + n > mem->limits.min * WA_PAGE_SIZE || d + n < d)
                {
                    //OC_ASSERT(0, "fill out of bounds");
                    return WA_TRAP_MEMORY_OUT_OF_BOUNDS;
                }
                else
                {
                    memset(mem->ptr + d, val, n);
                }
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_memory_copy:
            {
                wa_memory* mem = instance->memories[0];
                u32 d = *(u32*)&L0.valI32;
                u32 s = *(u32*)&L1.valI32;
                u32 n = *(u32*)&L2.valI32;

                if(s + n > mem->limits.min * WA_PAGE_SIZE || s + n < s
                   || d + n > mem->limits.min * WA_PAGE_SIZE || d + n < d)
                {
                    //OC_ASSERT(0, "copy out of bounds");
                    return WA_TRAP_MEMORY_OUT_OF_BOUNDS;
                }
                memmove(mem->ptr + d, mem->ptr + s, n);
                interpreter->pc += 3;
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
                   || d + n > mem->limits.min * WA_PAGE_SIZE || d + n < d)
                {
                    //OC_ASSERT(0, "memory init out of bounds");
                    return WA_TRAP_MEMORY_OUT_OF_BOUNDS;
                }
                memmove(mem->ptr + d, seg->init.ptr + s, n);
                interpreter->pc += 4;
            }
            break;

            case WA_INSTR_data_drop:
            {
                wa_data_segment* seg = &instance->data[I0.valI32];
                seg->init.len = 0;
                interpreter->pc += 1;
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
                   || d + n > table->limits.min || d + n < d)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }
                memmove(table->contents + d, elt->refs + s, n * sizeof(wa_value));
                interpreter->pc += 5;
            }
            break;

            case WA_INSTR_table_fill:
            {
                wa_table* table = instance->tables[I0.valI32];

                u32 d = *(u32*)&L1.valI32;
                wa_value val = L2;
                u32 n = *(u32*)&L3.valI32;

                if(d + n > table->limits.min || d + n < d)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }
                for(u32 i = 0; i < n; i++)
                {
                    table->contents[d + i] = val;
                }

                interpreter->pc += 4;
            }
            break;

            case WA_INSTR_table_copy:
            {
                wa_table* tx = instance->tables[I0.valI32];
                wa_table* ty = instance->tables[I1.valI32];

                u32 d = *(u32*)&L2.valI32;
                u32 s = *(u32*)&L3.valI32;
                u32 n = *(u32*)&L4.valI32;

                if(s + n > ty->limits.min || s + n < s
                   || d + n > tx->limits.min || d + n < d)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }
                memmove(tx->contents + d, ty->contents + s, n * sizeof(wa_value));
                interpreter->pc += 5;
            }
            break;

            case WA_INSTR_table_size:
            {
                wa_table* table = instance->tables[I0.valI32];
                L1.valI32 = table->limits.min;
                interpreter->pc += 2;
            }
            break;

            case WA_INSTR_table_grow:
            {
                wa_table* table = instance->tables[I0.valI32];
                wa_limits limits = table->limits;
                wa_value val = L1;
                u32 size = L2.valI32;

                i32 ret = -1;
                if((u64)limits.min + (u64)size <= UINT32_MAX
                   && (limits.kind != WA_LIMIT_MIN_MAX || limits.min + size <= limits.max))
                {
                    wa_value* contents = oc_arena_push_array(instance->arena, wa_value, limits.min + size);
                    memcpy(contents, table->contents, limits.min * sizeof(wa_value));
                    for(u32 i = 0; i < size; i++)
                    {
                        contents[limits.min + i] = val;
                    }
                    ret = limits.min;
                    table->limits.min += size;
                    table->contents = contents;
                }
                L3.valI32 = ret;
                interpreter->pc += 4;
            }
            break;

            case WA_INSTR_table_get:
            {
                wa_table* table = instance->tables[I0.valI32];
                u32 eltIndex = L1.valI32;

                if(eltIndex >= table->limits.min)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }
                L2 = table->contents[eltIndex];
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_table_set:
            {
                wa_table* table = instance->tables[I0.valI32];
                u32 eltIndex = L1.valI32;
                wa_value val = L2;

                if(eltIndex >= table->limits.min)
                {
                    return WA_TRAP_TABLE_OUT_OF_BOUNDS;
                }
                table->contents[eltIndex] = val;
                interpreter->pc += 3;
            }
            break;

            case WA_INSTR_elem_drop:
            {
                wa_element* elt = &instance->elements[I0.valI32];
                elt->initCount = 0;
                interpreter->pc += 1;
            }
            break;

            default:
                oc_log_error("invalid opcode %s\n", wa_instr_strings[opcode]);
                return WA_TRAP_INVALID_OP;
        }

        if(step)
        {
            break;
        }
    }

    if(step)
    {
        return WA_TRAP_STEP;
    }
    else
    {
        return WA_TRAP_SUSPENDED;
    }

end:
    interpreter->terminated = true;
    for(u32 retIndex = 0; retIndex < interpreter->retCount; retIndex++)
    {
        interpreter->returns[retIndex] = interpreter->locals[retIndex];
    }

    return WA_OK;
}

wa_status wa_instance_interpret_expr(wa_instance* instance,
                                     wa_func* func,
                                     wa_func_type* type,
                                     wa_code* code,
                                     u32 argCount,
                                     wa_value* args,
                                     u32 retCount,
                                     wa_value* returns)
{
    oc_arena_scope scratch = oc_scratch_begin();
    wa_interpreter* interpreter = wa_interpreter_create(scratch.arena);
    wa_interpreter_init(interpreter, instance, func, type, code, argCount, args, retCount, returns);
    wa_status status = wa_interpreter_run(interpreter, false);

    wa_interpreter_destroy(interpreter);
    oc_scratch_end(scratch);

    return status;
}

/*
wa_status wa_instance_invoke(wa_instance* instance,
                             wa_func* func,
                             u32 argCount,
                             wa_value* args,
                             u32 retCount,
                             wa_value* returns)
{
    if(argCount != func->type->paramCount || retCount != func->type->returnCount)
    {
        return WA_FAIL_INVALID_ARGS;
    }

    if(func->code)
    {
        return (wa_instance_interpret_expr(instance, func, func->type, func->code, argCount, args, retCount, returns));
    }
    else if(func->extInstance)
    {
        wa_func* extFunc = &func->extInstance->functions[func->extIndex];
        return wa_instance_invoke(func->extInstance, extFunc, argCount, args, retCount, returns);
    }
    else
    {
        /////////////////////////////////////////////////////
        //TODO: temporary
        /////////////////////////////////////////////////////

        wa_interpreter interpreter = {
            .instance = instance,
        };

        //TODO: host proc should return a status
        func->proc(&interpreter, args, returns, func->user);
        return WA_OK;
    }
}
*/

//-------------------------------------------------------------------------
// interpreter API
//-------------------------------------------------------------------------

wa_interpreter* wa_interpreter_create(oc_arena* arena)
{
    wa_interpreter* interpreter = oc_arena_push_type(arena, wa_interpreter);
    memset(interpreter, 0, sizeof(wa_interpreter));

    oc_base_allocator* alloc = oc_base_allocator_default();

    //TODO: should we rather allocate it in arena?
    interpreter->localsBuffer = oc_base_reserve(alloc, WA_LOCALS_BUFFER_SIZE * sizeof(wa_value));
    oc_base_commit(alloc, interpreter->localsBuffer, WA_LOCALS_BUFFER_SIZE * sizeof(wa_value));

    oc_arena_init(&interpreter->arena);

    return (interpreter);
}

void wa_interpreter_destroy(wa_interpreter* interpreter)
{
    oc_base_allocator* alloc = oc_base_allocator_default();
    oc_base_release(alloc, interpreter->localsBuffer, WA_LOCALS_BUFFER_SIZE * sizeof(wa_value));

    oc_arena_init(&interpreter->arena);
}

wa_instance* wa_interpreter_current_instance(wa_interpreter* interpreter)
{
    return (interpreter->instance);
}

//TODO
wa_status wa_interpreter_invoke(wa_interpreter* interpreter,
                                wa_instance* instance,
                                wa_func* function,
                                u32 argCount,
                                wa_value* args,
                                u32 retCount,
                                wa_value* returns)
{
    if(argCount != function->type->paramCount || retCount != function->type->returnCount)
    {
        return WA_FAIL_INVALID_ARGS;
    }

    if(function->code)
    {
        wa_interpreter_init(interpreter, instance, function, function->type, function->code, argCount, args, retCount, returns);
        wa_status status = wa_interpreter_run(interpreter, false);

        return status;
    }
    else if(function->extInstance)
    {
        wa_func* extFunc = &function->extInstance->functions[function->extIndex];
        return wa_interpreter_invoke(interpreter, function->extInstance, extFunc, argCount, args, retCount, returns);
    }
    else
    {
        //TODO: host proc should return a status
        function->proc(interpreter, args, returns, function->user);
        return WA_OK;
    }
}

//-------------------------------------------------------------------------
// debug helpers
//-------------------------------------------------------------------------

oc_str8 wa_module_get_function_name(wa_module* module, u32 index)
{
    oc_str8 res = { 0 };
    for(u32 entryIndex = 0; entryIndex < module->functionNameCount; entryIndex++)
    {
        wa_name_entry* entry = &module->functionNames[entryIndex];
        if(entry->index == index)
        {
            res = entry->name;
        }
    }
    return res;
}

void wa_print_stack_trace(wa_interpreter* interpreter)
{
    for(u32 level = 0; level <= interpreter->controlStackTop; level++)
    {
        wa_func* func = interpreter->controlStack[level].func;
        u64 addr = 0;
        if(level == interpreter->controlStackTop)
        {
            addr = interpreter->pc - func->code;
        }
        else
        {
            addr = interpreter->controlStack[level + 1].returnPC - 2 - func->code;
        }
        u32 functionIndex = func - interpreter->instance->functions;
        oc_str8 name = wa_module_get_function_name(interpreter->instance->module, functionIndex);

        printf("[%i] %.*s + 0x%08llx\n", level, oc_str8_ip(name), addr);
    }
}

//-------------------------------------------------------------------------
// debugger
//-------------------------------------------------------------------------
//TODO: move this in its own place

typedef struct wa_breakpoint
{
    oc_list_elt listElt;
    bool isLine;
    wa_line_loc lineLoc;
    wa_warm_loc warmLoc;
    wa_code savedOpcode;

} wa_breakpoint;

wa_breakpoint* wa_interpreter_find_breakpoint_any(wa_interpreter* interpreter, wa_warm_loc* loc)
{
    wa_breakpoint* result = 0;

    oc_list_for(interpreter->breakpoints, bp, wa_breakpoint, listElt)
    {
        if(bp->warmLoc.module == loc->module
           && bp->warmLoc.funcIndex == loc->funcIndex
           && bp->warmLoc.codeIndex == loc->codeIndex)
        {
            result = bp;
            break;
        }
    }

    return result;
}

wa_breakpoint* wa_interpreter_find_breakpoint(wa_interpreter* interpreter, wa_warm_loc* loc)
{
    wa_breakpoint* result = 0;

    oc_list_for(interpreter->breakpoints, bp, wa_breakpoint, listElt)
    {
        if(bp->isLine == false
           && bp->warmLoc.module == loc->module
           && bp->warmLoc.funcIndex == loc->funcIndex
           && bp->warmLoc.codeIndex == loc->codeIndex)
        {
            result = bp;
            break;
        }
    }

    return result;
}

wa_breakpoint* wa_interpreter_add_breakpoint(wa_interpreter* interpreter, wa_warm_loc* loc)
{
    wa_breakpoint* bp = wa_interpreter_find_breakpoint(interpreter, loc);
    if(bp == 0)
    {
        bp = oc_list_pop_front_entry(&interpreter->breakpointFreeList, wa_breakpoint, listElt);
        if(!bp)
        {
            bp = oc_arena_push_type(&interpreter->arena, wa_breakpoint);
        }
        bp->isLine = false;
        bp->warmLoc = *loc;
        oc_list_push_back(&interpreter->breakpoints, &bp->listElt);

        wa_func* func = &interpreter->instance->functions[bp->warmLoc.funcIndex];
        bp->savedOpcode = func->code[bp->warmLoc.codeIndex];
        func->code[bp->warmLoc.codeIndex].opcode = WA_INSTR_breakpoint;
    }
    return bp;
}

wa_breakpoint* wa_interpreter_find_breakpoint_line(wa_interpreter* interpreter, wa_line_loc* loc)
{
    wa_breakpoint* result = 0;

    oc_list_for(interpreter->breakpoints, bp, wa_breakpoint, listElt)
    {
        if(bp->isLine == true
           && bp->lineLoc.fileIndex == loc->fileIndex
           && bp->lineLoc.line == loc->line)
        {
            result = bp;
            break;
        }
    }

    return result;
}

wa_warm_loc wa_warm_loc_from_line_loc(wa_module* module, wa_line_loc loc);
wa_line_loc wa_line_loc_from_warm_loc(wa_module* module, wa_warm_loc loc);

wa_breakpoint* wa_interpreter_add_breakpoint_line(wa_interpreter* interpreter, wa_line_loc* loc)
{
    wa_breakpoint* bp = wa_interpreter_find_breakpoint_line(interpreter, loc);
    if(bp == 0)
    {
        wa_warm_loc warmLoc = wa_warm_loc_from_line_loc(interpreter->instance->module,
                                                        *loc);
        if(warmLoc.module)
        {
            bp = oc_list_pop_front_entry(&interpreter->breakpointFreeList, wa_breakpoint, listElt);
            if(!bp)
            {
                bp = oc_arena_push_type(&interpreter->arena, wa_breakpoint);
            }
            bp->isLine = true;
            bp->warmLoc = warmLoc;
            bp->lineLoc = *loc;
            oc_list_push_back(&interpreter->breakpoints, &bp->listElt);

            wa_func* func = &interpreter->instance->functions[bp->warmLoc.funcIndex];
            bp->savedOpcode = func->code[bp->warmLoc.codeIndex];
            func->code[bp->warmLoc.codeIndex].opcode = WA_INSTR_breakpoint;
        }
    }
    return bp;
}

void wa_interpreter_remove_breakpoint(wa_interpreter* interpreter, wa_breakpoint* bp)
{
    wa_func* func = &interpreter->instance->functions[bp->warmLoc.funcIndex];
    func->code[bp->warmLoc.codeIndex] = bp->savedOpcode;

    oc_list_remove(&interpreter->breakpoints, &bp->listElt);
    oc_list_push_back(&interpreter->breakpointFreeList, &bp->listElt);
}

wa_instr_op wa_breakpoint_saved_opcode(wa_breakpoint* bp)
{
    return bp->savedOpcode.opcode;
}

wa_status wa_interpreter_continue(wa_interpreter* interpreter)
{
    //TODO: if we're on a breakpoint, deactivate it, step, reactivate, continue

    wa_func* func = interpreter->controlStack[interpreter->controlStackTop].func;
    u64 funcIndex = func - interpreter->instance->functions;

    wa_breakpoint* bp = wa_interpreter_find_breakpoint_any(
        interpreter,
        &(wa_warm_loc){
            .module = interpreter->instance->module,
            .funcIndex = funcIndex,
            .codeIndex = interpreter->pc - func->code,
        });

    if(bp)
    {
        func->code[bp->warmLoc.codeIndex] = bp->savedOpcode;

        wa_status status = wa_interpreter_run(interpreter, true);
        //TODO: check if program terminated

        func->code[bp->warmLoc.codeIndex].opcode = WA_INSTR_breakpoint;
    }

    return wa_interpreter_run(interpreter, false);
}

wa_status wa_interpreter_step(wa_interpreter* interpreter)
{
    wa_func* func = interpreter->controlStack[interpreter->controlStackTop].func;
    u64 funcIndex = func - interpreter->instance->functions;

    wa_breakpoint* bp = wa_interpreter_find_breakpoint_any(
        interpreter,
        &(wa_warm_loc){
            .module = interpreter->instance->module,
            .funcIndex = funcIndex,
            .codeIndex = interpreter->pc - func->code,
        });

    if(bp)
    {
        func->code[bp->warmLoc.codeIndex] = bp->savedOpcode;
    }

    wa_status status = wa_interpreter_run(interpreter, true);

    if(bp)
    {
        func->code[bp->warmLoc.codeIndex].opcode = WA_INSTR_breakpoint;
    }

    return status;
}

wa_status wa_interpreter_step_line(wa_interpreter* interpreter)
{
    //NOTE: step the dumbest possible way
    wa_status status = WA_OK;
    wa_func* func = interpreter->controlStack[interpreter->controlStackTop].func;
    u64 funcIndex = func - interpreter->instance->functions;

    wa_line_loc startLoc = wa_line_loc_from_warm_loc(
        interpreter->instance->module,
        (wa_warm_loc){
            .module = interpreter->instance->module,
            .funcIndex = funcIndex,
            .codeIndex = interpreter->pc - func->code,
        });

    wa_line_loc lineLoc = startLoc;
    while(lineLoc.fileIndex == startLoc.fileIndex && lineLoc.line == startLoc.line)
    {
        wa_breakpoint* bp = wa_interpreter_find_breakpoint_any(
            interpreter,
            &(wa_warm_loc){
                .module = interpreter->instance->module,
                .funcIndex = funcIndex,
                .codeIndex = interpreter->pc - func->code,
            });

        if(bp)
        {
            func->code[bp->warmLoc.codeIndex] = bp->savedOpcode;
        }

        status = wa_interpreter_run(interpreter, true);

        if(bp)
        {
            func->code[bp->warmLoc.codeIndex].opcode = WA_INSTR_breakpoint;
        }

        if(status != WA_TRAP_STEP && status != WA_TRAP_BREAKPOINT)
        {
            break;
        }

        startLoc = wa_line_loc_from_warm_loc(
            interpreter->instance->module,
            (wa_warm_loc){
                .module = interpreter->instance->module,
                .funcIndex = funcIndex,
                .codeIndex = interpreter->pc - func->code,
            });
    }

    return status;
}

void wa_interpreter_suspend(wa_interpreter* interpreter)
{
    interpreter->suspend = true;
}

//-------------------------------------------------------------------------
// bytecode -> instr map
//-------------------------------------------------------------------------

typedef struct wa_wasm_loc
{
    wa_module* module;
    u32 offset;
} wa_wasm_loc;

wa_wasm_loc wa_wasm_loc_from_warm_loc(wa_warm_loc loc)
{
    u64 id = (u64)loc.funcIndex << 32 | (u64)loc.codeIndex;
    u64 hash = oc_hash_xx64_string((oc_str8){ .ptr = (char*)&id, .len = 8 });
    u64 index = hash % loc.module->warmToWasmMapLen;

    wa_instr* instr = 0;
    oc_list_for(loc.module->warmToWasmMap[index], mapping, wa_bytecode_mapping, listElt)
    {
        if(mapping->funcIndex == loc.funcIndex && mapping->codeIndex == loc.codeIndex)
        {
            instr = mapping->instr;
            break;
        }
    }
    wa_wasm_loc result = { 0 };
    if(instr)
    {
        result.module = loc.module;
        result.offset = instr->ast->loc.start - loc.module->toc.code.offset;
    }
    return (result);
}

wa_warm_loc wa_warm_loc_from_wasm_loc(wa_wasm_loc loc)
{
    wa_warm_loc result = { 0 };

    u64 id = loc.offset;
    u64 hash = oc_hash_xx64_string((oc_str8){ .ptr = (char*)&id, .len = 8 });
    u64 index = hash % loc.module->wasmToWarmMapLen;

    wa_instr* instr = 0;
    oc_list_for(loc.module->wasmToWarmMap[index], mapping, wa_bytecode_mapping, listElt)
    {
        if((mapping->instr->ast->loc.start - loc.module->toc.code.offset) == loc.offset)
        {
            result.module = loc.module;
            result.funcIndex = mapping->funcIndex;
            result.codeIndex = mapping->codeIndex;
            break;
        }
    }

    return (result);
}

wa_line_loc wa_line_loc_from_warm_loc(wa_module* module, wa_warm_loc loc)
{
    wa_line_loc res = { 0 };
    wa_wasm_loc wasmLoc = wa_wasm_loc_from_warm_loc(loc);

    for(u64 entryIndex = 0; entryIndex < module->wasmToLineCount; entryIndex++)
    {
        wa_wasm_to_line_entry* entry = &module->wasmToLine[entryIndex];
        if(entry->wasmOffset > wasmLoc.offset)
        {
            if(entryIndex)
            {
                res = module->wasmToLine[entryIndex - 1].loc;
            }
            break;
        }
    }

    return (res);
}

wa_warm_loc wa_warm_loc_from_line_loc(wa_module* module, wa_line_loc loc)
{
    wa_warm_loc result = { 0 };
    //TODO: this is super dumb for now, just pick the lowest line that's >= to the line we're looking for
    wa_wasm_loc wasmLoc = { 0 };

    u64 currentLine = UINT64_MAX;

    for(u64 entryIndex = 0; entryIndex < module->wasmToLineCount; entryIndex++)
    {
        wa_wasm_to_line_entry* entry = &module->wasmToLine[entryIndex];

        if(loc.fileIndex == entry->loc.fileIndex
           && entry->loc.line >= loc.line
           && entry->loc.line < currentLine)
        {
            currentLine = entry->loc.line;
            wasmLoc.module = module;
            wasmLoc.offset = entry->wasmOffset;
        }
    }

    if(wasmLoc.module)
    {
        result = wa_warm_loc_from_wasm_loc(wasmLoc);
    }
    return result;
}
