/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "orca.h"

int main(int argc, char** argv)
{
    oc_init();

    oc_file file = oc_file_open_with_request(OC_STR8("./test.txt"), OC_FILE_ACCESS_READ, 0);
    if(oc_file_is_nil(file))
    {
        oc_log_error("Couldn't open file\n");
    }
    else
    {
        char buffer[1024];
        u64 len = oc_file_read(file, 1024, buffer);
        oc_log_info("file contents: %.*s\n", (int)len, buffer);
    }

    return (0);
}
