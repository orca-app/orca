
#include "warm/warm.h"
#include "json.c"

//-------------------------------------------------------------------------
// test
//-------------------------------------------------------------------------

wa_typed_value parse_value_64(oc_str8 string)
{
    wa_typed_value res = { 0 };

    u64 offset = 0;
    u64 numberI64 = 0;
    i64 sign = 1;

    if(string.len && string.ptr[0] == '-')
    {
        sign = -1;
        offset++;
    }

    while(offset < string.len)
    {
        char c = string.ptr[offset];
        if(c >= '0' && c <= '9')
        {
            numberI64 *= 10;
            numberI64 += sign * (c - '0');
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
                decimals += sign * (c - '0');
                offset += 1;
                decimalCount += 1;
            }
            else
            {
                break;
            }
        }
        res.type = WA_TYPE_F64;
        res.value.valF64 = (f64)numberI64 + (f64)decimals / pow(10, decimalCount);
    }
    else
    {
        res.type = WA_TYPE_I64;
        res.value.valI64 = numberI64;
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
    u32 registeredCount;

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
    wa_global testspecGlobal_i32;
    wa_global testspecGlobal_i64;
    wa_global testspecGlobal_f32;
    wa_global testspecGlobal_f64;

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

    oc_arena_scope scratch = oc_scratch_begin();
    wa_func_type type = wa_func_get_type(scratch.arena, instance, func);
    OC_ASSERT(argCount == type.paramCount);

    wa_value* argVals = oc_arena_push_array(env->arena, wa_value, argCount);

    u32 argIndex = 0;
    oc_list_for(args->children, arg, json_node, listElt)
    {
        wa_typed_value val = test_parse_value(arg);
        argVals[argIndex] = val.value;
        argIndex++;
    }

    wa_interpreter* interpreter = wa_interpreter_create(scratch.arena);

    wa_test_result res = { 0 };

    res.count = type.returnCount;
    res.values = oc_arena_push_array(env->arena, wa_value, res.count);

    res.trap = wa_interpreter_invoke(interpreter, instance, func, argCount, argVals, res.count, res.values);
    res.status = (res.trap == WA_OK) ? WA_TEST_PASS : WA_TEST_FAIL;

    wa_interpreter_destroy(interpreter);

    oc_scratch_end(scratch);
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
    oc_str8 contents = { 0 };

    oc_file file = oc_file_open(filename, OC_FILE_ACCESS_READ, OC_FILE_OPEN_DEFAULT);
    if(oc_file_is_nil(file))
    {
        oc_log_error("Couldn't open file %.*s\n", oc_str8_ip(filename));
        return (0);
    }

    contents.len = oc_file_size(file);
    contents.ptr = oc_arena_push(arena, contents.len);

    oc_file_read(file, contents.len, contents.ptr);
    oc_file_close(file);

    wa_module* module = wa_module_create(arena, contents);

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

void test_print(wa_interpreter* interpreter, wa_value* args, wa_value* rets, void* user)
{
}

void test_print_i32(wa_interpreter* interpreter, wa_value* args, wa_value* rets, void* user)
{
}

void test_print_i64(wa_interpreter* interpreter, wa_value* args, wa_value* rets, void* user)
{
}

void test_print_f32(wa_interpreter* interpreter, wa_value* args, wa_value* rets, void* user)
{
}

void test_print_f64(wa_interpreter* interpreter, wa_value* args, wa_value* rets, void* user)
{
}

void test_print_i32_f32(wa_interpreter* interpreter, wa_value* args, wa_value* rets, void* user)
{
}

void test_print_f64_f64(wa_interpreter* interpreter, wa_value* args, wa_value* rets, void* user)
{
}

wa_status wa_test_instantiate(wa_test_env* env, wa_test_instance* testInstance, wa_module* module)
{
    u32 packageCount = env->registeredCount + 1;

    oc_arena_scope scratch = oc_scratch_begin();
    wa_import_package* packages = oc_arena_push_array(scratch.arena, wa_import_package, packageCount);

    u32 index = 1;
    oc_list_for(env->instances, instance, wa_test_instance, listElt)
    {
        if(instance->registeredName.len)
        {
            packages[index] = wa_instance_exports(scratch.arena, instance->instance, instance->registeredName);
            index++;
        }
    }

    packages[0] = (wa_import_package){
        .name = OC_STR8("spectest"),
        .bindingCount = 13,
    };

    wa_import_package_push_binding(env->arena,
                                   &packages[0],
                                   &(wa_import_binding){
                                       .kind = WA_BINDING_HOST_MEMORY,
                                       .name = OC_STR8("memory"),
                                       .hostMemory = &env->testspecMemory,
                                   });
    wa_import_package_push_binding(env->arena,
                                   &packages[0],
                                   &(wa_import_binding){
                                       .kind = WA_BINDING_HOST_TABLE,
                                       .name = OC_STR8("table"),
                                       .hostTable = &env->testspecTable,
                                   });
    wa_import_package_push_binding(env->arena,
                                   &packages[0],
                                   &(wa_import_binding){
                                       .kind = WA_BINDING_HOST_GLOBAL,
                                       .name = OC_STR8("global_i32"),
                                       .hostGlobal = &env->testspecGlobal_i32,
                                   });
    wa_import_package_push_binding(env->arena,
                                   &packages[0],
                                   &(wa_import_binding){
                                       .kind = WA_BINDING_HOST_GLOBAL,
                                       .name = OC_STR8("global_i64"),
                                       .hostGlobal = &env->testspecGlobal_i64,
                                   });
    wa_import_package_push_binding(env->arena,
                                   &packages[0],
                                   &(wa_import_binding){
                                       .kind = WA_BINDING_HOST_GLOBAL,
                                       .name = OC_STR8("global_f32"),
                                       .hostGlobal = &env->testspecGlobal_f32,
                                   });
    wa_import_package_push_binding(env->arena,
                                   &packages[0],
                                   &(wa_import_binding){
                                       .kind = WA_BINDING_HOST_GLOBAL,
                                       .name = OC_STR8("global_f64"),
                                       .hostGlobal = &env->testspecGlobal_f64,
                                   });
    wa_import_package_push_binding(env->arena,
                                   &packages[0],
                                   &(wa_import_binding){
                                       .kind = WA_BINDING_HOST_FUNCTION,
                                       .name = OC_STR8("print"),
                                       .hostFunction = {
                                           .type = { 0 },
                                           .proc = test_print,
                                       },
                                   });
    wa_import_package_push_binding(env->arena,
                                   &packages[0],
                                   &(wa_import_binding){
                                       .kind = WA_BINDING_HOST_FUNCTION,
                                       .name = OC_STR8("print_i32"),
                                       .hostFunction = {
                                           .type = {
                                               .paramCount = 1,
                                               .params = (wa_value_type[]){
                                                   WA_TYPE_I32,
                                               },
                                           },
                                           .proc = test_print_i32,
                                       },
                                   });
    wa_import_package_push_binding(env->arena,
                                   &packages[0],
                                   &(wa_import_binding){
                                       .kind = WA_BINDING_HOST_FUNCTION,
                                       .name = OC_STR8("print_i64"),
                                       .hostFunction = {
                                           .type = {
                                               .paramCount = 1,
                                               .params = (wa_value_type[]){
                                                   WA_TYPE_I64,
                                               },
                                           },
                                           .proc = test_print_i64,
                                       },
                                   });
    wa_import_package_push_binding(env->arena,
                                   &packages[0],
                                   &(wa_import_binding){
                                       .kind = WA_BINDING_HOST_FUNCTION,
                                       .name = OC_STR8("print_f32"),
                                       .hostFunction = {
                                           .type = {
                                               .paramCount = 1,
                                               .params = (wa_value_type[]){
                                                   WA_TYPE_F32,
                                               },
                                           },
                                           .proc = test_print_f32,
                                       },
                                   });
    wa_import_package_push_binding(env->arena,
                                   &packages[0],
                                   &(wa_import_binding){
                                       .kind = WA_BINDING_HOST_FUNCTION,
                                       .name = OC_STR8("print_f64"),
                                       .hostFunction = {
                                           .type = {
                                               .paramCount = 1,
                                               .params = (wa_value_type[]){
                                                   WA_TYPE_F64,
                                               },
                                           },
                                           .proc = test_print_f64,
                                       },
                                   });
    wa_import_package_push_binding(env->arena,
                                   &packages[0],
                                   &(wa_import_binding){
                                       .kind = WA_BINDING_HOST_FUNCTION,
                                       .name = OC_STR8("print_i32_f32"),
                                       .hostFunction = {
                                           .type = {
                                               .paramCount = 2,
                                               .params = (wa_value_type[]){
                                                   WA_TYPE_I32,
                                                   WA_TYPE_F32,
                                               },
                                           },
                                           .proc = test_print_i32_f32,
                                       },
                                   });
    wa_import_package_push_binding(env->arena,
                                   &packages[0],
                                   &(wa_import_binding){
                                       .kind = WA_BINDING_HOST_FUNCTION,
                                       .name = OC_STR8("print_f64_f64"),
                                       .hostFunction = {
                                           .type = {
                                               .paramCount = 2,
                                               .params = (wa_value_type[]){
                                                   WA_TYPE_F64,
                                                   WA_TYPE_F64,
                                               },
                                           },
                                           .proc = test_print_f64_f64,
                                       },
                                   });

    wa_instance_options options = {
        .packageCount = packageCount,
        .importPackages = packages,
    };
    testInstance->instance = wa_instance_create(env->arena, module, &options);

    oc_scratch_end(scratch);

    return wa_instance_status(testInstance->instance);
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
    else if(!oc_str8_cmp(failure, OC_STR8("call stack exhausted")))
    {
        expected = WA_TRAP_STACK_OVERFLOW;
    }
    else
    {
        oc_log_error("unhandled failure string %.*s\n", oc_str8_ip(failure));
    }
    return expected;
}

int test_file(oc_str8 testPath, oc_str8 testName, oc_str8 testDir, i32 filterLine, wa_test_env* env)
{
    oc_str8 contents = { 0 };
    {
        oc_file file = oc_file_open(testPath, OC_FILE_ACCESS_READ, OC_FILE_OPEN_DEFAULT);
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
        .ptr = oc_base_reserve(allocator, 2 * WA_PAGE_SIZE),
    };
    oc_base_commit(allocator, env->testspecMemory.ptr, env->testspecMemory.limits.min * WA_PAGE_SIZE);

    //spec test table
    env->testspecTable = (wa_table){
        .type = WA_TYPE_FUNC_REF,
        .limits = {
            .kind = WA_LIMIT_MIN_MAX,
            .min = 10,
            .max = 20,
        },
        .contents = oc_arena_push_array(env->arena, wa_value, 10),
    };

    env->testspecGlobal_i32 = (wa_global){
        .type = WA_TYPE_I32,
        .value.valI32 = 666,
    };
    env->testspecGlobal_i64 = (wa_global){
        .type = WA_TYPE_I64,
        .value.valI64 = 666,
    };
    env->testspecGlobal_f32 = (wa_global){
        .type = WA_TYPE_F32,
        .value.valF32 = 666,
    };
    env->testspecGlobal_f64 = (wa_global){
        .type = WA_TYPE_F64,
        .value.valF64 = 666,
    };

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

            if(wa_module_has_errors(module))
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
                    oc_log_error("couldn't link module: %s\n", wa_status_string(status));
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
            env->registeredCount++;
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

                if(wa_module_has_errors(module))
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
                if(!wa_module_has_errors(module))
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
                json_node* moduleType = json_find_assert(command, "module_type", JSON_STRING);

                if(!oc_str8_cmp(moduleType->string, OC_STR8("binary")))
                {
                    json_node* filename = json_find_assert(command, "filename", JSON_STRING);

                    oc_str8_list list = { 0 };
                    oc_str8_list_push(env->arena, &list, testDir);
                    oc_str8_list_push(env->arena, &list, filename->string);

                    oc_str8 filePath = oc_path_join(env->arena, list);

                    wa_module* module = wa_test_module_load(env->arena, filePath);

                    //TODO: check the failure reason
                    if(!wa_module_has_errors(module))
                    {
                        wa_test_fail(env, testName, command);
                    }
                    else
                    {
                        wa_test_pass(env, testName, command);
                    }
                }
                else if(!oc_str8_cmp(moduleType->string, OC_STR8("text")))
                {
                    wa_test_skip(env, testName, command);
                }
                else
                {
                    oc_log_error("unsupported module type %.*s\n", oc_str8_ip(moduleType->string));
                    wa_test_fail(env, testName, command);
                }
            }
            else if(!oc_str8_cmp(type->string, OC_STR8("assert_exhaustion")))
            {
                json_node* action = json_find_assert(command, "action", JSON_OBJECT);
                json_node* failure = json_find_assert(command, "text", JSON_STRING);

                wa_status expected = wa_test_failure_string_to_status(failure->string);
                if(expected == WA_OK)
                {
                    wa_test_fail(env, testName, command);
                    continue;
                }

                wa_instance* instance = wa_test_get_instance(env, action);
                if(!instance)
                {
                    wa_test_fail(env, testName, command);
                    continue;
                }

                wa_test_result result = wa_test_action(env, instance, action);

                if(result.trap != expected)
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
                oc_log_error("unsupported command %.*s\n", oc_str8_ip(type->string));
                wa_test_fail(env, testName, command);
            }
        }
    }

    env->totalPassed += env->passed;
    env->totalSkipped += env->skipped;
    env->totalFailed += env->failed;

    oc_base_release(allocator, env->testspecMemory.ptr, env->testspecMemory.limits.max * WA_PAGE_SIZE);

    return (0);
}

#include <sys/stat.h>
#include <dirent.h>

int test_main(int argc, char** argv)
{
    if(argc < 3)
    {
        printf("usage: warm test [jsonfile|dir] [line]");
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
            env.registeredCount = 0;

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
