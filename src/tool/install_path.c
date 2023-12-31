/*************************************************************************
*
*  Orca
*  Copyright 2024 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "orca.h"
#include "flag.h"
#include "util.h"

int installPath(int argc, char** argv)
{
    oc_arena a;
    oc_arena_init(&a);

    Flag_Context c;
    flag_init_context(&c);

    flag_help(&c, "Prints the path to the root directory where Orca is installed");

    if(!flag_parse(&c, argc, argv))
    {
        flag_print_usage(&c, "orca install-path", stderr);
        if(flag_error_is_help(&c))
        {
            return 0;
        }
        flag_print_error(&c, stderr);
        return 1;
    }

    oc_str8 orca_dir = system_orca_dir(&a);
    orca_dir = oc_path_canonical(&a, orca_dir);
    printf("%.*s", oc_str8_ip(orca_dir));

    return 0;
}
