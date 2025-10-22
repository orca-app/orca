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
void oc_card_spacer(oc_code_canvas* canvas, oc_card* movedCard, oc_vec2 delta)
{
    //TODO: create an initial state with all cards unmarked except the moved card

    //TODO: iterate on the existing states to generate new states from them
    {
        //TODO: for each state
        {
            //TODO: if the intersection set is empty, put that state in the list of valid states and continue

            //TODO: for each unmarked card that intersects with any card in the just moved set
            {
                //TODO: generate 4 axis aligned moves that pushed that card so that it doesn't intersect
                // with any marked card
                // prune moves that are opposite to the last move of the cards it intersects with (ie card must be pushed away, not swapped),
                // unless we'd prune all moves
                //TODO: for each created new states
                {
                    //TODO: copy that state N-1 times (N=number of moves), so that we get 4 states, to which we add
                    // our for moves.
                    // This
                }
            }
            // we end up with M new states, some of which we must prune because the cards we just moved might overlap
        }
    }
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TODO: another outline where we don't have to prune after the fact, but keep things order-independent
////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
typedef struct oc_card_spacer_item
{
    bool marked;
    bool working;
    oc_card* card;
    oc_vec2 newPos;
} oc_card_spacer_item;

typedef struct oc_card_spacer_state
{
    oc_list_elt listElt;
    oc_card_spacer_item* items;
} oc_card_spacer_state;

oc_card_spacer_state* oc_card_spacer_state_create(oc_arena* arena, u64 cardCount)
{
    oc_card_spacer_state* state = oc_arena_push_type(arena, oc_card_spacer_state);
    state->items = oc_arena_push_array(arena, oc_card_spacer_item, cardCount);
    return state;
}

typedef enum oc_card_spacer_direction
{
    OC_CARD_SPACER_UP,
    OC_CARD_SPACER_DOWN,
    OC_CARD_SPACER_LEFT,
    OC_CARD_SPACER_RIGHT,
} oc_card_spacer_direction;

oc_vec2 oc_card_spacer_move(oc_card_spacer_state* state, u64 itemIndex, oc_card_spacer_direction direction, u64 cardCount)
{
    //NOTE: pushes a card in one direction so that it doesn't intersect any marked cards

    oc_vec2 newPos = state->items[itemIndex].card->rect.xy;

    u64 intersectCount = 1;
    while(intersectCount)
    {
        intersectCount = 0;
        for(u64 i = 0; i < cardCount; i++)
        {
            if(state->items[i].marked
               && oc_card_spacer_intersect(&state->items[i], &state->items[itemIndex]))
            {
                intersectCount++;

                oc_rect itemRect = {
                    .xy = newPos,
                    .wh = state->items[itemIndex].card->rect.wh,
                };
                oc_rect intersectedRect = {
                    .xy = state->items[i].newPos,
                    .wh = state->items[i].card->rect.wh,
                };

                switch(direction)
                {
                    case OC_CARD_SPACER_UP:
                    {
                        newPos.y = intersectedRect.y - state->items[itemIndex].card->rect.h;
                    }
                    break;
                    case OC_CARD_SPACER_DOWN:
                    {
                        newPos.y = intersectedRect.y + intersectedRect.h;
                    }
                    break;
                    case OC_CARD_SPACER_LEFT:
                    {
                        newPos.x = intersectedRect.x - state->items[itemIndex].card->rect.w;
                    }
                    break;
                    case OC_CARD_SPACER_RIGHT:
                    {
                        newPos.x = intersectedRect.x + intersectedRect.w;
                    }
                    break;
                }
            }
        }
    }
    return newPos;
}

void oc_card_spacer(oc_code_canvas* canvas, oc_card* movedCard, oc_vec2 delta)
{
    // a "state" consists of
    // - unmarked cards: cards that have not been moved yet
    // - a working set: cards that have been moved and not examined for new collisions
    // - marked cards: cards that have been moved and have pushed any other intersecting cards
    // a state with an empty working set is a valid state

    oc_arena_scope scratch = oc_scratch_begin();
    oc_list states = { 0 };

    // Create an initial state with all cards unmarked except the moved card, which is marked and
    // in the working set

    oc_card_spacer_state* initialState = oc_card_spacer_state_create(scratch.arena, canvas->cardCount);
    oc_list_for_indexed(canvas->cards, it, oc_card, listElt)
    {
        initialState->items[it.index].card = it.elt;
        if(it.elt == movedCard)
        {
            initialState->items[it.index].marked = true;
            initialState->items[it.index].working = true;
        }
    }
    oc_list_push_back(&states, &initialState->listElt);

    oc_list validStates = { 0 };
    u32 validStateCount = 0;

    //iterate on the existing states to generate new states, until we have enough valid states
    while(validStateCount < 4)
    {
        oc_list newStates = { 0 };
        oc_list_for_safe(states, state, oc_card_spacer_state, listElt)
        {
            // for each card Cw in the working set
            for(u64 workingIndex = 0; workingIndex < canvas->cardCount; workingIndex++)
            {
                if(state->items[workingIndex].working)
                {
                    // for each unmarked card Cj that intersect with Cw
                    u64 intersectCount = 0;
                    for(u64 intersectIndex = 0; intersectIndex < canvas->cardCount; intersectIndex++)
                    {
                        if(!state->items[intersectIndex].marked
                           && oc_card_spacer_intersect(&state->items[workingIndex], &state->items[intersectIndex]))
                        {
                            intersectCount++;
                            // generate 4 axis aligned moves that push that card so that Cj doesn't intersect with any marked card.
                            // prune moves that are opposite to the moves of the cards it intersects with in the working set
                            // (note: this avoids cards being swapped)

                            u32 moveCount = 0;
                            oc_vec2 moves[4];
                            for(u32 dirIndex = 0; dirIndex < 4; dirIndex++)
                            {
                                bool prune = false;
                                for(u64 pusherIndex = 0; pusherIndex < canvas->cardCount; pusherIndex++)
                                {
                                    if(state->items[pusherIndex].working
                                       && oc_card_spacer_intersect(&state->items[pusherIndex], &state->items[intersectIndex]))
                                    {
                                        //TODO: if direction of pusher is opposite of dirindex, prune
                                    }
                                }
                                if(!prune)
                                {
                                    //TODO generate move in direction dirIndex
                                }
                            }

                            // for each move, create a new state by appending to state
                            for(u32 moveIndex = 0; moveIndex < moveCount; moveIndex++)
                            {
                                oc_card_spacer_state* newState = oc_card_spacer_state_create(scratch.arena, canvas->cardCount);
                                memcpy(newState->items, state->items, canvas->cardCount * sizeof(oc_card_spacer_item));

                                newState->items[workingIndex].working = false;
                                newState->items[intersectIndex].marked = true;
                                newState->items[intersectIndex].working = true;
                                newState->items[intersectIndex].newPos = oc_vec2_add(newState->items[intersectIndex].card->rect.xy,
                                                                                     moves[moveIndex]);

                                oc_list_push_back(&newStates, &newState->listElt);
                            }
                        }
                    }
                    if(!intersectCount)
                    {
                        oc_list_remove(&states, &state->listElt);
                        oc_list_push_back(&validStates, &state->listElt);
                        validStateCount++;
                    }
                }
            }
        }
    }

    //TODO: select best valid state
    oc_card_spacer_state* bestState = oc_list_first_entry(validStates, oc_card_spacer_state, listElt);
    for(u64 i = 0; i < canvas->cardCount; i++)
    {
        bestState->items[i].card->rect.xy = bestState->items[i].newPos;
    }

    oc_scratch_end(scratch);
}
*/

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
    bool marked;
    bool working;

} oc_card_spacer_item;

