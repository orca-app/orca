#include <stdio.h>
#include "util/wrapped_types.h"
#include "util/argparse.h"
#include "util/json.c"
#include "platform/io.h"

#define OC_NO_APP_LAYER 1
#include "orca.c"

json_node* json_expect_field(json_node* parent, oc_str8 name, json_node_kind kind)
{
    json_node* child = json_find(parent, name);
    if(!child)
    {
        printf("Error: field %.*s doesn't exist\n", oc_str8_ip(name));
        exit(-1);
    }
    else if(child->kind != kind)
    {
        printf("Error: unexpected kind %s for field %.*s (expected %s)\n",
               json_node_kind_strings[child->kind],
               oc_str8_ip(name),
               json_node_kind_strings[kind]);
        exit(-1);
    }
    return child;
}

json_node* json_find_kind(json_node* parent, oc_str8 name, json_node_kind kind)
{
    json_node* child = json_find(parent, name);
    if(child && child->kind != kind)
    {
        printf("Error: unexpected kind %s for field %.*s (expected %s)\n",
               json_node_kind_strings[child->kind],
               oc_str8_ip(name),
               json_node_kind_strings[kind]);
        exit(-1);
    }
    return child;
}

typedef struct param_type_desc
{
    oc_str8 string;
    bool isPointer;
    oc_str8 wasmType;
    oc_str8 guestType;

} param_type_desc;

typedef struct param_len_desc
{
    u64 countConst;
    oc_str8 countArg;

    oc_str8 checkProc;
    u64 checkArgCount;
    oc_str8* checkArgs;
} param_len_desc;

typedef struct param_desc
{
    oc_str8 name;
    param_type_desc typeDesc;
    param_len_desc len;

} param_desc;

typedef struct proc_desc
{
    oc_list_elt listElt;

    oc_str8 name;
    oc_str8 handler;

    param_type_desc returnType;

    u32 paramCount;
    param_desc* params;

    bool noReturn;

} proc_desc;

oc_str8 gen_guest_type(oc_arena* arena, json_node* node)
{
    oc_str8 result = { 0 };

    json_node* kindNode = json_expect_field(node, OC_STR8("kind"), JSON_STRING);

    if(!oc_str8_cmp(kindNode->string, OC_STR8("namedType")))
    {
        json_node* nameNode = json_expect_field(node, OC_STR8("name"), JSON_STRING);
        result = nameNode->string;
    }
    else if(!oc_str8_cmp(kindNode->string, OC_STR8("pointer")))
    {
        json_node* typeNode = json_expect_field(node, OC_STR8("type"), JSON_OBJECT);
        oc_str8 type = gen_guest_type(arena, typeNode);
        result = oc_str8_pushf(arena, "%.*s*", oc_str8_ip(type));
    }
    else
    {
        result = kindNode->string;
    }

    return result;
}

void gen_hostcall_prototype(oc_arena* arena, oc_str8_list* list, proc_desc* proc)
{
    if(proc->noReturn)
    {
        oc_str8_list_push(arena, list, OC_STR8("_Noreturn "));
    }

    oc_str8_list_push(arena, list, proc->returnType.guestType);

    oc_str8_list_pushf(arena,
                       list,
                       " %.*s(",
                       oc_str8_ip(proc->name));

    if(proc->paramCount == 0)
    {
        oc_str8_list_push(arena, list, OC_STR8("void"));
    }
    for(int i = 0; i < proc->paramCount; i++)
    {
        oc_str8_list_push(arena, list, proc->params[i].typeDesc.guestType);
        oc_str8_list_pushf(arena, list, " %.*s", oc_str8_ip(proc->params[i].name));
        if(i < proc->paramCount - 1)
        {
            oc_str8_list_push(arena, list, OC_STR8(", "));
        }
    }
    oc_str8_list_push(arena, list, OC_STR8(");\n"));
}

json_node* find_named_type(json_node* root, oc_str8 name)
{
    oc_list_for(root->children, child, json_node, listElt)
    {
        json_node* kindNode = json_expect_field(child, OC_STR8("kind"), JSON_STRING);

        if(!oc_str8_cmp(kindNode->string, OC_STR8("typename")))
        {
            json_node* nameNode = json_expect_field(child, OC_STR8("name"), JSON_STRING);

            if(!oc_str8_cmp(nameNode->string, name))
            {
                return child;
            }
        }
    }

    printf("Error: typename %.*s not found\n", oc_str8_ip(name));
    exit(-1);
    return 0;
}

