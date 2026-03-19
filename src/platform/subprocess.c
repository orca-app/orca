#include "subprocess.h"

oc_str8 oc_subprocess_error_string(oc_subprocess_error error)
{
    switch(error)
    {
        case OC_SUBPROCESS_OK:
            return OC_STR8("no error");

        case OC_SUBPROCESS_NOT_EXECUTABLE:
            return OC_STR8("file is not executable");

        case OC_SUBPROCESS_NOT_FOUND:
            return OC_STR8("file not found");

        case OC_SUBPROCESS_NO_CHILD:
            return OC_STR8("child process does not exist");

        case OC_SUBPROCESS_INTERRUPTED:
            return OC_STR8("process interrupted");

        case OC_SUBPROCESS_PIPE:
            return OC_STR8("broken pipe");

        case OC_SUBPROCESS_UNKNOWN:
            return OC_STR8("unknown error");
    }
}

oc_subprocess_result oc_subprocess_wait(oc_subprocess subprocess)
{
    return oc_subprocess_read_and_wait(0, subprocess);
}

oc_subprocess_result oc_subprocess_run(int argc, char** argv, oc_subprocess_run_options* options)
{
    oc_subprocess_run_options runOption = options ? *options : (oc_subprocess_run_options){ 0 };

    oc_subprocess_spawn_options spawnOptions = {
        .stdIn = runOption.stdIn,
        .stdOut = runOption.stdOut,
        .stdErr = runOption.stdErr,
    };
    oc_subprocess subprocess = oc_catch(oc_subprocess_spawn(argc, argv, &spawnOptions))
    {
        return oc_result_error(oc_subprocess_result, oc_last_error());
    }

    return oc_subprocess_read_and_wait(runOption.captureArena, subprocess);
}
