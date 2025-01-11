/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "orca.h"

int main()
{
    oc_init();

    oc_rect rect = { .x = 100, .y = 100, .w = 800, .h = 600 };
    oc_window window = oc_window_create(rect, OC_STR8("test"), 0);

    oc_window_bring_to_front(window);
    oc_window_focus(window);

    oc_window_center(window);

    while(!oc_should_quit())
    {
        oc_arena_scope scratch = oc_scratch_begin();
        oc_pump_events(0);
        oc_event* event = 0;
        while((event = oc_next_event(scratch.arena)) != 0)
        {
            switch(event->type)
            {
                case OC_EVENT_WINDOW_CLOSE:
                {
                    oc_request_quit();
                }
                break;

                case OC_EVENT_WINDOW_RESIZE:
                {
                    oc_log_info("resized, frame = {%f, %f, %f, %f}, content = {%f, %f, %f, %f}\n",
                                event->move.frame.x,
                                event->move.frame.y,
                                event->move.frame.w,
                                event->move.frame.h,
                                event->move.content.x,
                                event->move.content.y,
                                event->move.content.w,
                                event->move.content.h);
                }
                break;

                case OC_EVENT_WINDOW_MOVE:
                {
                    oc_log_info("moved, frame = {%f, %f, %f, %f}, content = {%f, %f, %f, %f}\n",
                                event->move.frame.x,
                                event->move.frame.y,
                                event->move.frame.w,
                                event->move.frame.h,
                                event->move.content.x,
                                event->move.content.y,
                                event->move.content.w,
                                event->move.content.h);
                }
                break;

                case OC_EVENT_MOUSE_MOVE:
                {
                    oc_log_info("mouse moved, pos = {%f, %f}, delta = {%f, %f}\n",
                                event->mouse.x,
                                event->mouse.y,
                                event->mouse.deltaX,
                                event->mouse.deltaY);
                }
                break;

                case OC_EVENT_MOUSE_WHEEL:
                {
                    oc_log_info("mouse wheel, delta = {%f, %f}\n",
                                event->mouse.deltaX,
                                event->mouse.deltaY);
                }
                break;

                case OC_EVENT_MOUSE_ENTER:
                {
                    oc_log_info("mouse enter\n");
                }
                break;

                case OC_EVENT_MOUSE_LEAVE:
                {
                    oc_log_info("mouse leave\n");
                }
                break;

                case OC_EVENT_MOUSE_BUTTON:
                {
                    oc_log_info("mouse button %i: %i\n",
                                event->key.keyCode,
                                event->key.action == OC_KEY_PRESS ? 1 : 0);
                }
                break;

                case OC_EVENT_KEYBOARD_KEY:
                {
                    oc_log_info("key %i: %s\n",
                                event->key.keyCode,
                                event->key.action == OC_KEY_PRESS ? "press" : (event->key.action == OC_KEY_RELEASE ? "release" : "repeat"));
                }
                break;

                case OC_EVENT_KEYBOARD_CHAR:
                {
                    oc_log_info("entered char %s\n", event->character.sequence);
                }
                break;

                default:
                    break;
            }
        }
        oc_scratch_end(scratch);
    }

    oc_terminate();

    return (0);
}
