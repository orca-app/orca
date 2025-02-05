/*************************************************************************
*
*  Orca
*  Copyright 2023 Martin Fouilleul and the Orca project contributors
*  See LICENSE.txt for licensing information
*
**************************************************************************/
#include "ui.h"
#include "math.h"
#include "app/app.h"
#include "platform/platform.h"
#include "platform/platform_clock.h"
#include "platform/platform_debug.h"
#include "util/hash.h"
#include "util/memory.h"

//-----------------------------------------------------------------------------
// style rules
//-----------------------------------------------------------------------------

typedef struct oc_ui_tag
{
    u64 hash;
} oc_ui_tag;

typedef struct oc_ui_tag_elt
{
    oc_list_elt listElt;
    oc_ui_tag tag;
} oc_ui_tag_elt;

typedef enum
{
    OC_UI_SEL_ID = 0,
    OC_UI_SEL_TAG,

} oc_ui_selector_kind;

typedef enum
{
    OC_UI_SEL_DESCENDANT = 0,
    OC_UI_SEL_AND = 1,
    //...
} oc_ui_selector_op;

typedef struct oc_ui_selector
{
    oc_list_elt listElt;
    oc_ui_selector_kind kind;
    oc_ui_selector_op op;

    oc_str8 string;
    u64 hash;

} oc_ui_selector;

typedef union oc_ui_pattern_specificity
{
    struct
    {
        u32 id;
        u32 tag;
    };

    u32 s[2];
} oc_ui_pattern_specificity;

typedef struct oc_ui_pattern
{
    oc_list l;
    oc_ui_pattern_specificity specificity;
} oc_ui_pattern;

typedef struct oc_ui_style_rule
{
    oc_list_elt boxElt;
    oc_list_elt rulesetElt;

    oc_ui_box* owner;
    oc_ui_pattern pattern;
    oc_ui_attr_mask mask;
    oc_ui_style style;
} oc_ui_style_rule;

//-----------------------------------------------------------------------------
// style variables structs
//-----------------------------------------------------------------------------

typedef struct oc_ui_var_stack
{
    oc_list_elt bucketElt;
    oc_str8 name;
    u64 hash;
    oc_list vars;
} oc_ui_var_stack;

typedef enum oc_ui_var_kind
{
    OC_UI_VAR_I32,
    OC_UI_VAR_F32,
    OC_UI_VAR_SIZE,
    OC_UI_VAR_COLOR,
    OC_UI_VAR_FONT,
} oc_ui_var_kind;

typedef struct oc_ui_value
{
    oc_ui_var_kind kind;

    union
    {
        oc_ui_size size;
        oc_color color;
        oc_font font;
        f32 f;
        i32 i;
    };
} oc_ui_value;

typedef struct oc_ui_var
{
    oc_list_elt boxElt;
    oc_list_elt stackElt;
    oc_ui_var_stack* stack;
    oc_ui_value value;

} oc_ui_var;

typedef struct oc_ui_var_map
{
    u64 mask;
    oc_list* buckets;
} oc_ui_var_map;

//-----------------------------------------------------------------------------
// context structs
//-----------------------------------------------------------------------------

enum
{
    OC_UI_MAX_INPUT_CHAR_PER_FRAME = 64
};

typedef struct oc_ui_input_text
{
    u8 count;
    oc_utf32 codePoints[OC_UI_MAX_INPUT_CHAR_PER_FRAME];

} oc_ui_input_text;

typedef struct oc_ui_stack_elt oc_ui_stack_elt;

struct oc_ui_stack_elt
{
    oc_ui_stack_elt* parent;

    union
    {
        oc_ui_box* box;
        oc_rect clip;
    };
};

enum
{
    OC_UI_BOX_MAP_BUCKET_COUNT = 1024
};

typedef struct oc_ui_context
{
    bool init;

    oc_input_state input;

    u64 frameCounter;
    f64 frameTime;
    f64 lastFrameDuration;

    oc_arena frameArena;
    oc_pool boxPool;
    oc_list boxMap[OC_UI_BOX_MAP_BUCKET_COUNT];

    oc_ui_box* root;
    oc_ui_box* overlay;
    oc_list overlayList;
    oc_ui_stack_elt* boxStack;
    oc_ui_stack_elt* clipStack;

    oc_list nextBoxTags;

    u32 z;
    oc_ui_box* hovered;
    oc_ui_box* focus;

    //TODO: reorganize
    oc_ui_var_map styleVariables;
    oc_ui_style_rule* workingRule;
    oc_font defaultFont;

} oc_ui_context;

oc_thread_local oc_ui_context* oc_uiCurrentContext = 0;

//-----------------------------------------------------------------------------
// Context init/cleanup and thread local context
//-----------------------------------------------------------------------------

oc_ui_context* oc_ui_context_create(oc_font defaultFont)
{
    oc_ui_context* ui = oc_malloc_type(oc_ui_context);

    memset(ui, 0, sizeof(oc_ui_context));
    oc_arena_init(&ui->frameArena);
    oc_pool_init(&ui->boxPool, sizeof(oc_ui_box));
    ui->defaultFont = defaultFont;
    ui->init = true;

    oc_ui_set_context(ui);

    return (ui);
}

void oc_ui_context_destroy(oc_ui_context* context)
{
    if(context == oc_ui_get_context())
    {
        oc_ui_set_context(0);
    }
    oc_arena_cleanup(&context->frameArena);
    oc_pool_cleanup(&context->boxPool);
    memset(context, 0, sizeof(oc_ui_context));
}

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

oc_rect oc_ui_box_clip_rect(oc_ui_box* box)
{
    oc_rect clipRect = { -FLT_MAX / 2., -FLT_MAX / 2., FLT_MAX, FLT_MAX };

    if(box->style.layout.overflow.x != OC_UI_OVERFLOW_ALLOW)
    {
        clipRect.x = box->rect.x;
        clipRect.w = box->rect.w;
    }
    if(box->style.layout.overflow.y != OC_UI_OVERFLOW_ALLOW)
    {
        clipRect.y = box->rect.y;
        clipRect.h = box->rect.h;
    }
    return clipRect;
}

oc_ui_box* oc_ui_box_top(void)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_stack_elt* elt = ui->boxStack;
    oc_ui_box* box = elt ? elt->box : 0;
    return (box);
}

void oc_ui_box_pop(void)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_box* box = oc_ui_box_top();
    if(box)
    {
        oc_ui_stack_pop(&ui->clipStack);
        oc_ui_stack_pop(&ui->boxStack);

        oc_list_for(box->styleVariables, var, oc_ui_var, boxElt)
        {
            oc_list_remove(&var->stack->vars, &var->stackElt);
        }
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
    oc_list_push_back(&box->tags, &elt->listElt);
}

void oc_ui_tag_next_str8(oc_str8 string)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_tag_elt* elt = oc_arena_push_type(&ui->frameArena, oc_ui_tag_elt);
    elt->tag = oc_ui_tag_make_str8(string);
    oc_list_push_back(&ui->nextBoxTags, &elt->listElt);
}

void oc_ui_tag_str8(oc_str8 string)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_box* box = oc_ui_box_top();

    if(!box)
    {
        //TODO: better error
        oc_log_error("no box to tag.");
    }
    else
    {
        oc_ui_tag_elt* elt = oc_arena_push_type(&ui->frameArena, oc_ui_tag_elt);
        elt->tag = oc_ui_tag_make_str8(string); //TODO: should we copy string?
        oc_list_push_back(&box->tags, &elt->listElt);
    }
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
    oc_list_for(path.list, elt, oc_str8_elt, listElt)
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
    oc_list_push_back(&(ui->boxMap[index]), &box->bucketElt);
}

oc_ui_box* oc_ui_box_lookup_key(oc_ui_key key)
{
    oc_ui_context* ui = oc_ui_get_context();
    u64 index = key.hash & (OC_UI_BOX_MAP_BUCKET_COUNT - 1);

    oc_list_for(ui->boxMap[index], box, oc_ui_box, bucketElt)
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
    copy->hash = oc_hash_xx64_string(copy->string);
    oc_list_push_back(&pattern->l, &copy->listElt);

    OC_DEBUG_ASSERT(selector.kind < 2);
    pattern->specificity.s[selector.kind]++;
}

void oc_ui_style_rule_begin(oc_str8 patternString)
{
    oc_ui_context* ui = oc_ui_get_context();
    if(ui)
    {
        if(ui->workingRule)
        {
            //TODO: better error
            oc_log_error("nested style rule definitions are not allowed.");
        }
        else
        {
            //NOTE: parse pattern from patternString
            oc_ui_pattern pattern = { 0 };
            u32 selectorStart = 0;
            oc_ui_selector_op nextOp = OC_UI_SEL_DESCENDANT;
            oc_ui_selector_kind nextKind = OC_UI_SEL_ID;

            for(u32 index = 0; index < patternString.len; index++)
            {
                switch(patternString.ptr[index])
                {
                    case ' ':
                    {
                        //NOTE: emit the current selector and start a new descendant selector
                        if(index - selectorStart)
                        {
                            oc_str8 string = oc_str8_slice(patternString, selectorStart, index);
                            oc_ui_pattern_push(
                                &ui->frameArena,
                                &pattern,
                                (oc_ui_selector){
                                    .op = nextOp,
                                    .kind = nextKind,
                                    .string = string,
                                });
                        }
                        nextOp = OC_UI_SEL_DESCENDANT;
                        nextKind = OC_UI_SEL_ID;

                        //NOTE: skip remaining whitespace
                        while(index + 1 < patternString.len && patternString.ptr[index + 1] == ' ')
                        {
                            index++;
                        }
                        selectorStart = index + 1;
                    }
                    break;

                    case '.':
                    {
                        //NOTE: emit the current selector and start new tag selector
                        if(index - selectorStart)
                        {
                            oc_str8 string = oc_str8_slice(patternString, selectorStart, index);
                            oc_ui_pattern_push(
                                &ui->frameArena,
                                &pattern,
                                (oc_ui_selector){
                                    .op = nextOp,
                                    .kind = nextKind,
                                    .string = string,
                                });

                            //NOTE: set next op to AND only if there was a selector before. Otherwise we're either
                            // after a ' ' or an (ignored) empty tag, and in both case we must keep the next op as it is.
                            nextOp = OC_UI_SEL_AND;
                        }
                        nextKind = OC_UI_SEL_TAG;
                        selectorStart = index + 1;
                    }
                    break;

                    default:
                        //NOTE: just append character to current selector
                        break;
                }
            }

            //NOTE: emit last selector
            if(patternString.len - selectorStart)
            {
                oc_str8 string = oc_str8_slice(patternString, selectorStart, patternString.len);

                oc_ui_pattern_push(
                    &ui->frameArena,
                    &pattern,
                    (oc_ui_selector){
                        .op = nextOp,
                        .kind = nextKind,
                        .string = string,
                    });
            }

            //NOTE: create the rule and put it on the workbench
            oc_ui_style_rule* rule = oc_arena_push_type(&ui->frameArena, oc_ui_style_rule);
            memset(rule, 0, sizeof(oc_ui_style_rule));
            rule->pattern = pattern;

            ui->workingRule = rule;
        }
    }
}

