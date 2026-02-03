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
    }
    else if(child->kind != kind)
    {
        printf("Error: unexpected kind %s for field %.*s (expected %s)\n",
               json_node_kind_strings[child->kind],
               oc_str8_ip(name),
               json_node_kind_strings[kind]);
        child = 0;
    }
    return child;
}

int gen_type(oc_arena* arena, oc_str8_list* list, json_node* node)
{
    int result = 0;

    json_node* kindNode = json_expect_field(node, OC_STR8("kind"), JSON_STRING);
    if(!kindNode)
    {
        result = -1;
        goto end;
    }
    if(!oc_str8_cmp(kindNode->string, OC_STR8("namedType")))
    {
        json_node* nameNode = json_expect_field(node, OC_STR8("name"), JSON_STRING);
        if(!nameNode)
        {
            result = -1;
            goto end;
        }
        oc_str8_list_push(arena, list, nameNode->string);
    }
    else if(!oc_str8_cmp(kindNode->string, OC_STR8("pointer")))
    {
        json_node* typeNode = json_expect_field(node, OC_STR8("type"), JSON_OBJECT);
        if(!typeNode)
        {
            result = -1;
            goto end;
        }
        gen_type(arena, list, typeNode);
        oc_str8_list_push(arena, list, OC_STR8("*"));
    }
    else
    {
        oc_str8_list_push(arena, list, kindNode->string);
    }

end:
    return result;
}

int pass_by_value(json_node* root, json_node* typeNode, bool* passByValue)
{
    int result = 0;

    json_node* typeKindNode = json_expect_field(typeNode, OC_STR8("kind"), JSON_STRING);
    if(!typeKindNode)
    {
        result = -1;
        goto end;
    }

    if(!oc_str8_cmp(typeKindNode->string, OC_STR8("struct")))
    {
        *passByValue = false;
    }
    else
    {
        *passByValue = true;
    }

end:
    return result;
}

int gen_hostcall_prototype(oc_arena* arena, oc_str8_list* list, json_node* root, json_node* procNode)
{
    int result = 0;

    json_node* nameNode = json_expect_field(procNode, OC_STR8("name"), JSON_STRING);
    if(!nameNode)
    {
        result = -1;
        goto end;
    }
    json_node* returnNode = json_expect_field(procNode, OC_STR8("return"), JSON_OBJECT);
    if(!returnNode)
    {
        result = -1;
        goto end;
    }
    json_node* paramsNode = json_expect_field(procNode, OC_STR8("params"), JSON_LIST);
    if(!paramsNode)
    {
        result = -1;
        goto end;
    }

    bool returnByValue;
    if(pass_by_value(root, returnNode, &returnByValue))
    {
        result = -1;
        goto end;
    }

    if(returnByValue)
    {
        gen_type(arena, list, returnNode);
    }
    else
    {
        oc_str8_list_push(arena, list, OC_STR8("void"));
    }

    oc_str8_list_pushf(arena,
                       list,
                       " %.*s(",
                       oc_str8_ip(nameNode->string));

    if(oc_list_empty(paramsNode->children) && returnByValue)
    {
        oc_str8_list_push(arena, list, OC_STR8("void"));
    }
    oc_list_for(paramsNode->children, paramNode, json_node, listElt)
    {
        if(paramNode->kind != JSON_OBJECT)
        {
            result = -1;
            goto end;
        }
        json_node* paramNameNode = json_expect_field(paramNode, OC_STR8("name"), JSON_STRING);
        if(!paramNameNode)
        {
            result = -1;
            goto end;
        }
        json_node* paramTypeNode = json_expect_field(paramNode, OC_STR8("type"), JSON_OBJECT);
        if(!paramTypeNode)
        {
            result = -1;
            goto end;
        }
        gen_type(arena, list, paramTypeNode);
        oc_str8_list_pushf(arena, list, " %.*s", oc_str8_ip(paramNameNode->string));
        if(paramNode->listElt.next || !returnByValue)
        {
            oc_str8_list_push(arena, list, OC_STR8(", "));
        }
    }

    if(!returnByValue)
    {
        gen_type(arena, list, returnNode);
        oc_str8_list_push(arena, list, OC_STR8("* _returnPointer"));
    }

    oc_str8_list_push(arena, list, OC_STR8(");\n"));

end:
    return result;
}

json_node* find_named_type(json_node* root, oc_str8 name)
{
    oc_list_for(root->children, child, json_node, listElt)
    {
        json_node* kindNode = json_expect_field(child, OC_STR8("kind"), JSON_STRING);
        if(!kindNode)
        {
            return 0;
        }
        if(!oc_str8_cmp(kindNode->string, OC_STR8("typename")))
        {
            json_node* nameNode = json_expect_field(child, OC_STR8("name"), JSON_STRING);
            if(!nameNode)
            {
                return 0;
            }
            if(!oc_str8_cmp(nameNode->string, name))
            {
                return child;
            }
        }
    }

    printf("Error: typename %.*s not found\n", oc_str8_ip(name));
    return 0;
}

