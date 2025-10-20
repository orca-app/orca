/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _USE_MATH_DEFINES //NOTE: necessary for MSVC
#include <math.h>

#include "orca.h"

oc_vec2 frameSize = { 1200, 838 };

oc_surface surface;
oc_canvas_renderer renderer;
oc_canvas_context context;
oc_font fontRegular;
oc_font fontBold;

typedef struct oc_card
{
    oc_list_elt listElt;
    u64 id;
    oc_rect rect;

} oc_card;

typedef enum oc_card_border
{
    OC_CARD_BORDER_TOP = 1,
    OC_CARD_BORDER_BOTTOM = 1 << 1,
    OC_CARD_BORDER_LEFT = 1 << 2,
    OC_CARD_BORDER_RIGHT = 1 << 3,
} oc_card_border;

typedef struct oc_code_canvas
{
    oc_arena arena;
    oc_list cards;
    oc_list freeList;
    u64 cardCount;
    u64 nextId;

    oc_card* draggedCard;
    oc_card* resizedCard;
    oc_card_border resizedBorders;
    bool panning;
    oc_vec2 pan;

} oc_code_canvas;

void oc_code_canvas_init(oc_code_canvas* canvas)
{
    oc_arena_init(&canvas->arena);
}

void oc_code_canvas_create_card(oc_code_canvas* canvas, f32 x, f32 y)
{
    oc_card* card = oc_list_pop_front_entry(&canvas->freeList, oc_card, listElt);
    if(!card)
    {
        card = oc_arena_push_type(&canvas->arena, oc_card);
    }
    card->id = canvas->nextId;
    canvas->nextId++;

    card->rect = (oc_rect){ x, y, 200, 200 };

    oc_list_push_back(&canvas->cards, &card->listElt);
    canvas->cardCount++;
}

/*
typedef struct oc_card_spacer_move
{
    oc_card* card;
    u32 movedRound;
    oc_vec2 newPos;
} oc_card_spacer_move;

typedef struct oc_card_spacer_state
{
    oc_list_elt listElt;
    oc_card_spacer_move* moves;

} oc_card_spacer_state;

void oc_card_spacer(oc_code_canvas* canvas, oc_card* movedCard, oc_vec2 delta)
{
    //TODO: generate valid placements
    oc_arena_scope scratch = oc_scratch_begin();

    //NOTE: Make first state
    oc_card_spacer_state* initialState = oc_arena_push_type(scratch.arena, oc_card_spacer_state);
    initialState.moves = oc_arena_push_array(scratch.arena, oc_card_spacer_move, canvas->cardCount);

    oc_list_for_indexed(canvas->cards, it, oc_card, listElt)
    {
        initialState.moves[it.index].card = it.elt;
        if(it.elt == movedCard)
        {
            initialState.moves[it.index].moved = true;
            initialState.moves[it.index].movedRound = 1;
        }
    }

    oc_list states = { 0 };
    oc_list_push_back(&states, &initialState->listElt);

    //NOTE: iterate to generate valid states
    oc_list newStates = { 0 };
    oc_list validState = { 0 };
    u32 validStateCount = 0;
    u32 round = 2;

    while(!validStateCount < 4) //TODO: review this condition
    {
        //NOTE: we take each state and create new possible states from it.
        oc_list_for(states, state, oc_card_spacer_state, listElt)
        {
            //NOTE: for each unmoved card that intersects any card in the just moved set,
            // we generate moves pushing that card away from intersecting any moved card
            for(u64 i=0; i<canvas->cardCount; i++)
            {
                if(state->moves[i].movedRound == 0)
                {
                    for(u64 j=0; j<canvas->cardCount; j++)
                    {
                        if(state->moves[j].movedRound == round-1
                        && oc_card_spacer_intersect(state->moves[i], state->moves[j]))
                        {
                            //TODO: generate moves
                        }
                    }
                }
            }
        }
        round++;
    }
    //TODO: select best placement

    oc_scratch_end(scratch);
}
*/

enum
{
    OC_CARD_BORDER_SIZE = 4,
    OC_CARD_MIN_WIDTH = 50,
    OC_CARD_MIN_HEIGHT = 50,
};