void oc_ui_style_rule_end()
{
    oc_ui_context* ui = oc_ui_get_context();
    if(ui)
    {
        if(ui->workingRule)
        {
            oc_ui_box* box = oc_ui_box_top();
            if(box)
            {
                oc_list_push_back(&box->rules, &ui->workingRule->boxElt);
            }
            ui->workingRule = 0;
        }
        //TODO: else error?
    }
}

//[WIP] ///////////////////////////////////////////////////////////

oc_ui_attr_mask oc_ui_attr_to_mask(oc_ui_attr attr)
{
    //NOTE: for now we have a one to one correspondance between attr and mask,
    //      be in the future we may have combined attr (eg. OC_UI_STYLE_MARGINS to set both margins, etc)

    oc_ui_attr_mask mask = 1 << attr;
    return mask;
}

void oc_ui_style_set_i32(oc_ui_attr attr, i32 i)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_style* style = 0;
    if(ui->workingRule)
    {
        style = &ui->workingRule->style;
    }
    else
    {
        oc_ui_box* box = oc_ui_box_top();
        if(!box)
        {
            //TODO: better error
            oc_log_error("error trying to set style attribute: no box.");
            return;
        }
        style = box->targetStyle;
    }

    switch(attr)
    {
        case OC_UI_AXIS:
            style->layout.axis = i;
            break;

        case OC_UI_ALIGN_X:
            style->layout.align.x = i;
            break;

        case OC_UI_ALIGN_Y:
            style->layout.align.y = i;
            break;

        case OC_UI_FLOATING_X:
            style->floating.x = i;
            break;

        case OC_UI_FLOATING_Y:
            style->floating.y = i;
            break;

        case OC_UI_OVERFLOW_X:
            style->layout.overflow.x = i;
            break;

        case OC_UI_OVERFLOW_Y:
            style->layout.overflow.y = i;
            break;

        case OC_UI_CONSTRAIN_X:
            style->layout.constrain.x = i;
            break;

        case OC_UI_CONSTRAIN_Y:
            style->layout.constrain.y = i;
            break;

        case OC_UI_DRAW_MASK:
            style->drawMask = i;
            break;

        case OC_UI_ANIMATION_MASK:
            style->animationMask = i;
            break;

        case OC_UI_CLICK_THROUGH:
            style->clickThrough = i;
            break;

        default:
            //TODO: better error
            oc_log_error("error trying to set attribute: type mismatch.");
            return;
    }

    if(ui->workingRule)
    {
        ui->workingRule->mask |= oc_ui_attr_to_mask(attr);
    }
}

void oc_ui_style_set_f32(oc_ui_attr attr, f32 f)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_style* style = 0;
    if(ui->workingRule)
    {
        style = &ui->workingRule->style;
    }
    else
    {
        oc_ui_box* box = oc_ui_box_top();
        if(!box)
        {
            //TODO: better error
            oc_log_error("error trying to set style attribute: no box.");
            return;
        }
        style = box->targetStyle;
    }

    switch(attr)
    {
        case OC_UI_MARGIN_X:
            style->layout.margin.x = f;
            break;

        case OC_UI_MARGIN_Y:
            style->layout.margin.y = f;
            break;

        case OC_UI_SPACING:
            style->layout.spacing = f;
            break;

        case OC_UI_FLOAT_TARGET_X:
            style->floatTarget.x = f;
            break;

        case OC_UI_FLOAT_TARGET_Y:
            style->floatTarget.y = f;
            break;

        case OC_UI_TEXT_SIZE:
            style->fontSize = f;
            break;

        case OC_UI_BORDER_SIZE:
            style->borderSize = f;
            break;

        case OC_UI_ROUNDNESS:
            style->roundness = f;
            break;

        case OC_UI_ANIMATION_TIME:
            style->animationTime = f;
            break;

        default:
            //TODO: better error
            oc_log_error("error trying to set attribute: type mismatch.");
            return;
    }

    if(ui->workingRule)
    {
        ui->workingRule->mask |= oc_ui_attr_to_mask(attr);
    }
}

void oc_ui_style_set_color(oc_ui_attr attr, oc_color color)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_style* style = 0;
    if(ui->workingRule)
    {
        style = &ui->workingRule->style;
    }
    else
    {
        oc_ui_box* box = oc_ui_box_top();
        if(!box)
        {
            //TODO: better error
            oc_log_error("error trying to set style attribute: no box.");
            return;
        }
        style = box->targetStyle;
    }
    switch(attr)
    {
        case OC_UI_COLOR:
            style->color = color;
            break;

        case OC_UI_BG_COLOR:
            style->bgColor = color;
            break;

        case OC_UI_BORDER_COLOR:
            style->borderColor = color;
            break;

        default:
            //TODO: better error
            oc_log_error("error trying to set attribute: type mismatch.");
            return;
    }

    if(ui->workingRule)
    {
        ui->workingRule->mask |= oc_ui_attr_to_mask(attr);
    }
}

void oc_ui_style_set_font(oc_ui_attr attr, oc_font font)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_style* style = 0;
    if(ui->workingRule)
    {
        style = &ui->workingRule->style;
    }
    else
    {
        oc_ui_box* box = oc_ui_box_top();
        if(!box)
        {
            //TODO: better error
            oc_log_error("error trying to set style attribute: no box.");
            return;
        }
        style = box->targetStyle;
    }
    if(attr == OC_UI_FONT)
    {
        style->font = font;
    }
    else
    { //TODO: better error
        oc_log_error("error trying to set attribute: type mismatch.");
        return;
    }

    if(ui->workingRule)
    {
        ui->workingRule->mask |= oc_ui_attr_to_mask(attr);
    }
}

void oc_ui_style_set_size(oc_ui_attr attr, oc_ui_size size)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_style* style = 0;
    if(ui->workingRule)
    {
        style = &ui->workingRule->style;
    }
    else
    {
        oc_ui_box* box = oc_ui_box_top();
        if(!box)
        {
            //TODO: better error
            oc_log_error("error trying to set style attribute: no box.");
            return;
        }
        style = box->targetStyle;
    }
    switch(attr)
    {
        case OC_UI_WIDTH:
            style->size.width = size;
            break;

        case OC_UI_HEIGHT:
            style->size.height = size;
            break;

        default:
            //TODO: better error
            oc_log_error("error trying to set attribute: type mismatch.");
            return;
    }

    if(ui->workingRule)
    {
        ui->workingRule->mask |= oc_ui_attr_to_mask(attr);
    }
}

////////////////////////////////////////////////////////
// style vars

//TODO: - each frame, clear vars map
//      - at end of each object, pop variables

oc_ui_var* oc_ui_var_find(oc_str8 name)
{
    oc_ui_context* ui = oc_ui_get_context();

    u64 hash = oc_hash_xx64_string(name);
    u64 index = hash & ui->styleVariables.mask;
    oc_list bucket = ui->styleVariables.buckets[index];
    oc_ui_var_stack* stack = 0;

    oc_list_for(bucket, elt, oc_ui_var_stack, bucketElt)
    {
        if(elt->hash == hash)
        {
            stack = elt;
            break;
        }
    }

    oc_ui_var* var = 0;
    if(stack)
    {
        var = oc_list_first_entry(stack->vars, oc_ui_var, stackElt);
    }

    return var;
}

void oc_ui_var_push(oc_str8 name, oc_ui_value value, bool alwaysSet, oc_list* scopeList)
{
    oc_ui_context* ui = oc_ui_get_context();

    oc_ui_var* var = oc_arena_push_type(&ui->frameArena, oc_ui_var);

    u64 hash = oc_hash_xx64_string(name);
    u64 index = hash & ui->styleVariables.mask;
    oc_list* bucket = &ui->styleVariables.buckets[index];
    oc_ui_var_stack* stack = 0;

    oc_list_for(*bucket, elt, oc_ui_var_stack, bucketElt)
    {
        if(elt->hash == hash)
        {
            stack = elt;
            break;
        }
    }
    if(!stack)
    {
        stack = oc_arena_push_type(&ui->frameArena, oc_ui_var_stack);
        memset(stack, 0, sizeof(oc_ui_var_stack));

        stack->name = oc_str8_push_copy(&ui->frameArena, name);
        stack->hash = hash;
        oc_list_push_back(bucket, &stack->bucketElt);
    }

    var->stack = stack;
    oc_list_push_front(&stack->vars, &var->stackElt);

    if(!scopeList)
    {
        //NOTE: we're pushing a variable in the current object's scope
        oc_ui_box* box = oc_ui_box_top();
        OC_DEBUG_ASSERT(box);

        scopeList = &box->styleVariables;
    }

    oc_list_push_front(scopeList, &var->boxElt);

    oc_ui_var* prev = oc_list_next_entry(var, oc_ui_var, stackElt);

    if(!alwaysSet && prev && prev->value.kind == value.kind)
    {
        var->value = prev->value;
    }
    else
    {
        var->value = value;
    }
}

