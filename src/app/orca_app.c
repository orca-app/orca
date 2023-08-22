#include "orca.h"

//This is used to pass raw events from the runtime
ORCA_EXPORT oc_event oc_rawEvent;

void ORCA_IMPORT(oc_request_quit_stub)(void);

void oc_request_quit()
{
    oc_request_quit_stub();
}
