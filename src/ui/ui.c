/************************************************************/ /**
*
*	@file: ui.c
*	@author: Martin Fouilleul
*	@date: 08/08/2022
*	@revision:
*
*****************************************************************/
#include "ui.h"
#include "platform/platform.h"
#include "platform/platform_clock.h"
#include "platform/platform_debug.h"
#include "util/hash.h"
#include "util/memory.h"

static oc_ui_style OC_UI_STYLE_DEFAULTS = {
    .size.width = { .kind = OC_UI_SIZE_CHILDREN,
                    .value = 0,
                    .relax = 0 },
    .size.height = { .kind = OC_UI_SIZE_CHILDREN,
                     .value = 0,
                     .relax = 0 },

    .layout = { .axis = OC_UI_AXIS_Y,
                .align = { OC_UI_ALIGN_START,
                           OC_UI_ALIGN_START } },
    .color = { 0, 0, 0, 1 },
    .fontSize = 16,
};

oc_thread_local oc_ui_context oc_uiThreadContext = { 0 };
oc_thread_local oc_ui_context* oc_uiCurrentContext = 0;

oc_ui_context* oc_ui_get_context(void)
{
    return (oc_uiCurrentContext);
}

void oc_ui_set_context(oc_ui_context* context)
{
    oc_uiCurrentContext = context;
}

//-----------------------------------------------------------------------------
// stacks
//-----------------------------------------------------------------------------
oc_ui_stack_elt* oc_ui_stack_push(oc_ui_context* ui, oc_ui_stack_elt** stack)
{
    oc_ui_stack_elt* elt = oc_arena_push_type(&ui->frameArena, oc_ui_stack_elt);
    memset(elt, 0, sizeof(oc_ui_stack_elt));
    elt->parent = *stack;
    *stack = elt;
    return (elt);
}

void oc_ui_stack_pop(oc_ui_stack_elt** stack)
{
    if(*stack)
    {
        *stack = (*stack)->parent;
    }
    else
    {
        oc_log_error("ui stack underflow\n");
    }
}

oc_rect oc_ui_intersect_rects(oc_rect lhs, oc_rect rhs)
{
    //NOTE(martin): intersect with current clip
    f32 x0 = oc_max(lhs.x, rhs.x);
    f32 y0 = oc_max(lhs.y, rhs.y);
    f32 x1 = oc_min(lhs.x + lhs.w, rhs.x + rhs.w);
    f32 y1 = oc_min(lhs.y + lhs.h, rhs.y + rhs.h);
    oc_rect r = { x0, y0, oc_max(0, x1 - x0), oc_max(0, y1 - y0) };
    return (r);
}

oc_rect oc_ui_clip_top(void)
{
    oc_rect r = { -FLT_MAX / 2, -FLT_MAX / 2, FLT_MAX, FLT_MAX };
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_stack_elt* elt = ui->clipStack;
    if(elt)
    {
        r = elt->clip;
    }
    return (r);
}

void oc_ui_clip_push(oc_rect clip)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_rect current = oc_ui_clip_top();
    oc_ui_stack_elt* elt = oc_ui_stack_push(ui, &ui->clipStack);
    elt->clip = oc_ui_intersect_rects(current, clip);
}

void oc_ui_clip_pop(void)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_stack_pop(&ui->clipStack);
}

oc_ui_box* oc_ui_box_top(void)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_stack_elt* elt = ui->boxStack;
    oc_ui_box* box = elt ? elt->box : 0;
    return (box);
}

void oc_ui_box_push(oc_ui_box* box)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_stack_elt* elt = oc_ui_stack_push(ui, &ui->boxStack);
    elt->box = box;
    if(box->flags & OC_UI_FLAG_CLIP)
    {
        oc_ui_clip_push(box->rect);
    }
}

void oc_ui_box_pop(void)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_box* box = oc_ui_box_top();
    if(box)
    {
        if(box->flags & OC_UI_FLAG_CLIP)
        {
            oc_ui_clip_pop();
        }
        oc_ui_stack_pop(&ui->boxStack);
    }
}

//-----------------------------------------------------------------------------
// tagging
//-----------------------------------------------------------------------------

oc_ui_tag oc_ui_tag_make_str8(oc_str8 string)
{
    oc_ui_tag tag = { .hash = oc_hash_xx64_string(string) };
    return (tag);
}

void oc_ui_tag_box_str8(oc_ui_box* box, oc_str8 string)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_tag_elt* elt = oc_arena_push_type(&ui->frameArena, oc_ui_tag_elt);
    elt->tag = oc_ui_tag_make_str8(string);
    oc_list_append(&box->tags, &elt->listElt);
}

void oc_ui_tag_next_str8(oc_str8 string)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_tag_elt* elt = oc_arena_push_type(&ui->frameArena, oc_ui_tag_elt);
    elt->tag = oc_ui_tag_make_str8(string);
    oc_list_append(&ui->nextBoxTags, &elt->listElt);
}

//-----------------------------------------------------------------------------
// key hashing and caching
//-----------------------------------------------------------------------------
oc_ui_key oc_ui_key_make_str8(oc_str8 string)
{
    oc_ui_context* ui = oc_ui_get_context();
    u64 seed = 0;
    oc_ui_box* parent = oc_ui_box_top();
    if(parent)
    {
        seed = parent->key.hash;
    }

    oc_ui_key key = { 0 };
    key.hash = oc_hash_xx64_string_seed(string, seed);
    return (key);
}

oc_ui_key oc_ui_key_make_path(oc_str8_list path)
{
    oc_ui_context* ui = oc_ui_get_context();
    u64 seed = 0;
    oc_ui_box* parent = oc_ui_box_top();
    if(parent)
    {
        seed = parent->key.hash;
    }
    oc_list_for(&path.list, elt, oc_str8_elt, listElt)
    {
        seed = oc_hash_xx64_string_seed(elt->string, seed);
    }
    oc_ui_key key = { seed };
    return (key);
}

bool oc_ui_key_equal(oc_ui_key a, oc_ui_key b)
{
    return (a.hash == b.hash);
}

void oc_ui_box_cache(oc_ui_context* ui, oc_ui_box* box)
{
    u64 index = box->key.hash & (OC_UI_BOX_MAP_BUCKET_COUNT - 1);
    oc_list_append(&(ui->boxMap[index]), &box->bucketElt);
}

oc_ui_box* oc_ui_box_lookup_key(oc_ui_key key)
{
    oc_ui_context* ui = oc_ui_get_context();
    u64 index = key.hash & (OC_UI_BOX_MAP_BUCKET_COUNT - 1);

    oc_list_for(&ui->boxMap[index], box, oc_ui_box, bucketElt)
    {
        if(oc_ui_key_equal(key, box->key))
        {
            return (box);
        }
    }
    return (0);
}

oc_ui_box* oc_ui_box_lookup_str8(oc_str8 string)
{
    oc_ui_key key = oc_ui_key_make_str8(string);
    return (oc_ui_box_lookup_key(key));
}

//-----------------------------------------------------------------------------
// styling
//-----------------------------------------------------------------------------

void oc_ui_pattern_push(oc_arena* arena, oc_ui_pattern* pattern, oc_ui_selector selector)
{
    oc_ui_selector* copy = oc_arena_push_type(arena, oc_ui_selector);
    *copy = selector;
    oc_list_append(&pattern->l, &copy->listElt);
}

oc_ui_pattern oc_ui_pattern_all(void)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_pattern pattern = { 0 };
    oc_ui_pattern_push(&ui->frameArena, &pattern, (oc_ui_selector){ .kind = OC_UI_SEL_ANY });
    return (pattern);
}

oc_ui_pattern oc_ui_pattern_owner(void)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_pattern pattern = { 0 };
    oc_ui_pattern_push(&ui->frameArena, &pattern, (oc_ui_selector){ .kind = OC_UI_SEL_OWNER });
    return (pattern);
}

void oc_ui_style_match_before(oc_ui_pattern pattern, oc_ui_style* style, oc_ui_style_mask mask)
{
    oc_ui_context* ui = oc_ui_get_context();
    if(ui)
    {
        oc_ui_style_rule* rule = oc_arena_push_type(&ui->frameArena, oc_ui_style_rule);
        rule->pattern = pattern;
        rule->mask = mask;
        rule->style = oc_arena_push_type(&ui->frameArena, oc_ui_style);
        *rule->style = *style;

        oc_list_append(&ui->nextBoxBeforeRules, &rule->boxElt);
    }
}

void oc_ui_style_match_after(oc_ui_pattern pattern, oc_ui_style* style, oc_ui_style_mask mask)
{
    oc_ui_context* ui = oc_ui_get_context();
    if(ui)
    {
        oc_ui_style_rule* rule = oc_arena_push_type(&ui->frameArena, oc_ui_style_rule);
        rule->pattern = pattern;
        rule->mask = mask;
        rule->style = oc_arena_push_type(&ui->frameArena, oc_ui_style);
        *rule->style = *style;

        oc_list_append(&ui->nextBoxAfterRules, &rule->boxElt);
    }
}

void oc_ui_style_next(oc_ui_style* style, oc_ui_style_mask mask)
{
    oc_ui_style_match_before(oc_ui_pattern_owner(), style, mask);
}

void oc_ui_style_box_before(oc_ui_box* box, oc_ui_pattern pattern, oc_ui_style* style, oc_ui_style_mask mask)
{
    oc_ui_context* ui = oc_ui_get_context();
    if(ui)
    {
        oc_ui_style_rule* rule = oc_arena_push_type(&ui->frameArena, oc_ui_style_rule);
        rule->pattern = pattern;
        rule->mask = mask;
        rule->style = oc_arena_push_type(&ui->frameArena, oc_ui_style);
        *rule->style = *style;

        oc_list_append(&box->beforeRules, &rule->boxElt);
        rule->owner = box;
    }
}

void oc_ui_style_box_after(oc_ui_box* box, oc_ui_pattern pattern, oc_ui_style* style, oc_ui_style_mask mask)
{
    oc_ui_context* ui = oc_ui_get_context();
    if(ui)
    {
        oc_ui_style_rule* rule = oc_arena_push_type(&ui->frameArena, oc_ui_style_rule);
        rule->pattern = pattern;
        rule->mask = mask;
        rule->style = oc_arena_push_type(&ui->frameArena, oc_ui_style);
        *rule->style = *style;

        oc_list_append(&box->afterRules, &rule->boxElt);
        rule->owner = box;
    }
}

//-----------------------------------------------------------------------------
// input
//-----------------------------------------------------------------------------

void oc_ui_process_event(oc_event* event)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_input_process_event(&ui->input, event);
}

oc_vec2 oc_ui_mouse_position(void)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_vec2 mousePos = oc_mouse_position(&ui->input);
    return (mousePos);
}

oc_vec2 oc_ui_mouse_delta(void)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_vec2 delta = oc_mouse_delta(&ui->input);
    return (delta);
}

oc_vec2 oc_ui_mouse_wheel(void)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_vec2 delta = oc_mouse_wheel(&ui->input);
    return (delta);
}

//-----------------------------------------------------------------------------
// ui boxes
//-----------------------------------------------------------------------------

bool oc_ui_rect_hit(oc_rect r, oc_vec2 p)
{
    return ((p.x > r.x)
            && (p.x < r.x + r.w)
            && (p.y > r.y)
            && (p.y < r.y + r.h));
}

bool oc_ui_box_hovering(oc_ui_box* box, oc_vec2 p)
{
    oc_ui_context* ui = oc_ui_get_context();

    oc_rect clip = oc_ui_clip_top();
    oc_rect rect = oc_ui_intersect_rects(clip, box->rect);
    bool hit = oc_ui_rect_hit(rect, p);
    bool result = hit && (!ui->hovered || box->z >= ui->hovered->z);
    return (result);
}

oc_ui_box* oc_ui_box_make_str8(oc_str8 string, oc_ui_flags flags)
{
    oc_ui_context* ui = oc_ui_get_context();

    oc_ui_key key = oc_ui_key_make_str8(string);
    oc_ui_box* box = oc_ui_box_lookup_key(key);

    if(!box)
    {
        box = oc_pool_alloc_type(&ui->boxPool, oc_ui_box);
        memset(box, 0, sizeof(oc_ui_box));

        box->key = key;
        box->fresh = true;
        oc_ui_box_cache(ui, box);
    }
    else
    {
        box->fresh = false;
    }

    //NOTE: setup hierarchy
    if(box->frameCounter != ui->frameCounter)
    {
        oc_list_init(&box->children);
        box->parent = oc_ui_box_top();
        if(box->parent)
        {
            oc_list_append(&box->parent->children, &box->listElt);
            box->parentClosed = box->parent->closed || box->parent->parentClosed;
        }

        if(box->flags & OC_UI_FLAG_OVERLAY)
        {
            oc_list_append(&ui->overlayList, &box->overlayElt);
        }
    }
    else
    {
        //maybe this should be a warning that we're trying to make the box twice in the same frame?
        oc_log_warning("trying to make ui box '%.*s' multiple times in the same frame\n", (int)box->string.len, box->string.ptr);
    }

    //NOTE: setup per-frame state
    box->frameCounter = ui->frameCounter;
    box->string = oc_str8_push_copy(&ui->frameArena, string);
    box->flags = flags;

    //NOTE: create style and setup non-inherited attributes to default values
    box->targetStyle = oc_arena_push_type(&ui->frameArena, oc_ui_style);
    oc_ui_apply_style_with_mask(box->targetStyle, &OC_UI_STYLE_DEFAULTS, ~0ULL);

    //NOTE: set tags, before rules and last box
    box->tags = ui->nextBoxTags;
    ui->nextBoxTags = (oc_list){ 0 };

    box->beforeRules = ui->nextBoxBeforeRules;
    oc_list_for(&box->beforeRules, rule, oc_ui_style_rule, boxElt)
    {
        rule->owner = box;
    }
    ui->nextBoxBeforeRules = (oc_list){ 0 };

    box->afterRules = ui->nextBoxAfterRules;
    oc_list_for(&box->afterRules, rule, oc_ui_style_rule, boxElt)
    {
        rule->owner = box;
    }
    ui->nextBoxAfterRules = (oc_list){ 0 };

    //NOTE: set scroll
    if(oc_ui_box_hovering(box, oc_ui_mouse_position()))
    {
        oc_vec2 wheel = oc_ui_mouse_wheel();
        if(box->flags & OC_UI_FLAG_SCROLL_WHEEL_X)
        {
            box->scroll.x += wheel.x;
        }
        if(box->flags & OC_UI_FLAG_SCROLL_WHEEL_Y)
        {
            box->scroll.y += wheel.y;
        }
    }
    return (box);
}

