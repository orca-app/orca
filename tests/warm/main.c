#define OC_NO_APP_LAYER
#include "orca.c"

#include "warm_test.c"
#include "warm/warm.h"
#include "warm/wasm.c"

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        printf("usage: warm module funcName [args...]\n");
        printf("       warm test [jsonfile|dir] [line]\n");
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

    oc_file file = oc_file_open(modulePath, OC_FILE_ACCESS_READ, OC_FILE_OPEN_DEFAULT);

    contents.len = oc_file_size(file);
    contents.ptr = oc_arena_push(&arena, contents.len);

    oc_file_read(file, contents.len, contents.ptr);
    oc_file_close(file);

    wa_module* module = wa_module_create(&arena, contents);

    if(wa_module_has_errors(module))
    {
        wa_module_print_errors(module);
        exit(-1);
    }

    wa_instance* instance = wa_instance_create(&arena, module, &(wa_instance_options){});
    if(instance->status != WA_OK)
    {
        oc_log_error("%.*s", oc_str8_ip(wa_status_string(instance->status)));
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

        wa_interpreter* interpreter = wa_interpreter_create(&arena);

        u32 retCount = 1;
        wa_value returns[32];

        wa_interpreter_invoke(interpreter, instance, func, argCount, args, retCount, returns);

        printf("results: ");
        for(u32 retIndex = 0; retIndex < retCount; retIndex++)
        {
            printf("%lli ", returns[retIndex].valI64);
        }
        printf("\n");

        wa_interpreter_destroy(interpreter);
    }
    return (0);
}