void oc_card_ui(oc_code_canvas* canvas, oc_card* card)
{
    //NOTE: get mouse pos in the parent's frame
    oc_vec2 mousePos = oc_ui_get_sig().mouse;
    oc_vec2 mouseDelta = oc_ui_get_sig().delta;

    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 idStr = oc_str8_pushf(scratch.arena, "card-%llu", card->id);
    oc_ui_box_str8(idStr)
    {
        oc_rect pannedRect = {
            card->rect.x - canvas->pan.x,
            card->rect.y - canvas->pan.y,
            card->rect.w,
            card->rect.h,
        };

        oc_ui_style_set_i32(OC_UI_POSITION, OC_UI_POSITION_FRAME);
        oc_ui_style_set_f32(OC_UI_OFFSET_X, pannedRect.x);
        oc_ui_style_set_f32(OC_UI_OFFSET_Y, pannedRect.y);
        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, pannedRect.w });
        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, pannedRect.h });

        oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 0.3, 0.3, 0.3, 1 });
        oc_ui_style_set_color(OC_UI_BORDER_COLOR, (oc_color){ 0.1, 0.1, 0.1, 1 });
        oc_ui_style_set_f32(OC_UI_BORDER_SIZE, OC_CARD_BORDER_SIZE);
        oc_ui_style_set_f32(OC_UI_ROUNDNESS, 5);

        if(card == canvas->draggedCard)
        {
            oc_ui_style_set_color(OC_UI_BORDER_COLOR, (oc_color){ 1, 0.05, 1, 1 });
            card->rect.x += mouseDelta.x;
            card->rect.y += mouseDelta.y;
        }
        else if(card == canvas->resizedCard)
        {
            oc_ui_style_set_color(OC_UI_BORDER_COLOR, (oc_color){ 1, 0, 1, 1 });

            if(canvas->resizedBorders & OC_CARD_BORDER_LEFT)
            {
                f32 delta = oc_clamp_high(mouseDelta.x, card->rect.w - OC_CARD_MIN_WIDTH);
                card->rect.x += delta;
                card->rect.w -= delta;
            }
            else if(canvas->resizedBorders & OC_CARD_BORDER_RIGHT)
            {
                f32 delta = oc_clamp_low(mouseDelta.x, OC_CARD_MIN_WIDTH - card->rect.w);
                card->rect.w += delta;
            }

            if(canvas->resizedBorders & OC_CARD_BORDER_TOP)
            {
                f32 delta = oc_clamp_high(mouseDelta.y, card->rect.h - OC_CARD_MIN_HEIGHT);
                card->rect.y += delta;
                card->rect.h -= delta;
            }
            else if(canvas->resizedBorders & OC_CARD_BORDER_BOTTOM)
            {
                f32 delta = oc_clamp_low(mouseDelta.y, OC_CARD_MIN_HEIGHT - card->rect.h);
                card->rect.h += delta;
            }
        }
        else if(oc_ui_get_sig().hover && oc_ui_get_sig().pressed)
        {
            canvas->draggedCard = card;
        }
        else
        {
            u32 hoveredBorders = 0;
            if(mousePos.y > pannedRect.y - OC_CARD_BORDER_SIZE
               && mousePos.y < pannedRect.y + pannedRect.h + OC_CARD_BORDER_SIZE)
            {
                if(mousePos.x > pannedRect.x - OC_CARD_BORDER_SIZE
                   && mousePos.x < pannedRect.x)
                {
                    hoveredBorders = OC_CARD_BORDER_LEFT;
                }
                else if(mousePos.x > pannedRect.x + pannedRect.w
                        && mousePos.x < pannedRect.x + pannedRect.w + OC_CARD_BORDER_SIZE)
                {
                    hoveredBorders = OC_CARD_BORDER_RIGHT;
                }
            }

            if(mousePos.x > pannedRect.x - OC_CARD_BORDER_SIZE
               && mousePos.x < pannedRect.x + pannedRect.w + OC_CARD_BORDER_SIZE)
            {
                if(mousePos.y > pannedRect.y - OC_CARD_BORDER_SIZE
                   && mousePos.y < pannedRect.y)
                {
                    hoveredBorders |= OC_CARD_BORDER_TOP;
                }
                else if(mousePos.y > pannedRect.y + pannedRect.h
                        && mousePos.y < pannedRect.y + pannedRect.h + OC_CARD_BORDER_SIZE)
                {
                    hoveredBorders |= OC_CARD_BORDER_BOTTOM;
                }
            }

            if(hoveredBorders)
            {
                oc_ui_style_set_color(OC_UI_BORDER_COLOR, (oc_color){ 1, 0, 1, 1 });
                if(oc_mouse_pressed(oc_ui_input(), OC_MOUSE_LEFT))
                {
                    canvas->resizedCard = card;
                    canvas->resizedBorders = hoveredBorders;
                }
            }
        }
    }

    oc_scratch_end(scratch);
}

