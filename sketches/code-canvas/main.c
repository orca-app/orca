/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/

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
    oc_vec2 initialPos;
} oc_card;

typedef enum oc_card_interaction
{
    OC_CARD_RESIZE_TOP = 1,
    OC_CARD_RESIZE_BOTTOM = 1 << 1,
    OC_CARD_RESIZE_LEFT = 1 << 2,
    OC_CARD_RESIZE_RIGHT = 1 << 3,
    OC_CARD_RESIZE = OC_CARD_RESIZE_TOP
                   | OC_CARD_RESIZE_BOTTOM
                   | OC_CARD_RESIZE_LEFT
                   | OC_CARD_RESIZE_RIGHT,
    OC_CARD_MOVE = 1 << 4,

} oc_card_interaction;

typedef struct oc_code_canvas
{
    oc_arena arena;
    oc_list cards;
    oc_list freeList;
    u64 cardCount;
    u64 nextId;

    oc_card* interactedCard;
    oc_card_interaction interaction;
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
    card->initialPos = card->rect.xy;

    oc_list_push_back(&canvas->cards, &card->listElt);
    canvas->cardCount++;
}

enum
{
    OC_CARD_BORDER_SIZE = 4,
    OC_CARD_SPACER_MARGIN = 16,
    OC_CARD_MIN_WIDTH = 50,
    OC_CARD_MIN_HEIGHT = 50,
};

typedef enum oc_card_spacer_direction
{
    OC_CARD_SPACER_UP,
    OC_CARD_SPACER_DOWN,
    OC_CARD_SPACER_LEFT,
    OC_CARD_SPACER_RIGTH,
} oc_card_spacer_direction;

typedef struct oc_card_spacer_item
{
    oc_card* card;
    oc_vec2 newPos;

} oc_card_spacer_item;

typedef struct oc_card_spacer_state
{
    u32 itemCount;
    u32 markedCount;
    u32 workingCount;

    oc_card_spacer_item* items;

} oc_card_spacer_state;

bool oc_rect_overlap(oc_rect a, oc_rect b)
{
    bool overlap = (a.x < b.x + b.w)
                && (a.x + a.w > b.x)
                && (a.y < b.y + b.h)
                && (a.y + a.h > b.y);
    return overlap;
}

oc_vec2 oc_card_spacer_move_direction(oc_card_spacer_state* state, oc_card_spacer_item* item, oc_card_spacer_direction direction)
{
    bool intersect = false;
    oc_vec2 newPos = item->card->initialPos;

    do
    {
        intersect = false;
        for(u64 i = 0; i < state->markedCount + state->workingCount; i++)
        {
            oc_card_spacer_item* other = &state->items[i];

            oc_rect rectItem = {
                .xy = newPos,
                .wh = item->card->rect.wh,
            };

            oc_rect rectOther = {
                other->newPos.x - OC_CARD_SPACER_MARGIN,
                other->newPos.y - OC_CARD_SPACER_MARGIN,
                other->card->rect.w + 2 * OC_CARD_SPACER_MARGIN,
                other->card->rect.h + 2 * OC_CARD_SPACER_MARGIN,
            };

            if(oc_rect_overlap(rectItem, rectOther))
            {
                intersect = true;
                switch(direction)
                {
                    case OC_CARD_SPACER_UP:
                    {
                        newPos.y = other->newPos.y - item->card->rect.h - OC_CARD_SPACER_MARGIN;
                    }
                    break;
                    case OC_CARD_SPACER_DOWN:
                    {
                        newPos.y = other->newPos.y + other->card->rect.h + OC_CARD_SPACER_MARGIN;
                    }
                    break;
                    case OC_CARD_SPACER_LEFT:
                    {
                        newPos.x = other->newPos.x - item->card->rect.w - OC_CARD_SPACER_MARGIN;
                    }
                    break;
                    case OC_CARD_SPACER_RIGTH:
                    {
                        newPos.x = other->newPos.x + other->card->rect.w + OC_CARD_SPACER_MARGIN;
                    }
                    break;
                }
            }
        }
    }
    while(intersect);

    return newPos;
}

