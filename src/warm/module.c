//-------------------------------------------------------------------------
// Module
//-------------------------------------------------------------------------
void wa_parse_module(wa_module* module, oc_str8 contents);
void wa_compile_code(oc_arena* arena, wa_module* module);

wa_module* wa_module_create(oc_arena* arena, oc_str8 contents)
{
    wa_module* module = oc_arena_push_type(arena, wa_module);
    memset(module, 0, sizeof(wa_module));

    module->arena = arena;

    wa_parse_module(module, contents);

    //NOTE: extract per-function local variables
    module->debugInfo.functionLocals = oc_arena_push_array(module->arena, wa_debug_function, module->functionCount);
    memset(module->debugInfo.functionLocals, 0, module->functionCount * sizeof(wa_debug_function));

    //NOTE: list of all types to deduplicate types
    oc_list types = { 0 };

    for(u64 unitIndex = 0; unitIndex < module->debugInfo.dwarf->unitCount; unitIndex++)
    {
        dw_unit* unit = &module->debugInfo.dwarf->units[unitIndex];
        dw_die* die = dw_die_find_next_with_tag(unit->rootDie, unit->rootDie, DW_TAG_subprogram);
        while(die)
        {
            dw_attr* funcNameAttr = dw_die_get_attr(die, DW_AT_name);
            if(funcNameAttr)
            {
                //TODO: better way of finding function
                bool found = false;
                u64 funcIndex = 0;
                for(; funcIndex < module->functionCount; funcIndex++)
                {
                    oc_str8 funcName = wa_module_get_function_name(module, funcIndex);
                    if(!oc_str8_cmp(funcName, funcNameAttr->string))
                    {
                        found = true;
                        break;
                    }
                }

                if(found)
                {
                    wa_debug_function* funcInfo = &module->debugInfo.functionLocals[funcIndex];
                    funcInfo->count = 0;

                    //NOTE: get frame base expr loc
                    dw_attr* frameBase = dw_die_get_attr(die, DW_AT_frame_base);
                    if(frameBase)
                    {
                        OC_DEBUG_ASSERT(frameBase->abbrev->form == DW_FORM_exprloc);
                        funcInfo->frameBase = &frameBase->loc;
                    }

                    //NOTE: get variables
                    {
                        //TODO: get with multiple tags, eg here we also need formal_parameter
                        dw_die* var = dw_die_find_next_with_tag(die, die, DW_TAG_variable);
                        while(var)
                        {
                            funcInfo->count++;
                            var = dw_die_find_next_with_tag(die, var, DW_TAG_variable);
                        }
                    }

                    funcInfo->vars = oc_arena_push_array(module->arena, wa_debug_variable, funcInfo->count);

                    {
                        dw_die* var = dw_die_find_next_with_tag(die, die, DW_TAG_variable);
                        u64 varIndex = 0;
                        while(var)
                        {
                            dw_attr* name = dw_die_get_attr(var, DW_AT_name);
                            if(name)
                            {
                                funcInfo->vars[varIndex].name = oc_str8_push_copy(module->arena, name->string);
                            }

                            dw_attr* loc = dw_die_get_attr(var, DW_AT_location);
                            if(loc)
                            {
                                funcInfo->vars[varIndex].loc = &loc->loc;
                                //TODO: compile the expr to wasm
                            }

                            dw_attr* type = dw_die_get_attr(var, DW_AT_type);
                            if(type)
                            {
                                funcInfo->vars[varIndex].type = wa_build_debug_type_from_dwarf(module->arena, module->debugInfo.dwarf, type->valU64, &types);
                            }

                            var = dw_die_find_next_with_tag(die, var, DW_TAG_variable);
                            varIndex++;
                        }
                    }
                }
            }

            die = dw_die_find_next_with_tag(unit->rootDie, die, DW_TAG_subprogram);
        }
    }

    if(oc_list_empty(module->errors))
    {
        //TODO: tune this
        module->debugInfo.warmToWasmMapLen = 4096;
        module->debugInfo.warmToWasmMap = oc_arena_push_array(arena, oc_list, 4096);
        memset(module->debugInfo.warmToWasmMap, 0, 4096 * sizeof(oc_list));

        module->debugInfo.wasmToWarmMapLen = 4096;
        module->debugInfo.wasmToWarmMap = oc_arena_push_array(arena, oc_list, 4096);
        memset(module->debugInfo.wasmToWarmMap, 0, 4096 * sizeof(oc_list));

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

wa_source_info* wa_module_get_source_info(wa_module* module)
{
    return &module->debugInfo.sourceInfo;
}

//-------------------------------------------------------------------------
// Module debug print
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
