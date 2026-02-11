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
