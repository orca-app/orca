/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

#include "orca.h"
#include "util.h"

bool isspace_cheap(int c)
{
    switch(c)
    {
        case ' ':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '\v':
            return true;
        default:
            return false;
    }
}

oc_str8 oc_str8_trim_space(oc_str8 s)
{
    u64 start, end;
    for(u64 i = 0; i < s.len; i++)
    {
        start = i;
        if(!isspace_cheap(s.ptr[i]))
        {
            break;
        }
    }
    for(u64 i = s.len; i > 0; i--)
    {
        end = i;
        if(!isspace_cheap(s.ptr[i - 1]))
        {
            break;
        }
    }

    if(end <= start)
    {
        return OC_STR8("");
    }
    return oc_str8_slice(s, start, end);
}
