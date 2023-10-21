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

    bool* help = flag_bool(&c, "h", "help", false, "show this help message and exit");

    flag_str(&c, "d", "resource", NULL, "copy a file to the app's resource directory");
    flag_str(&c, "D", "resource-dir", NULL, "copy a directory to the app's resource directory");
    flag_str(&c, "i", "icon", NULL, "an image file to use as the application's icon");
    flag_str(&c, "C", "out-dir", NULL, "where to place the final application bundle (defaults to the current directory)");
    flag_str(&c, "n", "name", "out", "the app's name");
    flag_str(&c, "O", "orca-dir", ".", NULL);
    flag_str(&c, NULL, "version", "0.0.0", "a version number to embed in the application bundle");
    // TODO: mtl-enable-capture
    // TODO: positional argument for module

    char** line = flag_str(&c, "", "line", "Hi!", "Line to output to the file");
    size_t* count = flag_size(&c, "", "count", 64, "Amount of lines to generate");

    if(!flag_parse(&c, argc, argv))
    {
        flag_print_usage(&c, "orca bundle", stderr);
        flag_print_error(&c, stderr);
        exit(1);
    }

    if(*help)
    {
        flag_print_usage(&c, "orca", stderr);
        exit(0);
    }

    int rest_argc = flag_rest_argc(&c);
    char** rest_argv = flag_rest_argv(&c);

    printf("Line: %s\n", *line);
    printf("Count: %zu\n", *count);

    return 0;
}