oc_code_canvas canvas = { 0 };

i32 ui_runloop(void* user)
{
    context = oc_canvas_context_create();

    oc_ui_context* ui = oc_ui_context_create(fontRegular);

    while(!oc_should_quit())
    {
        oc_arena_scope scratch = oc_scratch_begin();

        oc_event* event = 0;
        while((event = oc_next_event(scratch.arena)) != 0)
        {
            oc_ui_process_event(event);

            switch(event->type)
            {
                case OC_EVENT_WINDOW_CLOSE:
                {
                    oc_request_quit();
                }
                break;

                case OC_EVENT_WINDOW_RESIZE:
                {
                    frameSize = (oc_vec2){ event->move.content.w, event->move.content.h };
                }
                break;

                default:
                    break;
            }
        }

        oc_ui_frame(frameSize)
        {
            if(oc_mouse_released(oc_ui_input(), OC_MOUSE_LEFT))
            {
                canvas.draggedCard = 0;
                canvas.resizedCard = 0;
                canvas.resizedBorders = 0;
                canvas.panning = false;
            }

            if(canvas.panning)
            {
                oc_vec2 delta = oc_ui_get_sig().delta;
                canvas.pan.x -= delta.x;
                canvas.pan.y -= delta.y;
            }

            oc_list_for(canvas.cards, card, oc_card, listElt)
            {
                oc_card_ui(&canvas, card);
            }

            if(!canvas.draggedCard
               && !canvas.resizedCard
               && oc_ui_get_sig().pressed)
            {
                canvas.panning = true;
            }
        }

        oc_ui_draw();
        oc_canvas_render(renderer, context, surface);
        oc_canvas_present(renderer, surface);

        oc_scratch_end(scratch);
    }
    return (0);
}

int main()
{
    oc_init();
    oc_clock_init(); //TODO put that in oc_init()?

    oc_code_canvas_init(&canvas);
    oc_code_canvas_create_card(&canvas, 100, 100);
    oc_code_canvas_create_card(&canvas, 400, 100);
    oc_code_canvas_create_card(&canvas, 200, 500);

    oc_rect windowRect = { .x = 100, .y = 100, .w = frameSize.x, .h = frameSize.y };
    oc_window window = oc_window_create(windowRect, OC_STR8("Code Canvas"), 0);

    oc_rect contentRect = oc_window_get_content_rect(window);

    oc_window_set_title(window, OC_STR8("Code Canvas"));

    renderer = oc_canvas_renderer_create();
    surface = oc_canvas_surface_create_for_window(renderer, window);

    oc_arena_scope scratch = oc_scratch_begin();

    oc_font* fonts[2] = { &fontRegular, &fontBold };
    oc_str8 fontNames[2] = {
        oc_path_executable_relative(scratch.arena, OC_STR8("../OpenSans-Regular.ttf")),
        oc_path_executable_relative(scratch.arena, OC_STR8("../OpenSans-Bold.ttf"))
    };

    for(int i = 0; i < 2; i++)
    {
        oc_file file = oc_catch(oc_file_open(fontNames[i], OC_FILE_ACCESS_READ, 0))
        {
            oc_log_error("Couldn't open file %.*s\n", oc_str8_ip(fontNames[i]));
            return -1;
        }
        u64 size = oc_file_size(file);
        char* buffer = (char*)oc_arena_push(scratch.arena, size);
        oc_file_read(file, size, buffer);
        oc_file_close(file);
        oc_unicode_range ranges[5] = { OC_UNICODE_BASIC_LATIN,
                                       OC_UNICODE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
                                       OC_UNICODE_LATIN_EXTENDED_A,
                                       OC_UNICODE_LATIN_EXTENDED_B,
                                       OC_UNICODE_SPECIALS };

        *fonts[i] = oc_font_create_from_memory(oc_str8_from_buffer(size, buffer), 5, ranges);
    }
    oc_scratch_end(scratch);

    // start app
    oc_window_bring_to_front(window);
    oc_window_focus(window);

    oc_thread* runloopThread = oc_thread_create(ui_runloop, 0);

    while(!oc_should_quit())
    {
        oc_pump_events(-1);
        //TODO: what to do with mem scratch here?
    }

    i64 exitCode = 0;
    oc_thread_join(runloopThread, &exitCode);

    oc_surface_destroy(surface);
    oc_terminate();

    return (0);
}