oc_ui_box* oc_ui_box_begin_str8(oc_str8 string, oc_ui_flags flags)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_box* box = oc_ui_box_make_str8(string, flags);
    oc_ui_box_push(box);
    return (box);
}

oc_ui_box* oc_ui_box_end(void)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_box* box = oc_ui_box_top();
    OC_DEBUG_ASSERT(box, "box stack underflow");

    oc_ui_box_pop();

    return (box);
}

void oc_ui_box_set_draw_proc(oc_ui_box* box, oc_ui_box_draw_proc proc, void* data)
{
    box->drawProc = proc;
    box->drawData = data;
}

void oc_ui_box_set_closed(oc_ui_box* box, bool closed)
{
    box->closed = closed;
}

bool oc_ui_box_closed(oc_ui_box* box)
{
    return (box->closed);
}

void oc_ui_box_activate(oc_ui_box* box)
{
    box->active = true;
}

void oc_ui_box_deactivate(oc_ui_box* box)
{
    box->active = false;
}

bool oc_ui_box_active(oc_ui_box* box)
{
    return (box->active);
}

void oc_ui_box_set_hot(oc_ui_box* box, bool hot)
{
    box->hot = hot;
}

bool oc_ui_box_hot(oc_ui_box* box)
{
    return (box->hot);
}

oc_ui_sig oc_ui_box_sig(oc_ui_box* box)
{
    //NOTE: compute input signals
    oc_ui_sig sig = { 0 };

    oc_ui_context* ui = oc_ui_get_context();
    oc_input_state* input = &ui->input;

    sig.box = box;

    if(!box->closed && !box->parentClosed)
    {
        oc_vec2 mousePos = oc_ui_mouse_position();

        sig.mouse = (oc_vec2){ mousePos.x - box->rect.x, mousePos.y - box->rect.y };
        sig.delta = oc_ui_mouse_delta();
        sig.wheel = oc_ui_mouse_wheel();
        sig.hovering = oc_ui_box_hovering(box, mousePos);

        if(box->flags & OC_UI_FLAG_CLICKABLE)
        {
            if(sig.hovering)
            {
                sig.pressed = oc_mouse_pressed(input, OC_MOUSE_LEFT);
                if(sig.pressed)
                {
                    if(!box->dragging)
                    {
                        box->pressedMouse = sig.mouse;
                    }
                    box->dragging = true;
                }
                sig.doubleClicked = oc_mouse_double_clicked(input, OC_MOUSE_LEFT);
                sig.rightPressed = oc_mouse_pressed(input, OC_MOUSE_RIGHT);
            }

            sig.released = oc_mouse_released(input, OC_MOUSE_LEFT);
            if(sig.released)
            {
                if(box->dragging && sig.hovering)
                {
                    sig.clicked = true;
                }
            }

            if(!oc_mouse_down(input, OC_MOUSE_LEFT))
            {
                box->dragging = false;
            }

            sig.dragging = box->dragging;
        }
    }
    return (sig);
}

bool oc_ui_box_hidden(oc_ui_box* box)
{
    return (box->closed || box->parentClosed);
}

//-----------------------------------------------------------------------------
// Auto-layout
//-----------------------------------------------------------------------------

void oc_ui_animate_f32(oc_ui_context* ui, f32* value, f32 target, f32 animationTime)
{
    if(animationTime < 1e-6
       || fabs(*value - target) < 0.001)
    {
        *value = target;
    }
    else
    {

        /*NOTE:
			we use the euler approximation for df/dt = alpha(target - f)
			the implicit form is f(t) = target*(1-e^(-alpha*t)) for the rising front,
			and f(t) = e^(-alpha*t) for the falling front (e.g. classic RC circuit charge/discharge)

			Here we bake alpha = 1/tau = -ln(0.05)/tr, with tr the rise time to 95% of target
		*/
        f32 alpha = 3 / animationTime;
        f32 dt = ui->lastFrameDuration;

        *value += (target - *value) * alpha * dt;
    }
}

void oc_ui_animate_color(oc_ui_context* ui, oc_color* color, oc_color target, f32 animationTime)
{
    for(int i = 0; i < 4; i++)
    {
        oc_ui_animate_f32(ui, &color->c[i], target.c[i], animationTime);
    }
}

void oc_ui_animate_oc_ui_size(oc_ui_context* ui, oc_ui_size* size, oc_ui_size target, f32 animationTime)
{
    size->kind = target.kind;
    oc_ui_animate_f32(ui, &size->value, target.value, animationTime);
    oc_ui_animate_f32(ui, &size->relax, target.relax, animationTime);
}

void oc_ui_box_animate_style(oc_ui_context* ui, oc_ui_box* box)
{
    oc_ui_style* targetStyle = box->targetStyle;
    OC_DEBUG_ASSERT(targetStyle);

    f32 animationTime = targetStyle->animationTime;

    //NOTE: interpolate based on transition values
    oc_ui_style_mask mask = box->targetStyle->animationMask;

    if(box->fresh)
    {
        box->style = *targetStyle;
    }
    else
    {
        if(mask & OC_UI_STYLE_SIZE_WIDTH)
        {
            oc_ui_animate_oc_ui_size(ui, &box->style.size.c[OC_UI_AXIS_X], targetStyle->size.c[OC_UI_AXIS_X], animationTime);
        }
        else
        {
            box->style.size.c[OC_UI_AXIS_X] = targetStyle->size.c[OC_UI_AXIS_X];
        }

        if(mask & OC_UI_STYLE_SIZE_HEIGHT)
        {
            oc_ui_animate_oc_ui_size(ui, &box->style.size.c[OC_UI_AXIS_Y], targetStyle->size.c[OC_UI_AXIS_Y], animationTime);
        }
        else
        {
            box->style.size.c[OC_UI_AXIS_Y] = targetStyle->size.c[OC_UI_AXIS_Y];
        }

        if(mask & OC_UI_STYLE_COLOR)
        {
            oc_ui_animate_color(ui, &box->style.color, targetStyle->color, animationTime);
        }
        else
        {
            box->style.color = targetStyle->color;
        }

        if(mask & OC_UI_STYLE_BG_COLOR)
        {
            oc_ui_animate_color(ui, &box->style.bgColor, targetStyle->bgColor, animationTime);
        }
        else
        {
            box->style.bgColor = targetStyle->bgColor;
        }

        if(mask & OC_UI_STYLE_BORDER_COLOR)
        {
            oc_ui_animate_color(ui, &box->style.borderColor, targetStyle->borderColor, animationTime);
        }
        else
        {
            box->style.borderColor = targetStyle->borderColor;
        }

        if(mask & OC_UI_STYLE_FONT_SIZE)
        {
            oc_ui_animate_f32(ui, &box->style.fontSize, targetStyle->fontSize, animationTime);
        }
        else
        {
            box->style.fontSize = targetStyle->fontSize;
        }

        if(mask & OC_UI_STYLE_BORDER_SIZE)
        {
            oc_ui_animate_f32(ui, &box->style.borderSize, targetStyle->borderSize, animationTime);
        }
        else
        {
            box->style.borderSize = targetStyle->borderSize;
        }

        if(mask & OC_UI_STYLE_ROUNDNESS)
        {
            oc_ui_animate_f32(ui, &box->style.roundness, targetStyle->roundness, animationTime);
        }
        else
        {
            box->style.roundness = targetStyle->roundness;
        }

        //NOTE: float target is animated in compute rect
        box->style.floatTarget = targetStyle->floatTarget;

        //TODO: non animatable attributes. use mask
        box->style.layout = targetStyle->layout;
        box->style.font = targetStyle->font;
    }
}

void oc_ui_apply_style_with_mask(oc_ui_style* dst, oc_ui_style* src, oc_ui_style_mask mask)
{
    if(mask & OC_UI_STYLE_SIZE_WIDTH)
    {
        dst->size.c[OC_UI_AXIS_X] = src->size.c[OC_UI_AXIS_X];
    }
    if(mask & OC_UI_STYLE_SIZE_HEIGHT)
    {
        dst->size.c[OC_UI_AXIS_Y] = src->size.c[OC_UI_AXIS_Y];
    }
    if(mask & OC_UI_STYLE_LAYOUT_AXIS)
    {
        dst->layout.axis = src->layout.axis;
    }
    if(mask & OC_UI_STYLE_LAYOUT_ALIGN_X)
    {
        dst->layout.align.x = src->layout.align.x;
    }
    if(mask & OC_UI_STYLE_LAYOUT_ALIGN_Y)
    {
        dst->layout.align.y = src->layout.align.y;
    }
    if(mask & OC_UI_STYLE_LAYOUT_SPACING)
    {
        dst->layout.spacing = src->layout.spacing;
    }
    if(mask & OC_UI_STYLE_LAYOUT_MARGIN_X)
    {
        dst->layout.margin.x = src->layout.margin.x;
    }
    if(mask & OC_UI_STYLE_LAYOUT_MARGIN_Y)
    {
        dst->layout.margin.y = src->layout.margin.y;
    }
    if(mask & OC_UI_STYLE_FLOAT_X)
    {
        dst->floating.c[OC_UI_AXIS_X] = src->floating.c[OC_UI_AXIS_X];
        dst->floatTarget.x = src->floatTarget.x;
    }
    if(mask & OC_UI_STYLE_FLOAT_Y)
    {
        dst->floating.c[OC_UI_AXIS_Y] = src->floating.c[OC_UI_AXIS_Y];
        dst->floatTarget.y = src->floatTarget.y;
    }
    if(mask & OC_UI_STYLE_COLOR)
    {
        dst->color = src->color;
    }
    if(mask & OC_UI_STYLE_BG_COLOR)
    {
        dst->bgColor = src->bgColor;
    }
    if(mask & OC_UI_STYLE_BORDER_COLOR)
    {
        dst->borderColor = src->borderColor;
    }
    if(mask & OC_UI_STYLE_BORDER_SIZE)
    {
        dst->borderSize = src->borderSize;
    }
    if(mask & OC_UI_STYLE_ROUNDNESS)
    {
        dst->roundness = src->roundness;
    }
    if(mask & OC_UI_STYLE_FONT)
    {
        dst->font = src->font;
    }
    if(mask & OC_UI_STYLE_FONT_SIZE)
    {
        dst->fontSize = src->fontSize;
    }
    if(mask & OC_UI_STYLE_ANIMATION_TIME)
    {
        dst->animationTime = src->animationTime;
    }
    if(mask & OC_UI_STYLE_ANIMATION_MASK)
    {
        dst->animationMask = src->animationMask;
    }
}

bool oc_ui_style_selector_match(oc_ui_box* box, oc_ui_style_rule* rule, oc_ui_selector* selector)
{
    bool res = false;
    switch(selector->kind)
    {
        case OC_UI_SEL_ANY:
            res = true;
            break;

        case OC_UI_SEL_OWNER:
            res = (box == rule->owner);
            break;

        case OC_UI_SEL_TEXT:
            res = !oc_str8_cmp(box->string, selector->text);
            break;

        case OC_UI_SEL_TAG:
        {
            oc_list_for(&box->tags, elt, oc_ui_tag_elt, listElt)
            {
                if(elt->tag.hash == selector->tag.hash)
                {
                    res = true;
                    break;
                }
            }
        }
        break;

        case OC_UI_SEL_STATUS:
        {
            res = true;
            if(selector->status & OC_UI_HOVER)
            {
                res = res && oc_ui_box_hovering(box, oc_ui_mouse_position());
            }
            if(selector->status & OC_UI_ACTIVE)
            {
                res = res && box->active;
            }
            if(selector->status & OC_UI_DRAGGING)
            {
                res = res && box->dragging;
            }
        }
        break;

        case OC_UI_SEL_KEY:
            res = oc_ui_key_equal(box->key, selector->key);
        default:
            break;
    }
    return (res);
}

void oc_ui_style_rule_match(oc_ui_context* ui, oc_ui_box* box, oc_ui_style_rule* rule, oc_list* buildList, oc_list* tmpList)
{
    oc_ui_selector* selector = oc_list_first_entry(&rule->pattern.l, oc_ui_selector, listElt);
    bool match = oc_ui_style_selector_match(box, rule, selector);

    selector = oc_list_next_entry(&rule->pattern.l, selector, oc_ui_selector, listElt);
    while(match && selector && selector->op == OC_UI_SEL_AND)
    {
        match = match && oc_ui_style_selector_match(box, rule, selector);
        selector = oc_list_next_entry(&rule->pattern.l, selector, oc_ui_selector, listElt);
    }

    if(match)
    {
        if(!selector)
        {
            oc_ui_apply_style_with_mask(box->targetStyle, rule->style, rule->mask);
        }
        else
        {
            //NOTE create derived rule if there's more than one selector
            oc_ui_style_rule* derived = oc_arena_push_type(&ui->frameArena, oc_ui_style_rule);
            derived->mask = rule->mask;
            derived->style = rule->style;
            derived->pattern.l = (oc_list){ &selector->listElt, rule->pattern.l.last };

            oc_list_append(buildList, &derived->buildElt);
            oc_list_append(tmpList, &derived->tmpElt);
        }
    }
}

