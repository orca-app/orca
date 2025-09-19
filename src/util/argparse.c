/*************************************************************************
*
*  Orca
*  Copyright 2025 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include <stdio.h>
#include <stdarg.h>

#include "argparse.h"

//-----------------------------------------------------------------------
// Internal structs
//-----------------------------------------------------------------------
typedef enum oc_arg_parser_type
{
    OC_ARG_PARSER_STR8,
    OC_ARG_PARSER_I64,
    OC_ARG_PARSER_F64,
    OC_ARG_PARSER_FLAG,
} oc_arg_parser_type;

typedef struct oc_arg_parser_arg
{
    oc_list_elt listElt;

    oc_str8 name;
    oc_arg_parser_arg_options options;
    oc_arg_parser_type type;
    bool isArray;
    bool positional;

    union
    {
        oc_str8* valStr8;
        i64* valI64;
        f64* valF64;
        bool* valBool;

        struct
        {
            u32* count;
            oc_str8** values;
        } arrayStr8;

        struct
        {
            u32* count;
            i64** values;
        } arrayI64;

        struct
        {
            u32* count;
            f64** values;
        } arrayF64;

        u32* flagCount;

    } dest;

    //NOTE: used to collect array-values before allocating dest arrays
    u32 count;
    oc_list values;

} oc_arg_parser_arg;

typedef struct oc_arg_parser_value_elt
{
    oc_list_elt listElt;

    union
    {
        oc_str8 valStr8;
        i64 valI64;
        f64 valF64;
    };
} oc_arg_parser_value_elt;

//-----------------------------------------------------------------------
// Arg parser builder
//-----------------------------------------------------------------------
void oc_arg_parser_init(oc_arg_parser* parser, oc_arena* arena, oc_str8 name, oc_arg_parser_options* options)
{
    memset(parser, 0, sizeof(oc_arg_parser));
    parser->arena = arena;
    parser->name = name;
    if(options)
    {
        parser->options = *options;
    }
    oc_arg_parser_add_flag(parser,
                           OC_STR8("help"),
                           &parser->askHelp,
                           &(oc_arg_parser_arg_options){
                               .shortName = 'h',
                               .desc = OC_STR8("Print a short help mesage."),
                               .stop = true,
                           });
}

oc_arg_parser* oc_arg_parser_subparser(oc_arg_parser* parser, oc_str8 name, oc_str8* dest, oc_arg_parser_options* options)
{
    OC_ASSERT(parser, "parser argument must be a non-null");
    OC_ASSERT(name.len && name.ptr, "name argument must be a non-empty string.");
    OC_ASSERT(parser->posArgCount == 0, "can't add a subcommand to a parser that has positional arguments.");

    oc_arg_parser* subparser = oc_arena_push_type(parser->arena, oc_arg_parser);
    subparser->arena = parser->arena;
    subparser->name = name;
    subparser->commandDest = dest;
    if(options)
    {
        subparser->options = *options;
    }
    oc_list_push_back(&parser->subParsers, &subparser->listElt);
    parser->subParserCount++;
    subparser->parent = parser;

    oc_arg_parser_add_flag(subparser,
                           OC_STR8("help"),
                           &parser->askHelp,
                           &(oc_arg_parser_arg_options){
                               .shortName = 'h',
                               .desc = OC_STR8("Print a short help mesage."),
                               .stop = true,
                           });

    return subparser;
}

oc_arg_parser_arg* oc_arg_parser_arg_alloc(oc_arg_parser* parser, oc_str8 name, oc_arg_parser_type type, oc_arg_parser_arg_options* options)
{
    oc_arg_parser_arg* arg = oc_arena_push_type(parser->arena, oc_arg_parser_arg);
    arg->name = name;
    arg->type = type;
    if(options)
    {
        arg->options = *options;
    }
    return arg;
}

#define OC_ARG_PARSER_NAMED_ASSERTS()                                           \
    OC_ASSERT(dest, "dest argument must be non-null.");                         \
    OC_ASSERT(name.len&& name.ptr, "name argument must be a non-empty string"); \
    OC_DEBUG_ASSERT(options->nargs == 0, "options->nargs is ignored for single-value arguments.");

int oc_arg_parser_add_named_str8(oc_arg_parser* parser, oc_str8 name, oc_str8* dest, oc_arg_parser_arg_options* options)
{
    OC_ARG_PARSER_NAMED_ASSERTS();
    oc_arg_parser_arg* arg = oc_arg_parser_arg_alloc(parser, name, OC_ARG_PARSER_STR8, options);
    arg->dest.valStr8 = dest;

    oc_list_push_back(&parser->namedArgs, &arg->listElt);
    parser->namedArgCount++;

    return 0;
}

int oc_arg_parser_add_named_i64(oc_arg_parser* parser, oc_str8 name, i64* dest, oc_arg_parser_arg_options* options)
{
    OC_ARG_PARSER_NAMED_ASSERTS();

    oc_arg_parser_arg* arg = oc_arg_parser_arg_alloc(parser, name, OC_ARG_PARSER_I64, options);
    arg->dest.valI64 = dest;

    oc_list_push_back(&parser->namedArgs, &arg->listElt);
    parser->namedArgCount++;

    return 0;
}

int oc_arg_parser_add_named_f64(oc_arg_parser* parser, oc_str8 name, f64* dest, oc_arg_parser_arg_options* options)
{
    OC_ARG_PARSER_NAMED_ASSERTS();

    oc_arg_parser_arg* arg = oc_arg_parser_arg_alloc(parser, name, OC_ARG_PARSER_F64, options);
    arg->dest.valF64 = dest;

    oc_list_push_back(&parser->namedArgs, &arg->listElt);
    parser->namedArgCount++;

    return 0;
}

int oc_arg_parser_add_flag(oc_arg_parser* parser, oc_str8 name, bool* dest, oc_arg_parser_arg_options* options)
{
    OC_ARG_PARSER_NAMED_ASSERTS();
    OC_DEBUG_ASSERT(options->valueName.len == 0, "options->valueName is ignored for flag arguments.");

    oc_arg_parser_arg* arg = oc_arg_parser_arg_alloc(parser, name, OC_ARG_PARSER_FLAG, options);
    arg->dest.valBool = dest;

    oc_list_push_back(&parser->namedArgs, &arg->listElt);
    parser->namedArgCount++;

    return 0;
}

#define OC_ARG_PARSER_NAMED_ARRAY_ASSERTS()                                     \
    OC_ASSERT(destCount, "destCount argument must be non-null.");               \
    OC_ASSERT(destArray, "destArray argument must be non-null.");               \
    OC_ASSERT(name.len&& name.ptr, "name argument must be a non-empty string"); \
    OC_DEBUG_ASSERT(options->defaultValue.valStr8.len == 0                      \
                        && options->defaultValue.valStr8.ptr == 0,              \
                    "options->defaultValue is ignored for array-value arguments.");

int oc_arg_parser_add_named_str8_array(oc_arg_parser* parser, oc_str8 name, u32* destCount, oc_str8** destArray, oc_arg_parser_arg_options* options)
{
    OC_ARG_PARSER_NAMED_ARRAY_ASSERTS();

    oc_arg_parser_arg* arg = oc_arg_parser_arg_alloc(parser, name, OC_ARG_PARSER_STR8, options);
    arg->isArray = true;
    arg->dest.arrayStr8.count = destCount;
    arg->dest.arrayStr8.values = destArray;

    oc_list_push_back(&parser->namedArgs, &arg->listElt);
    parser->namedArgCount++;

    return 0;
}

int oc_arg_parser_add_named_i64_array(oc_arg_parser* parser, oc_str8 name, u32* destCount, i64** destArray, oc_arg_parser_arg_options* options)
{
    OC_ARG_PARSER_NAMED_ARRAY_ASSERTS();

    oc_arg_parser_arg* arg = oc_arg_parser_arg_alloc(parser, name, OC_ARG_PARSER_I64, options);
    arg->isArray = true;
    arg->dest.arrayI64.count = destCount;
    arg->dest.arrayI64.values = destArray;

    oc_list_push_back(&parser->namedArgs, &arg->listElt);
    parser->namedArgCount++;

    return 0;
}

int oc_arg_parser_add_named_f64_array(oc_arg_parser* parser, oc_str8 name, u32* destCount, f64** destArray, oc_arg_parser_arg_options* options)
{
    OC_ARG_PARSER_NAMED_ARRAY_ASSERTS();

    oc_arg_parser_arg* arg = oc_arg_parser_arg_alloc(parser, name, OC_ARG_PARSER_F64, options);
    arg->isArray = true;
    arg->dest.arrayF64.count = destCount;
    arg->dest.arrayF64.values = destArray;

    oc_list_push_back(&parser->namedArgs, &arg->listElt);
    parser->namedArgCount++;

    return 0;
}

int oc_arg_parser_add_flag_count(oc_arg_parser* parser, oc_str8 name, u32* destCount, oc_arg_parser_arg_options* options)
{
    OC_ASSERT(destCount, "destCount argument must be non-null.");
    OC_ASSERT(name.len && name.ptr, "name argument must be a non-empty string");
    OC_DEBUG_ASSERT(options->defaultValue.valStr8.len == 0
                        && options->defaultValue.valStr8.ptr == 0,
                    "options->defaultValue is ignored for array-value arguments.");

    oc_arg_parser_arg* arg = oc_arg_parser_arg_alloc(parser, name, OC_ARG_PARSER_FLAG, options);
    arg->isArray = true;
    arg->dest.flagCount = destCount;

    oc_list_push_back(&parser->namedArgs, &arg->listElt);
    parser->namedArgCount++;

    return 0;
}

#define OC_ARG_PARSER_POSITIONAL_ASSERTS()                                                                                                  \
    OC_ASSERT(dest, "dest argument must be non-null.");                                                                                     \
    OC_ASSERT(name.len&& name.ptr, "name argument must be a non-empty string");                                                             \
    OC_DEBUG_ASSERT(options->shortName == 0, "options->shortName is ignored for positional arguments.");                                    \
    OC_DEBUG_ASSERT(options->valueName.len == 0 && options->valueName.ptr == 0, "options->valueName is ignored for positional arguments."); \
    OC_DEBUG_ASSERT(options->allowRepeat == 0, "options->allowRepeat is ignored for positional arguments.");                                \
    OC_DEBUG_ASSERT(options->nargs == 0, "options->nargs is ignored for single-value arguments.");                                          \
    OC_ASSERT(parser->subParserCount == 0, "can't add positional arguments to a parser that has subcommands.");

int oc_arg_parser_add_positional_str8(oc_arg_parser* parser, oc_str8 name, oc_str8* dest, oc_arg_parser_arg_options* options)
{
    OC_ARG_PARSER_POSITIONAL_ASSERTS();

    oc_arg_parser_arg* arg = oc_arg_parser_arg_alloc(parser, name, OC_ARG_PARSER_STR8, options);
    arg->positional = true;
    arg->dest.valStr8 = dest;

    oc_list_push_back(&parser->posArgs, &arg->listElt);
    parser->posArgCount++;

    return 0;
}

int oc_arg_parser_add_positional_i64(oc_arg_parser* parser, oc_str8 name, i64* dest, oc_arg_parser_arg_options* options)
{
    OC_ARG_PARSER_POSITIONAL_ASSERTS();

    oc_arg_parser_arg* arg = oc_arg_parser_arg_alloc(parser, name, OC_ARG_PARSER_I64, options);
    arg->positional = true;
    arg->dest.valI64 = dest;

    oc_list_push_back(&parser->posArgs, &arg->listElt);
    parser->posArgCount++;

    return 0;
}

int oc_arg_parser_add_positional_f64(oc_arg_parser* parser, oc_str8 name, f64* dest, oc_arg_parser_arg_options* options)
{
    OC_ARG_PARSER_POSITIONAL_ASSERTS();

    oc_arg_parser_arg* arg = oc_arg_parser_arg_alloc(parser, name, OC_ARG_PARSER_F64, options);
    arg->positional = true;
    arg->dest.valF64 = dest;

    oc_list_push_back(&parser->posArgs, &arg->listElt);
    parser->posArgCount++;

    return 0;
}

#define OC_ARG_PARSER_POSITIONAL_ARRAY_ASSERTS()                                                                                            \
    OC_ASSERT(destCount, "destCount argument must be non-null.");                                                                           \
    OC_ASSERT(destArray, "destArray argument must be non-null.");                                                                           \
    OC_ASSERT(name.len&& name.ptr, "name argument must be a non-empty string");                                                             \
    OC_DEBUG_ASSERT(options->shortName == 0, "options->shortName is ignored for positional arguments.");                                    \
    OC_DEBUG_ASSERT(options->valueName.len == 0 && options->valueName.ptr == 0, "options->valueName is ignored for positional arguments."); \
    OC_DEBUG_ASSERT(options->allowRepeat == 0, "options->allowRepeat is ignored for positional arguments.");                                \
    OC_ASSERT(parser->subParserCount == 0, "can't add positional arguments to a parser that has subcommands.");                             \
    OC_DEBUG_ASSERT(options->defaultValue.valStr8.len == 0                                                                                  \
                        && options->defaultValue.valStr8.ptr == 0,                                                                          \
                    "options->defaultValue is ignored for array-value arguments.");

int oc_arg_parser_add_positional_str8_array(oc_arg_parser* parser, oc_str8 name, u32* destCount, oc_str8** destArray, oc_arg_parser_arg_options* options)
{
    OC_ARG_PARSER_POSITIONAL_ARRAY_ASSERTS();

    oc_arg_parser_arg* arg = oc_arg_parser_arg_alloc(parser, name, OC_ARG_PARSER_STR8, options);
    arg->isArray = true;
    arg->positional = true;
    arg->dest.arrayStr8.count = destCount;
    arg->dest.arrayStr8.values = destArray;

    oc_list_push_back(&parser->posArgs, &arg->listElt);
    parser->posArgCount++;

    return 0;
}

int oc_arg_parser_add_positional_i64_array(oc_arg_parser* parser, oc_str8 name, u32* destCount, i64** destArray, oc_arg_parser_arg_options* options)
{
    OC_ARG_PARSER_POSITIONAL_ARRAY_ASSERTS();

    oc_arg_parser_arg* arg = oc_arg_parser_arg_alloc(parser, name, OC_ARG_PARSER_I64, options);
    arg->isArray = true;
    arg->positional = true;
    arg->dest.arrayI64.count = destCount;
    arg->dest.arrayI64.values = destArray;

    oc_list_push_back(&parser->posArgs, &arg->listElt);
    parser->posArgCount++;

    return 0;
}

int oc_arg_parser_add_positional_f64_array(oc_arg_parser* parser, oc_str8 name, u32* destCount, f64** destArray, oc_arg_parser_arg_options* options)
{
    OC_ARG_PARSER_POSITIONAL_ARRAY_ASSERTS();

    oc_arg_parser_arg* arg = oc_arg_parser_arg_alloc(parser, name, OC_ARG_PARSER_F64, options);
    arg->isArray = true;
    arg->positional = true;
    arg->dest.arrayF64.count = destCount;
    arg->dest.arrayF64.values = destArray;

    oc_list_push_back(&parser->posArgs, &arg->listElt);
    parser->posArgCount++;

    return 0;
}

//-----------------------------------------------------------------------
// Help / Error reporting
//-----------------------------------------------------------------------

void oc_arg_parser_print_command(oc_arg_parser* parser)
{
    if(parser->parent)
    {
        oc_arg_parser_print_command(parser->parent);
        printf(" ");
    }
    printf("%.*s", oc_str8_ip(parser->name));
}

void oc_arg_parser_print_help(oc_arg_parser* parser)
{
    printf("usage: ");
    oc_arg_parser_print_command(parser);

    if(parser->namedArgCount > 5)
    {
        printf(" [options]");
    }
    else if(parser->namedArgCount)
    {
        //NOTE: print short flags
        bool shortFlags = false;
        oc_list_for(parser->namedArgs, arg, oc_arg_parser_arg, listElt)
        {
            if(arg->options.shortName != 0 && arg->type == OC_ARG_PARSER_FLAG)
            {
                if(!shortFlags)
                {
                    printf(" [-");
                    shortFlags = true;
                }
                printf("%c", arg->options.shortName);
            }
        }
        if(shortFlags)
        {
            printf("]");
        }

        //NOTE: print short required named args
        oc_list_for(parser->namedArgs, arg, oc_arg_parser_arg, listElt)
        {
            if(arg->options.shortName != 0 && arg->type != OC_ARG_PARSER_FLAG && arg->options.required)
            {
                oc_str8 valueName = arg->options.valueName.len ? arg->options.valueName : arg->name;
                printf(" -%c <%.*s>", arg->options.shortName, oc_str8_ip(valueName));
            }
        }

        //NOTE: print short optional named args
        bool shortOptionals = false;
        oc_list_for(parser->namedArgs, arg, oc_arg_parser_arg, listElt)
        {
            if(arg->options.shortName != 0 && arg->type != OC_ARG_PARSER_FLAG && !arg->options.required)
            {
                if(!shortOptionals)
                {
                    printf(" [-");
                    shortOptionals = true;
                }
                oc_str8 valueName = arg->options.valueName.len ? arg->options.valueName : arg->name;
                printf(" -%c <%.*s>", arg->options.shortName, oc_str8_ip(valueName));
            }
        }
        if(shortOptionals)
        {
            printf("]");
        }

        //NOTE: print long flags
        oc_list_for(parser->namedArgs, arg, oc_arg_parser_arg, listElt)
        {
            if(arg->options.shortName == 0 && arg->type == OC_ARG_PARSER_FLAG)
            {
                printf(" [--%.*s]", oc_str8_ip(arg->name));
            }
        }

        //NOTE: print long required arguments
        oc_list_for(parser->namedArgs, arg, oc_arg_parser_arg, listElt)
        {
            if(arg->options.shortName == 0 && arg->type != OC_ARG_PARSER_FLAG && arg->options.required)
            {
                oc_str8 valueName = arg->options.valueName.len ? arg->options.valueName : arg->name;
                printf(" --%.*s <%.*s>", oc_str8_ip(arg->name), oc_str8_ip(valueName));
            }
        }

        //NOTE: print long optional arguments
        oc_list_for(parser->namedArgs, arg, oc_arg_parser_arg, listElt)
        {
            if(arg->options.shortName == 0 && arg->type != OC_ARG_PARSER_FLAG && !arg->options.required)
            {
                oc_str8 valueName = arg->options.valueName.len ? arg->options.valueName : arg->name;
                printf(" [--%.*s <%.*s>]", oc_str8_ip(arg->name), oc_str8_ip(valueName));
            }
        }
    }

    //NOTE: print subcommands
    if(parser->subParserCount > 5)
    {
        if(parser->options.requireCommand)
        {
            printf(" <command> ...");
        }
        else
        {
            printf(" [command] ...");
        }
    }
    else if(parser->subParserCount)
    {
        printf(" %s", parser->options.requireCommand ? "<" : "[");

        oc_list_for_indexed(parser->subParsers, it, oc_arg_parser, listElt)
        {
            if(it.index)
            {
                printf("|");
            }
            printf("%.*s", oc_str8_ip(it.elt->name));
        }

        printf("%s", parser->options.requireCommand ? ">" : "]");
        printf(" ...");
    }

    if(parser->posArgCount)
    {
        oc_list_for(parser->posArgs, arg, oc_arg_parser_arg, listElt)
        {
            printf(" %s%.*s%s",
                   arg->options.required ? "" : "[",
                   oc_str8_ip(arg->name),
                   arg->options.required ? "" : "]");
        }
    }
    printf("\n");

    if(parser->options.desc.len)
    {
        printf("\n%.*s\n", oc_str8_ip(parser->options.desc));
    }
    printf("\n");

    //NOTE: print named args descriptions
    if(parser->namedArgCount)
    {
        printf("Options:\n");
        oc_list_for(parser->namedArgs, arg, oc_arg_parser_arg, listElt)
        {
            if(arg->options.shortName != 0)
            {
                printf("    -%c,\n", arg->options.shortName);
            }
            printf("    --%.*s", oc_str8_ip(arg->name));
            if(arg->type != OC_ARG_PARSER_FLAG)
            {
                oc_str8 valueName = arg->options.valueName.len ? arg->options.valueName : arg->name;
                printf(" <%.*s>", oc_str8_ip(valueName));
            }
            if(arg->options.required)
            {
                printf(" (required)");
            }
            printf("\n");
            if(arg->options.desc.len)
            {
                printf("        %.*s\n", oc_str8_ip(arg->options.desc));
            }
            printf("\n");
        }
    }

    //NOTE: print pos args descriptions
    if(parser->posArgCount)
    {
        printf("Positional arguments:\n");
        oc_list_for(parser->posArgs, arg, oc_arg_parser_arg, listElt)
        {
            printf("    %.*s%s\n", oc_str8_ip(arg->name), arg->options.required ? "(required)" : "");
            if(arg->options.desc.len)
            {
                printf("        %.*s\n", oc_str8_ip(arg->options.desc));
            }
            printf("\n");
        }
    }

    if(parser->subParserCount)
    {
        printf("Sub-commands:\n");
        oc_list_for(parser->subParsers, subparser, oc_arg_parser, listElt)
        {
            printf("    %.*s\n", oc_str8_ip(subparser->name));
            if(subparser->options.desc.len)
            {
                printf("        %.*s\n\n", oc_str8_ip(subparser->options.desc));
            }
        }
    }
}

void oc_arg_parser_error(oc_arg_parser* parser, const char* fmt, ...)
{
    printf("error: ");

    oc_arena_scope scratch = oc_scratch_begin();

    va_list ap;
    va_start(ap, fmt);
    oc_str8 s = oc_str8_pushfv(scratch.arena, fmt, ap);
    va_end(ap);

    printf("%.*s\n", oc_str8_ip(s));
    oc_arg_parser_print_help(parser);

    oc_scratch_end(scratch);
}

//-----------------------------------------------------------------------
// Parsing
//-----------------------------------------------------------------------

int oc_arg_parser_parse_arg(oc_arena* arena, oc_arg_parser* parser, oc_arg_parser_arg* arg, oc_str8 valueString, bool positional)
{
    OC_ASSERT(valueString.len && valueString.ptr && valueString.ptr[valueString.len] == '\0');

    switch(arg->type)
    {
        case OC_ARG_PARSER_STR8:
        {
            if(arg->isArray)
            {
                oc_arg_parser_value_elt* elt = oc_arena_push_type(arena, oc_arg_parser_value_elt);
                elt->valStr8 = valueString;
                oc_list_push_back(&arg->values, &elt->listElt);
            }
            else
            {
                *arg->dest.valStr8 = valueString;
            }
        }
        break;
        case OC_ARG_PARSER_I64:
        {
            char* endptr = 0;
            i64 val = strtoll(valueString.ptr, &endptr, 0);
            if(*endptr != '\0')
            {
                oc_arg_parser_error(parser, ": expected integer argument for option %s%.*s, got %.*s.\n",
                                    positional ? "" : "--",
                                    oc_str8_ip(arg->name),
                                    oc_str8_ip(valueString));
                return -1;
            }
            if(arg->isArray)
            {
                oc_arg_parser_value_elt* elt = oc_arena_push_type(arena, oc_arg_parser_value_elt);
                elt->valI64 = val;
                oc_list_push_back(&arg->values, &elt->listElt);
            }
            else
            {
                *arg->dest.valI64 = val;
            }
        }
        break;
        case OC_ARG_PARSER_F64:
        {
            char* endptr = 0;
            f64 val = strtod(valueString.ptr, &endptr);
            if(*endptr != '\0')
            {
                oc_arg_parser_error(parser, ": expected integer argument for option %s%.*s, got %.*s.\n",
                                    positional ? "" : "--",
                                    oc_str8_ip(arg->name),
                                    oc_str8_ip(valueString));
                return -1;
            }
            if(arg->isArray)
            {
                oc_arg_parser_value_elt* elt = oc_arena_push_type(arena, oc_arg_parser_value_elt);
                elt->valF64 = val;
                oc_list_push_back(&arg->values, &elt->listElt);
            }
            else
            {
                *arg->dest.valF64 = val;
            }
        }
        break;

        case OC_ARG_PARSER_FLAG:
        {
            oc_arg_parser_error(parser, ": unexpected value for flag option\n");
            return -1;
        }
        break;
    }

    arg->count++;

    return 0;
}

void oc_arg_parser_set_result_to_default(oc_arg_parser_arg* arg)
{
    OC_DEBUG_ASSERT(!arg->isArray);

    switch(arg->type)
    {
        case OC_ARG_PARSER_STR8:
            *arg->dest.valStr8 = arg->options.defaultValue.valStr8;
            break;
        case OC_ARG_PARSER_I64:
            *arg->dest.valI64 = arg->options.defaultValue.valI64;
            break;

        case OC_ARG_PARSER_F64:
            *arg->dest.valF64 = arg->options.defaultValue.valF64;
            break;

        case OC_ARG_PARSER_FLAG:
            *arg->dest.valBool = arg->options.defaultValue.valBool;
            break;
    }
}

void oc_arg_parser_set_dest_array(oc_arena* arena, oc_arg_parser_arg* arg)
{
    switch(arg->type)
    {
        case OC_ARG_PARSER_STR8:
        {
            *arg->dest.arrayStr8.count = arg->count;
            *arg->dest.arrayStr8.values = oc_arena_push_array(arena, oc_str8, arg->count);

            oc_list_for_indexed(arg->values, it, oc_arg_parser_value_elt, listElt)
            {
                (*arg->dest.arrayStr8.values)[it.index] = it.elt->valStr8;
            }
        }
        break;

        case OC_ARG_PARSER_I64:
        {
            *arg->dest.arrayI64.count = arg->count;
            *arg->dest.arrayI64.values = oc_arena_push_array(arena, i64, arg->count);

            oc_list_for_indexed(arg->values, it, oc_arg_parser_value_elt, listElt)
            {
                (*arg->dest.arrayI64.values)[it.index] = it.elt->valI64;
            }
        }
        break;

        case OC_ARG_PARSER_F64:
        {
            *arg->dest.arrayF64.count = arg->count;
            *arg->dest.arrayF64.values = oc_arena_push_array(arena, f64, arg->count);

            oc_list_for_indexed(arg->values, it, oc_arg_parser_value_elt, listElt)
            {
                (*arg->dest.arrayF64.values)[it.index] = it.elt->valF64;
            }
        }
        break;

        case OC_ARG_PARSER_FLAG:
            OC_ASSERT("error: unexpected array-value argument for flag type.");
            break;
    }
}

int oc_arg_parser_finalize_args(oc_arg_parser* parser, oc_list argList)
{
    oc_list_for(argList, arg, oc_arg_parser_arg, listElt)
    {
        if(!arg->count)
        {
            if(arg->options.required)
            {
                oc_arg_parser_error(parser, "missing argument %s%.*s\n", arg->positional ? "" : "--", oc_str8_ip(arg->name));
                return -1;
            }
            else
            {
                oc_arg_parser_set_result_to_default(arg);
            }
        }
        else if(arg->isArray)
        {
            oc_arg_parser_set_dest_array(parser->arena, arg);
        }
    }
    return 0;
}

int oc_arg_parser_parse(oc_arg_parser* parser, int argc, char** argv)
{
    int result = 0;

    //NOTE; skip first arg (name of executable or command)
    argc--;
    argv++;

    oc_arena_scope scratch = oc_scratch_begin_next(parser->arena);

    u32 posArgIndex = 0;

    for(u32 argIndex = 0; argIndex < argc; argIndex++)
    {
        oc_str8 argString = OC_STR8(argv[argIndex]);

        if(argString.len > 0 && argString.ptr[0] == '-')
        {
            //NOTE: named arg
            if(argString.len > 1 && argString.ptr[1] == '-')
            {
                //NOTE: long arg
                argString = oc_str8_slice(argString, 2, argString.len);

                oc_list_for(parser->namedArgs, arg, oc_arg_parser_arg, listElt)
                {
                    if(!oc_str8_cmp(argString, arg->name))
                    {
                        if(arg->count && !arg->options.allowRepeat)
                        {
                            oc_arg_parser_error(parser, "option --%.*s redeclared.\n", oc_str8_ip(arg->name));
                            result = -1;
                            goto end;
                        }

                        if(arg->type == OC_ARG_PARSER_FLAG)
                        {
                            if(arg->isArray)
                            {
                                (*arg->dest.flagCount)++;
                            }
                            else
                            {
                                *arg->dest.valBool = true;
                            }
                            arg->count++;
                        }
                        else
                        {
                            //NOTE: gather a number of arguments
                            i32 nargs = arg->options.nargs ? arg->options.nargs : 1;
                            i32 count = 0;
                            for(; count < nargs || nargs < 0; count++)
                            {
                                argIndex++;
                                if(argIndex >= argc)
                                {
                                    break;
                                }
                                oc_str8 valueString = OC_STR8(argv[argIndex]);
                                if(!valueString.len || valueString.ptr[0] == '-')
                                {
                                    break;
                                }
                                if(oc_arg_parser_parse_arg(scratch.arena, parser, arg, valueString, false) != 0)
                                {
                                    result = -1;
                                    goto end;
                                }
                            }
                            if(!count)
                            {
                                oc_arg_parser_error(parser, "option --%.*s requires an argument.\n", oc_str8_ip(arg->name));
                                result = -1;
                                goto end;
                            }
                            ///////////////////////////////////////////////////////////////////////////
                            //TODO
                            //should decrement argIndex here or continue
                            ///////////////////////////////////////////////////////////////////////////
                        }
                        if(arg->options.stop)
                        {
                            goto end;
                        }
                        break;
                    }
                }
            }
            else
            {
                //NOTE: short arg, collect all single letter flags
                for(u32 flagIndex = 1; flagIndex < argString.len; flagIndex++)
                {
                    char option = argString.ptr[flagIndex];

                    oc_list_for(parser->namedArgs, arg, oc_arg_parser_arg, listElt)
                    {
                        if(arg->options.shortName == option)
                        {
                            if(arg->count && !arg->options.allowRepeat)
                            {
                                oc_arg_parser_error(parser, "option --%.*s redeclared.\n", oc_str8_ip(arg->name));
                                result = -1;
                                goto end;
                            }

                            if(arg->type == OC_ARG_PARSER_FLAG)
                            {
                                //TODO: coalesce with above
                                if(arg->isArray)
                                {
                                    (*arg->dest.flagCount)++;
                                }
                                else
                                {
                                    *arg->dest.valBool = true;
                                }
                                arg->count++;
                            }
                            else
                            {
                                if(flagIndex != argString.len - 1 || argIndex + 1 >= argc)
                                {
                                    //NOTE: short option with argument must be the last of the sequence
                                    oc_arg_parser_error(parser, "option -%c requires an argument.\n", option);
                                    result = -1;
                                    goto end;
                                }

                                //NOTE: gather a number of arguments
                                i32 nargs = arg->options.nargs ? arg->options.nargs : 1;
                                i32 count = 0;
                                for(; count < nargs || nargs < 0; count++)
                                {
                                    argIndex++;
                                    if(argIndex >= argc)
                                    {
                                        break;
                                    }
                                    oc_str8 valueString = OC_STR8(argv[argIndex]);
                                    if(!valueString.len || valueString.ptr[0] == '-')
                                    {
                                        break;
                                    }
                                    if(oc_arg_parser_parse_arg(scratch.arena, parser, arg, valueString, false) != 0)
                                    {
                                        result = -1;
                                        goto end;
                                    }
                                }
                                if(!count)
                                {
                                    oc_arg_parser_error(parser, "option -%c requires an argument.\n", arg->options.shortName);
                                    result = -1;
                                    goto end;
                                }
                            }
                            if(arg->options.stop)
                            {
                                goto end;
                            }
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            //NOTE: positional arg or command
            if(posArgIndex == 0)
            {
                //NOTE: only the first positional arg can be a command
                oc_list_for(parser->subParsers, subParser, oc_arg_parser, listElt)
                {
                    if(!oc_str8_cmp(argString, subParser->name))
                    {
                        result = oc_arg_parser_parse(subParser, argc - argIndex, argv + argIndex);
                        if(subParser->commandDest)
                        {
                            *subParser->commandDest = subParser->name;
                        }
                        goto end;
                    }
                }
                if(parser->options.requireCommand)
                {
                    oc_arg_parser_error(parser, "%.*s requires a subcommand.\n", oc_str8_ip(parser->name));
                    result = -1;
                    goto end;
                }
            }

            if(posArgIndex >= parser->posArgCount)
            {
                //TODO: we should support variable number of args...
                oc_arg_parser_error(parser, "too many positional arguments.\n");
                result = -1;
                goto end;
            }

            oc_arg_parser_arg* arg = 0;
            oc_list_for_indexed(parser->posArgs, it, oc_arg_parser_arg, listElt)
            {
                if(it.index == posArgIndex)
                {
                    arg = it.elt;
                    break;
                }
            }
            OC_ASSERT(arg);

            //NOTE: gather a number of arguments
            i32 nargs = arg->options.nargs ? arg->options.nargs : 1;
            i32 count = 0;
            for(; count < nargs || nargs < 0; count++)
            {
                if(argIndex >= argc)
                {
                    break;
                }
                oc_str8 valueString = OC_STR8(argv[argIndex]);
                if(!valueString.len || valueString.ptr[0] == '-')
                {
                    break;
                }
                if(oc_arg_parser_parse_arg(scratch.arena, parser, arg, valueString, false) != 0)
                {
                    result = -1;
                    goto end;
                }
                argIndex++;
            }
            if(!count)
            {
                oc_arg_parser_error(parser, "option -%c requires an argument.\n", arg->options.shortName);
                result = -1;
                goto end;
            }

            posArgIndex++;
        }
    }

    //NOTE: collate array values and enforce required/default arguments

    if(oc_arg_parser_finalize_args(parser, parser->namedArgs) != 0)
    {
        result = -1;
        goto end;
    }
    if(oc_arg_parser_finalize_args(parser, parser->posArgs) != 0)
    {
        result = -1;
        goto end;
    }

    if(parser->options.requireCommand)
    {
        //NOTE: if we got here, we didn't find a command
        oc_arg_parser_error(parser, "%.*s requires a subcommand.\n", oc_str8_ip(parser->name));
        result = -1;
        goto end;
    }

end:
    if(parser->askHelp)
    {
        result = -1;
        oc_arg_parser_print_help(parser);
    }
    oc_scratch_end(scratch);
    return result;
}