oc_str8 get_native_type(oc_arena* arena, json_node* root, json_node* node)
{
    oc_str8 typeString = { 0 };
    json_node* kindNode = json_expect_field(node, OC_STR8("kind"), JSON_STRING);

    if(!oc_str8_cmp(kindNode->string, OC_STR8("namedType")))
    {
        json_node* nameNode = json_expect_field(node, OC_STR8("name"), JSON_STRING);
        json_node* typenameNode = find_named_type(root, nameNode->string);

        json_node* hostNode = json_find(typenameNode, OC_STR8("host"));
        if(hostNode)
        {
            if(hostNode->kind != JSON_STRING)
            {
                printf("Error: 'host' property should be a string.\n");
                exit(-1);
            }
            typeString = hostNode->string;
        }
        else
        {
            typeString = nameNode->string;
        }
    }
    else if(!oc_str8_cmp(kindNode->string, OC_STR8("pointer")))
    {
        json_node* typeNode = json_expect_field(node, OC_STR8("type"), JSON_OBJECT);

        oc_str8 sub = get_native_type(arena, root, typeNode);

        typeString = oc_str8_pushf(arena, "%.*s*", oc_str8_ip(sub));
    }
    else if(!oc_str8_cmp(kindNode->string, OC_STR8("struct")))
    {
        printf("Error: struct type found where namedType would be expected\n");
        exit(-1);
    }
    else
    {
        typeString = kindNode->string;
    }
    return typeString;
}

json_node* unwrap_named_types(json_node* root, json_node* node)
{
    json_node* kindNode = json_expect_field(node, OC_STR8("kind"), JSON_STRING);

    if(!oc_str8_cmp(kindNode->string, OC_STR8("namedType")))
    {
        json_node* nameNode = json_expect_field(node, OC_STR8("name"), JSON_STRING);
        json_node* typenameNode = find_named_type(root, nameNode->string);
        json_node* typeNode = json_expect_field(typenameNode, OC_STR8("type"), JSON_OBJECT);

        return unwrap_named_types(root, typeNode);
    }
    else
    {
        return node;
    }
}

param_type_desc parse_param_type(oc_arena* arena, json_node* root, json_node* node)
{
    param_type_desc desc = { 0 };

    json_node* unwrappedTypeNode = unwrap_named_types(root, node);
    json_node* unwrappedKindNode = json_expect_field(unwrappedTypeNode, OC_STR8("kind"), JSON_STRING);

    if(!oc_str8_cmp(unwrappedKindNode->string, OC_STR8("pointer")))
    {
        desc.isPointer = true;
        desc.wasmType = OC_STR8("WA_TYPE_I32");
    }
    else if(oc_str8_cmp(unwrappedKindNode->string, OC_STR8("bool"))
            || oc_str8_cmp(unwrappedKindNode->string, OC_STR8("u8"))
            || oc_str8_cmp(unwrappedKindNode->string, OC_STR8("i8"))
            || oc_str8_cmp(unwrappedKindNode->string, OC_STR8("u16"))
            || oc_str8_cmp(unwrappedKindNode->string, OC_STR8("i16"))
            || oc_str8_cmp(unwrappedKindNode->string, OC_STR8("u32"))
            || oc_str8_cmp(unwrappedKindNode->string, OC_STR8("i32")))
    {
        desc.wasmType = OC_STR8("WA_TYPE_I32");
    }
    else if(oc_str8_cmp(unwrappedKindNode->string, OC_STR8("u64"))
            || oc_str8_cmp(unwrappedKindNode->string, OC_STR8("i64")))
    {
        desc.wasmType = OC_STR8("WA_TYPE_I64");
    }
    else if(oc_str8_cmp(unwrappedKindNode->string, OC_STR8("f32")))
    {
        desc.wasmType = OC_STR8("WA_TYPE_F32");
    }
    else if(oc_str8_cmp(unwrappedKindNode->string, OC_STR8("f64")))
    {
        desc.wasmType = OC_STR8("WA_TYPE_F64");
    }
    else
    {
        printf("Error: hostcall param should be a scalar or pointer.\n");
        exit(-1);
    }

    desc.string = get_native_type(arena, root, node);
    desc.guestType = gen_guest_type(arena, node);
    return desc;
}