void oc_ui_styling_prepass(oc_ui_context* ui, oc_ui_box* box, oc_list* before, oc_list* after)
{
    //NOTE: inherit style from parent
    if(box->parent)
    {
        oc_ui_apply_style_with_mask(box->targetStyle,
                                    box->parent->targetStyle,
                                    OC_UI_STYLE_MASK_INHERITED);
    }

    //NOTE: append box before rules to before and tmp
    oc_list tmpBefore = { 0 };
    oc_list_for(&box->beforeRules, rule, oc_ui_style_rule, boxElt)
    {
        oc_list_append(before, &rule->buildElt);
        oc_list_append(&tmpBefore, &rule->tmpElt);
    }
    //NOTE: match before rules
    oc_list_for(before, rule, oc_ui_style_rule, buildElt)
    {
        oc_ui_style_rule_match(ui, box, rule, before, &tmpBefore);
    }

    //NOTE: prepend box after rules to after and append them to tmp
    oc_list tmpAfter = { 0 };
    oc_list_for_reverse(&box->afterRules, rule, oc_ui_style_rule, boxElt)
    {
        oc_list_push(after, &rule->buildElt);
        oc_list_append(&tmpAfter, &rule->tmpElt);
    }

    //NOTE: match after rules
    oc_list_for(after, rule, oc_ui_style_rule, buildElt)
    {
        oc_ui_style_rule_match(ui, box, rule, after, &tmpAfter);
    }

    //NOTE: compute static sizes
    oc_ui_box_animate_style(ui, box);

    if(oc_ui_box_hidden(box))
    {
        return;
    }

    oc_ui_style* style = &box->style;

    oc_rect textBox = { 0 };
    oc_ui_size desiredSize[2] = { box->style.size.c[OC_UI_AXIS_X],
                                  box->style.size.c[OC_UI_AXIS_Y] };

    if(desiredSize[OC_UI_AXIS_X].kind == OC_UI_SIZE_TEXT
       || desiredSize[OC_UI_AXIS_Y].kind == OC_UI_SIZE_TEXT)
    {
        textBox = oc_text_bounding_box(style->font, style->fontSize, box->string);
    }

    for(int i = 0; i < OC_UI_AXIS_COUNT; i++)
    {
        oc_ui_size size = desiredSize[i];

        if(size.kind == OC_UI_SIZE_TEXT)
        {
            f32 margin = style->layout.margin.c[i];
            box->rect.c[2 + i] = textBox.c[2 + i] + margin * 2;
        }
        else if(size.kind == OC_UI_SIZE_PIXELS)
        {
            box->rect.c[2 + i] = size.value;
        }
    }

    //NOTE: descend in children
    oc_list_for(&box->children, child, oc_ui_box, listElt)
    {
        oc_ui_styling_prepass(ui, child, before, after);
    }

    //NOTE: remove temporary rules
    oc_list_for(&tmpBefore, rule, oc_ui_style_rule, tmpElt)
    {
        oc_list_remove(before, &rule->buildElt);
    }
    oc_list_for(&tmpAfter, rule, oc_ui_style_rule, tmpElt)
    {
        oc_list_remove(after, &rule->buildElt);
    }
}

bool oc_ui_layout_downward_dependency(oc_ui_box* child, int axis)
{
    return (!oc_ui_box_hidden(child)
            && !child->style.floating.c[axis]
            && child->style.size.c[axis].kind != OC_UI_SIZE_PARENT
            && child->style.size.c[axis].kind != OC_UI_SIZE_PARENT_MINUS_PIXELS);
}

void oc_ui_layout_downward_dependent_size(oc_ui_context* ui, oc_ui_box* box, int axis)
{
    //NOTE: layout children and compute spacing
    f32 count = 0;
    oc_list_for(&box->children, child, oc_ui_box, listElt)
    {
        if(!oc_ui_box_hidden(child))
        {
            oc_ui_layout_downward_dependent_size(ui, child, axis);

            if(box->style.layout.axis == axis
               && !child->style.floating.c[axis])
            {
                count++;
            }
        }
    }
    box->spacing[axis] = oc_max(0, count - 1) * box->style.layout.spacing;

    oc_ui_size* size = &box->style.size.c[axis];
    if(size->kind == OC_UI_SIZE_CHILDREN)
    {
        //NOTE: if box is dependent on children, compute children's size. If we're in the layout
        //      axis this is the sum of each child size, otherwise it is the maximum child size
        f32 sum = 0;

        if(box->style.layout.axis == axis)
        {
            oc_list_for(&box->children, child, oc_ui_box, listElt)
            {
                if(oc_ui_layout_downward_dependency(child, axis))
                {
                    sum += child->rect.c[2 + axis];
                }
            }
        }
        else
        {
            oc_list_for(&box->children, child, oc_ui_box, listElt)
            {
                if(oc_ui_layout_downward_dependency(child, axis))
                {
                    sum = oc_max(sum, child->rect.c[2 + axis]);
                }
            }
        }
        f32 margin = box->style.layout.margin.c[axis];
        box->rect.c[2 + axis] = sum + box->spacing[axis] + 2 * margin;
    }
}

void oc_ui_layout_upward_dependent_size(oc_ui_context* ui, oc_ui_box* box, int axis)
{
    //NOTE: re-compute/set size of children that depend on box's size

    f32 margin = box->style.layout.margin.c[axis];
    f32 availableSize = oc_max(0, box->rect.c[2 + axis] - box->spacing[axis] - 2 * margin);

    oc_list_for(&box->children, child, oc_ui_box, listElt)
    {
        oc_ui_size* size = &child->style.size.c[axis];
        if(size->kind == OC_UI_SIZE_PARENT)
        {
            child->rect.c[2 + axis] = availableSize * size->value;
        }
        else if(size->kind == OC_UI_SIZE_PARENT_MINUS_PIXELS)
        {
            child->rect.c[2 + axis] = oc_max(0, availableSize - size->value);
        }
    }

    //NOTE: solve downard conflicts
    int overflowFlag = (OC_UI_FLAG_ALLOW_OVERFLOW_X << axis);
    f32 sum = 0;

    if(box->style.layout.axis == axis)
    {
        //NOTE: if we're solving in the layout axis, first compute total sum of children and
        //      total slack available
        f32 slack = 0;

        oc_list_for(&box->children, child, oc_ui_box, listElt)
        {
            if(!oc_ui_box_hidden(child)
               && !child->style.floating.c[axis])
            {
                sum += child->rect.c[2 + axis];
                slack += child->rect.c[2 + axis] * child->style.size.c[axis].relax;
            }
        }

        if(!(box->flags & overflowFlag))
        {
            //NOTE: then remove excess proportionally to each box slack, and recompute children sum.
            f32 totalContents = sum + box->spacing[axis] + 2 * box->style.layout.margin.c[axis];
            f32 excess = oc_clamp_low(totalContents - box->rect.c[2 + axis], 0);
            f32 alpha = oc_clamp(excess / slack, 0, 1);

            sum = 0;
            oc_list_for(&box->children, child, oc_ui_box, listElt)
            {
                f32 relax = child->style.size.c[axis].relax;
                child->rect.c[2 + axis] -= alpha * child->rect.c[2 + axis] * relax;
                sum += child->rect.c[2 + axis];
            }
        }
    }
    else
    {
        //NOTE: if we're solving on the secondary axis, we remove excess to each box individually
        //      according to its own slack. Children sum is the maximum child size.

        oc_list_for(&box->children, child, oc_ui_box, listElt)
        {
            if(!oc_ui_box_hidden(child) && !child->style.floating.c[axis])
            {
                if(!(box->flags & overflowFlag))
                {
                    f32 totalContents = child->rect.c[2 + axis] + 2 * box->style.layout.margin.c[axis];
                    f32 excess = oc_clamp_low(totalContents - box->rect.c[2 + axis], 0);
                    f32 relax = child->style.size.c[axis].relax;
                    child->rect.c[2 + axis] -= oc_min(excess, child->rect.c[2 + axis] * relax);
                }
                sum = oc_max(sum, child->rect.c[2 + axis]);
            }
        }
    }

    box->childrenSum[axis] = sum;

    //NOTE: recurse in children
    oc_list_for(&box->children, child, oc_ui_box, listElt)
    {
        oc_ui_layout_upward_dependent_size(ui, child, axis);
    }
}

void oc_ui_layout_compute_rect(oc_ui_context* ui, oc_ui_box* box, oc_vec2 pos)
{
    if(oc_ui_box_hidden(box))
    {
        return;
    }

    box->rect.x = pos.x;
    box->rect.y = pos.y;
    box->z = ui->z;
    ui->z++;

    oc_ui_axis layoutAxis = box->style.layout.axis;
    oc_ui_axis secondAxis = (layoutAxis == OC_UI_AXIS_X) ? OC_UI_AXIS_Y : OC_UI_AXIS_X;
    f32 spacing = box->style.layout.spacing;

    oc_ui_align* align = box->style.layout.align.c;

    oc_vec2 origin = { box->rect.x,
                       box->rect.y };
    oc_vec2 currentPos = origin;

    oc_vec2 margin = { box->style.layout.margin.x,
                       box->style.layout.margin.y };

    currentPos.x += margin.x;
    currentPos.y += margin.y;

    for(int i = 0; i < OC_UI_AXIS_COUNT; i++)
    {
        if(align[i] == OC_UI_ALIGN_END)
        {
            currentPos.c[i] = origin.c[i] + box->rect.c[2 + i] - (box->childrenSum[i] + box->spacing[i] + margin.c[i]);
        }
    }
    if(align[layoutAxis] == OC_UI_ALIGN_CENTER)
    {
        currentPos.c[layoutAxis] = origin.c[layoutAxis]
                                 + 0.5 * (box->rect.c[2 + layoutAxis] - (box->childrenSum[layoutAxis] + box->spacing[layoutAxis]));
    }

    currentPos.x -= box->scroll.x;
    currentPos.y -= box->scroll.y;

    oc_list_for(&box->children, child, oc_ui_box, listElt)
    {
        if(align[secondAxis] == OC_UI_ALIGN_CENTER)
        {
            currentPos.c[secondAxis] = origin.c[secondAxis] + 0.5 * (box->rect.c[2 + secondAxis] - child->rect.c[2 + secondAxis]);
        }

        oc_vec2 childPos = currentPos;
        for(int i = 0; i < OC_UI_AXIS_COUNT; i++)
        {
            if(child->style.floating.c[i])
            {
                oc_ui_style* style = child->targetStyle;
                if((child->targetStyle->animationMask & (OC_UI_STYLE_FLOAT_X << i))
                   && !child->fresh)
                {
                    oc_ui_animate_f32(ui, &child->floatPos.c[i], child->style.floatTarget.c[i], style->animationTime);
                }
                else
                {
                    child->floatPos.c[i] = child->style.floatTarget.c[i];
                }
                childPos.c[i] = origin.c[i] + child->floatPos.c[i];
            }
        }

        oc_ui_layout_compute_rect(ui, child, childPos);

        if(!child->style.floating.c[layoutAxis])
        {
            currentPos.c[layoutAxis] += child->rect.c[2 + layoutAxis] + spacing;
        }
    }
}

void oc_ui_layout_find_next_hovered_recursive(oc_ui_context* ui, oc_ui_box* box, oc_vec2 p)
{
    if(oc_ui_box_hidden(box))
    {
        return;
    }

    bool hit = oc_ui_rect_hit(box->rect, p);
    if(hit && (box->flags & OC_UI_FLAG_BLOCK_MOUSE))
    {
        ui->hovered = box;
    }
    if(hit || !(box->flags & OC_UI_FLAG_CLIP))
    {
        oc_list_for(&box->children, child, oc_ui_box, listElt)
        {
            oc_ui_layout_find_next_hovered_recursive(ui, child, p);
        }
    }
}

void oc_ui_layout_find_next_hovered(oc_ui_context* ui, oc_vec2 p)
{
    ui->hovered = 0;
    oc_ui_layout_find_next_hovered_recursive(ui, ui->root, p);
}

void oc_ui_solve_layout(oc_ui_context* ui)
{
    oc_list beforeRules = { 0 };
    oc_list afterRules = { 0 };

    //NOTE: style and compute static sizes
    oc_ui_styling_prepass(ui, ui->root, &beforeRules, &afterRules);

    //NOTE: reparent overlay boxes
    oc_list_for(&ui->overlayList, box, oc_ui_box, overlayElt)
    {
        if(box->parent)
        {
            oc_list_remove(&box->parent->children, &box->listElt);
            oc_list_append(&ui->overlay->children, &box->listElt);
        }
    }

    //NOTE: compute layout
    for(int axis = 0; axis < OC_UI_AXIS_COUNT; axis++)
    {
        oc_ui_layout_downward_dependent_size(ui, ui->root, axis);
        oc_ui_layout_upward_dependent_size(ui, ui->root, axis);
    }
    oc_ui_layout_compute_rect(ui, ui->root, (oc_vec2){ 0, 0 });

    oc_vec2 p = oc_ui_mouse_position();
    oc_ui_layout_find_next_hovered(ui, p);
}