void oc_ui_var_default_i32_str8(oc_str8 name, i32 i)
{
    oc_ui_var_push(
        name,
        (oc_ui_value){
            .kind = OC_UI_VAR_I32,
            .i = i,
        },
        false,
        0);
}

void oc_ui_var_default_f32_str8(oc_str8 name, f32 f)
{
    oc_ui_var_push(
        name,
        (oc_ui_value){
            .kind = OC_UI_VAR_F32,
            .f = f,
        },
        false,
        0);
}

void oc_ui_var_default_size_str8(oc_str8 name, oc_ui_size size)
{
    oc_ui_var_push(
        name,
        (oc_ui_value){
            .kind = OC_UI_VAR_SIZE,
            .size = size,
        },
        false,
        0);
}

void oc_ui_var_default_color_str8(oc_str8 name, oc_color color)
{
    oc_ui_var_push(
        name,
        (oc_ui_value){
            .kind = OC_UI_VAR_COLOR,
            .color = color,
        },
        false,
        0);
}

void oc_ui_var_default_font_str8(oc_str8 name, oc_font font)
{
    oc_ui_var_push(
        name,
        (oc_ui_value){
            .kind = OC_UI_VAR_FONT,
            .font = font,
        },
        false,
        0);
}

void oc_ui_var_default_str8(oc_str8 name, oc_str8 defaultName)
{
    oc_ui_var* var = oc_ui_var_find(name);
    if(!var)
    {
        oc_ui_value value = {
            .kind = OC_UI_VAR_I32,
            .i = 0,
        };
        oc_ui_var* defaultVar = oc_ui_var_find(defaultName);
        if(defaultVar)
        {
            value = defaultVar->value;
        }
        else
        {
            //TODO: signal error
        }
        oc_ui_var_push(
            name,
            value,
            false,
            0);
    }
}

//C-string versions
void oc_ui_var_default_i32(const char* name, i32 i)
{
    oc_ui_var_default_i32_str8(OC_STR8(name), i);
}

void oc_ui_var_default_f32(const char* name, f32 f)
{
    oc_ui_var_default_f32_str8(OC_STR8(name), f);
}

void oc_ui_var_default_size(const char* name, oc_ui_size size)
{
    oc_ui_var_default_size_str8(OC_STR8(name), size);
}

void oc_ui_var_default_color(const char* name, oc_color color)
{
    oc_ui_var_default_color_str8(OC_STR8(name), color);
}

void oc_ui_var_default_font(const char* name, oc_font font)
{
    oc_ui_var_default_font_str8(OC_STR8(name), font);
}

void oc_ui_var_default(const char* name, const char* src)
{
    oc_ui_var_default_str8(OC_STR8(name), OC_STR8(src));
}

//NOTE: setting variable value
void oc_ui_var_set_i32_str8(oc_str8 name, i32 i)
{
    oc_ui_var_push(
        name,
        (oc_ui_value){
            .kind = OC_UI_VAR_I32,
            .i = i,
        },
        true,
        0);
}

void oc_ui_var_set_f32_str8(oc_str8 name, f32 f)
{
    oc_ui_var_push(
        name,
        (oc_ui_value){
            .kind = OC_UI_VAR_F32,
            .f = f,
        },
        true,
        0);
}

void oc_ui_var_set_size_str8(oc_str8 name, oc_ui_size size)
{
    oc_ui_var_push(
        name,
        (oc_ui_value){
            .kind = OC_UI_VAR_SIZE,
            .size = size,
        },
        true,
        0);
}

void oc_ui_var_set_color_str8(oc_str8 name, oc_color color)
{
    oc_ui_var_push(
        name,
        (oc_ui_value){
            .kind = OC_UI_VAR_COLOR,
            .color = color,
        },
        true,
        0);
}

void oc_ui_var_set_font_str8(oc_str8 name, oc_font font)
{
    oc_ui_var_push(
        name,
        (oc_ui_value){
            .kind = OC_UI_VAR_FONT,
            .font = font,
        },
        true,
        0);
}

void oc_ui_var_set_str8(oc_str8 name, oc_str8 defaultName)
{
    oc_ui_value value = {
        .kind = OC_UI_VAR_I32,
        .i = 0,
    };
    oc_ui_var* defaultVar = oc_ui_var_find(defaultName);
    if(defaultVar)
    {
        value = defaultVar->value;
    }
    else
    {
        //TODO: signal error
    }
    oc_ui_var_push(name, value, true, 0);
}

//C-string versions
void oc_ui_var_set_i32(const char* name, i32 i)
{
    oc_ui_var_set_i32_str8(OC_STR8(name), i);
}

void oc_ui_var_set_f32(const char* name, f32 f)
{
    oc_ui_var_set_f32_str8(OC_STR8(name), f);
}

void oc_ui_var_set_size(const char* name, oc_ui_size size)
{
    oc_ui_var_set_size_str8(OC_STR8(name), size);
}

void oc_ui_var_set_color(const char* name, oc_color color)
{
    oc_ui_var_set_color_str8(OC_STR8(name), color);
}

void oc_ui_var_set_font(const char* name, oc_font font)
{
    oc_ui_var_set_font_str8(OC_STR8(name), font);
}

void oc_ui_var_set(const char* name, const char* src)
{
    oc_ui_var_set_str8(OC_STR8(name), OC_STR8(src));
}

//NOTE: getting variable value
oc_ui_value oc_ui_var_get_typed(oc_str8 name, oc_ui_var_kind kind)
{
    oc_ui_value value;

    oc_ui_var* var = oc_ui_var_find(name);
    if(var && var->value.kind == kind)
    {
        value = var->value;
    }
    else
    {
        //NOTE memset because designated initializer could wrongly init the union if we change fields in the future
        memset(&value, 0, sizeof(oc_ui_value));
        value.kind = kind;
    }
    return value;
}

i32 oc_ui_var_get_i32_str8(oc_str8 name)
{
    oc_ui_value val = oc_ui_var_get_typed(name, OC_UI_VAR_I32);
    return val.i;
}

f32 oc_ui_var_get_f32_str8(oc_str8 name)
{
    oc_ui_value val = oc_ui_var_get_typed(name, OC_UI_VAR_F32);
    return val.f;
}

oc_ui_size oc_ui_var_get_size_str8(oc_str8 name)
{
    oc_ui_value val = oc_ui_var_get_typed(name, OC_UI_VAR_SIZE);
    return val.size;
}

oc_color oc_ui_var_get_color_str8(oc_str8 name)
{
    oc_ui_value val = oc_ui_var_get_typed(name, OC_UI_VAR_COLOR);
    return val.color;
}

oc_font oc_ui_var_get_font_str8(oc_str8 name)
{
    oc_ui_value val = oc_ui_var_get_typed(name, OC_UI_VAR_FONT);
    return val.font;
}

//C-string versions
i32 oc_ui_var_get_i32(const char* name)
{
    return oc_ui_var_get_i32_str8(OC_STR8(name));
}

f32 oc_ui_var_get_f32(const char* name)
{
    return oc_ui_var_get_f32_str8(OC_STR8(name));
}

oc_ui_size oc_ui_var_get_size(const char* name)
{
    return oc_ui_var_get_size_str8(OC_STR8(name));
}

oc_color oc_ui_var_get_color(const char* name)
{
    return oc_ui_var_get_color_str8(OC_STR8(name));
}

oc_font oc_ui_var_get_font(const char* name)
{
    return oc_ui_var_get_font_str8(OC_STR8(name));
}

//NOTE: setting a style attribute from a variable
void oc_ui_style_set_var_str8(oc_ui_attr attr, oc_str8 name)
{
    oc_ui_value value = { 0 };
    oc_ui_var* var = oc_ui_var_find(name);
    if(var)
    {
        value = var->value;
    }
    else
    {
        switch(attr)
        {
            case OC_UI_WIDTH:
            case OC_UI_HEIGHT:
                value.kind = OC_UI_VAR_SIZE;
                break;

            case OC_UI_AXIS:
            case OC_UI_ALIGN_X:
            case OC_UI_ALIGN_Y:
            case OC_UI_FLOATING_X:
            case OC_UI_FLOATING_Y:
            case OC_UI_OVERFLOW_X:
            case OC_UI_OVERFLOW_Y:
            case OC_UI_CONSTRAIN_X:
            case OC_UI_CONSTRAIN_Y:
            case OC_UI_DRAW_MASK:
            case OC_UI_ANIMATION_MASK:
            case OC_UI_CLICK_THROUGH:
                value.kind = OC_UI_VAR_I32;
                break;

            case OC_UI_MARGIN_X:
            case OC_UI_MARGIN_Y:
            case OC_UI_SPACING:
            case OC_UI_FLOAT_TARGET_X:
            case OC_UI_FLOAT_TARGET_Y:
            case OC_UI_TEXT_SIZE:
            case OC_UI_BORDER_SIZE:
            case OC_UI_ROUNDNESS:
            case OC_UI_ANIMATION_TIME:
                value.kind = OC_UI_VAR_F32;
                break;

            case OC_UI_COLOR:
            case OC_UI_BG_COLOR:
            case OC_UI_BORDER_COLOR:
                value.kind = OC_UI_VAR_COLOR;
                break;

            case OC_UI_FONT:
                value.kind = OC_UI_VAR_FONT;
                break;

            case OC_UI_ATTR_COUNT:
                OC_ASSERT(0, "unreachable");
                break;
        }
    }
    switch(value.kind)
    {
        case OC_UI_VAR_I32:
            oc_ui_style_set_i32(attr, value.i);
            break;
        case OC_UI_VAR_F32:
            oc_ui_style_set_f32(attr, value.f);
            break;
        case OC_UI_VAR_SIZE:
            oc_ui_style_set_size(attr, value.size);
            break;
        case OC_UI_VAR_COLOR:
            oc_ui_style_set_color(attr, value.color);
            break;
        case OC_UI_VAR_FONT:
            oc_ui_style_set_font(attr, value.font);
            break;
    }
}

