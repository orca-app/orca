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
                    json_node* nameNode = json_expect_field(child, OC_STR8("name"), JSON_STRING);
                    if(!nameNode)
                    {
                        result = -1;
                        goto end;
                    }
                    json_node* handlerNode = json_expect_field(child, OC_STR8("handler"), JSON_STRING);
                    if(!handlerNode)
                    {
                        result = -1;
                        goto end;
                    }
                    json_node* returnNode = json_expect_field(child, OC_STR8("return"), JSON_OBJECT);
                    if(!returnNode)
                    {
                        result = -1;
                        goto end;
                    }
                    json_node* paramsNode = json_expect_field(child, OC_STR8("params"), JSON_LIST);
                    if(!paramsNode)
                    {
                        result = -1;
                        goto end;
                    }

                    //NOTE: generate hostcall declaration
                    gen_type(scratch.arena, &hostcallDeclList, returnNode);
                    oc_str8_list_pushf(scratch.arena,
                                       &hostcallDeclList,
                                       " %.*s(",
                                       oc_str8_ip(nameNode->string));

                    if(oc_list_empty(paramsNode->children))
                    {
                        oc_str8_list_push(scratch.arena, &hostcallDeclList, OC_STR8("void"));
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
                        gen_type(scratch.arena, &hostcallDeclList, paramTypeNode);
                        oc_str8_list_pushf(scratch.arena, &hostcallDeclList, " %.*s", oc_str8_ip(paramNameNode->string));
                        if(paramNode->listElt.next)
                        {
                            oc_str8_list_push(scratch.arena, &hostcallDeclList, OC_STR8(", "));
                        }
                    }
                    oc_str8_list_push(scratch.arena, &hostcallDeclList, OC_STR8(");\n"));
                }
            }
        }
        oc_str8 hostcallDecl = oc_str8_list_join(scratch.arena, hostcallDeclList);
        printf("%.*s\n", oc_str8_ip(hostcallDecl));
    }

end:
    oc_scratch_end(scratch);
    return result;
}