//-----------------------------------------------------------------------------
// Drawing
//-----------------------------------------------------------------------------

void oc_ui_rectangle_fill(oc_rect rect, f32 roundness)
{
    if(roundness)
    {
        oc_rounded_rectangle_fill(rect.x, rect.y, rect.w, rect.h, roundness);
    }
    else
    {
        oc_rectangle_fill(rect.x, rect.y, rect.w, rect.h);
    }
}

void oc_ui_rectangle_stroke(oc_rect rect, f32 roundness)
{
    if(roundness)
    {
        oc_rounded_rectangle_stroke(rect.x, rect.y, rect.w, rect.h, roundness);
    }
    else
    {
        oc_rectangle_stroke(rect.x, rect.y, rect.w, rect.h);
    }
}

void oc_ui_draw_box(oc_ui_box* box)
{
    if(oc_ui_box_hidden(box))
    {
        return;
    }

    oc_ui_style* style = &box->style;

    bool draw = true;

    {
        oc_rect clip = oc_clip_top();
        oc_rect expRect = {
            box->rect.x - 0.5 * style->borderSize,
            box->rect.y - 0.5 * style->borderSize,
            box->rect.w + style->borderSize,
            box->rect.h + style->borderSize
        };

        if((expRect.x + expRect.w < clip.x)
           || (expRect.y + expRect.h < clip.y)
           || (expRect.x > clip.x + clip.w)
           || (expRect.y > clip.y + clip.h))
        {
            draw = false;
        }
    }

    if(box->flags & OC_UI_FLAG_CLIP)
    {
        oc_clip_push(box->rect.x, box->rect.y, box->rect.w, box->rect.h);
    }

    if(draw && (box->flags & OC_UI_FLAG_DRAW_BACKGROUND))
    {
        oc_set_color(style->bgColor);
        oc_ui_rectangle_fill(box->rect, style->roundness);
    }

    if(draw
       && (box->flags & OC_UI_FLAG_DRAW_PROC)
       && box->drawProc)
    {
        box->drawProc(box, box->drawData);
    }

    oc_list_for(&box->children, child, oc_ui_box, listElt)
    {
        oc_ui_draw_box(child);
    }

    if(draw && (box->flags & OC_UI_FLAG_DRAW_TEXT))
    {
        oc_rect textBox = oc_text_bounding_box(style->font, style->fontSize, box->string);

        f32 x = 0;
        f32 y = 0;
        switch(style->layout.align.x)
        {
            case OC_UI_ALIGN_START:
                x = box->rect.x + style->layout.margin.x;
                break;

            case OC_UI_ALIGN_END:
                x = box->rect.x + box->rect.w - style->layout.margin.x - textBox.w;
                break;

            case OC_UI_ALIGN_CENTER:
                x = box->rect.x + 0.5 * (box->rect.w - textBox.w);
                break;
        }

        switch(style->layout.align.y)
        {
            case OC_UI_ALIGN_START:
                y = box->rect.y + style->layout.margin.y - textBox.y;
                break;

            case OC_UI_ALIGN_END:
                y = box->rect.y + box->rect.h - style->layout.margin.y - textBox.h + textBox.y;
                break;

            case OC_UI_ALIGN_CENTER:
                y = box->rect.y + 0.5 * (box->rect.h - textBox.h) - textBox.y;
                break;
        }

        oc_set_font(style->font);
        oc_set_font_size(style->fontSize);
        oc_set_color(style->color);

        oc_move_to(x, y);
        oc_text_outlines(box->string);
        oc_fill();
    }

    if(box->flags & OC_UI_FLAG_CLIP)
    {
        oc_clip_pop();
    }

    if(draw && (box->flags & OC_UI_FLAG_DRAW_BORDER))
    {
        oc_set_width(style->borderSize);
        oc_set_color(style->borderColor);
        oc_ui_rectangle_stroke(box->rect, style->roundness);
    }
}

void oc_ui_draw()
{
    oc_ui_context* ui = oc_ui_get_context();

    //NOTE: draw
    bool oldTextFlip = oc_get_text_flip();
    oc_set_text_flip(false);

    oc_ui_draw_box(ui->root);

    oc_set_text_flip(oldTextFlip);
}

//-----------------------------------------------------------------------------
// frame begin/end
//-----------------------------------------------------------------------------

void oc_ui_begin_frame(oc_vec2 size, oc_ui_style* defaultStyle, oc_ui_style_mask defaultMask)
{
    oc_ui_context* ui = oc_ui_get_context();

    oc_arena_clear(&ui->frameArena);

    ui->frameCounter++;
    f64 time = oc_clock_time(OC_CLOCK_MONOTONIC);
    ui->lastFrameDuration = time - ui->frameTime;
    ui->frameTime = time;

    ui->clipStack = 0;
    ui->z = 0;

    defaultMask &= OC_UI_STYLE_COLOR
                 | OC_UI_STYLE_BG_COLOR
                 | OC_UI_STYLE_BORDER_COLOR
                 | OC_UI_STYLE_FONT
                 | OC_UI_STYLE_FONT_SIZE;

    oc_ui_style_match_before(oc_ui_pattern_all(), defaultStyle, defaultMask);
    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, size.x },
                                     .size.height = { OC_UI_SIZE_PIXELS, size.y } },
                     OC_UI_STYLE_SIZE);

    ui->root = oc_ui_box_begin("_root_", 0);

    oc_ui_style_mask contentStyleMask = OC_UI_STYLE_SIZE
                                      | OC_UI_STYLE_LAYOUT
                                      | OC_UI_STYLE_FLOAT;

    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                     .size.height = { OC_UI_SIZE_PARENT, 1 },
                                     .layout = { OC_UI_AXIS_Y, OC_UI_ALIGN_START, OC_UI_ALIGN_START },
                                     .floating = { true, true },
                                     .floatTarget = { 0, 0 } },
                     contentStyleMask);

    oc_ui_box* contents = oc_ui_box_make("_contents_", 0);

    oc_ui_style_next(&(oc_ui_style){ .layout = { OC_UI_AXIS_Y, OC_UI_ALIGN_START, OC_UI_ALIGN_START },
                                     .floating = { true, true },
                                     .floatTarget = { 0, 0 } },
                     OC_UI_STYLE_LAYOUT | OC_UI_STYLE_FLOAT_X | OC_UI_STYLE_FLOAT_Y);

    ui->overlay = oc_ui_box_make("_overlay_", 0);
    ui->overlayList = (oc_list){ 0 };

    ui->nextBoxBeforeRules = (oc_list){ 0 };
    ui->nextBoxAfterRules = (oc_list){ 0 };
    ui->nextBoxTags = (oc_list){ 0 };

    oc_ui_box_push(contents);
}

void oc_ui_end_frame(void)
{
    oc_ui_context* ui = oc_ui_get_context();

    oc_ui_box_pop();

    oc_ui_box* box = oc_ui_box_end();
    OC_DEBUG_ASSERT(box == ui->root, "unbalanced box stack");

    //TODO: check balancing of style stacks

    //NOTE: layout
    oc_ui_solve_layout(ui);

    //NOTE: prune unused boxes
    for(int i = 0; i < OC_UI_BOX_MAP_BUCKET_COUNT; i++)
    {
        oc_list_for_safe(&ui->boxMap[i], box, oc_ui_box, bucketElt)
        {
            if(box->frameCounter < ui->frameCounter)
            {
                oc_list_remove(&ui->boxMap[i], &box->bucketElt);
            }
        }
    }

    oc_input_next_frame(&ui->input);
}

//-----------------------------------------------------------------------------
// Init / cleanup
//-----------------------------------------------------------------------------
void oc_ui_init(oc_ui_context* ui)
{
    oc_uiCurrentContext = &oc_uiThreadContext;

    memset(ui, 0, sizeof(oc_ui_context));
    oc_arena_init(&ui->frameArena);
    oc_pool_init(&ui->boxPool, sizeof(oc_ui_box));
    ui->init = true;

    oc_ui_set_context(ui);
}

void oc_ui_cleanup(void)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_arena_cleanup(&ui->frameArena);
    oc_pool_cleanup(&ui->boxPool);
    ui->init = false;
}

//-----------------------------------------------------------------------------
// label
//-----------------------------------------------------------------------------

oc_ui_sig oc_ui_label_str8(oc_str8 label)
{
    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_TEXT, 0, 0 },
                                     .size.height = { OC_UI_SIZE_TEXT, 0, 0 } },
                     OC_UI_STYLE_SIZE_WIDTH | OC_UI_STYLE_SIZE_HEIGHT);

    oc_ui_flags flags = OC_UI_FLAG_CLIP
                      | OC_UI_FLAG_DRAW_TEXT;
    oc_ui_box* box = oc_ui_box_make_str8(label, flags);

    oc_ui_sig sig = oc_ui_box_sig(box);
    return (sig);
}

oc_ui_sig oc_ui_label(const char* label)
{
    return (oc_ui_label_str8(OC_STR8((char*)label)));
}

//------------------------------------------------------------------------------
// button
//------------------------------------------------------------------------------

oc_ui_sig oc_ui_button_behavior(oc_ui_box* box)
{
    oc_ui_sig sig = oc_ui_box_sig(box);

    if(sig.hovering)
    {
        oc_ui_box_set_hot(box, true);
        if(sig.dragging)
        {
            oc_ui_box_activate(box);
        }
    }
    else
    {
        oc_ui_box_set_hot(box, false);
    }
    if(!sig.dragging)
    {
        oc_ui_box_deactivate(box);
    }
    return (sig);
}

oc_ui_sig oc_ui_button_str8(oc_str8 label)
{
    oc_ui_context* ui = oc_ui_get_context();

    oc_ui_style defaultStyle = { .size.width = { OC_UI_SIZE_TEXT },
                                 .size.height = { OC_UI_SIZE_TEXT },
                                 .layout.align.x = OC_UI_ALIGN_CENTER,
                                 .layout.align.y = OC_UI_ALIGN_CENTER,
                                 .layout.margin.x = 5,
                                 .layout.margin.y = 5,
                                 .bgColor = { 0.5, 0.5, 0.5, 1 },
                                 .borderColor = { 0.2, 0.2, 0.2, 1 },
                                 .borderSize = 1,
                                 .roundness = 10 };

    oc_ui_style_mask defaultMask = OC_UI_STYLE_SIZE_WIDTH
                                 | OC_UI_STYLE_SIZE_HEIGHT
                                 | OC_UI_STYLE_LAYOUT_MARGIN_X
                                 | OC_UI_STYLE_LAYOUT_MARGIN_Y
                                 | OC_UI_STYLE_LAYOUT_ALIGN_X
                                 | OC_UI_STYLE_LAYOUT_ALIGN_Y
                                 | OC_UI_STYLE_BG_COLOR
                                 | OC_UI_STYLE_BORDER_COLOR
                                 | OC_UI_STYLE_BORDER_SIZE
                                 | OC_UI_STYLE_ROUNDNESS;

    oc_ui_style_next(&defaultStyle, defaultMask);

    oc_ui_style activeStyle = { .bgColor = { 0.3, 0.3, 0.3, 1 },
                                .borderColor = { 0.2, 0.2, 0.2, 1 },
                                .borderSize = 2 };
    oc_ui_style_mask activeMask = OC_UI_STYLE_BG_COLOR
                                | OC_UI_STYLE_BORDER_COLOR
                                | OC_UI_STYLE_BORDER_SIZE;
    oc_ui_pattern activePattern = { 0 };
    oc_ui_pattern_push(&ui->frameArena,
                       &activePattern,
                       (oc_ui_selector){ .kind = OC_UI_SEL_STATUS,
                                         .status = OC_UI_ACTIVE | OC_UI_HOVER });
    oc_ui_style_match_before(activePattern, &activeStyle, activeMask);

    oc_ui_flags flags = OC_UI_FLAG_CLICKABLE
                      | OC_UI_FLAG_CLIP
                      | OC_UI_FLAG_DRAW_BACKGROUND
                      | OC_UI_FLAG_DRAW_BORDER
                      | OC_UI_FLAG_DRAW_TEXT
                      | OC_UI_FLAG_HOT_ANIMATION
                      | OC_UI_FLAG_ACTIVE_ANIMATION;

    oc_ui_box* box = oc_ui_box_make_str8(label, flags);
    oc_ui_tag_box(box, "button");

    oc_ui_sig sig = oc_ui_button_behavior(box);
    return (sig);
}

oc_ui_sig oc_ui_button(const char* label)
{
    return (oc_ui_button_str8(OC_STR8((char*)label)));
}

void oc_ui_checkbox_draw(oc_ui_box* box, void* data)
{
    bool checked = *(bool*)data;
    if(checked)
    {
        oc_move_to(box->rect.x + 0.2 * box->rect.w, box->rect.y + 0.5 * box->rect.h);
        oc_line_to(box->rect.x + 0.4 * box->rect.w, box->rect.y + 0.75 * box->rect.h);
        oc_line_to(box->rect.x + 0.8 * box->rect.w, box->rect.y + 0.2 * box->rect.h);

        oc_set_color(box->style.color);
        oc_set_width(0.2 * box->rect.w);
        oc_set_joint(OC_JOINT_MITER);
        oc_set_max_joint_excursion(0.2 * box->rect.h);
        oc_stroke();
    }
}

