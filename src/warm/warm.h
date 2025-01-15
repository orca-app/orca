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

typedef struct wa_global
{
    wa_value_type type;
    bool mut;
    wa_value value;
} wa_global;

typedef struct wa_import wa_import;
typedef struct wa_module wa_module;
typedef struct wa_instance wa_instance;
typedef struct wa_func wa_func;

const char* wa_value_type_string(wa_value_type t);
const char* wa_status_string(wa_status status);

bool wa_module_has_errors(wa_module* module);
void wa_module_print_errors(wa_module* module);

wa_import_package wa_instance_exports(oc_arena* arena, wa_instance* instance, oc_str8 name);

//wip...
wa_status wa_instance_status(wa_instance* instance);
#endif // __WARM_H_