int get_native_type(oc_arena* arena, oc_str8* typeString, json_node* root, json_node* node)
{
    json_node* kindNode = json_expect_field(node, OC_STR8("kind"), JSON_STRING);
    if(!kindNode)
    {
        return -1;
    }

    if(!oc_str8_cmp(kindNode->string, OC_STR8("namedType")))
    {
        json_node* nameNode = json_expect_field(node, OC_STR8("name"), JSON_STRING);
        if(!nameNode)
        {
            return -1;
        }

        json_node* typenameNode = find_named_type(root, nameNode->string);
        if(!typenameNode)
        {
            return -1;
        }

        json_node* hostNode = json_find(typenameNode, OC_STR8("host"));
        if(hostNode)
        {
            if(hostNode->kind != JSON_STRING)
            {
                printf("Error: 'host' property should be a string.\n");
                return -1;
            }
            *typeString = hostNode->string;
        }
        else
        {
            *typeString = nameNode->string;
        }
    }
    else if(!oc_str8_cmp(kindNode->string, OC_STR8("pointer")))
    {
        json_node* typeNode = json_expect_field(node, OC_STR8("type"), JSON_OBJECT);
        if(!typeNode)
        {
            return -1;
        }
        oc_str8 sub = { 0 };
        if(get_native_type(arena, &sub, root, typeNode))
        {
            return -1;
        }
        *typeString = oc_str8_pushf(arena, "%.*s*", oc_str8_ip(sub));
    }
    else if(!oc_str8_cmp(kindNode->string, OC_STR8("struct")))
    {
        printf("Error: struct type found where namedType would be expected\n");
        return -1;
    }
    else
    {
        *typeString = kindNode->string;
    }

    return 0;
}

int gen_hostapi_binding(oc_arena* arena, oc_str8_list* list, json_node* root, json_node* procNode)
{
    int result = 0;

    json_node* nameNode = json_expect_field(procNode, OC_STR8("name"), JSON_STRING);
    if(!nameNode)
    {
        result = -1;
        goto end;
    }
    json_node* handlerNode = json_expect_field(procNode, OC_STR8("handler"), JSON_STRING);
    if(!handlerNode)
    {
        result = -1;
        goto end;
    }
    json_node* returnNode = json_expect_field(procNode, OC_STR8("return"), JSON_OBJECT);
    if(!returnNode)
    {
        result = -1;
        goto end;
    }
    json_node* paramsNode = json_expect_field(procNode, OC_STR8("params"), JSON_LIST);
    if(!paramsNode)
    {
        result = -1;
        goto end;
    }

    oc_str8_list_pushf(arena,
                       list,
                       "void %.*s_stub(wa_interpreter* interpreter, wa_value* params, wa_value* returns, void* user)\n{\n",
                       oc_str8_ip(handlerNode->string));

    oc_list_for_indexed(paramsNode->children, it, json_node, listElt)
    {
        json_node* paramNode = it.elt;

        json_node* paramNameNode = json_expect_field(paramNode, OC_STR8("name"), JSON_STRING);
        if(!paramNameNode)
        {
            result = -1;
            goto end;
        }
        json_node* paramTypeNode = json_expect_field(paramNode, OC_STR8("type"), JSON_OBJECT);
        if(!paramTypeNode)
        {
            result = -1;
            goto end;
        }

        oc_str8 paramType = { 0 };
        if(get_native_type(arena, &paramType, root, paramTypeNode))
        {
            result = -1;
            goto end;
        }

        bool passByValue;
        if(pass_by_value(root, paramType, &passByValue))
        {
            return -1;
        }
        if(passByValue)
        {
            oc_str8_list_pushf(arena,
                               list,
                               "\t%.*s %.*s = *(%.*s*)&params[%llu];\n",
                               oc_str8_ip(paramType),
                               oc_str8_ip(paramNameNode->string),
                               oc_str8_ip(paramType),
                               it.index);
        }
        else
        {
            oc_str8_list_pushf(arena,
                               list,
                               "\t%.*s* %.*s = (%.*s*)((char*)mem + *(i32*)&params[%llu]);\n",
                               oc_str8_ip(paramType),
                               oc_str8_ip(paramNameNode->string),
                               oc_str8_ip(paramType),
                               it.index);

            //TODO: check that pointer is valid
        }
    }
    //TODO: call handler

    oc_str8_list_push(arena, list, OC_STR8("}\n"));

end:
    return result;
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
                if(!kindNode)
                {
                    result = -1;
                    goto end;
                }

                if(!oc_str8_cmp(kindNode->string, OC_STR8("typename")))
                {
                    json_node* nameNode = json_expect_field(child, OC_STR8("name"), JSON_STRING);
                    if(!nameNode)
                    {
                        result = -1;
                        goto end;
                    }
                    json_node* typeNode = json_expect_field(child, OC_STR8("type"), JSON_OBJECT);
                    if(!typeNode)
                    {
                        result = -1;
                        goto end;
                    }
                    json_node* typeKindNode = json_expect_field(typeNode, OC_STR8("kind"), JSON_STRING);
                    if(!typeKindNode)
                    {
                        result = -1;
                        goto end;
                    }
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
                if(!kindNode)
                {
                    result = -1;
                    goto end;
                }

                if(!oc_str8_cmp(kindNode->string, OC_STR8("proc")))
                {
                    //NOTE: generate hostcall definition
                    if(gen_hostcall_prototype(scratch.arena, &hostcallDeclList, root, child))
                    {
                        result = -1;
                        goto end;
                    }

                    if(gen_hostapi_binding(scratch.arena, &hostapiBindingList, root, child))
                    {
                        result = -1;
                        goto end;
                    }
                }
            }
        }
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