oc_ui_sig oc_ui_checkbox(const char* name, bool* checked)
{
    oc_ui_context* ui = oc_ui_get_context();

    oc_ui_style defaultStyle = { .size.width = { OC_UI_SIZE_PIXELS, 20 },
                                 .size.height = { OC_UI_SIZE_PIXELS, 20 },
                                 .bgColor = { 1, 1, 1, 1 },
                                 .color = { 0, 0, 0, 1 },
                                 .borderColor = { 0.2, 0.2, 0.2, 1 },
                                 .borderSize = 1,
                                 .roundness = 5 };

    oc_ui_style_mask defaultMask = OC_UI_STYLE_SIZE_WIDTH
                                 | OC_UI_STYLE_SIZE_HEIGHT
                                 | OC_UI_STYLE_BG_COLOR
                                 | OC_UI_STYLE_COLOR
                                 | OC_UI_STYLE_BORDER_COLOR
                                 | OC_UI_STYLE_BORDER_SIZE
                                 | OC_UI_STYLE_ROUNDNESS;

    oc_ui_style_next(&defaultStyle, defaultMask);

    oc_ui_style activeStyle = { .bgColor = { 0.5, 0.5, 0.5, 1 },
                                .borderColor = { 0.2, 0.2, 0.2, 1 },
                                .borderSize = 2 };
    oc_ui_style_mask activeMask = OC_UI_STYLE_BG_COLOR
                                | OC_UI_STYLE_BORDER_COLOR
                                | OC_UI_STYLE_BORDER_SIZE;
    oc_ui_pattern activePattern = { 0 };
    oc_ui_pattern_push(&ui->frameArena,
                       &activePattern,
                       (oc_ui_selector){ .kind = OC_UI_SEL_STATUS,
                                         .status = OC_UI_ACTIVE | OC_UI_HOVER });
    oc_ui_style_match_before(activePattern, &activeStyle, activeMask);

    oc_ui_flags flags = OC_UI_FLAG_CLICKABLE
                      | OC_UI_FLAG_CLIP
                      | OC_UI_FLAG_DRAW_BACKGROUND
                      | OC_UI_FLAG_DRAW_PROC
                      | OC_UI_FLAG_DRAW_BORDER
                      | OC_UI_FLAG_HOT_ANIMATION
                      | OC_UI_FLAG_ACTIVE_ANIMATION;

    oc_ui_box* box = oc_ui_box_make(name, flags);
    oc_ui_tag_box(box, "checkbox");

    oc_ui_sig sig = oc_ui_button_behavior(box);
    if(sig.clicked)
    {
        *checked = !*checked;
    }
    oc_ui_box_set_draw_proc(box, oc_ui_checkbox_draw, checked);

    return (sig);
}

//------------------------------------------------------------------------------
// slider / scrollbar
//------------------------------------------------------------------------------
oc_ui_box* oc_ui_slider(const char* label, f32 thumbRatio, f32* scrollValue)
{
    oc_ui_style_match_before(oc_ui_pattern_all(), &(oc_ui_style){ 0 }, OC_UI_STYLE_LAYOUT);
    oc_ui_box* frame = oc_ui_box_begin(label, 0);
    {
        f32 beforeRatio = (*scrollValue) * (1. - thumbRatio);
        f32 afterRatio = (1. - *scrollValue) * (1. - thumbRatio);

        oc_ui_axis trackAxis = (frame->rect.w > frame->rect.h) ? OC_UI_AXIS_X : OC_UI_AXIS_Y;
        oc_ui_axis secondAxis = (trackAxis == OC_UI_AXIS_Y) ? OC_UI_AXIS_X : OC_UI_AXIS_Y;
        f32 roundness = 0.5 * frame->rect.c[2 + secondAxis];
        f32 animationTime = 0.5;

        oc_ui_style trackStyle = { .size.width = { OC_UI_SIZE_PARENT, 1 },
                                   .size.height = { OC_UI_SIZE_PARENT, 1 },
                                   .layout.axis = trackAxis,
                                   .layout.align.x = OC_UI_ALIGN_START,
                                   .layout.align.y = OC_UI_ALIGN_START,
                                   .bgColor = { 0.5, 0.5, 0.5, 1 },
                                   .roundness = roundness };

        oc_ui_style beforeStyle = trackStyle;
        beforeStyle.size.c[trackAxis] = (oc_ui_size){ OC_UI_SIZE_PARENT, beforeRatio };

        oc_ui_style afterStyle = trackStyle;
        afterStyle.size.c[trackAxis] = (oc_ui_size){ OC_UI_SIZE_PARENT, afterRatio };

        oc_ui_style thumbStyle = trackStyle;
        thumbStyle.size.c[trackAxis] = (oc_ui_size){ OC_UI_SIZE_PARENT, thumbRatio };
        thumbStyle.bgColor = (oc_color){ 0.3, 0.3, 0.3, 1 };

        oc_ui_style_mask styleMask = OC_UI_STYLE_SIZE_WIDTH
                                   | OC_UI_STYLE_SIZE_HEIGHT
                                   | OC_UI_STYLE_LAYOUT
                                   | OC_UI_STYLE_BG_COLOR
                                   | OC_UI_STYLE_ROUNDNESS;

        oc_ui_flags trackFlags = OC_UI_FLAG_CLIP
                               | OC_UI_FLAG_DRAW_BACKGROUND
                               | OC_UI_FLAG_HOT_ANIMATION
                               | OC_UI_FLAG_ACTIVE_ANIMATION;

        oc_ui_style_next(&trackStyle, styleMask);
        oc_ui_box* track = oc_ui_box_begin("track", trackFlags);

        oc_ui_style_next(&beforeStyle, OC_UI_STYLE_SIZE_WIDTH | OC_UI_STYLE_SIZE_HEIGHT);
        oc_ui_box* beforeSpacer = oc_ui_box_make("before", 0);

        oc_ui_flags thumbFlags = OC_UI_FLAG_CLICKABLE
                               | OC_UI_FLAG_DRAW_BACKGROUND
                               | OC_UI_FLAG_HOT_ANIMATION
                               | OC_UI_FLAG_ACTIVE_ANIMATION;

        oc_ui_style_next(&thumbStyle, styleMask);
        oc_ui_box* thumb = oc_ui_box_make("thumb", thumbFlags);

        oc_ui_style_next(&afterStyle, OC_UI_STYLE_SIZE_WIDTH | OC_UI_STYLE_SIZE_HEIGHT);
        oc_ui_box* afterSpacer = oc_ui_box_make("after", 0);

        oc_ui_box_end();

        //NOTE: interaction
        oc_ui_sig thumbSig = oc_ui_box_sig(thumb);
        oc_ui_sig trackSig = oc_ui_box_sig(track);
        if(thumbSig.dragging)
        {
            f32 trackExtents = track->rect.c[2 + trackAxis] - thumb->rect.c[2 + trackAxis];
            *scrollValue = (trackSig.mouse.c[trackAxis] - thumb->pressedMouse.c[trackAxis]) / trackExtents;
            *scrollValue = oc_clamp(*scrollValue, 0, 1);
        }

        if(oc_ui_box_active(frame))
        {
            //NOTE: activated from outside
            oc_ui_box_set_hot(track, true);
            oc_ui_box_set_hot(thumb, true);
            oc_ui_box_activate(track);
            oc_ui_box_activate(thumb);
        }

        if(trackSig.hovering)
        {
            oc_ui_box_set_hot(track, true);
            oc_ui_box_set_hot(thumb, true);
        }
        else if(thumbSig.wheel.c[trackAxis] == 0)
        {
            oc_ui_box_set_hot(track, false);
            oc_ui_box_set_hot(thumb, false);
        }

        if(thumbSig.dragging)
        {
            oc_ui_box_activate(track);
            oc_ui_box_activate(thumb);
        }
        else if(thumbSig.wheel.c[trackAxis] == 0)
        {
            oc_ui_box_deactivate(track);
            oc_ui_box_deactivate(thumb);
            oc_ui_box_deactivate(frame);
        }
    }
    oc_ui_box_end();

    return (frame);
}

//------------------------------------------------------------------------------
// panels
//------------------------------------------------------------------------------
void oc_ui_panel_begin(const char* str, oc_ui_flags flags)
{
    flags = flags
          | OC_UI_FLAG_CLIP
          | OC_UI_FLAG_BLOCK_MOUSE
          | OC_UI_FLAG_ALLOW_OVERFLOW_X
          | OC_UI_FLAG_ALLOW_OVERFLOW_Y
          | OC_UI_FLAG_SCROLL_WHEEL_X
          | OC_UI_FLAG_SCROLL_WHEEL_Y;

    oc_ui_box_begin(str, flags);
}

void oc_ui_panel_end(void)
{
    oc_ui_box* panel = oc_ui_box_top();
    oc_ui_sig sig = oc_ui_box_sig(panel);

    f32 contentsW = oc_clamp_low(panel->childrenSum[0], panel->rect.w);
    f32 contentsH = oc_clamp_low(panel->childrenSum[1], panel->rect.h);

    contentsW = oc_clamp_low(contentsW, 1);
    contentsH = oc_clamp_low(contentsH, 1);

    oc_ui_box* scrollBarX = 0;
    oc_ui_box* scrollBarY = 0;

    bool needsScrollX = contentsW > panel->rect.w;
    bool needsScrollY = contentsH > panel->rect.h;

    if(needsScrollX)
    {
        f32 thumbRatioX = panel->rect.w / contentsW;
        f32 sliderX = panel->scroll.x / (contentsW - panel->rect.w);

        oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1., 0 },
                                         .size.height = { OC_UI_SIZE_PIXELS, 10, 0 },
                                         .floating.x = true,
                                         .floating.y = true,
                                         .floatTarget = { 0, panel->rect.h - 10 } },
                         OC_UI_STYLE_SIZE
                             | OC_UI_STYLE_FLOAT);

        scrollBarX = oc_ui_slider("scrollerX", thumbRatioX, &sliderX);

        panel->scroll.x = sliderX * (contentsW - panel->rect.w);
        if(sig.hovering)
        {
            oc_ui_box_activate(scrollBarX);
        }
    }

    if(needsScrollY)
    {
        f32 thumbRatioY = panel->rect.h / contentsH;
        f32 sliderY = panel->scroll.y / (contentsH - panel->rect.h);

        f32 spacerSize = needsScrollX ? 10 : 0;

        oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, 10, 0 },
                                         .size.height = { OC_UI_SIZE_PARENT_MINUS_PIXELS, spacerSize, 0 },
                                         .floating.x = true,
                                         .floating.y = true,
                                         .floatTarget = { panel->rect.w - 10, 0 } },
                         OC_UI_STYLE_SIZE
                             | OC_UI_STYLE_FLOAT);

        scrollBarY = oc_ui_slider("scrollerY", thumbRatioY, &sliderY);

        panel->scroll.y = sliderY * (contentsH - panel->rect.h);
        if(sig.hovering)
        {
            oc_ui_box_activate(scrollBarY);
        }
    }
    panel->scroll.x = oc_clamp(panel->scroll.x, 0, contentsW - panel->rect.w);
    panel->scroll.y = oc_clamp(panel->scroll.y, 0, contentsH - panel->rect.h);

    oc_ui_box_end();
}

//------------------------------------------------------------------------------
// tooltips
//------------------------------------------------------------------------------

oc_ui_sig oc_ui_tooltip_begin(const char* name)
{
    oc_ui_context* ui = oc_ui_get_context();

    oc_vec2 p = oc_ui_mouse_position();

    oc_ui_style style = { .size.width = { OC_UI_SIZE_CHILDREN },
                          .size.height = { OC_UI_SIZE_CHILDREN },
                          .floating.x = true,
                          .floating.y = true,
                          .floatTarget = { p.x, p.y } };
    oc_ui_style_mask mask = OC_UI_STYLE_SIZE | OC_UI_STYLE_FLOAT;

    oc_ui_style_next(&style, mask);

    oc_ui_flags flags = OC_UI_FLAG_OVERLAY
                      | OC_UI_FLAG_DRAW_BACKGROUND
                      | OC_UI_FLAG_DRAW_BORDER;

    oc_ui_box* tooltip = oc_ui_box_make(name, flags);
    oc_ui_box_push(tooltip);

    return (oc_ui_box_sig(tooltip));
}

void oc_ui_tooltip_end(void)
{
    oc_ui_box_pop(); // tooltip
}

//------------------------------------------------------------------------------
// Menus
//------------------------------------------------------------------------------

void oc_ui_menu_bar_begin(const char* name)
{
    oc_ui_style style = {
        .size.width = { OC_UI_SIZE_PARENT, 1, 0 },
        .size.height = { OC_UI_SIZE_CHILDREN },
        .layout.axis = OC_UI_AXIS_X,
        .layout.spacing = 20,
    };
    oc_ui_style_mask mask = OC_UI_STYLE_SIZE
                          | OC_UI_STYLE_LAYOUT_AXIS
                          | OC_UI_STYLE_LAYOUT_SPACING;

    oc_ui_style_next(&style, mask);
    oc_ui_box* bar = oc_ui_box_begin(name, OC_UI_FLAG_DRAW_BACKGROUND);

    oc_ui_sig sig = oc_ui_box_sig(bar);
    oc_ui_context* ui = oc_ui_get_context();
    if(!sig.hovering && oc_mouse_released(&ui->input, OC_MOUSE_LEFT))
    {
        oc_ui_box_deactivate(bar);
    }
}

void oc_ui_menu_bar_end(void)
{
    oc_ui_box_end(); // menu bar
}

