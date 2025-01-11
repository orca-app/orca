#ifndef __WARM_H_
#define __WARM_H_

#define OC_NO_APP_LAYER 1
#include "orca.h"
#include "wasm/wasm.h"

typedef struct wa_instance wa_instance;

typedef struct wa_typed_value
{
    wa_value_type type;
    wa_value value;

} wa_typed_value;

typedef void (*wa_host_proc)(wa_instance* instance, wa_value* args, wa_value* returns, void* user); //TODO: complete with memory, return status / etc

typedef struct wa_import wa_import;
typedef struct wa_module wa_module;
typedef struct wa_instance wa_instance;
typedef struct wa_func wa_func;

typedef struct wa_global
{
    wa_value_type type;
    bool mut;
    wa_value value;
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

typedef struct wa_table
{
    wa_value_type type;
    wa_limits limits;
    wa_value* contents;
} wa_table;

typedef struct wa_memory
{
    wa_limits limits;
    char* ptr;
} wa_memory;

enum
{
    WA_PAGE_SIZE = 64 * 1 << 10,
};

typedef enum wa_binding_kind
{
    WA_BINDING_WASM_GLOBAL,
    WA_BINDING_WASM_FUNCTION,
    WA_BINDING_WASM_MEMORY,
    WA_BINDING_WASM_TABLE,

    WA_BINDING_HOST_GLOBAL,
    WA_BINDING_HOST_FUNCTION,
    WA_BINDING_HOST_MEMORY,
    WA_BINDING_HOST_TABLE,

} wa_binding_kind;

typedef struct wa_host_function
{
    wa_func_type type;
    wa_host_proc proc;
    void* userData;
} wa_host_function;

typedef struct wa_import_binding
{
    oc_str8 name;
    wa_binding_kind kind;
    wa_instance* instance;

    union
    {
        u32 wasmGlobal;
        u32 wasmMemory;
        u32 wasmTable;
        u32 wasmFunction;

        wa_global* hostGlobal;
        wa_memory* hostMemory;
        wa_table* hostTable;
        wa_host_function hostFunction;
    };
} wa_import_binding;

typedef struct wa_import_package
{
    oc_str8 name;
    u32 bindingCount;
    wa_import_binding* bindings;
} wa_import_package;

typedef struct wa_instance_options
{
    u32 packageCount;
    wa_import_package* importPackages;

    //...
} wa_instance_options;

const char* wa_value_type_string(wa_value_type t);
const char* wa_status_string(wa_status status);

bool wa_module_has_errors(wa_module* module);
void wa_module_print_errors(wa_module* module);

wa_module* wa_module_create(oc_arena* arena, oc_str8 contents);
wa_instance* wa_instance_create(oc_arena* arena, wa_module* module, wa_instance_options* options);
wa_import_package wa_instance_exports(oc_arena* arena, wa_instance* instance, oc_str8 name);
wa_func* wa_instance_find_function(wa_instance* instance, oc_str8 name);
wa_global* wa_instance_find_global(wa_instance* instance, oc_str8 name);

wa_status wa_instance_invoke(wa_instance* instance,
                             wa_func* func,
                             u32 argCount,
                             wa_value* args,
                             u32 retCount,
                             wa_value* returns);

//wip...
wa_func_type* wa_function_get_type(wa_func* func);
wa_status wa_instance_status(wa_instance* instance);
#endif // __WARM_H_
