// flag.h -- command-line flag parsing
//
//   Inspired by Go's flag module: https://pkg.go.dev/flag
//
#ifndef FLAG_H_
    #define FLAG_H_

    #include <assert.h>
    #include <ctype.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdbool.h>
    #include <stdint.h>
    #include <stddef.h>
    #include <inttypes.h>
    #include <limits.h>
    #include <string.h>
    #include <errno.h>

    #include "util/memory.h"
    #include "util/strings.h"

// TODO: *_var function variants
// void flag_bool_var(bool *var, const char *name, bool def, const char *desc);
// void flag_bool_uint64(uint64_t *var, const char *name, bool def, const char *desc);
// etc.
// WARNING! *_var functions may break the flag_name() functionality

typedef enum
{
    FLAG_BOOL = 0,
    FLAG_UINT64,
    FLAG_SIZE,
    FLAG_STR,
    COUNT_FLAG_TYPES,
} Flag_Type;

static_assert(COUNT_FLAG_TYPES == 4, "Exhaustive Flag_Value definition");

typedef union
{
    char* as_str;
    uint64_t as_uint64;
    bool as_bool;
    size_t as_size;
} Flag_Value;

typedef enum
{
    FLAG_NO_ERROR = 0,
    FLAG_ERROR_UNKNOWN,
    FLAG_ERROR_NO_VALUE,
    FLAG_ERROR_INVALID_NUMBER,
    FLAG_ERROR_INTEGER_OVERFLOW,
    FLAG_ERROR_INVALID_SIZE_SUFFIX,
    COUNT_FLAG_ERRORS,
} Flag_Error;

typedef struct
{
    Flag_Type type;
    char* shortName;
    char* longName;
    char* valueName;
    char* desc;
    Flag_Value val;
    Flag_Value def;
} Flag;

    #ifndef FLAGS_CAP
        #define FLAGS_CAP 256
    #endif

typedef struct
{
    Flag flags[FLAGS_CAP];
    size_t flags_count;

    Flag_Error flag_error;
    char* flag_error_name;

    int rest_argc;
    char** rest_argv;

    oc_arena a;
} Flag_Context;

void flag_init_context(Flag_Context* c);
char* flag_name(void* val);
bool* flag_bool(Flag_Context* c, const char* shortName, const char* longName, bool def, const char* desc);
uint64_t* flag_uint64(Flag_Context* c, const char* shortName, const char* longName, uint64_t def, const char* desc);
size_t* flag_size(Flag_Context* c, const char* shortName, const char* longName, uint64_t def, const char* desc);
char** flag_str(Flag_Context* c, const char* shortName, const char* longName, const char* def, const char* desc);
bool flag_parse(Flag_Context* c, int argc, char** argv);
int flag_rest_argc(Flag_Context* c);
char** flag_rest_argv(Flag_Context* c);
void flag_print_error(Flag_Context* c, FILE* stream);
void flag_print_usage(Flag_Context* c, const char* prefix, FILE* stream);

#endif // FLAG_H_

//////////////////////////////

#ifdef FLAG_IMPLEMENTATION

oc_str8 oc_str8_toupper_inplace(oc_str8 str)
{
    for(int i = 0; i < str.len; i++)
    {
        str.ptr[i] = toupper(str.ptr[i]);
    }
    return str;
}

void flag_init_context(Flag_Context* c)
{
    *c = (Flag_Context){ 0 };
    oc_arena_init(&c->a);
}

Flag* flag_new(Flag_Context* c, Flag_Type type, const char* shortName, const char* longName, const char* desc)
{
    assert(c->flags_count < FLAGS_CAP);
    assert(longName);
    Flag* flag = &c->flags[c->flags_count++];
    *flag = (Flag){
        .type = type,
        // NOTE: I won't touch them I promise Kappa
        .shortName = (char*)(shortName ? shortName : ""),
        .longName = (char*)longName,
        .desc = (char*)(desc ? desc : ""),
    };
    return flag;
}