void oc_ui_menu_begin(const char* label)
{
    oc_ui_box* container = oc_ui_box_make(label, 0);
    oc_ui_box_push(container);

    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_TEXT },
                                     .size.height = { OC_UI_SIZE_TEXT } },
                     OC_UI_STYLE_SIZE);

    oc_ui_box* button = oc_ui_box_make(label, OC_UI_FLAG_CLICKABLE | OC_UI_FLAG_DRAW_TEXT);
    oc_ui_box* bar = container->parent;

    oc_ui_sig sig = oc_ui_box_sig(button);
    oc_ui_sig barSig = oc_ui_box_sig(bar);

    oc_ui_context* ui = oc_ui_get_context();

    oc_ui_style style = { .size.width = { OC_UI_SIZE_CHILDREN },
                          .size.height = { OC_UI_SIZE_CHILDREN },
                          .floating.x = true,
                          .floating.y = true,
                          .floatTarget = { button->rect.x,
                                           button->rect.y + button->rect.h },
                          .layout.axis = OC_UI_AXIS_Y,
                          .layout.spacing = 5,
                          .layout.margin.x = 0,
                          .layout.margin.y = 5,
                          .bgColor = { 0.2, 0.2, 0.2, 1 } };

    oc_ui_style_mask mask = OC_UI_STYLE_SIZE
                          | OC_UI_STYLE_FLOAT
                          | OC_UI_STYLE_LAYOUT
                          | OC_UI_STYLE_BG_COLOR;

    oc_ui_flags flags = OC_UI_FLAG_OVERLAY
                      | OC_UI_FLAG_DRAW_BACKGROUND
                      | OC_UI_FLAG_DRAW_BORDER;

    oc_ui_style_next(&style, mask);
    oc_ui_box* menu = oc_ui_box_make("panel", flags);

    if(oc_ui_box_active(bar))
    {
        if(sig.hovering)
        {
            oc_ui_box_activate(button);
        }
        else if(barSig.hovering)
        {
            oc_ui_box_deactivate(button);
        }
    }
    else
    {
        oc_ui_box_deactivate(button);
        if(sig.pressed)
        {
            oc_ui_box_activate(bar);
            oc_ui_box_activate(button);
        }
    }

    oc_ui_box_set_closed(menu, !oc_ui_box_active(button));
    oc_ui_box_push(menu);
}

void oc_ui_menu_end(void)
{
    oc_ui_box_pop(); // menu
    oc_ui_box_pop(); // container
}

oc_ui_sig oc_ui_menu_button(const char* name)
{
    oc_ui_context* ui = oc_ui_get_context();

    oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_TEXT },
                                     .size.height = { OC_UI_SIZE_TEXT },
                                     .layout.margin.x = 5,
                                     .bgColor = { 0, 0, 0, 0 } },
                     OC_UI_STYLE_SIZE
                         | OC_UI_STYLE_LAYOUT_MARGIN_X
                         | OC_UI_STYLE_BG_COLOR);

    oc_ui_pattern pattern = { 0 };
    oc_ui_pattern_push(&ui->frameArena, &pattern, (oc_ui_selector){ .kind = OC_UI_SEL_STATUS, .status = OC_UI_HOVER });

    oc_ui_style style = { .bgColor = { 0, 0, 1, 1 } };
    oc_ui_style_mask mask = OC_UI_STYLE_BG_COLOR;
    oc_ui_style_match_before(pattern, &style, mask);

    oc_ui_flags flags = OC_UI_FLAG_CLICKABLE
                      | OC_UI_FLAG_CLIP
                      | OC_UI_FLAG_DRAW_TEXT
                      | OC_UI_FLAG_DRAW_BACKGROUND;

    oc_ui_box* box = oc_ui_box_make(name, flags);
    oc_ui_sig sig = oc_ui_box_sig(box);
    return (sig);
}

void oc_ui_select_popup_draw_arrow(oc_ui_box* box, void* data)
{
    f32 r = oc_min(box->parent->style.roundness, box->rect.w);
    f32 cr = r * 4 * (sqrt(2) - 1) / 3;

    oc_move_to(box->rect.x, box->rect.y);
    oc_line_to(box->rect.x + box->rect.w - r, box->rect.y);
    oc_cubic_to(box->rect.x + box->rect.w - cr, box->rect.y,
                box->rect.x + box->rect.w, box->rect.y + cr,
                box->rect.x + box->rect.w, box->rect.y + r);
    oc_line_to(box->rect.x + box->rect.w, box->rect.y + box->rect.h - r);
    oc_cubic_to(box->rect.x + box->rect.w, box->rect.y + box->rect.h - cr,
                box->rect.x + box->rect.w - cr, box->rect.y + box->rect.h,
                box->rect.x + box->rect.w - r, box->rect.y + box->rect.h);
    oc_line_to(box->rect.x, box->rect.y + box->rect.h);

    oc_set_color(box->style.bgColor);
    oc_fill();

    oc_move_to(box->rect.x + 0.25 * box->rect.w, box->rect.y + 0.45 * box->rect.h);
    oc_line_to(box->rect.x + 0.5 * box->rect.w, box->rect.y + 0.75 * box->rect.h);
    oc_line_to(box->rect.x + 0.75 * box->rect.w, box->rect.y + 0.45 * box->rect.h);
    oc_close_path();

    oc_set_color(box->style.color);
    oc_fill();
}

oc_ui_select_popup_info oc_ui_select_popup(const char* name, oc_ui_select_popup_info* info)
{
    oc_ui_select_popup_info result = *info;

    oc_ui_context* ui = oc_ui_get_context();

    oc_ui_container(name, 0)
    {
        oc_ui_box* button = oc_ui_box_make("button",
                                           OC_UI_FLAG_CLICKABLE
                                               | OC_UI_FLAG_DRAW_BACKGROUND
                                               | OC_UI_FLAG_DRAW_BORDER
                                               | OC_UI_FLAG_ALLOW_OVERFLOW_X
                                               | OC_UI_FLAG_CLIP);

        f32 maxOptionWidth = 0;
        f32 lineHeight = 0;
        oc_rect bbox = { 0 };
        for(int i = 0; i < info->optionCount; i++)
        {
            bbox = oc_text_bounding_box(button->style.font, button->style.fontSize, info->options[i]);
            maxOptionWidth = oc_max(maxOptionWidth, bbox.w);
        }
        f32 buttonWidth = maxOptionWidth + 2 * button->style.layout.margin.x + button->rect.h;

        oc_ui_style_box_before(button,
                               oc_ui_pattern_owner(),
                               &(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, buttonWidth },
                                               .size.height = { OC_UI_SIZE_CHILDREN },
                                               .layout.margin.x = 5,
                                               .layout.margin.y = 1,
                                               .roundness = 5,
                                               .borderSize = 1,
                                               .borderColor = { 0.3, 0.3, 0.3, 1 } },
                               OC_UI_STYLE_SIZE
                                   | OC_UI_STYLE_LAYOUT_MARGIN_X
                                   | OC_UI_STYLE_LAYOUT_MARGIN_Y
                                   | OC_UI_STYLE_ROUNDNESS
                                   | OC_UI_STYLE_BORDER_SIZE
                                   | OC_UI_STYLE_BORDER_COLOR);
        oc_ui_box_push(button);
        {
            oc_ui_label_str8(info->options[info->selectedIndex]);

            oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, button->rect.h },
                                             .size.height = { OC_UI_SIZE_PIXELS, button->rect.h },
                                             .floating.x = true,
                                             .floating.y = true,
                                             .floatTarget = { button->rect.w - button->rect.h, 0 },
                                             .color = { 0, 0, 0, 1 },
                                             .bgColor = { 0.7, 0.7, 0.7, 1 } },
                             OC_UI_STYLE_SIZE
                                 | OC_UI_STYLE_FLOAT
                                 | OC_UI_STYLE_COLOR
                                 | OC_UI_STYLE_BG_COLOR);

            oc_ui_box* arrow = oc_ui_box_make("arrow", OC_UI_FLAG_DRAW_PROC);
            oc_ui_box_set_draw_proc(arrow, oc_ui_select_popup_draw_arrow, 0);
        }
        oc_ui_box_pop();

        //panel
        oc_ui_box* panel = oc_ui_box_make("panel",
                                          OC_UI_FLAG_DRAW_BACKGROUND
                                              | OC_UI_FLAG_BLOCK_MOUSE
                                              | OC_UI_FLAG_OVERLAY);

        //TODO: set width to max(button.w, max child...)
        f32 containerWidth = oc_max(maxOptionWidth + 2 * panel->style.layout.margin.x,
                                    button->rect.w);

        oc_ui_style_box_before(panel,
                               oc_ui_pattern_owner(),
                               &(oc_ui_style){ .size.width = { OC_UI_SIZE_PIXELS, containerWidth },
                                               .size.height = { OC_UI_SIZE_CHILDREN },
                                               .floating.x = true,
                                               .floating.y = true,
                                               .floatTarget = { button->rect.x,
                                                                button->rect.y + button->rect.h },
                                               .layout.axis = OC_UI_AXIS_Y,
                                               .layout.margin.x = 0,
                                               .layout.margin.y = 5,
                                               .bgColor = { 0.2, 0.2, 0.2, 1 } },
                               OC_UI_STYLE_SIZE
                                   | OC_UI_STYLE_FLOAT
                                   | OC_UI_STYLE_LAYOUT
                                   | OC_UI_STYLE_BG_COLOR);

        oc_ui_box_push(panel);
        {
            for(int i = 0; i < info->optionCount; i++)
            {
                oc_ui_style_next(&(oc_ui_style){ .size.width = { OC_UI_SIZE_PARENT, 1 },
                                                 .size.height = { OC_UI_SIZE_TEXT },
                                                 .layout.axis = OC_UI_AXIS_Y,
                                                 .layout.align.x = OC_UI_ALIGN_START,
                                                 .layout.margin.x = 5,
                                                 .layout.margin.y = 2.5 },
                                 OC_UI_STYLE_SIZE
                                     | OC_UI_STYLE_LAYOUT_AXIS
                                     | OC_UI_STYLE_LAYOUT_ALIGN_X
                                     | OC_UI_STYLE_LAYOUT_MARGIN_X
                                     | OC_UI_STYLE_LAYOUT_MARGIN_Y);

                oc_ui_pattern pattern = { 0 };
                oc_ui_pattern_push(&ui->frameArena, &pattern, (oc_ui_selector){ .kind = OC_UI_SEL_STATUS, .status = OC_UI_HOVER });
                oc_ui_style_match_before(pattern, &(oc_ui_style){ .bgColor = { 0, 0, 1, 1 } }, OC_UI_STYLE_BG_COLOR);

                oc_ui_box* box = oc_ui_box_make_str8(info->options[i],
                                                     OC_UI_FLAG_DRAW_TEXT
                                                         | OC_UI_FLAG_CLICKABLE
                                                         | OC_UI_FLAG_DRAW_BACKGROUND);
                oc_ui_sig sig = oc_ui_box_sig(box);
                if(sig.pressed)
                {
                    result.selectedIndex = i;
                }
            }
        }
        oc_ui_box_pop();

        oc_ui_context* ui = oc_ui_get_context();
        if(oc_ui_box_active(panel) && oc_mouse_pressed(&ui->input, OC_MOUSE_LEFT))
        {
            oc_ui_box_deactivate(panel);
        }
        else if(oc_ui_box_sig(button).pressed)
        {
            oc_ui_box_activate(panel);
        }
        oc_ui_box_set_closed(panel, !oc_ui_box_active(panel));
    }
    return (result);
}

//------------------------------------------------------------------------------
// text box
//------------------------------------------------------------------------------
oc_str32 oc_ui_edit_replace_selection_with_codepoints(oc_ui_context* ui, oc_str32 codepoints, oc_str32 input)
{
    u32 start = oc_min(ui->editCursor, ui->editMark);
    u32 end = oc_max(ui->editCursor, ui->editMark);

    oc_str32 before = oc_str32_slice(codepoints, 0, start);
    oc_str32 after = oc_str32_slice(codepoints, end, codepoints.len);

    oc_str32_list list = { 0 };
    oc_str32_list_push(&ui->frameArena, &list, before);
    oc_str32_list_push(&ui->frameArena, &list, input);
    oc_str32_list_push(&ui->frameArena, &list, after);

    codepoints = oc_str32_list_join(&ui->frameArena, list);

    ui->editCursor = start + input.len;
    ui->editMark = ui->editCursor;
    return (codepoints);
}

oc_str32 oc_ui_edit_delete_selection(oc_ui_context* ui, oc_str32 codepoints)
{
    return (oc_ui_edit_replace_selection_with_codepoints(ui, codepoints, (oc_str32){ 0 }));
}

void oc_ui_edit_copy_selection_to_clipboard(oc_ui_context* ui, oc_str32 codepoints)
{
#if !OC_PLATFORM_ORCA
    if(ui->editCursor == ui->editMark)
    {
        return;
    }
    u32 start = oc_min(ui->editCursor, ui->editMark);
    u32 end = oc_max(ui->editCursor, ui->editMark);
    oc_str32 selection = oc_str32_slice(codepoints, start, end);
    oc_str8 string = oc_utf8_push_from_codepoints(&ui->frameArena, selection);

    oc_clipboard_clear();
    oc_clipboard_set_string(string);
#endif
}

oc_str32 oc_ui_edit_replace_selection_with_clipboard(oc_ui_context* ui, oc_str32 codepoints)
{
#if OC_PLATFORM_ORCA
    oc_str32 result = { 0 };
#else
    oc_str8 string = oc_clipboard_get_string(&ui->frameArena);
    oc_str32 input = oc_utf8_push_to_codepoints(&ui->frameArena, string);
    oc_str32 result = oc_ui_edit_replace_selection_with_codepoints(ui, codepoints, input);
#endif
    return (result);
}