param_desc parse_param(oc_arena* arena, json_node* root, json_node* paramNode)
{
    param_desc param = { 0 };

    json_node* paramNameNode = json_expect_field(paramNode, OC_STR8("name"), JSON_STRING);
    json_node* paramTypeNode = json_expect_field(paramNode, OC_STR8("type"), JSON_OBJECT);

    json_node* paramLenNode = json_find(paramNode, OC_STR8("len"));

    json_node* unwrappedTypeNode = unwrap_named_types(root, paramTypeNode);
    json_node* unwrappedKindNode = json_expect_field(unwrappedTypeNode, OC_STR8("kind"), JSON_STRING);

    param.name = paramNameNode->string;
    param.typeDesc = parse_param_type(arena, root, paramTypeNode);
    param.len.countConst = 1;

    if(paramLenNode)
    {
        if(paramLenNode->kind != JSON_OBJECT)
        {
            printf("Error: param len property must be an object\n");
            exit(-1);
        }
        json_node* countArgNode = json_find_kind(paramLenNode, OC_STR8("count"), JSON_STRING);
        if(countArgNode)
        {
            param.len.countArg = countArgNode->string;
        }
        json_node* countConstNode = json_find_kind(paramLenNode, OC_STR8("components"), JSON_NUM_I64);
        if(countConstNode)
        {
            param.len.countConst = countConstNode->numI64;
        }

        json_node* checkProcNode = json_find_kind(paramLenNode, OC_STR8("proc"), JSON_STRING);
        if(checkProcNode)
        {
            param.len.checkProc = checkProcNode->string;
            json_node* argsNode = json_expect_field(paramLenNode, OC_STR8("args"), JSON_LIST);
            param.len.checkArgCount = oc_list_count(argsNode->children);
            param.len.checkArgs = oc_arena_push_array(arena, oc_str8, param.len.checkArgCount);

            oc_list_for_indexed(argsNode->children, it, json_node, listElt)
            {
                if(it.elt->kind != JSON_STRING)
                {
                    printf("Error: check arg should be a string\n");
                    exit(-1);
                }
                param.len.checkArgs[it.index] = it.elt->string;
            }
        }
    }

    return param;
}

proc_desc* parse_proc(oc_arena* arena, json_node* root, json_node* procNode)
{
    json_node* nameNode = json_expect_field(procNode, OC_STR8("name"), JSON_STRING);
    json_node* handlerNode = json_expect_field(procNode, OC_STR8("handler"), JSON_STRING);
    json_node* returnNode = json_expect_field(procNode, OC_STR8("return"), JSON_OBJECT);
    json_node* paramsNode = json_expect_field(procNode, OC_STR8("params"), JSON_LIST);

    json_node* attrNode = json_find_kind(procNode, OC_STR8("attributes"), JSON_LIST);

    proc_desc* proc = oc_arena_push_type(arena, proc_desc);

    proc->name = nameNode->string;
    proc->handler = handlerNode->string;
    proc->returnType = parse_param_type(arena, root, returnNode);
    proc->paramCount = oc_list_count(paramsNode->children);
    proc->params = oc_arena_push_array(arena, param_desc, proc->paramCount);

    oc_list_for_indexed(paramsNode->children, it, json_node, listElt)
    {
        proc->params[it.index] = parse_param(arena, root, it.elt);
    }

    if(attrNode)
    {
        oc_list_for(attrNode->children, child, json_node, listElt)
        {
            if(child->kind == JSON_STRING && !oc_str8_cmp(child->string, OC_STR8("noreturn")))
            {
                proc->noReturn = true;
            }
        }
    }

    return proc;
}