char* flag_name(void* val)
{
    Flag* flag = (Flag*)((char*)val - offsetof(Flag, val));
    return flag->longName;
}

bool* flag_bool(Flag_Context* c, const char* shortName, const char* longName, bool def, const char* desc)
{
    Flag* flag = flag_new(c, FLAG_BOOL, shortName, longName, desc);
    flag->def.as_bool = def;
    flag->val.as_bool = def;
    return &flag->val.as_bool;
}

uint64_t* flag_uint64(Flag_Context* c, const char* shortName, const char* longName, uint64_t def, const char* desc)
{
    Flag* flag = flag_new(c, FLAG_UINT64, shortName, longName, desc);
    flag->val.as_uint64 = def;
    flag->def.as_uint64 = def;
    return &flag->val.as_uint64;
}

size_t* flag_size(Flag_Context* c, const char* shortName, const char* longName, uint64_t def, const char* desc)
{
    Flag* flag = flag_new(c, FLAG_SIZE, shortName, longName, desc);
    flag->val.as_size = def;
    flag->def.as_size = def;
    return &flag->val.as_size;
}

char** flag_str(Flag_Context* c, const char* shortName, const char* longName, const char* def, const char* desc)
{
    Flag* flag = flag_new(c, FLAG_STR, shortName, longName, desc);
    flag->val.as_str = (char*)def;
    flag->def.as_str = (char*)def;
    flag->valueName = oc_str8_toupper_inplace(oc_str8_push_cstring(&c->a, longName)).ptr;
    return &flag->val.as_str;
}

static char* flag_shift_args(int* argc, char*** argv)
{
    assert(*argc > 0);
    char* result = **argv;
    *argv += 1;
    *argc -= 1;
    return result;
}

static bool flag_shift_value(char* flag, char** value, int* argc, char*** argv)
{
    char* equalLoc = strchr(flag, '=');
    if(equalLoc)
    {
        *value = equalLoc + 1;
        // Do not shift args if getting a value like --foo=bar
    }
    else
    {
        if(*argc == 0)
        {
            return false;
        }
        *value = flag_shift_args(argc, argv);
    }

    return true;
}

int flag_rest_argc(Flag_Context* c)
{
    return c->rest_argc;
}

char** flag_rest_argv(Flag_Context* c)
{
    return c->rest_argv;
}

