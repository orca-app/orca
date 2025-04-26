/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "warm.h"
#include "module.h"

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

    wa_code cachedRegs[WA_MAX_REG];

} wa_interpreter;

wa_breakpoint* wa_interpreter_find_breakpoint_any(wa_interpreter* interpreter, wa_warm_loc* loc);
wa_breakpoint* wa_interpreter_find_breakpoint(wa_interpreter* interpreter, wa_warm_loc* loc);
wa_breakpoint* wa_interpreter_add_breakpoint(wa_interpreter* interpreter, wa_warm_loc* loc);
wa_breakpoint* wa_interpreter_find_breakpoint_line(wa_interpreter* interpreter, wa_line_loc* loc);

wa_breakpoint* wa_interpreter_add_breakpoint_line(wa_interpreter* interpreter, wa_line_loc* loc);
void wa_interpreter_remove_breakpoint(wa_interpreter* interpreter, wa_breakpoint* bp);
wa_instr_op wa_breakpoint_saved_opcode(wa_breakpoint* bp);
void wa_interpreter_cache_registers(wa_interpreter* interpreter);
wa_status wa_interpreter_continue(wa_interpreter* interpreter);
wa_status wa_interpreter_step(wa_interpreter* interpreter);
wa_status wa_interpreter_step_line(wa_interpreter* interpreter);
void wa_interpreter_suspend(wa_interpreter* interpreter);
