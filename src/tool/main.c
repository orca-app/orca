#include <stdio.h>

#include "util/memory.c"
#include "platform/win32_memory.c"
#include "util/strings.c"
#include "platform/native_debug.c"

#include "flag.h"

int main(int argc, char** argv)
{
    Flag_Context c;
    flag_init_context(&c);

    // TODO: flag_version?

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
        flag_print_usage(&c, "orca", stderr);
        if(flag_error_is_help(&c))
        {
            exit(0);
        }
        flag_print_error(&c, stderr);
        exit(1);
    }

    if(!flag_parse_positional(&c))
    {
        flag_print_usage(&c, "orca", stderr);
        flag_print_error(&c, stderr);
        exit(1);
    }

    int rest_argc = flag_rest_argc(&c);
    char** rest_argv = flag_rest_argv(&c);

    printf("Module: %s\n", *module);

    return 0;
}
