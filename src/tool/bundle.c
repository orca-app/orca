#include "flag.h"
#include "util/strings.h"

int winBundle(oc_str8_list resourceFiles, oc_str8_list resourceDirs, char* icon, char* outDir, char* name, char* orcaDir, char* version);

int bundle(int argc, char** argv)
{
    Flag_Context c;
    flag_init_context(&c);

    char** name = flag_str(&c, "n", "name", "out", "the app's name");
    char** icon = flag_str(&c, "i", "icon", NULL, "an image file to use as the application's icon");
    char** version = flag_str(&c, NULL, "version", "0.0.0", "a version number to embed in the application bundle");
    oc_str8_list* resourceFiles = flag_strs(&c, "d", "resource", "copy a file to the app's resource directory");
    oc_str8_list* resourceDirs = flag_strs(&c, "D", "resource-dir", "copy a directory to the app's resource directory");
    char** outDir = flag_str(&c, "C", "out-dir", NULL, "where to place the final application bundle (defaults to the current directory)");
    char** orcaDir = flag_str(&c, "O", "orca-dir", ".", NULL);
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

#ifdef _WIN32
    return winBundle(*resourceFiles, *resourceDirs, *icon, *outDir, *name, *orcaDir, *version);
#else
    #error Can't build the bundle script on this platform!
#endif
}

#ifdef _WIN32
int winBundle(
    oc_str8_list resourceFiles,
    oc_str8_list resourceDirs,
    char* icon,
    char* outDir,
    char* name,
    char* orcaDir,
    char* version)
{
    printf("Resource files:\n");
    oc_str8_list_for(resourceFiles, it)
    {
        printf("  %s\n", it->string.ptr);
    }

    fprintf(stderr, "TODO: Actually bundle :)\n");
    return 1;
}
#endif