bool flag_parse(Flag_Context* c, int argc, char** argv)
{
    flag_shift_args(&argc, &argv);

    while(argc > 0)
    {
        char* flag = flag_shift_args(&argc, &argv);

        if(*flag != '-')
        {
            // NOTE: pushing non-flag arg back into args
            c->rest_argc = argc + 1;
            c->rest_argv = argv - 1;
            return true;
        }

        if(strcmp(flag, "--") == 0)
        {
            // NOTE: but if it's the terminator we don't need to push it back
            c->rest_argc = argc;
            c->rest_argv = argv;
            return true;
        }

        // NOTE: remove the dash(es)
        flag += 1;
        bool longFlag = false;
        if(*flag == '-')
        {
            flag += 1;
            longFlag = true;
        }

        bool found = false;
        for(size_t i = 0; i < c->flags_count; ++i)
        {
            const char* expectedName = longFlag ? c->flags[i].longName : c->flags[i].shortName;
            if(strncmp(expectedName, flag, strlen(expectedName)) == 0)
            {
                static_assert(COUNT_FLAG_TYPES == 4, "Exhaustive flag type parsing");
                switch(c->flags[i].type)
                {
                    case FLAG_BOOL:
                    {
                        c->flags[i].val.as_bool = true;
                    }
                    break;

                    case FLAG_STR:
                    {
                        char* value;
                        if(!flag_shift_value(flag, &value, &argc, &argv))
                        {
                            c->flag_error = FLAG_ERROR_NO_VALUE;
                            c->flag_error_name = flag;
                            return false;
                        }
                        c->flags[i].val.as_str = value;
                    }
                    break;

                    case FLAG_UINT64:
                    {
                        char* arg;
                        if(!flag_shift_value(flag, &arg, &argc, &argv))
                        {
                            c->flag_error = FLAG_ERROR_NO_VALUE;
                            c->flag_error_name = flag;
                            return false;
                        }

                        static_assert(sizeof(unsigned long long int) == sizeof(uint64_t), "The original author designed this for x86_64 machine with the compiler that expects unsigned long long int and uint64_t to be the same thing, so they could use strtoull() function to parse it. Please adjust this code for your case and maybe even send the patch to upstream to make it work on a wider range of environments.");
                        char* endptr;
                        // TODO: replace strtoull with a custom solution
                        // That way we can get rid of the dependency on errno and static_assert
                        unsigned long long int result = strtoull(arg, &endptr, 10);

                        if(*endptr != '\0')
                        {
                            c->flag_error = FLAG_ERROR_INVALID_NUMBER;
                            c->flag_error_name = flag;
                            return false;
                        }

                        if(result == ULLONG_MAX && errno == ERANGE)
                        {
                            c->flag_error = FLAG_ERROR_INTEGER_OVERFLOW;
                            c->flag_error_name = flag;
                            return false;
                        }

                        c->flags[i].val.as_uint64 = result;
                    }
                    break;

                    case FLAG_SIZE:
                    {
                        char* arg;
                        if(!flag_shift_value(flag, &arg, &argc, &argv))
                        {
                            c->flag_error = FLAG_ERROR_NO_VALUE;
                            c->flag_error_name = flag;
                            return false;
                        }

                        static_assert(sizeof(unsigned long long int) == sizeof(size_t), "The original author designed this for x86_64 machine with the compiler that expects unsigned long long int and size_t to be the same thing, so they could use strtoull() function to parse it. Please adjust this code for your case and maybe even send the patch to upstream to make it work on a wider range of environments.");
                        char* endptr;
                        // TODO: replace strtoull with a custom solution
                        // That way we can get rid of the dependency on errno and static_assert
                        unsigned long long int result = strtoull(arg, &endptr, 10);

                        // TODO: handle more multiplicative suffixes like in dd(1). From the dd(1) man page:
                        // > N and BYTES may be followed by the following
                        // > multiplicative suffixes: c =1, w =2, b =512, kB =1000, K
                        // > =1024, MB =1000*1000, M =1024*1024, xM =M, GB
                        // > =1000*1000*1000, G =1024*1024*1024, and so on for T, P,
                        // > E, Z, Y.
                        if(strcmp(endptr, "K") == 0)
                        {
                            result *= 1024;
                        }
                        else if(strcmp(endptr, "M") == 0)
                        {
                            result *= 1024 * 1024;
                        }
                        else if(strcmp(endptr, "G") == 0)
                        {
                            result *= 1024 * 1024 * 1024;
                        }
                        else if(strcmp(endptr, "") != 0)
                        {
                            c->flag_error = FLAG_ERROR_INVALID_SIZE_SUFFIX;
                            c->flag_error_name = flag;
                            // TODO: capability to report what exactly is the wrong suffix
                            return false;
                        }

                        if(result == ULLONG_MAX && errno == ERANGE)
                        {
                            c->flag_error = FLAG_ERROR_INTEGER_OVERFLOW;
                            c->flag_error_name = flag;
                            return false;
                        }

                        c->flags[i].val.as_size = result;
                    }
                    break;

                    case COUNT_FLAG_TYPES:
                    default:
                    {
                        assert(0 && "unreachable");
                        exit(69);
                    }
                }

                found = true;
            }
        }

        if(!found)
        {
            c->flag_error = FLAG_ERROR_UNKNOWN;
            c->flag_error_name = flag;
            return false;
        }
    }

    c->rest_argc = argc;
    c->rest_argv = argv;
    return true;
}

typedef struct
{
    int width;
    int cur;
    int indent;
} Flag_Textwrap;

void flag_wrap_fnewline(FILE* f, Flag_Textwrap* w)
{
    fputs("\n", f);
    for(int i = 0; i < w->indent; i++)
    {
        fputs(" ", f);
    }
    w->cur = w->indent;
}