typedef struct oc_card_spacer_state
{
    u64 itemCount;
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
    oc_vec2 newPos = item->card->rect.xy;

    do
    {
        intersect = false;
        for(u64 i = 0; i < state->itemCount; i++)
        {
            oc_card_spacer_item* other = &state->items[i];
            if(other->marked)
            {
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
    }
    while(intersect);

    return newPos;
}

void oc_card_spacer(oc_code_canvas* canvas, oc_card* initialCard)
{
    oc_arena_scope scratch = oc_scratch_begin();

    oc_card_spacer_state state = {
        .itemCount = canvas->cardCount,
        .items = oc_arena_push_array(scratch.arena, oc_card_spacer_item, canvas->cardCount),
    };

    oc_card_spacer_item* item = 0;
    oc_list_for_indexed(canvas->cards, it, oc_card, listElt)
    {
        state.items[it.index].card = it.elt;
        state.items[it.index].newPos = it.elt->rect.xy;

        //TODO: test, change that
        if(it.elt != initialCard)
        {
            state.items[it.index].marked = true;
        }
        else
        {
            item = &state.items[it.index];
        }
    }

    //TODO test, change that
    item->card->rect.xy = oc_card_spacer_move_direction(&state, item, OC_CARD_SPACER_UP);

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
                if(canvas.draggedCard)
                {
                    oc_card_spacer(&canvas, canvas.draggedCard);
                }

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
