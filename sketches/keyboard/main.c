/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "orca.h"

int main()
{
    oc_init();

    oc_rect rect = { .x = 100, .y = 100, .w = 200, .h = 200 };
    oc_window window = oc_window_create(rect, OC_STR8("test"), 0);

    oc_window_bring_to_front(window);
    oc_window_focus(window);

    oc_window_center(window);

    oc_log_info("keycode for enter = %i\n", OC_KEY_ENTER);

    while(!oc_should_quit())
    {
        oc_pump_events(0);
        oc_event* event = 0;
        while((event = oc_next_event(oc_scratch())) != 0)
        {
            switch(event->type)
            {
                case OC_EVENT_WINDOW_CLOSE:
                {
                    oc_request_quit();
                }
                break;

                case OC_EVENT_KEYBOARD_KEY:
                {
                    if(event->key.action == OC_KEY_PRESS)
                    {
                        if(event->key.keyCode < 128)
                        {
                            oc_log_info("Key:\n\tscanCode = %i\n\tkeyCode = %i (%c)\n",
                                        event->key.scanCode,
                                        event->key.keyCode,
                                        event->key.keyCode);
                        }
                        else
                        {
                            oc_log_info("Key:\n\tscanCode = %i\n\tkeyCode = %i\n",
                                        event->key.scanCode,
                                        event->key.keyCode);
                        }
                    }
                }
                break;

                default:
                    break;
            }
        }
        oc_arena_clear(oc_scratch());
    }
    oc_terminate();
    return (0);
}