typedef enum
{
    OC_UI_EDIT_MOVE,
    OC_UI_EDIT_SELECT,
    OC_UI_EDIT_SELECT_EXTEND,
    OC_UI_EDIT_DELETE,
    OC_UI_EDIT_CUT,
    OC_UI_EDIT_COPY,
    OC_UI_EDIT_PASTE,
    OC_UI_EDIT_SELECT_ALL
} oc_ui_edit_op;

typedef enum
{
    OC_UI_EDIT_MOVE_NONE = 0,
    OC_UI_EDIT_MOVE_ONE,
    OC_UI_EDIT_MOVE_WORD,
    OC_UI_EDIT_MOVE_LINE
} oc_ui_edit_move;

typedef struct oc_ui_edit_command
{
    oc_key_code key;
    oc_keymod_flags mods;

    oc_ui_edit_op operation;
    oc_ui_edit_move move;
    int direction;

} oc_ui_edit_command;

const oc_ui_edit_command OC_UI_EDIT_COMMANDS_MACOS[] = {
    //NOTE(martin): move one left
    {
        .key = OC_KEY_LEFT,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_ONE,
        .direction = -1 },
    //NOTE(martin): move one right
    {
        .key = OC_KEY_RIGHT,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_ONE,
        .direction = 1 },
    //NOTE(martin): move one word left
    {
        .key = OC_KEY_LEFT,
        .mods = OC_KEYMOD_ALT,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = -1 },
    //NOTE(martin): move one word right
    {
        .key = OC_KEY_RIGHT,
        .mods = OC_KEYMOD_ALT,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = 1 },
    //NOTE(martin): move start
    {
        .key = OC_KEY_Q,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_LINE,
        .direction = -1 },
    { .key = OC_KEY_UP,
      .operation = OC_UI_EDIT_MOVE,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = -1 },
    //NOTE(martin): move end
    {
        .key = OC_KEY_E,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_LINE,
        .direction = 1 },
    { .key = OC_KEY_DOWN,
      .operation = OC_UI_EDIT_MOVE,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = 1 },
    //NOTE(martin): select one left
    {
        .key = OC_KEY_LEFT,
        .mods = OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT,
        .move = OC_UI_EDIT_MOVE_ONE,
        .direction = -1 },
    //NOTE(martin): select one right
    {
        .key = OC_KEY_RIGHT,
        .mods = OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT,
        .move = OC_UI_EDIT_MOVE_ONE,
        .direction = 1 },
    //NOTE(martin): select one word left
    {
        .key = OC_KEY_LEFT,
        .mods = OC_KEYMOD_ALT | OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = -1 },
    //NOTE(martin): select one word right
    {
        .key = OC_KEY_RIGHT,
        .mods = OC_KEYMOD_ALT | OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = 1 },
    //NOTE(martin): extend select to start
    {
        .key = OC_KEY_Q,
        .mods = OC_KEYMOD_CTRL | OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT_EXTEND,
        .move = OC_UI_EDIT_MOVE_LINE,
        .direction = -1 },
    { .key = OC_KEY_UP,
      .mods = OC_KEYMOD_SHIFT,
      .operation = OC_UI_EDIT_SELECT_EXTEND,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = -1 },
    //NOTE(martin): extend select to end
    {
        .key = OC_KEY_E,
        .mods = OC_KEYMOD_CTRL | OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT_EXTEND,
        .move = OC_UI_EDIT_MOVE_LINE,
        .direction = 1 },
    { .key = OC_KEY_DOWN,
      .mods = OC_KEYMOD_SHIFT,
      .operation = OC_UI_EDIT_SELECT_EXTEND,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = 1 },
    //NOTE(martin): select all
    {
        .key = OC_KEY_Q,
        .mods = OC_KEYMOD_CMD,
        .operation = OC_UI_EDIT_SELECT_ALL,
        .move = OC_UI_EDIT_MOVE_NONE },
    //NOTE(martin): delete
    {
        .key = OC_KEY_DELETE,
        .operation = OC_UI_EDIT_DELETE,
        .move = OC_UI_EDIT_MOVE_ONE,
        .direction = 1 },
    //NOTE(martin): delete word
    {
        .key = OC_KEY_DELETE,
        .mods = OC_KEYMOD_ALT,
        .operation = OC_UI_EDIT_DELETE,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = 1 },
    //NOTE(martin): backspace
    {
        .key = OC_KEY_BACKSPACE,
        .operation = OC_UI_EDIT_DELETE,
        .move = OC_UI_EDIT_MOVE_ONE,
        .direction = -1 },
    //NOTE(martin): backspace word
    {
        .key = OC_KEY_BACKSPACE,
        .mods = OC_KEYMOD_ALT,
        .operation = OC_UI_EDIT_DELETE,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = -1 },
    //NOTE(martin): cut
    {
        .key = OC_KEY_X,
        .mods = OC_KEYMOD_CMD,
        .operation = OC_UI_EDIT_CUT,
        .move = OC_UI_EDIT_MOVE_NONE },
    //NOTE(martin): copy
    {
        .key = OC_KEY_C,
        .mods = OC_KEYMOD_CMD,
        .operation = OC_UI_EDIT_COPY,
        .move = OC_UI_EDIT_MOVE_NONE },
    //NOTE(martin): paste
    {
        .key = OC_KEY_V,
        .mods = OC_KEYMOD_CMD,
        .operation = OC_UI_EDIT_PASTE,
        .move = OC_UI_EDIT_MOVE_NONE }
};

const oc_ui_edit_command OC_UI_EDIT_COMMANDS_WINDOWS[] = {
    //NOTE(martin): move one left
    {
        .key = OC_KEY_LEFT,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_ONE,
        .direction = -1 },
    //NOTE(martin): move one right
    {
        .key = OC_KEY_RIGHT,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_ONE,
        .direction = 1 },
    //NOTE(martin): move one word left
    {
        .key = OC_KEY_LEFT,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = -1 },
    //NOTE(martin): move one word right
    {
        .key = OC_KEY_RIGHT,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = 1 },
    //NOTE(martin): move start
    {
        .key = OC_KEY_HOME,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_LINE,
        .direction = -1 },
    { .key = OC_KEY_UP,
      .operation = OC_UI_EDIT_MOVE,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = -1 },
    //NOTE(martin): move end
    {
        .key = OC_KEY_END,
        .operation = OC_UI_EDIT_MOVE,
        .move = OC_UI_EDIT_MOVE_LINE,
        .direction = 1 },
    { .key = OC_KEY_DOWN,
      .operation = OC_UI_EDIT_MOVE,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = 1 },
    //NOTE(martin): select one left
    {
        .key = OC_KEY_LEFT,
        .mods = OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT,
        .move = OC_UI_EDIT_MOVE_ONE,
        .direction = -1 },
    //NOTE(martin): select one right
    {
        .key = OC_KEY_RIGHT,
        .mods = OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT,
        .move = OC_UI_EDIT_MOVE_ONE,
        .direction = 1 },
    //NOTE(martin): select one word left
    {
        .key = OC_KEY_LEFT,
        .mods = OC_KEYMOD_CTRL | OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = -1 },
    //NOTE(martin): select one word right
    {
        .key = OC_KEY_RIGHT,
        .mods = OC_KEYMOD_CTRL | OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = 1 },
    //NOTE(martin): extend select to start
    {
        .key = OC_KEY_HOME,
        .mods = OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT_EXTEND,
        .move = OC_UI_EDIT_MOVE_LINE,
        .direction = -1 },
    { .key = OC_KEY_UP,
      .mods = OC_KEYMOD_SHIFT,
      .operation = OC_UI_EDIT_SELECT_EXTEND,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = -1 },
    //NOTE(martin): extend select to end
    {
        .key = OC_KEY_END,
        .mods = OC_KEYMOD_SHIFT,
        .operation = OC_UI_EDIT_SELECT_EXTEND,
        .move = OC_UI_EDIT_MOVE_LINE,
        .direction = 1 },
    { .key = OC_KEY_DOWN,
      .mods = OC_KEYMOD_SHIFT,
      .operation = OC_UI_EDIT_SELECT_EXTEND,
      .move = OC_UI_EDIT_MOVE_LINE,
      .direction = 1 },
    //NOTE(martin): select all
    {
        .key = OC_KEY_A,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_SELECT_ALL,
        .move = OC_UI_EDIT_MOVE_NONE },
    //NOTE(martin): delete
    {
        .key = OC_KEY_DELETE,
        .operation = OC_UI_EDIT_DELETE,
        .move = OC_UI_EDIT_MOVE_ONE,
        .direction = 1 },
    //NOTE(martin): delete word
    {
        .key = OC_KEY_DELETE,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_DELETE,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = 1 },
    //NOTE(martin): backspace
    {
        .key = OC_KEY_BACKSPACE,
        .operation = OC_UI_EDIT_DELETE,
        .move = OC_UI_EDIT_MOVE_ONE,
        .direction = -1 },
    //NOTE(martin): backspace word
    {
        .key = OC_KEY_BACKSPACE,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_DELETE,
        .move = OC_UI_EDIT_MOVE_WORD,
        .direction = -1 },
    //NOTE(martin): cut
    {
        .key = OC_KEY_X,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_CUT,
        .move = OC_UI_EDIT_MOVE_NONE },
    //NOTE(martin): copy
    {
        .key = OC_KEY_C,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_COPY,
        .move = OC_UI_EDIT_MOVE_NONE },
    //NOTE(martin): paste
    {
        .key = OC_KEY_V,
        .mods = OC_KEYMOD_CTRL,
        .operation = OC_UI_EDIT_PASTE,
        .move = OC_UI_EDIT_MOVE_NONE }
};

const u32 OC_UI_EDIT_COMMAND_MACOS_COUNT = sizeof(OC_UI_EDIT_COMMANDS_MACOS) / sizeof(oc_ui_edit_command);
const u32 OC_UI_EDIT_COMMAND_WINDOWS_COUNT = sizeof(OC_UI_EDIT_COMMANDS_WINDOWS) / sizeof(oc_ui_edit_command);

bool oc_ui_edit_is_word_separator(u32 codepoint)
{
    //NOTE(ilia): Printable ascii character, except for alphanumeric and _
    return ('!' <= codepoint && codepoint <= '~'
            && !('0' <= codepoint && codepoint <= '9')
            && !('A' <= codepoint && codepoint <= 'Z')
            && !('a' <= codepoint && codepoint <= 'z')
            && codepoint != '_');
}

bool oc_ui_edit_is_whitespace(u32 codepoint)
{
    return (codepoint == ' ' || (0x09 <= codepoint && codepoint <= 0x0d) || codepoint == 0x85 || codepoint == 0xa0
            || codepoint == 0x1680 || (0x2000 <= codepoint && codepoint <= 0x200a) || codepoint == 0x202f || codepoint == 0x205f || codepoint == 0x3000);
}

void oc_ui_edit_perform_move(oc_ui_context* ui, oc_ui_edit_move move, int direction, oc_str32 codepoints)
{
    switch(move)
    {
        case OC_UI_EDIT_MOVE_NONE:
            break;

        case OC_UI_EDIT_MOVE_ONE:
        {
            if(direction < 0 && ui->editCursor > 0)
            {
                ui->editCursor--;
            }
            else if(direction > 0 && ui->editCursor < codepoints.len)
            {
                ui->editCursor++;
            }
        }
        break;

        case OC_UI_EDIT_MOVE_LINE:
        {
            if(direction < 0)
            {
                ui->editCursor = 0;
            }
            else if(direction > 0)
            {
                ui->editCursor = codepoints.len;
            }
        }
        break;

        case OC_UI_EDIT_MOVE_WORD:
        {
            //NOTE(ilia): a simple word break algorithm borrowed from Qt
            //            https://github.com/qt/qtbase/blob/cbea2f5705c39e31600cb7fff552db92198afd34/src/gui/text/qtextlayout.cpp#L643-L714
            //            proper implementation would involve bringing in ICU or querying unicode ranges and parsing ICU rules
            if(direction < 0)
            {
                while(ui->editCursor > 0 && oc_ui_edit_is_whitespace(codepoints.ptr[ui->editCursor - 1]))
                {
                    ui->editCursor--;
                }

                if(ui->editCursor > 0 && oc_ui_edit_is_word_separator(codepoints.ptr[ui->editCursor - 1]))
                {
                    ui->editCursor--;
                    while(ui->editCursor > 0 && oc_ui_edit_is_word_separator(codepoints.ptr[ui->editCursor - 1]))
                    {
                        ui->editCursor--;
                    }
                }
                else
                {
                    while(ui->editCursor > 0
                          && !oc_ui_edit_is_whitespace(codepoints.ptr[ui->editCursor - 1])
                          && !oc_ui_edit_is_word_separator(codepoints.ptr[ui->editCursor - 1]))
                    {
                        ui->editCursor--;
                    }
                }
            }
            else if(direction > 0)
            {
                if(ui->editCursor < codepoints.len && oc_ui_edit_is_word_separator(codepoints.ptr[ui->editCursor]))
                {
                    ui->editCursor++;
                    while(ui->editCursor < codepoints.len && oc_ui_edit_is_word_separator(codepoints.ptr[ui->editCursor]))
                    {
                        ui->editCursor++;
                    }
                }
                else
                {
                    while(ui->editCursor < codepoints.len
                          && !oc_ui_edit_is_whitespace(codepoints.ptr[ui->editCursor])
                          && !oc_ui_edit_is_word_separator(codepoints.ptr[ui->editCursor]))
                    {
                        ui->editCursor++;
                    }
                }

                while(ui->editCursor < codepoints.len && oc_ui_edit_is_whitespace(codepoints.ptr[ui->editCursor]))
                {
                    ui->editCursor++;
                }
            }
            break;
        }
    }
}