void gen_handler_prototype(oc_arena* arena, oc_str8_list* list, proc_desc* proc)
{
    oc_str8_list_pushf(arena,
                       list,
                       "%.*s %.*s(",
                       oc_str8_ip(proc->returnType.string),
                       oc_str8_ip(proc->handler));
    for(int i = 0; i < proc->paramCount; i++)
    {
        oc_str8_list_pushf(arena,
                           list,
                           "%.*s %.*s",
                           oc_str8_ip(proc->params[i].typeDesc.string),
                           oc_str8_ip(proc->params[i].name));
        if(i < proc->paramCount - 1)
        {
            oc_str8_list_push(arena, list, OC_STR8(", "));
        }
    }
    oc_str8_list_push(arena, list, OC_STR8(");\n"));
}

void gen_hostapi_stub(oc_arena* arena, oc_str8_list* list, proc_desc* proc)
{
    oc_str8_list_pushf(arena,
                       list,
                       "void %.*s_stub(wa_interpreter* interpreter, wa_value* params, wa_value* returns, void* user)\n{\n",
                       oc_str8_ip(proc->handler));

    oc_str8_list_push(arena,
                      list,
                      OC_STR8("\twa_instance* instance = wa_interpreter_current_instance(interpreter);\n"
                              "\toc_str8 memStr8 = wa_instance_get_memory_str8(instance);\n"
                              "\tchar* _mem = memStr8.ptr;\n"
                              "\tu32 _memSize = memStr8.len;\n\n"));

    for(u64 i = 0; i < proc->paramCount; i++)
    {
        param_desc* param = &proc->params[i];

        if(param->typeDesc.isPointer)
        {
            oc_str8_list_pushf(arena,
                               list,
                               "\tu64 %.*s_offset = (u64)(*(u32*)&params[%llu]);\n",
                               oc_str8_ip(param->name),
                               oc_str8_ip(param->typeDesc.string),
                               i);

            oc_str8_list_pushf(arena,
                               list,
                               "\t%.*s %.*s = (%.*s)((char*)_mem + %.*s_offset);\n",
                               oc_str8_ip(param->typeDesc.string),
                               oc_str8_ip(param->name),
                               oc_str8_ip(param->typeDesc.string),
                               oc_str8_ip(param->name));
        }
        else
        {
            oc_str8_list_pushf(arena,
                               list,
                               "\t%.*s %.*s = *(%.*s*)&params[%llu];\n",
                               oc_str8_ip(param->typeDesc.string),
                               oc_str8_ip(param->name),
                               oc_str8_ip(param->typeDesc.string),
                               i);
        }
    }

    //NOTE: validation rules for pointer arguments
    for(int i = 0; i < proc->paramCount; i++)
    {
        param_desc* param = &proc->params[i];

        if(param->typeDesc.isPointer)
        {
            oc_str8_list_pushf(arena, list, "\t{\n\t\t// Check argument '%.*s'\n", oc_str8_ip(param->name));

            //NOTE: Compute size of arg, detecting overflow
            oc_str8_list_pushf(arena,
                               list,
                               "\t\tu64 size = %u * sizeof(*%.*s);\n",
                               param->len.countConst,
                               oc_str8_ip(param->name));

            oc_str8_list_pushf(arena,
                               list,
                               "\t\tOC_ASSERT_DIALOG(size/%u == sizeof(*%.*s), \"argument length overflows\");\n",
                               param->len.countConst,
                               oc_str8_ip(param->name));

            if(param->len.countArg.len)
            {
                oc_str8_list_pushf(arena,
                                   list,
                                   "\t\tOC_ASSERT_DIALOG((%.*s * size)/size == %.*s, \"argument length overflows\");\n",
                                   oc_str8_ip(param->len.countArg),
                                   oc_str8_ip(param->len.countArg));

                oc_str8_list_pushf(arena,
                                   list,
                                   "\t\tsize *= %.*s;\n",
                                   oc_str8_ip(param->len.countArg));
            }

            //NOTE: check size of arg, detecting out-of-bounds and overflow
            oc_str8_list_pushf(arena,
                               list,
                               "\t\tOC_ASSERT_DIALOG((%.*s_offset + size < _memSize) && (%.*s_offset + size >= %.*s_offset), \"argument is out of bounds\");\n",
                               oc_str8_ip(param->name),
                               oc_str8_ip(param->name),
                               oc_str8_ip(param->name));

            //NOTE: check with validation function if present
            if(param->len.checkProc.len)
            {
                oc_str8_list_pushf(arena,
                                   list,
                                   "\t\tOC_ASSERT(%.*s(instance,",
                                   oc_str8_ip(param->len.checkProc));

                for(int i = 0; i < param->len.checkArgCount; i++)
                {
                    oc_str8_list_push(arena, list, param->len.checkArgs[i]);

                    if(i < param->len.checkArgCount - 1)
                    {
                        oc_str8_list_push(arena, list, OC_STR8(", "));
                    }
                }
                oc_str8_list_push(arena, list, OC_STR8("), \"argument check failed\");\n"));
            }

            oc_str8_list_pushf(arena, list, "\t}\n");
        }
    }

    if(proc->paramCount)
    {
        oc_str8_list_push(arena, list, OC_STR8("\n"));
    }

    oc_str8_list_push(arena, list, OC_STR8("\t"));

    if(oc_str8_cmp(proc->returnType.string, OC_STR8("void")))
    {
        if(proc->returnType.isPointer)
        {
            printf("Error: return type should be a scalar.\n");
            exit(-1);
        }
        oc_str8_list_pushf(arena, list, "*(%.*s*)&returns[0] = ", oc_str8_ip(proc->returnType.string));
    }

    oc_str8_list_pushf(arena, list, "%.*s(", oc_str8_ip(proc->handler));
    for(int i = 0; i < proc->paramCount; i++)
    {
        param_desc* param = &proc->params[i];
        oc_str8_list_push(arena, list, param->name);
        if(i < proc->paramCount - 1)
        {
            oc_str8_list_push(arena, list, OC_STR8(", "));
        }
    }
    oc_str8_list_push(arena, list, OC_STR8(");\n}\n\n"));
}