void oc_card_spacer(oc_code_canvas* canvas, oc_card* initialCard)
{
    oc_arena_scope scratch = oc_scratch_begin();

    //NOTE: create initial state
    oc_card_spacer_state state = {
        .items = oc_arena_push_array(scratch.arena, oc_card_spacer_item, canvas->cardCount),
        .itemCount = canvas->cardCount,
        .markedCount = 0,
        .workingCount = 1,
    };

    oc_list_for_indexed(canvas->cards, it, oc_card, listElt)
    {
        state.items[it.index].card = it.elt;
        state.items[it.index].newPos = it.elt->initialPos;

        if(it.elt == initialCard)
        {
            oc_card_spacer_item tmp = state.items[0];
            state.items[0] = state.items[it.index];
            state.items[it.index] = tmp;
        }
    }

    bool hadMoves = false;

    do
    {
        hadMoves = false;
        u64 oldWorkingCount = state.workingCount;

        for(u64 unmarkedIndex = state.markedCount + state.workingCount;
            unmarkedIndex < state.itemCount;
            unmarkedIndex++)
        {

            oc_card_spacer_item* unmarkedItem = &state.items[unmarkedIndex];
            /*
                //NOTE: collect the set of cards it intersects in the working set
                //TODO: and the directions they were moved

                u64 intersectingCount = 0;
                oc_card_spacer_item* intersecting = oc_arena_push_array(scratch.arena, oc_card_spacer_item*, state.workingCount);

                for(u64 workingIndex = 0; workingIndex < state.itemCount; workingIndex++)
                {
                    if(state.items[workingIndex].working)
                    {
                        oc_card_spacer_item* workingItem = &state.items[workingIndex];
                        if(oc_card_spacer_overlap(unmarkedItem, workingItem))
                        {
                            intersecting[intersectingCount] = workingItem;
                            intersectingCount++;
                        }
                    }
                }
                */

            //NOTE: Compute smallest move for this card
            oc_vec2 moves[4] = { 0 };
            f32 smallestNorm = FLT_MAX;
            i32 smallestIndex = 0;

            for(int i = 0; i < 4; i++)
            {
                moves[i] = oc_card_spacer_move_direction(&state, unmarkedItem, i);
                f32 norm = oc_max(fabs(unmarkedItem->card->initialPos.x - moves[i].x), fabs(unmarkedItem->card->initialPos.y - moves[i].y));
                if(norm < smallestNorm)
                {
                    smallestNorm = norm;
                    smallestIndex = i;
                }
            }

            if(smallestNorm > 0)
            {
                //NOTE: apply smallest move and put card in working set
                unmarkedItem->newPos = moves[smallestIndex];

                oc_card_spacer_item tmp = state.items[state.markedCount + state.workingCount];
                state.items[state.markedCount + state.workingCount] = state.items[unmarkedIndex];
                state.items[unmarkedIndex] = tmp;
                state.workingCount++;

                hadMoves = true;
            }
        }

        //NOTE: move old working set to marked set
        state.markedCount += oldWorkingCount;
        state.workingCount -= oldWorkingCount;
    }
    while(hadMoves);

    //NOTE: apply state
    for(u64 i = 0; i < state.itemCount; i++)
    {
        state.items[i].card->rect.xy = state.items[i].newPos;
    }

    oc_scratch_end(scratch);
}

