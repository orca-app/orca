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

typedef struct wa_import wa_import;
typedef struct wa_module wa_module;
typedef struct wa_instance wa_instance;
typedef struct wa_func wa_func;

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
wa_status wa_instance_status(wa_instance* instance);
#endif // __WARM_H_