void flag_wrap_fprint(FILE* f, Flag_Textwrap* w, oc_str8 str)
{
    if(w->cur + str.len > w->width)
    {
        // wrap
        flag_wrap_fnewline(f, w);
        fprintf(f, "%.*s", (int)str.len, str.ptr);
    }
    else
    {
        fprintf(f, "%.*s", (int)str.len, str.ptr);
        w->cur += str.len;
    }
}

void flag_wrap_fprintln(FILE* f, Flag_Textwrap* w, oc_str8 str)
{
    flag_wrap_fprint(f, w, str);
    flag_wrap_fnewline(f, w);
}

void flag_wrap_fadvanceto(FILE* f, Flag_Textwrap* w, int pos)
{
    if(w->cur >= pos)
    {
        fputs("\n", f);
        for(int i = 0; i < pos; i++)
        {
            fputs(" ", f);
        }
    }
    else
    {
        for(int i = 0; i < pos - w->cur; i++)
        {
            fputs(" ", f);
        }
    }
    w->cur = pos;
}

void flag_wrap_fprint_autowrap(FILE* f, Flag_Textwrap* w, oc_arena* a, oc_str8 str)
{
    oc_str8_list space = { 0 };
    oc_str8_list_push(a, &space, OC_STR8(" "));
    oc_str8_list words = oc_str8_split(a, str, space);
    oc_str8_list_for(words, wordEl)
    {
        flag_wrap_fprint(f, w, wordEl->string);
        if(w->cur < w->width)
        {
            flag_wrap_fprint(f, w, OC_STR8(" "));
        }
    }
}

