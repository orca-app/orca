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

void gen_type(oc_arena* arena, oc_str8_list* list, json_node* node)
{
    json_node* kindNode = json_expect_field(node, OC_STR8("kind"), JSON_STRING);

    if(!oc_str8_cmp(kindNode->string, OC_STR8("namedType")))
    {
        json_node* nameNode = json_expect_field(node, OC_STR8("name"), JSON_STRING);

        oc_str8_list_push(arena, list, nameNode->string);
    }
    else if(!oc_str8_cmp(kindNode->string, OC_STR8("pointer")))
    {
        json_node* typeNode = json_expect_field(node, OC_STR8("type"), JSON_OBJECT);

        gen_type(arena, list, typeNode);
        oc_str8_list_push(arena, list, OC_STR8("*"));
    }
    else
    {
        oc_str8_list_push(arena, list, kindNode->string);
    }
}

void gen_hostcall_prototype(oc_arena* arena, oc_str8_list* list, json_node* root, json_node* procNode)
{
    json_node* nameNode = json_expect_field(procNode, OC_STR8("name"), JSON_STRING);
    json_node* returnNode = json_expect_field(procNode, OC_STR8("return"), JSON_OBJECT);
    json_node* paramsNode = json_expect_field(procNode, OC_STR8("params"), JSON_LIST);

    gen_type(arena, list, returnNode);

    oc_str8_list_pushf(arena,
                       list,
                       " %.*s(",
                       oc_str8_ip(nameNode->string));

    if(oc_list_empty(paramsNode->children))
    {
        oc_str8_list_push(arena, list, OC_STR8("void"));
    }
    oc_list_for(paramsNode->children, paramNode, json_node, listElt)
    {
        if(paramNode->kind != JSON_OBJECT)
        {
            printf("Error: param specification must be an object.\n");
            exit(-1);
        }
        json_node* paramNameNode = json_expect_field(paramNode, OC_STR8("name"), JSON_STRING);
        json_node* paramTypeNode = json_expect_field(paramNode, OC_STR8("type"), JSON_OBJECT);

        gen_type(arena, list, paramTypeNode);
        oc_str8_list_pushf(arena, list, " %.*s", oc_str8_ip(paramNameNode->string));
        if(paramNode->listElt.next)
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

typedef struct param_type_desc
{
    oc_str8 string;
    bool isPointer;
} param_type_desc;

typedef struct param_desc
{
    oc_list_elt listElt;
    oc_str8 name;
    param_type_desc typeDesc;

} param_desc;

param_type_desc parse_param_type(oc_arena* arena, json_node* root, json_node* node)
{
    param_type_desc desc = { 0 };

    json_node* unwrappedTypeNode = unwrap_named_types(root, node);
    json_node* unwrappedKindNode = json_expect_field(unwrappedTypeNode, OC_STR8("kind"), JSON_STRING);

    if(!oc_str8_cmp(unwrappedKindNode->string, OC_STR8("pointer")))
    {
        desc.isPointer = true;
    }
    else if(!oc_str8_cmp(unwrappedKindNode->string, OC_STR8("struct")))
    {
        printf("Error: hostcall param should be a scalar or pointer.\n");
        exit(-1);
    }
    desc.string = get_native_type(arena, root, node);
    return desc;
}

param_desc* parse_param(oc_arena* arena, oc_list* paramList, json_node* root, json_node* paramNode)
{
    json_node* paramNameNode = json_expect_field(paramNode, OC_STR8("name"), JSON_STRING);
    json_node* paramTypeNode = json_expect_field(paramNode, OC_STR8("type"), JSON_OBJECT);

    json_node* unwrappedTypeNode = unwrap_named_types(root, paramTypeNode);
    json_node* unwrappedKindNode = json_expect_field(unwrappedTypeNode, OC_STR8("kind"), JSON_STRING);

    param_desc* param = oc_arena_push_type(arena, param_desc);
    param->name = paramNameNode->string;
    param->typeDesc = parse_param_type(arena, root, paramTypeNode);

    oc_list_push_back(paramList, &param->listElt);

    return param;
}

void gen_hostapi_binding(oc_arena* arena, oc_str8_list* list, json_node* root, json_node* procNode)
{
    json_node* nameNode = json_expect_field(procNode, OC_STR8("name"), JSON_STRING);
    json_node* handlerNode = json_expect_field(procNode, OC_STR8("handler"), JSON_STRING);
    json_node* returnNode = json_expect_field(procNode, OC_STR8("return"), JSON_OBJECT);
    json_node* paramsNode = json_expect_field(procNode, OC_STR8("params"), JSON_LIST);

    oc_str8_list_pushf(arena,
                       list,
                       "void %.*s_stub(wa_interpreter* interpreter, wa_value* params, wa_value* returns, void* user)\n{\n",
                       oc_str8_ip(handlerNode->string));

    param_type_desc returnDesc = parse_param_type(arena, root, returnNode);

    oc_list params = { 0 };

    oc_list_for_indexed(paramsNode->children, it, json_node, listElt)
    {
        json_node* paramNode = it.elt;

        param_desc* param = parse_param(arena, &params, root, paramNode);

        if(param->typeDesc.isPointer)
        {
            oc_str8_list_pushf(arena,
                               list,
                               "\t%.*s %.*s = (%.*s)((char*)mem + *(i32*)&params[%llu]);\n",
                               oc_str8_ip(param->typeDesc.string),
                               oc_str8_ip(param->name),
                               oc_str8_ip(param->typeDesc.string),
                               it.index);

            oc_str8_list_pushf(arena,
                               list,
                               "\tOC_ASSERT_DIALOG(((char*)%.*s >= (char*)_mem) && ((char*)%.*s - (char*)_mem) < _memSize, \"pointer is out of bounds\");\n",
                               oc_str8_ip(param->name),
                               oc_str8_ip(param->name));

            //TODO: apply pointer validation rules
        }
        else
        {
            oc_str8_list_pushf(arena,
                               list,
                               "\t%.*s %.*s = *(%.*s*)&params[%llu];\n",
                               oc_str8_ip(param->typeDesc.string),
                               oc_str8_ip(param->name),
                               oc_str8_ip(param->typeDesc.string),
                               it.index);
        }
    }

    if(!oc_list_empty(params))
    {
        oc_str8_list_push(arena, list, OC_STR8("\n"));
    }

    oc_str8_list_push(arena, list, OC_STR8("\t"));

    if(oc_str8_cmp(returnDesc.string, OC_STR8("void")))
    {
        if(returnDesc.isPointer)
        {
            printf("Error: return type should be a scalar.\n");
            exit(-1);
        }
        oc_str8_list_pushf(arena, list, "*(%.*s*)&ret[0] = ", oc_str8_ip(returnDesc.string));
    }

    oc_str8_list_pushf(arena, list, "%.*s(", oc_str8_ip(handlerNode->string));
    oc_list_for(params, param, param_desc, listElt)
    {
        oc_str8_list_push(arena, list, param->name);
        if(param->listElt.next)
        {
            oc_str8_list_push(arena, list, OC_STR8(", "));
        }
    }
    oc_str8_list_push(arena, list, OC_STR8(");\n}\n\n"));
}

int main(int argc, char** argv)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_str8 inputPath = { 0 };
    oc_str8 hostcallPath = { 0 };
    oc_str8 bindingPath = { 0 };

    oc_arg_parser argParser = { 0 };
    oc_arg_parser_init(&argParser,
                       scratch.arena,
                       OC_STR8("gen_host_interface"),
                       &(oc_arg_parser_options){
                           .desc = OC_STR8("Generates binding code from wasm to host APIs."),
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
    if(result == 0)
    {
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

        //NOTE: forward declare types
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
                }
            }
        }

        oc_str8_list_push(scratch.arena, &hostcallDeclList, OC_STR8("\n"));

        oc_list_for(root->children, child, json_node, listElt)
        {
            if(child->kind == JSON_OBJECT)
            {
                json_node* kindNode = json_expect_field(child, OC_STR8("kind"), JSON_STRING);

                if(!oc_str8_cmp(kindNode->string, OC_STR8("proc")))
                {
                    //NOTE: generate hostcall definition
                    gen_hostcall_prototype(scratch.arena, &hostcallDeclList, root, child);
                    gen_hostapi_binding(scratch.arena, &hostapiBindingList, root, child);
                }
            }
        }

        //TODO: generate code to bind stubs to wasm import names

        printf("\n --- host call declarations ---\n");
        oc_str8 hostcallDecl = oc_str8_list_join(scratch.arena, hostcallDeclList);
        printf("%.*s\n", oc_str8_ip(hostcallDecl));

        printf("\n --- host api bindings ---\n");
        oc_str8 hostapiBinding = oc_str8_list_join(scratch.arena, hostapiBindingList);
        printf("%.*s\n", oc_str8_ip(hostapiBinding));
    }

end:
    oc_scratch_end(scratch);
    return result;
}