void gen_hostapi_binding(oc_arena* arena, oc_str8_list* list, oc_str8 apiName, oc_list procList)
{
    oc_str8_list_pushf(arena,
                       list,
                       "void bindgen_link_%.*s_api(oc_arena* arena, wa_import_package* package)\n{\n",
                       oc_str8_ip(apiName));

    oc_list_for(procList, proc, proc_desc, listElt)
    {
        oc_str8_list_pushf(arena,
                           list,
                           "\t{\n\t\t// Bind %.*s to %.*s\n",
                           oc_str8_ip(proc->handler),
                           oc_str8_ip(proc->name));

        oc_str8_list_push(arena, list, OC_STR8("\t\twa_value_type paramTypes[] = {"));

        for(int i = 0; i < proc->paramCount; i++)
        {
            oc_str8_list_push(arena, list, proc->params[i].typeDesc.wasmType);
            if(i < proc->paramCount - 1)
            {
                oc_str8_list_push(arena, list, OC_STR8(", "));
            }
        }

        oc_str8_list_push(arena, list, OC_STR8("};\n"));

        oc_str8_list_pushf(arena, list, "\t\twa_value_type returnTypes[] = {%.*s};\n", oc_str8_ip(proc->returnType.wasmType));
        oc_str8_list_pushf(arena, list, "\t\twa_import_binding binding = {0};\n");
        oc_str8_list_pushf(arena, list, "\t\tbinding.name = OC_STR8(\"%.*s\");\n", oc_str8_ip(proc->name));
        oc_str8_list_pushf(arena, list, "\t\tbinding.kind = WA_BINDING_HOST_FUNCTION;\n");
        oc_str8_list_pushf(arena, list, "\t\tbinding.hostFunction.proc = %.*s_stub;\n", oc_str8_ip(proc->handler));
        oc_str8_list_pushf(arena, list, "\t\tbinding.hostFunction.type.paramCount = %u;\n", proc->paramCount);
        oc_str8_list_pushf(arena, list, "\t\tbinding.hostFunction.type.returnCount = 1;\n");
        oc_str8_list_pushf(arena, list, "\t\tbinding.hostFunction.type.params = paramTypes;\n");
        oc_str8_list_pushf(arena, list, "\t\tbinding.hostFunction.type.returns = returnTypes;\n");
        oc_str8_list_pushf(arena, list, "\t\twa_import_package_push_binding(arena, package, &binding);\n");
        oc_str8_list_push(arena, list, OC_STR8("\t}\n"));
    }

    oc_str8_list_push(arena, list, OC_STR8("}\n"));
}

