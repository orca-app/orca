/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#pragma once

#include "typedefs.h"
#include "memory.h"
#include "lists.h"
#include "strings.h"

//NOTE: options for arguments creation
typedef struct oc_arg_parser_arg_options
{
    oc_str8 desc;      // description of the option in help message
    char shortName;    // (named only) optional short name
    oc_str8 valueName; // (named only) name to display for the value in help message
    bool required;     // error if this argument is not provided
    bool allowRepeat;  // (named only) allow repeating the argument. For single-value arguments,
                       // each new supplied argument changes the final value. For array arguments,
                       // each new supplied argument append its value to the array.
    i32 nargs;         // (array-value only) consume n values. Default means 1. nargs >= 1 consumes a fixed number
                       // of values. nargs < 0 consumes all available values. Only valid for
                       // array arguments.
    bool stop;         // stop parsing after encountering this argument

    union
    {
        oc_str8 valStr8;
        i64 valI64;
        f64 valF64;
        bool valBool;

    } defaultValue; // (singe-value only) default value in case argument is not provided.

} oc_arg_parser_arg_options;

//NOTE: options for parser and subcommands creation
typedef struct oc_arg_parser_options
{
    bool requireCommand; // error if no commands are found
    oc_str8 desc;        // description to display in the help message.

} oc_arg_parser_options;

typedef struct oc_arg_parser oc_arg_parser;

typedef struct oc_arg_parser
{
    oc_list_elt listElt;
    oc_arg_parser* parent;
    oc_arena* arena;

    oc_str8 name;

    oc_arg_parser_options options;
    bool askHelp;

    u32 subParserCount;
    oc_list subParsers;
    oc_str8* commandDest;

    u32 namedArgCount;
    oc_list namedArgs;

    u32 posArgCount;
    oc_list posArgs;

} oc_arg_parser;

/*TODO: caveats:
    - All persistent data allocated by parsers _which includes returned arrays_ live on the main parser's arena.
    - Currently parser can only be used once. If parsing multiple times, you need to clear the arena and re-create the parser.
*/

//NOTE: parser/subparser creation.
void oc_arg_parser_init(oc_arg_parser* parser, oc_arena* arena, oc_str8 name, oc_arg_parser_options* options);
oc_arg_parser* oc_arg_parser_subparser(oc_arg_parser* parser, oc_str8 name, oc_str8* dest, oc_arg_parser_options* options);

//NOTE: arguments building
int oc_arg_parser_add_named_str8(oc_arg_parser* parser, oc_str8 name, oc_str8* dest, oc_arg_parser_arg_options* options);
int oc_arg_parser_add_named_i64(oc_arg_parser* parser, oc_str8 name, i64* dest, oc_arg_parser_arg_options* options);
int oc_arg_parser_add_named_f64(oc_arg_parser* parser, oc_str8 name, f64* dest, oc_arg_parser_arg_options* options);
int oc_arg_parser_add_flag(oc_arg_parser* parser, oc_str8 name, bool* dest, oc_arg_parser_arg_options* options);

int oc_arg_parser_add_named_str8_array(oc_arg_parser* parser, oc_str8 name, u32* destCount, oc_str8** destArray, oc_arg_parser_arg_options* options);
int oc_arg_parser_add_named_i64_array(oc_arg_parser* parser, oc_str8 name, u32* destCount, i64** destArray, oc_arg_parser_arg_options* options);
int oc_arg_parser_add_named_f64_array(oc_arg_parser* parser, oc_str8 name, u32* destCount, f64** destArray, oc_arg_parser_arg_options* options);
int oc_arg_parser_add_flag_count(oc_arg_parser* parser, oc_str8 name, u32* destCount, oc_arg_parser_arg_options* options);

int oc_arg_parser_add_positional_str8(oc_arg_parser* parser, oc_str8 name, oc_str8* dest, oc_arg_parser_arg_options* options);
int oc_arg_parser_add_positional_i64(oc_arg_parser* parser, oc_str8 name, i64* dest, oc_arg_parser_arg_options* options);
int oc_arg_parser_add_positional_f64(oc_arg_parser* parser, oc_str8 name, f64* dest, oc_arg_parser_arg_options* options);

int oc_arg_parser_add_positional_str8_array(oc_arg_parser* parser, oc_str8 name, u32* destCount, oc_str8** destArray, oc_arg_parser_arg_options* options);
int oc_arg_parser_add_positional_i64_array(oc_arg_parser* parser, oc_str8 name, u32* destCount, i64** destArray, oc_arg_parser_arg_options* options);
int oc_arg_parser_add_positional_f64_array(oc_arg_parser* parser, oc_str8 name, u32* destCount, f64** destArray, oc_arg_parser_arg_options* options);

//NOTE: help message
void oc_arg_parser_print_help(oc_arg_parser* parser);

//NOTE: parsing command line arguments
int oc_arg_parser_parse(oc_arg_parser* parser, int argc, char** argv);