void oc_ui_style_set_var(oc_ui_attr attr, const char* name)
{
    oc_ui_style_set_var_str8(attr, OC_STR8(name));
}

// Themes

//TODO: define pre-computed theme name hashes

void oc_ui_style_theme_light()
{
    //TODO: push these vars using precomputed name hashes

    oc_ui_var_set_color_str8(OC_UI_THEME_PRIMARY, (oc_color){ 0.000, 0.392, 0.980, 1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_PRIMARY_HOVER, (oc_color){ 0.000, 0.384, 0.839, 1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_PRIMARY_ACTIVE, (oc_color){ 0.000, 0.310, 0.702, 1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_PRIMARY_DISABLED, (oc_color){ 0.596, 0.804, 0.992, 1, OC_COLOR_SPACE_SRGB });

    oc_ui_var_set_color_str8(OC_UI_THEME_TEXT_0, (oc_color){ 0.110, 0.122, 0.137, 1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_TEXT_1, (oc_color){ 0.110, 0.122, 0.137, .942, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_TEXT_2, (oc_color){ 0.110, 0.122, 0.137, .834, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_TEXT_3, (oc_color){ 0.110, 0.122, 0.137, .57, OC_COLOR_SPACE_SRGB });

    oc_ui_var_set_color_str8(OC_UI_THEME_BG_0, (oc_color){ 1, 1, 1, 1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_BG_1, (oc_color){ 1, 1, 1, 1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_BG_2, (oc_color){ 1, 1, 1, 1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_BG_3, (oc_color){ 1, 1, 1, 1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_BG_4, (oc_color){ 1, 1, 1, 1, OC_COLOR_SPACE_SRGB });

    oc_ui_var_set_color_str8(OC_UI_THEME_FILL_0, (oc_color){ 0.180, 0.196, 0.220, .1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_FILL_1, (oc_color){ 0.180, 0.196, 0.220, .17, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_FILL_2, (oc_color){ 0.180, 0.196, 0.220, .23, OC_COLOR_SPACE_SRGB });

    oc_ui_var_set_color_str8(OC_UI_THEME_BORDER, (oc_color){ 0.110, 0.122, 0.137, .16, OC_COLOR_SPACE_SRGB });

    oc_ui_var_set_color_str8(OC_UI_THEME_TOOLTIP, (oc_color){ 0.255, 0.275, 0.298, 1, OC_COLOR_SPACE_SRGB });

    oc_ui_var_set_f32_str8(OC_UI_THEME_ROUNDNESS_SMALL, 3);
    oc_ui_var_set_f32_str8(OC_UI_THEME_ROUNDNESS_REGULAR, 6);
    oc_ui_var_set_f32_str8(OC_UI_THEME_ROUNDNESS_LARGE, 12);

    oc_ui_var_set_f32_str8(OC_UI_THEME_SPACING_EXTRA_TIGHT, 4);
    oc_ui_var_set_f32_str8(OC_UI_THEME_SPACING_TIGHT, 8);
    oc_ui_var_set_f32_str8(OC_UI_THEME_SPACING_REGULAR_TIGHT, 12);
    oc_ui_var_set_f32_str8(OC_UI_THEME_SPACING_REGULAR, 16);
    oc_ui_var_set_f32_str8(OC_UI_THEME_SPACING_REGULAR_LOOSE, 20);
    oc_ui_var_set_f32_str8(OC_UI_THEME_SPACING_LOOSE, 24);
    oc_ui_var_set_f32_str8(OC_UI_THEME_SPACING_EXTRA_LOOSE, 32);

    oc_ui_var_set_f32_str8(OC_UI_THEME_TEXT_SIZE_SMALL, 12);
    oc_ui_var_set_f32_str8(OC_UI_THEME_TEXT_SIZE_REGULAR, 14);
    oc_ui_var_set_f32_str8(OC_UI_THEME_TEXT_SIZE_HEADER_0, 32);
    oc_ui_var_set_f32_str8(OC_UI_THEME_TEXT_SIZE_HEADER_1, 28);
    oc_ui_var_set_f32_str8(OC_UI_THEME_TEXT_SIZE_HEADER_2, 24);
    oc_ui_var_set_f32_str8(OC_UI_THEME_TEXT_SIZE_HEADER_3, 20);
    oc_ui_var_set_f32_str8(OC_UI_THEME_TEXT_SIZE_HEADER_4, 18);

    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_var_set_font_str8(OC_UI_THEME_FONT_REGULAR, ui->defaultFont);
}

void oc_ui_style_theme_dark()
{
    oc_ui_var_set_color_str8(OC_UI_THEME_PRIMARY, (oc_color){ 0.33, 0.66, 1, 1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_PRIMARY_HOVER, (oc_color){ 0.5, 0.757, 1, 1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_PRIMARY_ACTIVE, (oc_color){ 0.66, 0.84, 1, 1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_PRIMARY_DISABLED, (oc_color){ 0.074, 0.361, 0.722, 1, OC_COLOR_SPACE_SRGB });

    oc_ui_var_set_color_str8(OC_UI_THEME_TEXT_0, (oc_color){ 0.976, 0.976, 0.976, 1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_TEXT_1, (oc_color){ 0.976, 0.976, 0.976, .64, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_TEXT_2, (oc_color){ 0.976, 0.976, 0.976, .38, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_TEXT_3, (oc_color){ 0.976, 0.976, 0.976, .15, OC_COLOR_SPACE_SRGB });

    oc_ui_var_set_color_str8(OC_UI_THEME_BG_0, (oc_color){ 0.086, 0.086, 0.102, 1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_BG_1, (oc_color){ 0.137, 0.141, 0.165, 1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_BG_2, (oc_color){ 0.208, 0.212, 0.231, 1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_BG_3, (oc_color){ 0.263, 0.267, 0.29, 1, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_BG_4, (oc_color){ 0.31, 0.318, 0.349, 1, OC_COLOR_SPACE_SRGB });

    oc_ui_var_set_color_str8(OC_UI_THEME_FILL_0, (oc_color){ 1, 1, 1, .033, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_FILL_1, (oc_color){ 1, 1, 1, .045, OC_COLOR_SPACE_SRGB });
    oc_ui_var_set_color_str8(OC_UI_THEME_FILL_2, (oc_color){ 1, 1, 1, 0.063, OC_COLOR_SPACE_SRGB });

    oc_ui_var_set_color_str8(OC_UI_THEME_BORDER, (oc_color){ 1, 1, 1, 0.018, OC_COLOR_SPACE_SRGB });

    oc_ui_var_set_color_str8(OC_UI_THEME_TOOLTIP, (oc_color){ 0.786, 0.792, 0.804, 1, OC_COLOR_SPACE_SRGB });

    oc_ui_var_set_f32_str8(OC_UI_THEME_ROUNDNESS_SMALL, 3);
    oc_ui_var_set_f32_str8(OC_UI_THEME_ROUNDNESS_REGULAR, 6);
    oc_ui_var_set_f32_str8(OC_UI_THEME_ROUNDNESS_LARGE, 12);

    oc_ui_var_set_f32_str8(OC_UI_THEME_SPACING_EXTRA_TIGHT, 4);
    oc_ui_var_set_f32_str8(OC_UI_THEME_SPACING_TIGHT, 8);
    oc_ui_var_set_f32_str8(OC_UI_THEME_SPACING_REGULAR_TIGHT, 12);
    oc_ui_var_set_f32_str8(OC_UI_THEME_SPACING_REGULAR, 16);
    oc_ui_var_set_f32_str8(OC_UI_THEME_SPACING_REGULAR_LOOSE, 20);
    oc_ui_var_set_f32_str8(OC_UI_THEME_SPACING_LOOSE, 24);
    oc_ui_var_set_f32_str8(OC_UI_THEME_SPACING_EXTRA_LOOSE, 32);

    oc_ui_var_set_f32_str8(OC_UI_THEME_TEXT_SIZE_SMALL, 12);
    oc_ui_var_set_f32_str8(OC_UI_THEME_TEXT_SIZE_REGULAR, 14);
    oc_ui_var_set_f32_str8(OC_UI_THEME_TEXT_SIZE_HEADER_0, 32);
    oc_ui_var_set_f32_str8(OC_UI_THEME_TEXT_SIZE_HEADER_1, 28);
    oc_ui_var_set_f32_str8(OC_UI_THEME_TEXT_SIZE_HEADER_2, 24);
    oc_ui_var_set_f32_str8(OC_UI_THEME_TEXT_SIZE_HEADER_3, 20);
    oc_ui_var_set_f32_str8(OC_UI_THEME_TEXT_SIZE_HEADER_4, 18);

    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_var_set_font_str8(OC_UI_THEME_FONT_REGULAR, ui->defaultFont);
}

//-----------------------------------------------------------------------------
// input
//-----------------------------------------------------------------------------

void oc_ui_process_event(oc_event* event)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_input_process_event(&ui->frameArena, &ui->input, event);
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

oc_input_state* oc_ui_input()
{
    oc_ui_context* ui = oc_ui_get_context();
    return &ui->input;
}

f64 oc_ui_frame_time()
{
    oc_ui_context* ui = oc_ui_get_context();
    return ui->frameTime;
}

oc_arena* oc_ui_frame_arena()
{
    oc_ui_context* ui = oc_ui_get_context();
    return &ui->frameArena;
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

oc_ui_sig oc_ui_box_compute_signals(oc_ui_box* box)
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
        sig.lastPressedMouse = box->pressedMouse;

        //TODO: we might want to restrict that only to clickable boxes?
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
            sig.tripleClicked = oc_mouse_triple_clicked(input, OC_MOUSE_LEFT);
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

        sig.pasted = oc_clipboard_pasted(input);
    }
    return (sig);
}

oc_ui_box* oc_ui_box_begin_str8(oc_str8 string)
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

    box->keyString = oc_str8_push_copy(&ui->frameArena, string);
    box->text = (oc_str8){ 0 };

    //NOTE: setup hierarchy
    if(box->frameCounter != ui->frameCounter)
    {
        oc_list_init(&box->children);
        box->parent = oc_ui_box_top();
        if(box->parent)
        {
            oc_list_push_back(&box->parent->children, &box->listElt);
            box->parentClosed = box->parent->closed || box->parent->parentClosed;
        }
    }
    else
    {
        //maybe this should be a warning that we're trying to make the box twice in the same frame?
        oc_log_warning("trying to make ui box '%.*s' multiple times in the same frame\n", (int)string.len, string.ptr);
    }

    box->frameCounter = ui->frameCounter;

    //NOTE: create style
    box->targetStyle = oc_arena_push_type(&ui->frameArena, oc_ui_style);
    memset(box->targetStyle, 0, sizeof(oc_ui_style));

    //NOTE: set tags, rules and variables
    box->rules = (oc_list){ 0 };
    box->tags = ui->nextBoxTags;
    ui->nextBoxTags = (oc_list){ 0 };
    box->styleVariables = (oc_list){ 0 };

    //NOTE: compute box signals
    box->sig = oc_ui_box_compute_signals(box);
    if(box->sig.hovering)
    {
        oc_ui_tag_box_str8(box, OC_STR8_LIT("hover"));
    }

    //NOTE push box
    {
        oc_ui_stack_elt* elt = oc_ui_stack_push(ui, &ui->boxStack);
        elt->box = box;
    }

    //NOTE push mouse clip
    {
        oc_rect currentClip = oc_ui_clip_top();
        oc_rect clipRect = oc_ui_box_clip_rect(box);

        oc_ui_stack_elt* elt = oc_ui_stack_push(ui, &ui->clipStack);

        if(box->overlay)
        {
            elt->clip = clipRect;
        }
        else
        {

            elt->clip = oc_ui_intersect_rects(currentClip, clipRect);
        }
    }

    //NOTE: clear overlay. This must be done _after_ mouse clip is computed, since it depends
    //      on last frame's value of overlay.
    box->overlayElt = (oc_list_elt){ 0 };
    box->overlay = false;

    return (box);
}

void oc_ui_set_overlay(bool overlay)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_box* box = oc_ui_box_top();

    if(overlay && !box->overlay)
    {
        oc_list_push_back(&ui->overlayList, &box->overlayElt);
        box->overlay = true;
    }
    else if(!overlay && box->overlay)
    {
        oc_list_remove(&ui->overlayList, &box->overlayElt);
        box->overlay = false;
    }
}

oc_ui_box* oc_ui_scrollbar_str8(oc_str8 name, oc_rect rect, f32 thumbRatio, f32* scrollValue, bool horizontal)
{
    oc_ui_box* track = oc_ui_box_str8(name)
    {
        oc_ui_style_set_i32(OC_UI_FLOATING_X, 1);
        oc_ui_style_set_i32(OC_UI_FLOATING_Y, 1);
        oc_ui_style_set_f32(OC_UI_FLOAT_TARGET_X, rect.x);
        oc_ui_style_set_f32(OC_UI_FLOAT_TARGET_Y, rect.y);
        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, rect.w });
        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, rect.h });

        if(horizontal)
        {
            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_X);
        }
        else
        {
            oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
        }
        oc_ui_axis trackAxis = horizontal ? OC_UI_AXIS_X : OC_UI_AXIS_Y;
        oc_ui_axis secondAxis = (trackAxis == OC_UI_AXIS_Y) ? OC_UI_AXIS_X : OC_UI_AXIS_Y;

        f32 minThumbRatio = 17. / oc_max(rect.w, rect.h);
        thumbRatio = oc_min(oc_max(thumbRatio, minThumbRatio), 1.);
        f32 beforeRatio = (*scrollValue) * (1. - thumbRatio);
        f32 afterRatio = (1. - *scrollValue) * (1. - thumbRatio);

        oc_ui_box("before-spacer")
        {
            oc_ui_style_set_size(OC_UI_WIDTH + trackAxis, (oc_ui_size){ OC_UI_SIZE_PARENT, beforeRatio });
            oc_ui_style_set_size(OC_UI_WIDTH + secondAxis, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
        }

        oc_ui_box* thumb = oc_ui_box("thumb")
        {
            oc_ui_style_set_size(OC_UI_WIDTH + trackAxis, (oc_ui_size){ OC_UI_SIZE_PARENT, thumbRatio });
            oc_ui_style_set_size(OC_UI_WIDTH + secondAxis, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });

            f32 roundness = 0.5 * rect.c[2 + secondAxis];
            oc_ui_style_set_f32(OC_UI_ROUNDNESS, roundness);

            oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_2);

            oc_ui_style_rule(".hover")
            {
                oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_1);
            }
            oc_ui_style_rule(".active")
            {
                oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_FILL_1);
            }
        }

        oc_ui_box("after-spacer")
        {
            oc_ui_style_set_size(OC_UI_WIDTH + trackAxis, (oc_ui_size){ OC_UI_SIZE_PARENT, afterRatio });
            oc_ui_style_set_size(OC_UI_WIDTH + secondAxis, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
        }

        //NOTE: interaction
        oc_ui_sig thumbSig = oc_ui_box_sig(thumb);
        oc_ui_sig trackSig = oc_ui_box_sig(track);

        if(thumbSig.dragging)
        {
            f32 trackExtents = rect.c[2 + trackAxis] * (1. - thumbRatio);
            *scrollValue = (trackSig.mouse.c[trackAxis] - thumb->pressedMouse.c[trackAxis]) / trackExtents;
            *scrollValue = oc_clamp(*scrollValue, 0, 1);
        }

        if(oc_ui_box_active(track))
        {
            //NOTE: activated from outside
            oc_ui_box_set_hot(track, true);
            oc_ui_box_set_hot(thumb, true);
            oc_ui_box_set_active(track, true);
            oc_ui_box_set_active(thumb, true);
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
            oc_ui_box_set_active(track, true);
            oc_ui_box_set_active(thumb, true);
        }
        else if(thumbSig.wheel.c[trackAxis] == 0)
        {
            oc_ui_box_set_active(track, false);
            oc_ui_box_set_active(thumb, false);
        }
    }
    return (track);
}

oc_ui_box* oc_ui_scrollbar(const char* name, oc_rect rect, f32 thumbRatio, f32* scrollValue, bool horizontal)
{
    return oc_ui_scrollbar_str8(OC_STR8(name), rect, thumbRatio, scrollValue, horizontal);
}

oc_ui_box* oc_ui_box_end(void)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_box* box = oc_ui_box_top();
    OC_DEBUG_ASSERT(box, "box stack underflow");

    oc_ui_sig sig = oc_ui_box_sig(box);

    f32 contentsW = oc_clamp_low(box->childrenSum[0] + 2 * box->style.layout.margin.x, box->rect.w);
    contentsW = oc_clamp_low(contentsW, 1);
    bool needsScrollX = contentsW > box->rect.w;

    f32 contentsH = oc_clamp_low(box->childrenSum[1] + 2 * box->style.layout.margin.y, box->rect.h);
    contentsH = oc_clamp_low(contentsH, 1);
    bool needsScrollY = contentsH > box->rect.h;

    if(needsScrollX && box->style.layout.overflow.x == OC_UI_OVERFLOW_SCROLL)
    {
        f32 thumbRatioX = box->rect.w / contentsW;
        f32 scrollValueX = box->scroll.x / (contentsW - box->rect.w);

        oc_ui_box* scrollBarX = oc_ui_scrollbar("scrollerX",
                                                (oc_rect){ 0, box->rect.h - 8, box->rect.w, 8 },
                                                thumbRatioX,
                                                &scrollValueX,
                                                true);

        box->scroll.x = scrollValueX * (contentsW - box->rect.w);

        //wheel
        if(oc_ui_box_hovering(box, oc_ui_mouse_position()))
        {
            oc_vec2 wheel = oc_ui_mouse_wheel();
            box->scroll.x += wheel.x;
            box->scroll.x = oc_clamp(box->scroll.x, 0, contentsW - box->rect.w);
        }

        box->scroll.x = oc_clamp(box->scroll.x, 0, contentsW - box->rect.w);
    }

    if(needsScrollY && box->style.layout.overflow.y == OC_UI_OVERFLOW_SCROLL)
    {
        f32 thumbRatioY = box->rect.h / contentsH;
        f32 scrollValueY = box->scroll.y / (contentsH - box->rect.h);
        f32 spacerSize = needsScrollX ? 10 : 0;

        oc_ui_box* scrollBarY = oc_ui_scrollbar("scrollerY",
                                                (oc_rect){ box->rect.w - 8, 0, 8, box->rect.h - spacerSize },
                                                thumbRatioY,
                                                &scrollValueY,
                                                false);

        box->scroll.y = scrollValueY * (contentsH - box->rect.h);

        //wheel
        if(oc_ui_box_hovering(box, oc_ui_mouse_position()))
        {
            oc_vec2 wheel = oc_ui_mouse_wheel();
            box->scroll.y += wheel.y;
            box->scroll.y = oc_clamp(box->scroll.y, 0, contentsH - box->rect.h);
        }

        box->scroll.y = oc_clamp(box->scroll.y, 0, contentsH - box->rect.h);
    }

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

void oc_ui_box_set_active(oc_ui_box* box, bool active)
{
    box->active = active;
    if(active)
    {
        oc_ui_tag_box_str8(box, OC_STR8_LIT("active"));
    }
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

bool oc_ui_box_focus(oc_ui_box* box)
{
    return box == oc_ui_get_context()->focus;
}

void oc_ui_box_set_focus(oc_ui_box* box, bool focus)
{
    oc_ui_context* ui = oc_ui_get_context();
    if(focus)
    {
        ui->focus = box;
    }
    else if(ui->focus == box)
    {
        ui->focus = 0;
    }
}

oc_ui_sig oc_ui_box_sig(oc_ui_box* box)
{
    return box->sig;
}

bool oc_ui_box_hidden(oc_ui_box* box)
{
    return (box->closed || box->parentClosed);
}

void oc_ui_set_text(oc_str8 text)
{
    oc_ui_context* ui = oc_ui_get_context();
    oc_ui_box* box = oc_ui_box_top();
    if(box)
    {
        box->text = oc_str8_push_copy(&ui->frameArena, text);
    }
    //TODO: else error?
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
    oc_ui_attr_mask mask = box->targetStyle->animationMask;

    if(box->fresh)
    {
        box->style = *targetStyle;
    }
    else
    {
        if(mask & OC_UI_MASK_SIZE_WIDTH)
        {
            oc_ui_animate_oc_ui_size(ui, &box->style.size.c[OC_UI_AXIS_X], targetStyle->size.c[OC_UI_AXIS_X], animationTime);
        }
        else
        {
            box->style.size.c[OC_UI_AXIS_X] = targetStyle->size.c[OC_UI_AXIS_X];
        }

        if(mask & OC_UI_MASK_SIZE_HEIGHT)
        {
            oc_ui_animate_oc_ui_size(ui, &box->style.size.c[OC_UI_AXIS_Y], targetStyle->size.c[OC_UI_AXIS_Y], animationTime);
        }
        else
        {
            box->style.size.c[OC_UI_AXIS_Y] = targetStyle->size.c[OC_UI_AXIS_Y];
        }

        if(mask & OC_UI_MASK_COLOR)
        {
            oc_ui_animate_color(ui, &box->style.color, targetStyle->color, animationTime);
        }
        else
        {
            box->style.color = targetStyle->color;
        }

        if(mask & OC_UI_MASK_BG_COLOR)
        {
            oc_ui_animate_color(ui, &box->style.bgColor, targetStyle->bgColor, animationTime);
        }
        else
        {
            box->style.bgColor = targetStyle->bgColor;
        }

        if(mask & OC_UI_MASK_BORDER_COLOR)
        {
            oc_ui_animate_color(ui, &box->style.borderColor, targetStyle->borderColor, animationTime);
        }
        else
        {
            box->style.borderColor = targetStyle->borderColor;
        }

        if(mask & OC_UI_MASK_FONT_SIZE)
        {
            oc_ui_animate_f32(ui, &box->style.fontSize, targetStyle->fontSize, animationTime);
        }
        else
        {
            box->style.fontSize = targetStyle->fontSize;
        }

        if(mask & OC_UI_MASK_BORDER_SIZE)
        {
            oc_ui_animate_f32(ui, &box->style.borderSize, targetStyle->borderSize, animationTime);
        }
        else
        {
            box->style.borderSize = targetStyle->borderSize;
        }

        if(mask & OC_UI_MASK_ROUNDNESS)
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

bool oc_ui_style_apply_check(oc_ui_attr attr,
                             oc_ui_attr_mask mask,
                             oc_ui_pattern_specificity specificity,
                             oc_ui_pattern_specificity* specArray)
{
    bool result = (mask & oc_ui_attr_to_mask(attr))
               && (specificity.id >= specArray[attr].id
                   || (specificity.id == specArray[attr].id && specificity.tag >= specArray[attr].tag));
    if(result)
    {
        specArray[attr] = specificity;
    }
    return result;
}

void oc_ui_style_apply(oc_ui_style* dst,
                       oc_ui_style* src,
                       oc_ui_attr_mask mask,
                       oc_ui_pattern_specificity specificity,
                       oc_ui_pattern_specificity* specArray)
{
    if(oc_ui_style_apply_check(OC_UI_WIDTH, mask, specificity, specArray))
    {
        dst->size.c[OC_UI_AXIS_X] = src->size.c[OC_UI_AXIS_X];
    }
    if(oc_ui_style_apply_check(OC_UI_HEIGHT, mask, specificity, specArray))
    {
        dst->size.c[OC_UI_AXIS_Y] = src->size.c[OC_UI_AXIS_Y];
    }
    if(oc_ui_style_apply_check(OC_UI_AXIS, mask, specificity, specArray))
    {
        dst->layout.axis = src->layout.axis;
    }
    if(oc_ui_style_apply_check(OC_UI_ALIGN_X, mask, specificity, specArray))
    {
        dst->layout.align.x = src->layout.align.x;
    }
    if(oc_ui_style_apply_check(OC_UI_ALIGN_Y, mask, specificity, specArray))
    {
        dst->layout.align.y = src->layout.align.y;
    }
    if(oc_ui_style_apply_check(OC_UI_SPACING, mask, specificity, specArray))
    {
        dst->layout.spacing = src->layout.spacing;
    }
    if(oc_ui_style_apply_check(OC_UI_MARGIN_X, mask, specificity, specArray))
    {
        dst->layout.margin.x = src->layout.margin.x;
    }
    if(oc_ui_style_apply_check(OC_UI_MARGIN_Y, mask, specificity, specArray))
    {
        dst->layout.margin.y = src->layout.margin.y;
    }
    if(oc_ui_style_apply_check(OC_UI_FLOATING_X, mask, specificity, specArray))
    {
        dst->floating.c[OC_UI_AXIS_X] = src->floating.c[OC_UI_AXIS_X];
    }
    if(oc_ui_style_apply_check(OC_UI_FLOATING_Y, mask, specificity, specArray))
    {
        dst->floating.c[OC_UI_AXIS_Y] = src->floating.c[OC_UI_AXIS_Y];
    }
    if(oc_ui_style_apply_check(OC_UI_FLOAT_TARGET_X, mask, specificity, specArray))
    {
        dst->floatTarget.x = src->floatTarget.x;
    }
    if(oc_ui_style_apply_check(OC_UI_FLOAT_TARGET_Y, mask, specificity, specArray))
    {
        dst->floatTarget.y = src->floatTarget.y;
    }

    if(oc_ui_style_apply_check(OC_UI_COLOR, mask, specificity, specArray))
    {
        dst->color = src->color;
    }
    if(oc_ui_style_apply_check(OC_UI_BG_COLOR, mask, specificity, specArray))
    {
        dst->bgColor = src->bgColor;
    }
    if(oc_ui_style_apply_check(OC_UI_BORDER_COLOR, mask, specificity, specArray))
    {
        dst->borderColor = src->borderColor;
    }
    if(oc_ui_style_apply_check(OC_UI_BORDER_SIZE, mask, specificity, specArray))
    {
        dst->borderSize = src->borderSize;
    }
    if(oc_ui_style_apply_check(OC_UI_ROUNDNESS, mask, specificity, specArray))
    {
        dst->roundness = src->roundness;
    }
    if(oc_ui_style_apply_check(OC_UI_FONT, mask, specificity, specArray))
    {
        dst->font = src->font;
    }
    if(oc_ui_style_apply_check(OC_UI_TEXT_SIZE, mask, specificity, specArray))
    {
        dst->fontSize = src->fontSize;
    }
    if(oc_ui_style_apply_check(OC_UI_ANIMATION_TIME, mask, specificity, specArray))
    {
        dst->animationTime = src->animationTime;
    }
    if(oc_ui_style_apply_check(OC_UI_ANIMATION_MASK, mask, specificity, specArray))
    {
        dst->animationMask = src->animationMask;
    }
}

bool oc_ui_style_selector_match(oc_ui_box* box, oc_ui_style_rule* rule, oc_ui_selector* selector)
{
    bool res = false;

    if(selector->kind == OC_UI_SEL_ID)
    {
        res = !oc_str8_cmp(box->keyString, selector->string);
    }
    else
    {
        oc_list_for(box->tags, elt, oc_ui_tag_elt, listElt)
        {
            if(elt->tag.hash == selector->hash)
            {
                res = true;
                break;
            }
        }
    }

    return (res);
}

oc_ui_style_rule* oc_ui_style_rule_match(oc_ui_context* ui, oc_ui_box* box, oc_ui_style_rule* rule, oc_ui_pattern_specificity* specArray)
{
    oc_ui_selector* selector = oc_list_first_entry(rule->pattern.l, oc_ui_selector, listElt);
    bool match = oc_ui_style_selector_match(box, rule, selector);

    selector = oc_list_next_entry(selector, oc_ui_selector, listElt);
    while(match && selector && selector->op == OC_UI_SEL_AND)
    {
        match = match && oc_ui_style_selector_match(box, rule, selector);
        selector = oc_list_next_entry(selector, oc_ui_selector, listElt);
    }

    oc_ui_style_rule* derived = 0;
    if(match)
    {
        if(!selector)
        {
            oc_ui_style_apply(box->targetStyle,
                              &rule->style,
                              rule->mask,
                              rule->pattern.specificity,
                              specArray);
        }
        else
        {
            //NOTE create derived rule if there's more than one selector
            derived = oc_arena_push_type(&ui->frameArena, oc_ui_style_rule);
            derived->mask = rule->mask;
            derived->style = rule->style;
            derived->pattern.l = (oc_list){ &selector->listElt, rule->pattern.l.last };
            derived->pattern.specificity = rule->pattern.specificity;
        }
    }
    return derived;
}

/*
void oc_ui_styling_prepass(oc_ui_context* ui,
                           oc_ui_box* box,
                           u32 rulesetCount,
                           oc_ui_style_rule** ruleset,
                           u32 derivedCount,
                           oc_ui_style_rule** derivedRules)
{
    oc_arena_scope scratch = oc_scratch_begin();

    //NOTE: form local ruleset from parent ruleset, derivedRules, and box rules
    u32 localRuleCount = rulesetCount + derivedCount + box->ruleCount;
    oc_ui_style_rule** localRules = oc_arena_push_array(scratch.arena, oc_ui_style_rule*, localRuleCount);

    memcpy(localRules, ruleset, rulesetCount * sizeof(oc_ui_style_rule*));
    memcpy(localRules + rulesetCount * sizeof(oc_ui_style_rule*), derivedRules, derivedCount * sizeof(oc_ui_style_rule*));

    u32 index = rulesetCount + derivedCount;
    oc_list_for(box->rules, rule, oc_ui_style_rule, boxElt)
    {
        localRules[index] = rule;
        index++;
    }

    //NOTE: match ruleset, applying style and producing at most as many derived rules
    oc_ui_style_rule** localDerived = oc_arena_push_array(scratch.arena, oc_ui_style_rule*, localRuleCount);
    u32 localDerivedCount = 0;
    for(u32 i = 0; i < localRuleCount; i++)
    {
        oc_ui_style_rule* derived = oc_ui_style_match(scratch.arena, box, rule);
        if(derived)
        {
            localDerived[localDerivedCount] = derived;
            localDerivedCount++;
        }
    }

    //NOTE: recurse in children
    oc_list_for(box->children, child, oc_ui_box, parentElt)
    {
        oc_ui_styling_prepass(ui,
                              child,
                              localRuleCount,
                              localRuleset,
                              localDerivedCount,
                              localRerived);
    }

    oc_scratch_end();
}
*/

void oc_ui_styling_prepass(oc_ui_context* ui, oc_ui_box* box, oc_list* ruleset)
{
    oc_list saveParent = *ruleset;

    //NOTE(martin): add box rules to the ruleset
    oc_list_for(box->rules, rule, oc_ui_style_rule, boxElt)
    {
        oc_list_push_back(ruleset, &rule->rulesetElt);
    }

    //NOTE(martin): match ruleset against box, which may produce derived rules

    oc_arena_scope scratch = oc_scratch_begin();

    oc_ui_pattern_specificity* specArray = oc_arena_push_array(scratch.arena, oc_ui_pattern_specificity, OC_UI_ATTR_COUNT);
    memset(specArray, 0, sizeof(oc_ui_pattern_specificity) * OC_UI_ATTR_COUNT);

    oc_list derivedRules = { 0 };
    oc_list_for(*ruleset, rule, oc_ui_style_rule, rulesetElt)
    {
        oc_ui_style_rule* derived = oc_ui_style_rule_match(ui, box, rule, specArray);
        if(derived)
        {
            oc_list_push_back(&derivedRules, &derived->rulesetElt);
        }
    }

    oc_scratch_end(scratch);

    //NOTE(martin): add derived rules to ruleset and recurse in children
    if(ruleset->last)
    {
        ruleset->last->next = derivedRules.first;
    }
    if(derivedRules.first)
    {
        derivedRules.first->prev = ruleset->last;
    }
    if(derivedRules.last)
    {
        ruleset->last = derivedRules.last;
    }

    oc_list_for(box->children, child, oc_ui_box, listElt)
    {
        oc_ui_styling_prepass(ui, child, ruleset);
    }

    //NOTE(martin): restore ruleset to its previous state
    *ruleset = saveParent;
    if(ruleset->last)
    {
        ruleset->last->next = 0;
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
        textBox = oc_font_text_metrics(style->font, style->fontSize, box->text).logical;
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
    //NOTE: layout children and compute spacing and minimum size
    i32 count = 0;
    f32 minSum = 0;

    oc_list_for(box->children, child, oc_ui_box, listElt)
    {
        if(!oc_ui_box_hidden(child))
        {
            oc_ui_layout_downward_dependent_size(ui, child, axis);

            if(!child->style.floating.c[axis])
            {
                if(box->style.layout.axis == axis)
                {
                    count++;
                    minSum += child->minSize[axis];
                }
                else
                {
                    minSum = oc_max(minSum, child->minSize[axis]);
                }
            }
        }
    }
    box->spacing[axis] = oc_max(0, count - 1) * box->style.layout.spacing;

    switch(box->style.size.c[axis].kind)
    {
        case OC_UI_SIZE_TEXT:
        case OC_UI_SIZE_PIXELS:
            box->minSize[axis] = box->rect.c[2 + axis];
            break;

        case OC_UI_SIZE_CHILDREN:
        case OC_UI_SIZE_PARENT:
        case OC_UI_SIZE_PARENT_MINUS_PIXELS:
        {
            bool constrain = box->style.layout.constrain.c[axis];
            if(constrain)
            {
                box->minSize[axis] = minSum + box->spacing[axis] + 2 * box->style.layout.margin.c[axis];
            }
        }
        break;
    }
    box->minSize[axis] = oc_max(box->minSize[axis], box->style.size.c[axis].minSize);

    oc_ui_size* size = &box->style.size.c[axis];
    if(size->kind == OC_UI_SIZE_CHILDREN)
    {
        //NOTE: if box is dependent on children, compute children's size. If we're in the layout
        //      axis this is the sum of each child size, otherwise it is the maximum child size
        f32 sum = 0;

        if(box->style.layout.axis == axis)
        {
            oc_list_for(box->children, child, oc_ui_box, listElt)
            {
                if(oc_ui_layout_downward_dependency(child, axis))
                {
                    sum += child->rect.c[2 + axis];
                }
            }
        }
        else
        {
            oc_list_for(box->children, child, oc_ui_box, listElt)
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

    oc_list_for(box->children, child, oc_ui_box, listElt)
    {
        oc_ui_size* size = &child->style.size.c[axis];
        if(size->kind == OC_UI_SIZE_PARENT)
        {
            child->rect.c[2 + axis] = oc_max(child->minSize[axis], availableSize * size->value);
        }
        else if(size->kind == OC_UI_SIZE_PARENT_MINUS_PIXELS)
        {
            child->rect.c[2 + axis] = oc_max(child->minSize[axis], oc_max(0, availableSize - size->value));
        }
    }

    //NOTE: solve downward conflicts
    bool constrain = box->style.layout.constrain.c[axis];
    if(constrain)
    {
        if(box->style.layout.axis == axis)
        {
            f32 prevSum = FLT_MAX;
            int count = 0;

            //NOTE: take into account the _original size_ minus the _original slack_ in minimum size. This way the widget
            //      never gives up more than wantedSize * relax.
            oc_list_for(box->children, child, oc_ui_box, listElt)
            {
                if(!oc_ui_box_hidden(child)
                   && !child->style.floating.c[axis])
                {
                    child->minSize[axis] = oc_max(child->minSize[axis], child->rect.c[2 + axis] * (1 - child->style.size.c[axis].relax));
                }
            }

            //NOTE: Loop while we can remove excess. Each iterations reaffects excess to boxes that still have some slack.
            while(1)
            {
                //NOTE: if we're solving in the layout axis, first compute total sum of children and
                //      total slack available
                f32 sum = 0;
                f32 slack = 0;

                oc_list_for(box->children, child, oc_ui_box, listElt)
                {
                    if(!oc_ui_box_hidden(child)
                       && !child->style.floating.c[axis])
                    {
                        sum += child->rect.c[2 + axis];
                        slack += oc_min(child->rect.c[2 + axis] * child->style.size.c[axis].relax,
                                        child->rect.c[2 + axis] - child->minSize[axis]);
                    }
                }
                if(prevSum - sum < 1)
                {
                    break;
                }
                count++;
                prevSum = sum;

                //NOTE: then remove excess proportionally to each box slack
                f32 totalContents = sum + box->spacing[axis] + 2 * box->style.layout.margin.c[axis];
                f32 excess = oc_clamp_low(totalContents - box->rect.c[2 + axis], 0);
                f32 alpha = oc_clamp(excess / slack, 0, 1);

                oc_list_for(box->children, child, oc_ui_box, listElt)
                {
                    if(!oc_ui_box_hidden(child) && !child->style.floating.c[axis])
                    {
                        f32 relax = child->style.size.c[axis].relax;
                        f32 minSize = child->minSize[axis];
                        f32 remove = alpha * oc_clamp(child->rect.c[2 + axis] * relax, 0, child->rect.c[2 + axis] - child->minSize[axis]);

                        child->rect.c[2 + axis] = oc_max(minSize, child->rect.c[2 + axis] - remove);
                    }
                }
            }
        }
        else
        {
            //NOTE: if we're solving on the secondary axis, we remove excess to each box individually
            //      according to its own slack.
            oc_list_for(box->children, child, oc_ui_box, listElt)
            {
                if(!oc_ui_box_hidden(child) && !child->style.floating.c[axis])
                {
                    f32 totalContents = child->rect.c[2 + axis] + 2 * box->style.layout.margin.c[axis];
                    f32 excess = oc_clamp_low(totalContents - box->rect.c[2 + axis], 0);
                    f32 relax = child->style.size.c[axis].relax;
                    f32 minSize = child->minSize[axis];
                    f32 remove = oc_min(excess, child->rect.c[2 + axis] * relax);

                    child->rect.c[2 + axis] = oc_max(minSize, child->rect.c[2 + axis] - remove);
                }
            }
        }
    }
    box->rect.c[2 + axis] = oc_max(box->minSize[axis], box->rect.c[2 + axis]);

    f32 sum = 0;

    //NOTE: recurse in children and recompute children sum
    oc_list_for(box->children, child, oc_ui_box, listElt)
    {
        oc_ui_layout_upward_dependent_size(ui, child, axis);

        if(!oc_ui_box_hidden(child)
           && !child->style.floating.c[axis])
        {
            if(box->style.layout.axis == axis)
            {
                sum += child->rect.c[2 + axis];
            }
            else
            {
                sum = oc_max(sum, child->rect.c[2 + axis]);
            }
        }
    }
    box->childrenSum[axis] = sum;

    OC_ASSERT(box->rect.c[2 + axis] >= box->minSize[axis], "parent->keyString = %.*s, box->keyStrig = %.*s, axis = %i, box->size[axis].kind = %i, box->rect.c[2+axis] = %f, box->minSize[axis] = %f",
              oc_str8_ip(box->parent->keyString),
              oc_str8_ip(box->keyString),
              axis,
              box->style.size.c[axis].kind,
              box->rect.c[2 + axis],
              box->minSize[axis]);
}

void oc_ui_layout_upward_dependent_fixup(oc_ui_context* ui, oc_ui_box* box, int axis)
{
    f32 margin = box->style.layout.margin.c[axis];
    f32 availableSize = oc_max(0, box->rect.c[2 + axis] - box->spacing[axis] - 2 * margin);

    if(axis == box->style.layout.axis)
    {
        f32 availableToParentSized = availableSize;
        f32 relaxSum = 0;

        oc_list_for(box->children, child, oc_ui_box, listElt)
        {
            oc_ui_size* size = &child->style.size.c[axis];
            if(size->kind == OC_UI_SIZE_PARENT || size->kind == OC_UI_SIZE_PARENT_MINUS_PIXELS)
            {
                f32 wantedSize = 0;
                if(size->kind == OC_UI_SIZE_PARENT)
                {
                    wantedSize = availableSize * size->value;
                }
                else
                {
                    wantedSize = availableSize - size->value;
                }
                if(wantedSize > child->rect.c[2 + axis])
                {
                    relaxSum += size->relax;
                }
            }
            availableToParentSized -= child->rect.c[2 + axis];
        }
        availableToParentSized = oc_max(0, availableToParentSized);

        if(availableToParentSized && relaxSum)
        {
            oc_list_for(box->children, child, oc_ui_box, listElt)
            {
                oc_ui_size* size = &child->style.size.c[axis];
                if(size->kind == OC_UI_SIZE_PARENT || size->kind == OC_UI_SIZE_PARENT_MINUS_PIXELS)
                {
                    f32 wantedSize = 0;
                    if(size->kind == OC_UI_SIZE_PARENT)
                    {
                        wantedSize = availableSize * size->value;
                    }
                    else
                    {
                        wantedSize = availableSize - size->value;
                    }

                    if(wantedSize > child->rect.c[2 + axis])
                    {
                        child->rect.c[2 + axis] += availableToParentSized * (size->relax / relaxSum);
                    }
                }
            }
        }
    }

    oc_list_for(box->children, child, oc_ui_box, listElt)
    {
        if(axis != box->style.layout.axis)
        {
            oc_ui_size* size = &child->style.size.c[axis];
            if(size->kind == OC_UI_SIZE_PARENT)
            {
                child->rect.c[2 + axis] = oc_max(child->rect.c[2 + axis], availableSize * size->value);
            }
            else if(size->kind == OC_UI_SIZE_PARENT_MINUS_PIXELS)
            {
                child->rect.c[2 + axis] = oc_max(child->rect.c[2 + axis], oc_max(0, availableSize - size->value));
            }
        }
        oc_ui_layout_upward_dependent_fixup(ui, child, axis);
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

    oc_list_for(box->children, child, oc_ui_box, listElt)
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
                if((child->targetStyle->animationMask & (OC_UI_MASK_FLOATING_X << i))
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
    if(isnan(box->rect.w) || isnan(box->rect.h))
    {
        oc_log_error("error in box %.*s\n", oc_str8_ip(box->keyString));
        OC_ASSERT(0);
    }
}

void oc_ui_layout_find_next_hovered_recursive(oc_ui_context* ui, oc_ui_box* box, oc_vec2 p)
{
    if(oc_ui_box_hidden(box))
    {
        return;
    }

    bool hit = oc_ui_rect_hit(box->rect, p);

    if(hit && !box->style.clickThrough)
    {
        ui->hovered = box;
    }
    if(hit || oc_ui_rect_hit(oc_ui_box_clip_rect(box), p))
    {
        oc_list_for(box->children, child, oc_ui_box, listElt)
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
    //NOTE: style and compute static sizes
    oc_list rules = { 0 };
    oc_ui_styling_prepass(ui, ui->root, &rules);

    //NOTE: reparent overlay boxes
    oc_list_for(ui->overlayList, box, oc_ui_box, overlayElt)
    {
        if(box->parent)
        {
            oc_list_remove(&box->parent->children, &box->listElt);
            oc_list_push_back(&ui->overlay->children, &box->listElt);
        }
    }

    //NOTE: compute layout
    for(int axis = 0; axis < OC_UI_AXIS_COUNT; axis++)
    {
        oc_ui_layout_downward_dependent_size(ui, ui->root, axis);
        oc_ui_layout_upward_dependent_size(ui, ui->root, axis);

        //        oc_ui_layout_upward_dependent_fixup(ui, ui->root, axis);
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

    //NOTE: detect if box is completely outside current graphics clip
    bool draw = true;
    {
        oc_rect clipRect = oc_clip_top();
        oc_rect expRect = {
            box->rect.x - 0.5 * style->borderSize,
            box->rect.y - 0.5 * style->borderSize,
            box->rect.w + style->borderSize,
            box->rect.h + style->borderSize
        };

        if((expRect.x + expRect.w < clipRect.x)
           || (expRect.y + expRect.h < clipRect.y)
           || (expRect.x > clipRect.x + clipRect.w)
           || (expRect.y > clipRect.y + clipRect.h))
        {
            draw = false;
        }
    }

    {
        //NOTE: push clip rect
        oc_rect clipRect = oc_ui_box_clip_rect(box);

        if(box->style.layout.overflow.x != OC_UI_OVERFLOW_ALLOW
           || box->style.layout.overflow.y != OC_UI_OVERFLOW_ALLOW)
        {
            oc_clip_push(clipRect.x, clipRect.y, clipRect.w, clipRect.h);
        }
    }

    if(draw)
    {
        //NOTE: background
        if(!(style->drawMask & OC_UI_DRAW_MASK_BACKGROUND)
           && style->bgColor.a != 0)
        {
            oc_set_color(style->bgColor);
            oc_ui_rectangle_fill(box->rect, style->roundness);
        }

        //NOTE: custom proc
        if(!(style->drawMask & OC_UI_DRAW_MASK_PROC)
           && box->drawProc)
        {
            box->drawProc(box, box->drawData);
        }
    }

    //NOTE: draw children
    oc_list_for(box->children, child, oc_ui_box, listElt)
    {
        oc_ui_draw_box(child);
    }

    //NOTE: draw text
    if(draw
       && !(style->drawMask & OC_UI_DRAW_MASK_TEXT)
       && box->text.len
       && !oc_font_is_nil(style->font)
       && style->fontSize > 0)
    {
        oc_rect textBox = oc_font_text_metrics(style->font, style->fontSize, box->text).logical;

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
        oc_text_outlines(box->text);
        oc_fill();
    }

    if(box->style.layout.overflow.x != OC_UI_OVERFLOW_ALLOW
       || box->style.layout.overflow.y != OC_UI_OVERFLOW_ALLOW)
    {
        oc_clip_pop();
    }

    //NOTE: draw border
    if(draw
       && !(style->drawMask & OC_UI_DRAW_MASK_BORDER)
       && style->borderSize != 0
       && style->borderColor.a != 0)
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

void oc_ui_begin_frame(oc_vec2 size)
{
    oc_ui_context* ui = oc_ui_get_context();

    ui->frameCounter++;
    f64 time = oc_clock_time(OC_CLOCK_MONOTONIC);
    ui->lastFrameDuration = time - ui->frameTime;
    ui->frameTime = time;

    ui->clipStack = 0;
    ui->z = 0;

    ui->nextBoxTags = (oc_list){ 0 };
    ui->overlayList = (oc_list){ 0 };

    ui->styleVariables.mask = (4 << 10) - 1;
    ui->styleVariables.buckets = oc_arena_push_array(&ui->frameArena, oc_list, 4 << 10);
    //TODO: we could avoid this with a framecounter for each bucket
    memset(ui->styleVariables.buckets, 0, sizeof(oc_list) * (4 << 10));

    ui->root = oc_ui_box_begin("_root_");

    oc_ui_style_theme_dark();

    oc_ui_style_set_var_str8(OC_UI_BG_COLOR, OC_UI_THEME_BG_0);
    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PIXELS, size.x });
    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PIXELS, size.y });

    oc_ui_box_begin("_contents_");

    oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
    oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
    oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
    oc_ui_style_set_i32(OC_UI_ALIGN_X, OC_UI_ALIGN_START);
    oc_ui_style_set_i32(OC_UI_ALIGN_Y, OC_UI_ALIGN_START);
    oc_ui_style_set_i32(OC_UI_FLOATING_X, 1);
    oc_ui_style_set_i32(OC_UI_FLOATING_Y, 1);
    oc_ui_style_set_f32(OC_UI_FLOAT_TARGET_X, 0);
    oc_ui_style_set_f32(OC_UI_FLOAT_TARGET_Y, 0);
}

void oc_ui_end_frame(void)
{
    oc_ui_context* ui = oc_ui_get_context();

    oc_ui_box_pop(); // _contents_

    ui->overlay = oc_ui_box("_overlay_")
    {
        oc_ui_style_set_i32(OC_UI_CLICK_THROUGH, 1);

        oc_ui_style_set_size(OC_UI_WIDTH, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
        oc_ui_style_set_size(OC_UI_HEIGHT, (oc_ui_size){ OC_UI_SIZE_PARENT, 1 });
        oc_ui_style_set_i32(OC_UI_AXIS, OC_UI_AXIS_Y);
        oc_ui_style_set_i32(OC_UI_ALIGN_X, OC_UI_ALIGN_START);
        oc_ui_style_set_i32(OC_UI_ALIGN_Y, OC_UI_ALIGN_START);
        oc_ui_style_set_i32(OC_UI_FLOATING_X, 1);
        oc_ui_style_set_i32(OC_UI_FLOATING_Y, 1);
        oc_ui_style_set_f32(OC_UI_FLOAT_TARGET_X, 0);
        oc_ui_style_set_f32(OC_UI_FLOAT_TARGET_Y, 0);
    }

    oc_ui_box* box = oc_ui_box_end(); // root
    OC_DEBUG_ASSERT(box == ui->root, "unbalanced box stack");

    //TODO: check balancing of style stacks

    //NOTE: layout
    oc_ui_solve_layout(ui);

    //NOTE: prune unused boxes
    for(int i = 0; i < OC_UI_BOX_MAP_BUCKET_COUNT; i++)
    {
        oc_list_for_safe(ui->boxMap[i], box, oc_ui_box, bucketElt)
        {
            if(box->frameCounter < ui->frameCounter)
            {
                oc_list_remove(&ui->boxMap[i], &box->bucketElt);
            }
        }
    }

    oc_arena_clear(&ui->frameArena);
    oc_input_next_frame(&ui->input);
}