int main(int argc, char** argv)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8 inputPath = { 0 };
    oc_str8 hostcallPath = { 0 };
    oc_str8 bindingPath = { 0 };
    oc_str8 apiName = { 0 };

    oc_arg_parser argParser = { 0 };
    oc_arg_parser_init(&argParser,
                       scratch.arena,
                       OC_STR8("gen_host_interface"),
                       &(oc_arg_parser_options){
                           .desc = OC_STR8("Generates binding code from wasm to host APIs."),
                       });

    oc_arg_parser_add_named_str8(&argParser,
                                 OC_STR8("api"),
                                 &apiName,
                                 &(oc_arg_parser_arg_options){
                                     .desc = OC_STR8("name of the API to generate binding code for."),
                                     .required = true,
                                 });

    oc_arg_parser_add_named_str8(&argParser,
                                 OC_STR8("in"),
                                 &inputPath,
                                 &(oc_arg_parser_arg_options){
                                     .desc = OC_STR8("path of host interface specification file."),
                                     .required = true,
                                 });

    oc_arg_parser_add_named_str8(&argParser,
                                 OC_STR8("hostcalls"),
                                 &hostcallPath,
                                 &(oc_arg_parser_arg_options){
                                     .desc = OC_STR8("output path for hostcalls definitions."),
                                     .required = true,
                                 });

    oc_arg_parser_add_named_str8(&argParser,
                                 OC_STR8("binding"),
                                 &bindingPath,
                                 &(oc_arg_parser_arg_options){
                                     .desc = OC_STR8("output path for host binding code."),
                                     .required = true,
                                 });

    int result = oc_arg_parser_parse(&argParser, argc, argv);
    if(result)
    {
        return result;
    }

    oc_str8 contents = { 0 };
    {
        oc_file in = oc_catch(oc_file_open(inputPath, OC_FILE_ACCESS_READ, 0))
        {
            printf("Error: can't open input file '%.*s'\n", oc_str8_ip(inputPath));
            result = -1;
            goto end;
        }
        contents.len = oc_file_size(in);
        contents.ptr = oc_arena_push_array_uninitialized(scratch.arena, char, contents.len);
        oc_file_read(in, contents.len, contents.ptr);
        oc_file_close(in);
    }

    oc_file hostCallsFile = oc_catch(oc_file_open(hostcallPath,
                                                  OC_FILE_ACCESS_WRITE,
                                                  &(oc_file_open_options){
                                                      .flags = OC_FILE_OPEN_CREATE | OC_FILE_OPEN_TRUNCATE,
                                                  }))
    {
        printf("Error: can't open hostcalls header '%.*s' for writing\n", oc_str8_ip(hostcallPath));
        result = -1;
        goto end;
    }

    oc_file stubsFile = oc_catch(oc_file_open(bindingPath,
                                              OC_FILE_ACCESS_WRITE,
                                              &(oc_file_open_options){
                                                  .flags = OC_FILE_OPEN_CREATE | OC_FILE_OPEN_TRUNCATE,
                                              }))
    {
        printf("Error: can't open stubs file '%.*s' for writing\n", oc_str8_ip(bindingPath));
        result = -1;
        goto end;
    }

    oc_str8_list hostcallDeclList = { 0 };
    oc_str8_list hostapiBindingList = { 0 };

    json_parser parser = {
        .arena = scratch.arena,
        .contents = contents,
    };

    json_node* root = json_parse_item(&parser);
    if(root->kind != JSON_LIST)
    {
        printf("Error: invalid specification schema\n");
        result = -1;
        goto end;
    }

    //NOTE: parse proc defs
    oc_list procList = { 0 };
    oc_list_for(root->children, child, json_node, listElt)
    {
        if(child->kind == JSON_OBJECT)
        {
            json_node* kindNode = json_expect_field(child, OC_STR8("kind"), JSON_STRING);

            if(!oc_str8_cmp(kindNode->string, OC_STR8("proc")))
            {
                proc_desc* proc = parse_proc(scratch.arena, root, child);
                oc_list_push_back(&procList, &proc->listElt);
            }
        }
    }

    //NOTE: generate hostcall header
    {
        oc_str8_list_push(scratch.arena,
                          &hostcallDeclList,
                          OC_STR8("//---------------------------------------------------\n"));
        oc_str8_list_push(scratch.arena,
                          &hostcallDeclList,
                          OC_STR8("// hostcalls declarations generated from host_interface.json\n"));
        oc_str8_list_push(scratch.arena,
                          &hostcallDeclList,
                          OC_STR8("//---------------------------------------------------\n"));

        oc_str8_list_push(scratch.arena,
                          &hostcallDeclList,
                          OC_STR8("#pragma once\n\n"));

        oc_str8_list_push(scratch.arena,
                          &hostcallDeclList,
                          OC_STR8("#include \"orca.h\"\n\n"));

        //NOTE: forward declare types for hostcalls headers
        oc_list_for(root->children, child, json_node, listElt)
        {
            if(child->kind == JSON_OBJECT)
            {
                json_node* kindNode = json_expect_field(child, OC_STR8("kind"), JSON_STRING);

                if(!oc_str8_cmp(kindNode->string, OC_STR8("typename")))
                {
                    json_node* nameNode = json_expect_field(child, OC_STR8("name"), JSON_STRING);
                    json_node* typeNode = json_expect_field(child, OC_STR8("type"), JSON_OBJECT);
                    json_node* typeKindNode = json_expect_field(typeNode, OC_STR8("kind"), JSON_STRING);

                    if(!oc_str8_cmp(typeKindNode->string, OC_STR8("struct")))
                    {
                        oc_str8_list_pushf(scratch.arena,
                                           &hostcallDeclList,
                                           "typedef struct %.*s %.*s;\n",
                                           oc_str8_ip(nameNode->string),
                                           oc_str8_ip(nameNode->string));
                    }
                    else if(!oc_str8_cmp(typeKindNode->string, OC_STR8("union")))
                    {
                        oc_str8_list_pushf(scratch.arena,
                                           &hostcallDeclList,
                                           "typedef union %.*s %.*s;\n",
                                           oc_str8_ip(nameNode->string),
                                           oc_str8_ip(nameNode->string));
                    }
                }
            }
        }
        oc_str8_list_push(scratch.arena, &hostcallDeclList, OC_STR8("\n"));

        //NOTE: generate hostcalls prototypes
        oc_list_for(procList, proc, proc_desc, listElt)
        {
            gen_hostcall_prototype(scratch.arena, &hostcallDeclList, proc);
        }
    }

    //NOTE: generate hostapi stubs and binding code
    {
        oc_str8_list_push(scratch.arena,
                          &hostapiBindingList,
                          OC_STR8("//-----------------------------------------------------------\n"));
        oc_str8_list_push(scratch.arena,
                          &hostapiBindingList,
                          OC_STR8("// host api binding code generated from host_interface.json\n"));
        oc_str8_list_push(scratch.arena,
                          &hostapiBindingList,
                          OC_STR8("//-----------------------------------------------------------\n\n"));

        oc_str8_list_push(scratch.arena,
                          &hostapiBindingList,
                          OC_STR8("// Hostcall handlers prototypes\n"));

        oc_list_for(procList, proc, proc_desc, listElt)
        {
            gen_handler_prototype(scratch.arena, &hostapiBindingList, proc);
        }
        oc_str8_list_push(scratch.arena,
                          &hostapiBindingList,
                          OC_STR8("\n// Handler stubs\n"));

        oc_list_for(procList, proc, proc_desc, listElt)
        {
            gen_hostapi_stub(scratch.arena, &hostapiBindingList, proc);
        }

        oc_str8_list_push(scratch.arena,
                          &hostapiBindingList,
                          OC_STR8("\n// Handlers binding code\n"));

        gen_hostapi_binding(scratch.arena, &hostapiBindingList, apiName, procList);
    }

    oc_str8 hostcallDecl = oc_str8_list_join(scratch.arena, hostcallDeclList);
    oc_file_write(hostCallsFile, hostcallDecl.len, hostcallDecl.ptr);

    oc_str8 hostapiBinding = oc_str8_list_join(scratch.arena, hostapiBindingList);
    oc_file_write(stubsFile, hostapiBinding.len, hostapiBinding.ptr);

    oc_file_close(hostCallsFile);
    oc_file_close(stubsFile);

end:
    oc_scratch_end(scratch);

    if(result)
    {
        printf("FAILED\n");
    }
    return result;
}