/*
This tries to mimic Python's argparse very closely, because I like it. For example:

usage: orca bundle [-h] [-d RESOURCE_FILES] [-D RESOURCE_DIRS] [-i ICON] [-C OUT_DIR] [-n NAME] [-O ORCA_DIR]
                   [--version VERSION] [--mtl-enable-capture]
                   module

positional arguments:
  module                a .wasm file containing the application's wasm module

options:
  -h, --help            show this help message and exit
  -d RESOURCE_FILES, --resource RESOURCE_FILES
                        copy a file to the app's resource directory
  -D RESOURCE_DIRS, --resource-dir RESOURCE_DIRS
                        copy a directory to the app's resource directory
  -i ICON, --icon ICON  an image file to use as the application's icon
  -C OUT_DIR, --out-dir OUT_DIR
                        where to place the final application bundle (defaults to the current directory)
  -n NAME, --name NAME  the app's name
  -O ORCA_DIR, --orca-dir ORCA_DIR
  --version VERSION     a version number to embed in the application bundle
  --mtl-enable-capture  Enable Metal frame capture for the application bundle (macOS only)
*/
void flag_print_usage(Flag_Context* c, const char* cmd, FILE* f)
{
    oc_arena* a = &c->a;
    Flag_Textwrap w = (Flag_Textwrap){
        .width = 100,
    };

    // Usage w/ short flags
    {
        oc_str8 prefix = oc_str8_pushf(a, "usage: %s ", cmd);
        fputs(prefix.ptr, f);
        w.indent = prefix.len;

        for(size_t i = 0; i < c->flags_count; i++)
        {
            Flag* flag = &c->flags[i];
            bool hasShort = flag->shortName[0] != 0;
            const char* name = hasShort ? flag->shortName : flag->longName;
            const char* dashes = hasShort ? "-" : "--";

            // For now all flags are optional, so they get brackets.
            oc_str8 flagUsage;
            if(flag->valueName)
            {
                flagUsage = oc_str8_pushf(a, "[%s%s %s] ", dashes, name, flag->valueName);
            }
            else
            {
                flagUsage = oc_str8_pushf(a, "[%s%s] ", dashes, name);
            }

            flag_wrap_fprint(f, &w, flagUsage);
        }

        w.indent = 0;
        flag_wrap_fnewline(f, &w);
    }

    // TODO: positional arguments

    // All flags
    flag_wrap_fnewline(f, &w);
    flag_wrap_fprintln(f, &w, OC_STR8("options:"));
    for(size_t i = 0; i < c->flags_count; i++)
    {
        flag_wrap_fprint(f, &w, OC_STR8("  "));

        Flag* flag = &c->flags[i];
        bool hasShort = flag->shortName[0] != 0;
        oc_str8 valueNameOut = flag->valueName
                                 ? oc_str8_pushf(a, " %s", flag->valueName)
                                 : OC_STR8("");

        oc_str8 shortOut, longOut;
        if(hasShort)
        {
            shortOut = oc_str8_pushf(a, "-%s%s", flag->shortName, valueNameOut.ptr);
        }
        longOut = oc_str8_pushf(a, "--%s%s", flag->longName, valueNameOut.ptr);

        oc_str8 bothFlags = hasShort
                              ? oc_str8_pushf(a, "%s, %s", shortOut.ptr, longOut.ptr)
                              : longOut;
        flag_wrap_fprint(f, &w, bothFlags);

        if(flag->desc[0] != 0)
        {
            w.indent = 24;
            flag_wrap_fadvanceto(f, &w, w.indent);
            flag_wrap_fprint_autowrap(f, &w, a, oc_str8_push_cstring(a, flag->desc));
            w.indent = 0;
        }
        flag_wrap_fnewline(f, &w);
    }

    flag_wrap_fnewline(f, &w);
    return;

    for(size_t i = 0; i < c->flags_count; ++i)
    {
        Flag* flag = &c->flags[i];

        fprintf(f, "    --%s\n", flag->longName);
        fprintf(f, "        %s\n", flag->desc);
        static_assert(COUNT_FLAG_TYPES == 4, "Exhaustive flag type defaults printing");
        switch(c->flags[i].type)
        {
            case FLAG_BOOL:
                if(flag->def.as_bool)
                {
                    fprintf(f, "        Default: %s\n", flag->def.as_bool ? "true" : "false");
                }
                break;
            case FLAG_UINT64:
                fprintf(f, "        Default: %" PRIu64 "\n", flag->def.as_uint64);
                break;
            case FLAG_SIZE:
                fprintf(f, "        Default: %zu\n", flag->def.as_size);
                break;
            case FLAG_STR:
                if(flag->def.as_str)
                {
                    fprintf(f, "        Default: %s\n", flag->def.as_str);
                }
                break;
            default:
                assert(0 && "unreachable");
                exit(69);
        }
    }
}

void flag_print_error(Flag_Context* c, FILE* stream)
{
    static_assert(COUNT_FLAG_ERRORS == 6, "Exhaustive flag error printing");
    switch(c->flag_error)
    {
        case FLAG_NO_ERROR:
            // NOTE: don't call flag_print_error() if flag_parse() didn't return false, okay? ._.
            fprintf(stream, "Operation Failed Successfully! Please tell the developer of this software that they don't know what they are doing! :)");
            break;
        case FLAG_ERROR_UNKNOWN:
            fprintf(stream, "ERROR: -%s: unknown flag\n", c->flag_error_name);
            break;
        case FLAG_ERROR_NO_VALUE:
            fprintf(stream, "ERROR: -%s: no value provided\n", c->flag_error_name);
            break;
        case FLAG_ERROR_INVALID_NUMBER:
            fprintf(stream, "ERROR: -%s: invalid number\n", c->flag_error_name);
            break;
        case FLAG_ERROR_INTEGER_OVERFLOW:
            fprintf(stream, "ERROR: -%s: integer overflow\n", c->flag_error_name);
            break;
        case FLAG_ERROR_INVALID_SIZE_SUFFIX:
            fprintf(stream, "ERROR: -%s: invalid size suffix\n", c->flag_error_name);
            break;
        case COUNT_FLAG_ERRORS:
        default:
            assert(0 && "unreachable");
            exit(69);
    }
}

#endif

// Original copyright notice:

// Copyright 2021 Alexey Kutepov <reximkut@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