void oc_card_ui(oc_code_canvas* canvas, oc_card* card)
{
    //NOTE: get mouse pos in the parent's frame
    oc_vec2 mousePos = oc_ui_get_sig().mouse;
    oc_vec2 mouseDelta = oc_ui_get_sig().delta;

    oc_arena_scope scratch = oc_scratch_begin();
    oc_str8 idStr = oc_str8_pushf(scratch.arena, "card-%llu", card->id);
    oc_ui_box_str8(idStr)
    {
        oc_ui_set_text(idStr);
        oc_ui_style_set_var_str8(OC_UI_FONT, OC_UI_THEME_FONT_REGULAR);
        oc_ui_style_set_f32(OC_UI_TEXT_SIZE, 32);
        oc_ui_style_set_color(OC_UI_COLOR, (oc_color){ 0, 0, 0, 1 });

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

        if(card->initialPos.x != card->rect.x
           || card->initialPos.y != card->rect.y)
        {
            oc_ui_style_set_color(OC_UI_BG_COLOR, (oc_color){ 0.4, 0.4, 0.4, 1 });
        }
        oc_ui_attribute_mask animationMask = OC_UI_MASK_BG_COLOR;

        if(card == canvas->interactedCard)
        {
            if(canvas->interaction == OC_CARD_MOVE)
            {
                oc_ui_style_set_color(OC_UI_BORDER_COLOR, (oc_color){ 1, 0.05, 1, 1 });
                card->rect.x += mouseDelta.x;
                card->rect.y += mouseDelta.y;
                card->initialPos = card->rect.xy;
            }
            else
            {
                oc_ui_style_set_color(OC_UI_BORDER_COLOR, (oc_color){ 1, 0, 1, 1 });

                if(canvas->interaction & OC_CARD_RESIZE_LEFT)
                {
                    f32 delta = oc_clamp_high(mouseDelta.x, card->rect.w - OC_CARD_MIN_WIDTH);
                    card->rect.x += delta;
                    card->rect.w -= delta;
                }
                else if(canvas->interaction & OC_CARD_RESIZE_RIGHT)
                {
                    f32 delta = oc_clamp_low(mouseDelta.x, OC_CARD_MIN_WIDTH - card->rect.w);
                    card->rect.w += delta;
                }

                if(canvas->interaction & OC_CARD_RESIZE_TOP)
                {
                    f32 delta = oc_clamp_high(mouseDelta.y, card->rect.h - OC_CARD_MIN_HEIGHT);
                    card->rect.y += delta;
                    card->rect.h -= delta;
                }
                else if(canvas->interaction & OC_CARD_RESIZE_BOTTOM)
                {
                    f32 delta = oc_clamp_low(mouseDelta.y, OC_CARD_MIN_HEIGHT - card->rect.h);
                    card->rect.h += delta;
                }
            }
        }
        else if(oc_ui_get_sig().hover && oc_ui_get_sig().pressed)
        {
            canvas->interactedCard = card;
            canvas->interaction = OC_CARD_MOVE;
        }
        else
        {
            animationMask |= OC_UI_MASK_OFFSET_X | OC_UI_MASK_OFFSET_Y;

            u32 hoveredBorders = 0;

            if(mousePos.y > pannedRect.y - OC_CARD_BORDER_SIZE
               && mousePos.y < pannedRect.y + pannedRect.h + OC_CARD_BORDER_SIZE)
            {
                if(mousePos.x > pannedRect.x - OC_CARD_BORDER_SIZE
                   && mousePos.x < pannedRect.x)
                {
                    hoveredBorders = OC_CARD_RESIZE_LEFT;
                }
                else if(mousePos.x > pannedRect.x + pannedRect.w
                        && mousePos.x < pannedRect.x + pannedRect.w + OC_CARD_BORDER_SIZE)
                {
                    hoveredBorders = OC_CARD_RESIZE_RIGHT;
                }
            }

            if(mousePos.x > pannedRect.x - OC_CARD_BORDER_SIZE
               && mousePos.x < pannedRect.x + pannedRect.w + OC_CARD_BORDER_SIZE)
            {
                if(mousePos.y > pannedRect.y - OC_CARD_BORDER_SIZE
                   && mousePos.y < pannedRect.y)
                {
                    hoveredBorders |= OC_CARD_RESIZE_TOP;
                }
                else if(mousePos.y > pannedRect.y + pannedRect.h
                        && mousePos.y < pannedRect.y + pannedRect.h + OC_CARD_BORDER_SIZE)
                {
                    hoveredBorders |= OC_CARD_RESIZE_BOTTOM;
                }
            }

            if(hoveredBorders)
            {
                oc_ui_style_set_color(OC_UI_BORDER_COLOR, (oc_color){ 1, 0, 1, 1 });
                if(oc_mouse_pressed(oc_ui_input(), OC_MOUSE_LEFT))
                {
                    canvas->interactedCard = card;
                    canvas->interaction = hoveredBorders;
                }
            }
        }
        oc_ui_style_set_i32(OC_UI_ANIMATION_MASK, animationMask);
        oc_ui_style_set_f32(OC_UI_ANIMATION_TIME, 0.3);
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
            if(canvas.interactedCard)
            {
                oc_list_remove(&canvas.cards, &canvas.interactedCard->listElt);
                oc_list_push_back(&canvas.cards, &canvas.interactedCard->listElt);

                oc_vec2 mouseDelta = oc_mouse_delta(oc_ui_input());
                if(fabs(mouseDelta.x) > 0 || fabs(mouseDelta.y) > 0)
                {
                    oc_card_spacer(&canvas, canvas.interactedCard);
                }

                if(oc_mouse_released(oc_ui_input(), OC_MOUSE_LEFT))
                {
                    oc_list_for(canvas.cards, card, oc_card, listElt)
                    {
                        card->initialPos = card->rect.xy;
                    }
                    canvas.interactedCard = 0;
                    canvas.interaction = 0;
                }
            }
            else if(oc_mouse_released(oc_ui_input(), OC_MOUSE_LEFT))
            {
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

            if(!canvas.interactedCard
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
