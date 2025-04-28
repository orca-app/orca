/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <stdio.h>

#include "warm.h"

//-------------------------------------------------------------------------
// Module
//-------------------------------------------------------------------------

void wa_parse_module(wa_module* module, oc_str8 contents);
void wa_compile_code(oc_arena* arena, wa_module* module);
wa_debug_info* wa_debug_info_create(wa_module* module, oc_str8 contents);

wa_module* wa_module_create(oc_arena* arena, oc_str8 contents)
{
    wa_module* module = oc_arena_push_type(arena, wa_module);
    memset(module, 0, sizeof(wa_module));

    module->arena = arena;

    wa_parse_module(module, contents);

    if(oc_list_empty(module->errors))
    {
        module->debugInfo = wa_debug_info_create(module, contents);

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
// Module debug print
//-------------------------------------------------------------------------

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