oc_str32 oc_ui_edit_perform_operation(oc_ui_context* ui, oc_ui_edit_op operation, oc_ui_edit_move move, int direction, oc_str32 codepoints)
{
    switch(operation)
    {
        case OC_UI_EDIT_MOVE:
        {
            //NOTE(martin): we place the cursor on the direction-most side of the selection
            //              before performing the move
            u32 cursor = direction < 0 ? oc_min(ui->editCursor, ui->editMark) : oc_max(ui->editCursor, ui->editMark);
            ui->editCursor = cursor;

            if(ui->editCursor == ui->editMark || move != OC_UI_EDIT_MOVE_ONE)
            {
                //NOTE: we special case move-one when there is a selection
                //      (just place the cursor at begining/end of selection)
                oc_ui_edit_perform_move(ui, move, direction, codepoints);
            }
            ui->editMark = ui->editCursor;
        }
        break;

        case OC_UI_EDIT_SELECT:
        {
            oc_ui_edit_perform_move(ui, move, direction, codepoints);
        }
        break;

        case OC_UI_EDIT_SELECT_EXTEND:
        {
            if((direction > 0) != (ui->editCursor > ui->editMark))
            {
                u32 tmp = ui->editCursor;
                ui->editCursor = ui->editMark;
                ui->editMark = tmp;
            }
            oc_ui_edit_perform_move(ui, move, direction, codepoints);
        }
        break;

        case OC_UI_EDIT_DELETE:
        {
            if(ui->editCursor == ui->editMark)
            {
                oc_ui_edit_perform_move(ui, move, direction, codepoints);
            }
            codepoints = oc_ui_edit_delete_selection(ui, codepoints);
            ui->editMark = ui->editCursor;
        }
        break;

        case OC_UI_EDIT_CUT:
        {
            oc_ui_edit_copy_selection_to_clipboard(ui, codepoints);
            codepoints = oc_ui_edit_delete_selection(ui, codepoints);
        }
        break;

        case OC_UI_EDIT_COPY:
        {
            oc_ui_edit_copy_selection_to_clipboard(ui, codepoints);
        }
        break;

        case OC_UI_EDIT_PASTE:
        {
            codepoints = oc_ui_edit_replace_selection_with_clipboard(ui, codepoints);
        }
        break;

        case OC_UI_EDIT_SELECT_ALL:
        {
            ui->editCursor = 0;
            ui->editMark = codepoints.len;
        }
        break;
    }
    ui->editCursorBlinkStart = ui->frameTime;

    return (codepoints);
}

void oc_ui_text_box_render(oc_ui_box* box, void* data)
{
    oc_str32 codepoints = *(oc_str32*)data;
    oc_ui_context* ui = oc_ui_get_context();

    u32 firstDisplayedChar = 0;
    if(oc_ui_box_active(box))
    {
        firstDisplayedChar = ui->editFirstDisplayedChar;
    }

    oc_ui_style* style = &box->style;
    oc_font_extents extents = oc_font_get_scaled_extents(style->font, style->fontSize);
    f32 lineHeight = extents.ascent + extents.descent;

    oc_str32 before = oc_str32_slice(codepoints, 0, firstDisplayedChar);
    oc_rect beforeBox = oc_text_bounding_box_utf32(style->font, style->fontSize, before);

    f32 textMargin = 5; //TODO: make that configurable

    f32 textX = textMargin + box->rect.x - beforeBox.w;
    f32 textTop = box->rect.y + 0.5 * (box->rect.h - lineHeight);
    f32 textY = textTop + extents.ascent;

    if(box->active)
    {
        u32 selectStart = oc_min(ui->editCursor, ui->editMark);
        u32 selectEnd = oc_max(ui->editCursor, ui->editMark);

        oc_str32 beforeSelect = oc_str32_slice(codepoints, 0, selectStart);
        oc_rect beforeSelectBox = oc_text_bounding_box_utf32(style->font, style->fontSize, beforeSelect);
        beforeSelectBox.x += textX;
        beforeSelectBox.y += textY;

        if(selectStart != selectEnd)
        {
            oc_str32 select = oc_str32_slice(codepoints, selectStart, selectEnd);
            oc_str32 afterSelect = oc_str32_slice(codepoints, selectEnd, codepoints.len);
            oc_rect selectBox = oc_text_bounding_box_utf32(style->font, style->fontSize, select);
            oc_rect afterSelectBox = oc_text_bounding_box_utf32(style->font, style->fontSize, afterSelect);

            selectBox.x += beforeSelectBox.x + beforeSelectBox.w;
            selectBox.y += textY;

            oc_set_color_rgba(0, 0, 1, 1);
            oc_rectangle_fill(selectBox.x, selectBox.y, selectBox.w, lineHeight);

            oc_set_font(style->font);
            oc_set_font_size(style->fontSize);
            oc_set_color(style->color);

            oc_move_to(textX, textY);
            oc_codepoints_outlines(beforeSelect);
            oc_fill();

            oc_set_color_rgba(1, 1, 1, 1);
            oc_codepoints_outlines(select);
            oc_fill();

            oc_set_color(style->color);
            oc_codepoints_outlines(afterSelect);
            oc_fill();
        }
        else
        {
            if(!((u64)(2 * (ui->frameTime - ui->editCursorBlinkStart)) & 1))
            {
                f32 caretX = box->rect.x + textMargin - beforeBox.w + beforeSelectBox.w;
                f32 caretY = textTop;
                oc_set_color(style->color);
                oc_rectangle_fill(caretX, caretY, 1, lineHeight);
            }
            oc_set_font(style->font);
            oc_set_font_size(style->fontSize);
            oc_set_color(style->color);

            oc_move_to(textX, textY);
            oc_codepoints_outlines(codepoints);
            oc_fill();
        }
    }
    else
    {
        oc_set_font(style->font);
        oc_set_font_size(style->fontSize);
        oc_set_color(style->color);

        oc_move_to(textX, textY);
        oc_codepoints_outlines(codepoints);
        oc_fill();
    }
}

oc_ui_text_box_result oc_ui_text_box(const char* name, oc_arena* arena, oc_str8 text)
{
    oc_ui_context* ui = oc_ui_get_context();

    oc_ui_text_box_result result = { .text = text };

    oc_ui_flags frameFlags = OC_UI_FLAG_CLICKABLE
                           | OC_UI_FLAG_DRAW_BACKGROUND
                           | OC_UI_FLAG_DRAW_BORDER
                           | OC_UI_FLAG_CLIP
                           | OC_UI_FLAG_DRAW_PROC;

    oc_ui_box* frame = oc_ui_box_make(name, frameFlags);
    oc_ui_style* style = &frame->style;
    f32 textMargin = 5; //TODO parameterize this margin! must be the same as in oc_ui_text_box_render

    oc_font_extents extents = oc_font_get_scaled_extents(style->font, style->fontSize);

    oc_ui_sig sig = oc_ui_box_sig(frame);

    if(sig.pressed)
    {
        if(!oc_ui_box_active(frame))
        {
            oc_ui_box_activate(frame);

            //NOTE: focus
            ui->focus = frame;
            ui->editFirstDisplayedChar = 0;
            ui->editCursor = 0;
            ui->editMark = 0;
        }
        ui->editCursorBlinkStart = ui->frameTime;
    }

    if(sig.pressed || sig.dragging)
    {
        //NOTE: set cursor/extend selection on mouse press or drag
        oc_vec2 pos = oc_ui_mouse_position();
        f32 cursorX = pos.x - frame->rect.x - textMargin;

        oc_str32 codepoints = oc_utf8_push_to_codepoints(&ui->frameArena, text);
        i32 newCursor = codepoints.len;
        f32 x = 0;
        for(int i = ui->editFirstDisplayedChar; i < codepoints.len; i++)
        {
            oc_rect bbox = oc_text_bounding_box_utf32(style->font, style->fontSize, oc_str32_slice(codepoints, i, i + 1));
            if(x + 0.5 * bbox.w > cursorX)
            {
                newCursor = i;
                break;
            }
            x += bbox.w;
        }
        //NOTE: put cursor the closest to new cursor (this maximizes the resulting selection,
        //      and seems to be the standard behaviour across a number of text editor)
        if(sig.pressed && abs(newCursor - ui->editCursor) > abs(newCursor - ui->editMark))
        {
            i32 tmp = ui->editCursor;
            ui->editCursor = ui->editMark;
            ui->editMark = tmp;
        }
        //NOTE: set the new cursor, and set or leave the mark depending on mode
        ui->editCursor = newCursor;
        if(sig.pressed && !(oc_key_mods(&ui->input) & OC_KEYMOD_SHIFT))
        {
            ui->editMark = ui->editCursor;
        }
    }

    if(sig.hovering)
    {
        oc_ui_box_set_hot(frame, true);
    }
    else
    {
        oc_ui_box_set_hot(frame, false);

        if(oc_mouse_pressed(&ui->input, OC_MOUSE_LEFT) || oc_mouse_pressed(&ui->input, OC_MOUSE_RIGHT) || oc_mouse_pressed(&ui->input, OC_MOUSE_MIDDLE))
        {
            if(oc_ui_box_active(frame))
            {
                oc_ui_box_deactivate(frame);

                //NOTE loose focus
                ui->focus = 0;
            }
        }
    }

    if(oc_ui_box_active(frame))
    {
        oc_str32 oldCodepoints = oc_utf8_push_to_codepoints(&ui->frameArena, text);
        oc_str32 codepoints = oldCodepoints;
        ui->editCursor = oc_clamp(ui->editCursor, 0, codepoints.len);
        ui->editMark = oc_clamp(ui->editMark, 0, codepoints.len);

        //NOTE replace selection with input codepoints
        oc_str32 input = oc_input_text_utf32(&ui->input, &ui->frameArena);
        if(input.len)
        {
            codepoints = oc_ui_edit_replace_selection_with_codepoints(ui, codepoints, input);
            ui->editCursorBlinkStart = ui->frameTime;
        }

        //NOTE handle shortcuts
        oc_keymod_flags mods = oc_key_mods(&ui->input);
        const oc_ui_edit_command* editCommands;
        u32 editCommandCount;
        oc_host_platform hostPlatform = oc_get_host_platform();
        switch(hostPlatform)
        {
            case OC_HOST_PLATFORM_MACOS:
                editCommands = OC_UI_EDIT_COMMANDS_MACOS;
                editCommandCount = OC_UI_EDIT_COMMAND_MACOS_COUNT;
                break;
            case OC_HOST_PLATFORM_WINDOWS:
                editCommands = OC_UI_EDIT_COMMANDS_WINDOWS;
                editCommandCount = OC_UI_EDIT_COMMAND_WINDOWS_COUNT;
                break;
            default:
                OC_ASSERT(0, "unknown host platform: %i", hostPlatform);
        }

        for(int i = 0; i < editCommandCount; i++)
        {
            const oc_ui_edit_command* command = &(editCommands[i]);

            if((oc_key_pressed(&ui->input, command->key) || oc_key_repeated(&ui->input, command->key))
               && (mods & ~OC_KEYMOD_MAIN_MODIFIER) == command->mods)
            {
                codepoints = oc_ui_edit_perform_operation(ui, command->operation, command->move, command->direction, codepoints);
                break;
            }
        }

        //NOTE(martin): check changed/accepted
        if(oldCodepoints.ptr != codepoints.ptr)
        {
            result.changed = true;
            result.text = oc_utf8_push_from_codepoints(arena, codepoints);
        }

        if(oc_key_pressed(&ui->input, OC_KEY_ENTER))
        {
            //TODO(martin): extract in gui_edit_complete() (and use below)
            result.accepted = true;
            oc_ui_box_deactivate(frame);
            ui->focus = 0;
        }

        //NOTE slide contents
        {
            if(ui->editCursor < ui->editFirstDisplayedChar)
            {
                ui->editFirstDisplayedChar = ui->editCursor;
            }
            else
            {
                i32 firstDisplayedChar = ui->editFirstDisplayedChar;
                oc_str32 firstToCursor = oc_str32_slice(codepoints, firstDisplayedChar, ui->editCursor);
                oc_rect firstToCursorBox = oc_text_bounding_box_utf32(style->font, style->fontSize, firstToCursor);

                while(firstToCursorBox.w > (frame->rect.w - 2 * textMargin))
                {
                    firstDisplayedChar++;
                    firstToCursor = oc_str32_slice(codepoints, firstDisplayedChar, ui->editCursor);
                    firstToCursorBox = oc_text_bounding_box_utf32(style->font, style->fontSize, firstToCursor);
                }

                ui->editFirstDisplayedChar = firstDisplayedChar;
            }
        }

        //NOTE: set renderer
        oc_str32* renderCodepoints = oc_arena_push_type(&ui->frameArena, oc_str32);
        *renderCodepoints = oc_str32_push_copy(&ui->frameArena, codepoints);
        oc_ui_box_set_draw_proc(frame, oc_ui_text_box_render, renderCodepoints);
    }
    else
    {
        //NOTE: set renderer
        oc_str32* renderCodepoints = oc_arena_push_type(&ui->frameArena, oc_str32);
        *renderCodepoints = oc_utf8_push_to_codepoints(&ui->frameArena, text);
        oc_ui_box_set_draw_proc(frame, oc_ui_text_box_render, renderCodepoints);
    }

    return (result);
}
