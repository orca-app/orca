#include <stdio.h>

#include "util/memory.c"
#include "platform/win32_memory.c"
#include "util/strings.c"
#include "platform/native_debug.c"

#include "flag.h"

int bundle(int argc, char** argv);
int version(int argc, char** argv);

int main(int argc, char** argv)
{
    Flag_Context c;
    flag_init_context(&c);

    // TODO: flag_version?

    bool* doBundle = flag_command(&c, "bundle", "Package a WebAssembly module into a standalone Orca application.");
    bool* doSource = flag_command(&c, "source", "Commands for helping compile the Orca source code into your project.");
    bool* doVersion = flag_command(&c, "version", "Print the current Orca version.");

    // Hacks to achieve the following:
    // - `orca` => `orca -h`
    // - `orca -v`, `orca --version` => `orca version`
    if(argc == 1)
    {
        flag_print_usage(&c, "orca", stderr);
        return 0;
    }
    if(strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)
    {
        argv[1] = "version";
    }

    if(!flag_parse(&c, argc, argv))
    {
        flag_print_usage(&c, "orca", stderr);
        if(flag_error_is_help(&c))
        {
            exit(0);
        }
        flag_print_error(&c, stderr);
        return 1;
    }

    if(!flag_parse_command(&c))
    {
        flag_print_usage(&c, "orca", stderr);
        flag_print_error(&c, stderr);
        return 1;
    }

    int rest_argc = flag_rest_argc(&c);
    char** rest_argv = flag_rest_argv(&c);

    if(*doBundle)
    {
        return bundle(rest_argc, rest_argv);
    }
    else if(*doVersion)
    {
        return version(rest_argc, rest_argv);
    }
    else
    {
        fprintf(stderr, "ERROR: didn't handle all available commands or something, o no\n");
        return 1;
    }

    return 0;
}

int bundle(int argc, char** argv)
{
    Flag_Context c;
    flag_init_context(&c);

    flag_str(&c, "d", "resource", NULL, "copy a file to the app's resource directory");
    flag_str(&c, "D", "resource-dir", NULL, "copy a directory to the app's resource directory");
    flag_str(&c, "i", "icon", NULL, "an image file to use as the application's icon");
    flag_str(&c, "C", "out-dir", NULL, "where to place the final application bundle (defaults to the current directory)");
    flag_str(&c, "n", "name", "out", "the app's name");
    flag_str(&c, "O", "orca-dir", ".", NULL);
    flag_str(&c, NULL, "version", "0.0.0", "a version number to embed in the application bundle");
    // TODO: mtl-enable-capture

    char** module = flag_pos(&c, "module", "a .wasm file containing the application's wasm module");

    if(!flag_parse(&c, argc, argv))
    {
        flag_print_usage(&c, "orca bundle", stderr);
        if(flag_error_is_help(&c))
        {
            return 0;
        }
        flag_print_error(&c, stderr);
        return 1;
    }

    if(!flag_parse_positional(&c))
    {
        flag_print_usage(&c, "orca bundle", stderr);
        flag_print_error(&c, stderr);
        return 1;
    }

    printf("Module: %s\n", *module);

    return 0;
}

#ifndef ORCA_TOOL_VERSION
    #define ORCA_TOOL_VERSION unknown
#endif

// I love C so much
#define _TOSTRING(x) #x
#define TOSTRING(x) _TOSTRING(x)

int version(int argc, char** argv)
{
    fprintf(stderr, "Orca CLI tool version: ");
    printf(TOSTRING(ORCA_TOOL_VERSION));
    fprintf(stderr, "\n");

    // TODO: Print runtime / install version info

    return 0;
}
