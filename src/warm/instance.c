//-------------------------------------------------------------------------
// instance
//-------------------------------------------------------------------------

wa_status wa_instance_status(wa_instance* instance)
{
    return instance->status;
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
