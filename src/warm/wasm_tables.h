/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "warm_internal.h"

typedef enum wa_instruction_prefix
{
    WA_INSTR_PREFIX_EXTENDED = 0xfc,
    WA_INSTR_PREFIX_VECTOR = 0xfd,
} wa_instruction_prefix;

extern const char* wa_instr_strings[];
extern const wa_instr_op wa_instr_decode_basic[];
extern const wa_instr_op wa_instr_decode_extended[];
extern const wa_instr_op wa_instr_decode_vector[];

extern const u64 wa_instr_decode_basic_len;
extern const u64 wa_instr_decode_extended_len;
extern const u64 wa_instr_decode_vector_len;

typedef enum wa_immediate_type
{
    WA_IMM_ZERO,
    WA_IMM_I32,
    WA_IMM_I64,
    WA_IMM_F32,
    WA_IMM_F64,
    WA_IMM_VALUE_TYPE,
    WA_IMM_REF_TYPE,
    WA_IMM_LOCAL_INDEX,
    WA_IMM_GLOBAL_INDEX,
    WA_IMM_FUNC_INDEX,
    WA_IMM_TYPE_INDEX,
    WA_IMM_TABLE_INDEX,
    WA_IMM_ELEM_INDEX,
    WA_IMM_DATA_INDEX,
    WA_IMM_MEM_ARG,
    WA_IMM_LANE_INDEX,
    WA_IMM_V128,
    WA_IMM_LABEL,

} wa_immediate_type;

typedef enum wa_opd_kind
{
    WA_OPD_UNKNOWN = 0,
    WA_OPD_CONST_I32,
    WA_OPD_CONST_I64,
    WA_OPD_CONST_F32,
    WA_OPD_CONST_F64,
    WA_OPD_LOCAL_INDEX,
    WA_OPD_GLOBAL_INDEX,
    WA_OPD_JUMP_TARGET,
    WA_OPD_MEM_ARG,
    WA_OPD_FUNC_INDEX,
} wa_opd_kind;

enum
{
    WA_INSTR_IN_MAX_COUNT = 3,
    WA_INSTR_OUT_MAX_COUNT = 3,
    WA_INSTR_OPD_MAX_COUNT = 4,
};

typedef struct wa_instr_info
{
    u32 immCount;
    wa_immediate_type imm[WA_INSTR_IMM_MAX_COUNT];

    u32 inCount;
    wa_value_type in[WA_INSTR_IN_MAX_COUNT];

    u32 outCount;
    wa_value_type out[WA_INSTR_OUT_MAX_COUNT];

    u32 opdCount;
    wa_opd_kind opd[WA_INSTR_OPD_MAX_COUNT];

    bool defined;
} wa_instr_info;

extern const wa_instr_info wa_instr_infos[];
