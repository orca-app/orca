/************************************************************//**
*
*	@file: ui.c
*	@author: Martin Fouilleul
*	@date: 08/08/2022
*	@revision:
*
*****************************************************************/
#include"platform/platform.h"
#include"platform/platform_assert.h"
#include"platform/platform_clock.h"
#include"util/memory.h"
#include"util/hash.h"
#include"ui.h"

static ui_style UI_STYLE_DEFAULTS =
{
	.size.width = {.kind = UI_SIZE_CHILDREN,
	               .value = 0,
	               .relax = 0},
	.size.height = {.kind = UI_SIZE_CHILDREN,
	                .value = 0,
	                .relax = 0},

	.layout = {.axis = UI_AXIS_Y,
	           .align = {UI_ALIGN_START,
	                     UI_ALIGN_START}},
	.color = {0, 0, 0, 1},
	.fontSize = 16,
};

mp_thread_local ui_context __uiThreadContext = {0};
mp_thread_local ui_context* __uiCurrentContext = 0;

ui_context* ui_get_context(void)
{
	return(__uiCurrentContext);
}

void ui_set_context(ui_context* context)
{
	__uiCurrentContext = context;
}

//-----------------------------------------------------------------------------
// stacks
//-----------------------------------------------------------------------------
ui_stack_elt* ui_stack_push(ui_context* ui, ui_stack_elt** stack)
{
	ui_stack_elt* elt = mem_arena_alloc_type(&ui->frameArena, ui_stack_elt);
	memset(elt, 0, sizeof(ui_stack_elt));
	elt->parent = *stack;
	*stack = elt;
	return(elt);
}

void ui_stack_pop(ui_stack_elt** stack)
{
	if(*stack)
	{
		*stack = (*stack)->parent;
	}
	else
	{
		log_error("ui stack underflow\n");
	}
}

mp_rect ui_intersect_rects(mp_rect lhs, mp_rect rhs)
{
	//NOTE(martin): intersect with current clip
	f32 x0 = maximum(lhs.x, rhs.x);
	f32 y0 = maximum(lhs.y, rhs.y);
	f32 x1 = minimum(lhs.x + lhs.w, rhs.x + rhs.w);
	f32 y1 = minimum(lhs.y + lhs.h, rhs.y + rhs.h);
	mp_rect r = {x0, y0, maximum(0, x1-x0), maximum(0, y1-y0)};
	return(r);
}

mp_rect ui_clip_top(void)
{
	mp_rect r = {-FLT_MAX/2, -FLT_MAX/2, FLT_MAX, FLT_MAX};
	ui_context* ui = ui_get_context();
	ui_stack_elt* elt = ui->clipStack;
	if(elt)
	{
		r = elt->clip;
	}
	return(r);
}

void ui_clip_push(mp_rect clip)
{
	ui_context* ui = ui_get_context();
	mp_rect current = ui_clip_top();
	ui_stack_elt* elt = ui_stack_push(ui, &ui->clipStack);
	elt->clip = ui_intersect_rects(current, clip);
}

void ui_clip_pop(void)
{
	ui_context* ui = ui_get_context();
	ui_stack_pop(&ui->clipStack);
}

ui_box* ui_box_top(void)
{
	ui_context* ui = ui_get_context();
	ui_stack_elt* elt = ui->boxStack;
	ui_box* box = elt ? elt->box : 0;
	return(box);
}

void ui_box_push(ui_box* box)
{
	ui_context* ui = ui_get_context();
	ui_stack_elt* elt = ui_stack_push(ui, &ui->boxStack);
	elt->box = box;
	if(box->flags & UI_FLAG_CLIP)
	{
		ui_clip_push(box->rect);
	}
}

void ui_box_pop(void)
{
	ui_context* ui = ui_get_context();
	ui_box* box = ui_box_top();
	if(box)
	{
		if(box->flags & UI_FLAG_CLIP)
		{
			ui_clip_pop();
		}
		ui_stack_pop(&ui->boxStack);
	}
}

//-----------------------------------------------------------------------------
// tagging
//-----------------------------------------------------------------------------

ui_tag ui_tag_make_str8(str8 string)
{
	ui_tag tag = {.hash = mp_hash_xx64_string(string)};
	return(tag);
}

void ui_tag_box_str8(ui_box* box, str8 string)
{
	ui_context* ui = ui_get_context();
	ui_tag_elt* elt = mem_arena_alloc_type(&ui->frameArena, ui_tag_elt);
	elt->tag = ui_tag_make_str8(string);
	list_append(&box->tags, &elt->listElt);
}

void ui_tag_next_str8(str8 string)
{
	ui_context* ui = ui_get_context();
	ui_tag_elt* elt = mem_arena_alloc_type(&ui->frameArena, ui_tag_elt);
	elt->tag = ui_tag_make_str8(string);
	list_append(&ui->nextBoxTags, &elt->listElt);
}

//-----------------------------------------------------------------------------
// key hashing and caching
//-----------------------------------------------------------------------------
ui_key ui_key_make_str8(str8 string)
{
	ui_context* ui = ui_get_context();
	u64 seed = 0;
	ui_box* parent = ui_box_top();
	if(parent)
	{
		seed = parent->key.hash;
	}

	ui_key key = {0};
	key.hash = mp_hash_xx64_string_seed(string, seed);
	return(key);
}

ui_key ui_key_make_path(str8_list path)
{
	ui_context* ui = ui_get_context();
	u64 seed = 0;
	ui_box* parent = ui_box_top();
	if(parent)
	{
		seed = parent->key.hash;
	}
	for_list(&path.list, elt, str8_elt, listElt)
	{
		seed = mp_hash_xx64_string_seed(elt->string, seed);
	}
	ui_key key = {seed};
	return(key);
}

bool ui_key_equal(ui_key a, ui_key b)
{
	return(a.hash == b.hash);
}

void ui_box_cache(ui_context* ui, ui_box* box)
{
	u64 index = box->key.hash & (UI_BOX_MAP_BUCKET_COUNT-1);
	list_append(&(ui->boxMap[index]), &box->bucketElt);
}

ui_box* ui_box_lookup_key(ui_key key)
{
	ui_context* ui = ui_get_context();
	u64 index = key.hash & (UI_BOX_MAP_BUCKET_COUNT-1);

	for_list(&ui->boxMap[index], box, ui_box, bucketElt)
	{
		if(ui_key_equal(key, box->key))
		{
			return(box);
		}
	}
	return(0);
}

ui_box* ui_box_lookup_str8(str8 string)
{
	ui_key key = ui_key_make_str8(string);
	return(ui_box_lookup_key(key));
}

//-----------------------------------------------------------------------------
// styling
//-----------------------------------------------------------------------------

void ui_pattern_push(mem_arena* arena, ui_pattern* pattern, ui_selector selector)
{
	ui_selector* copy = mem_arena_alloc_type(arena, ui_selector);
	*copy = selector;
	list_append(&pattern->l, &copy->listElt);
}

ui_pattern ui_pattern_all(void)
{
	ui_context* ui = ui_get_context();
	ui_pattern pattern = {0};
	ui_pattern_push(&ui->frameArena, &pattern, (ui_selector){.kind = UI_SEL_ANY});
	return(pattern);
}

ui_pattern ui_pattern_owner(void)
{
	ui_context* ui = ui_get_context();
	ui_pattern pattern = {0};
	ui_pattern_push(&ui->frameArena, &pattern, (ui_selector){.kind = UI_SEL_OWNER});
	return(pattern);
}

void ui_style_match_before(ui_pattern pattern, ui_style* style, ui_style_mask mask)
{
	ui_context* ui = ui_get_context();
	if(ui)
	{
		ui_style_rule* rule = mem_arena_alloc_type(&ui->frameArena, ui_style_rule);
		rule->pattern = pattern;
		rule->mask = mask;
		rule->style = mem_arena_alloc_type(&ui->frameArena, ui_style);
		*rule->style = *style;

		list_append(&ui->nextBoxBeforeRules, &rule->boxElt);
	}
}

void ui_style_match_after(ui_pattern pattern, ui_style* style, ui_style_mask mask)
{
	ui_context* ui = ui_get_context();
	if(ui)
	{
		ui_style_rule* rule = mem_arena_alloc_type(&ui->frameArena, ui_style_rule);
		rule->pattern = pattern;
		rule->mask = mask;
		rule->style = mem_arena_alloc_type(&ui->frameArena, ui_style);
		*rule->style = *style;

		list_append(&ui->nextBoxAfterRules, &rule->boxElt);
	}
}

void ui_style_next(ui_style* style, ui_style_mask mask)
{
	ui_style_match_before(ui_pattern_owner(), style, mask);
}

void ui_style_box_before(ui_box* box, ui_pattern pattern, ui_style* style, ui_style_mask mask)
{
	ui_context* ui = ui_get_context();
	if(ui)
	{
		ui_style_rule* rule = mem_arena_alloc_type(&ui->frameArena, ui_style_rule);
		rule->pattern = pattern;
		rule->mask = mask;
		rule->style = mem_arena_alloc_type(&ui->frameArena, ui_style);
		*rule->style = *style;

		list_append(&box->beforeRules, &rule->boxElt);
		rule->owner = box;
	}
}

void ui_style_box_after(ui_box* box, ui_pattern pattern, ui_style* style, ui_style_mask mask)
{
	ui_context* ui = ui_get_context();
	if(ui)
	{
		ui_style_rule* rule = mem_arena_alloc_type(&ui->frameArena, ui_style_rule);
		rule->pattern = pattern;
		rule->mask = mask;
		rule->style = mem_arena_alloc_type(&ui->frameArena, ui_style);
		*rule->style = *style;

		list_append(&box->afterRules, &rule->boxElt);
		rule->owner = box;
	}
}

//-----------------------------------------------------------------------------
// input
//-----------------------------------------------------------------------------

void ui_process_event(mp_event* event)
{
	ui_context* ui = ui_get_context();
	mp_input_process_event(&ui->input, event);
}

vec2 ui_mouse_position(void)
{
	ui_context* ui = ui_get_context();
	vec2 mousePos = mp_mouse_position(&ui->input);
	return(mousePos);
}

vec2 ui_mouse_delta(void)
{
	ui_context* ui = ui_get_context();
	vec2 delta = mp_mouse_delta(&ui->input);
	return(delta);
}

vec2 ui_mouse_wheel(void)
{
	ui_context* ui = ui_get_context();
	vec2 delta = mp_mouse_wheel(&ui->input);
	return(delta);
}

//-----------------------------------------------------------------------------
// ui boxes
//-----------------------------------------------------------------------------

bool ui_rect_hit(mp_rect r, vec2 p)
{
	return( (p.x > r.x)
	      &&(p.x < r.x + r.w)
	      &&(p.y > r.y)
	      &&(p.y < r.y + r.h));
}

bool ui_box_hovering(ui_box* box, vec2 p)
{
	ui_context* ui = ui_get_context();

	mp_rect clip = ui_clip_top();
	mp_rect rect = ui_intersect_rects(clip, box->rect);
	bool hit = ui_rect_hit(rect, p);
	bool result = hit && (!ui->hovered || box->z >= ui->hovered->z);
	return(result);
}

ui_box* ui_box_make_str8(str8 string, ui_flags flags)
{
	ui_context* ui = ui_get_context();

	ui_key key = ui_key_make_str8(string);
	ui_box* box = ui_box_lookup_key(key);

	if(!box)
	{
		box = mem_pool_alloc_type(&ui->boxPool, ui_box);
		memset(box, 0, sizeof(ui_box));

		box->key = key;
		box->fresh = true;
		ui_box_cache(ui, box);
	}
	else
	{
		box->fresh = false;
	}

	//NOTE: setup hierarchy
	if(box->frameCounter != ui->frameCounter)
	{
		list_init(&box->children);
		box->parent = ui_box_top();
		if(box->parent)
		{
			list_append(&box->parent->children, &box->listElt);
			box->parentClosed = box->parent->closed || box->parent->parentClosed;
		}

		if(box->flags & UI_FLAG_OVERLAY)
		{
			list_append(&ui->overlayList, &box->overlayElt);
		}
	}
	else
	{
		//maybe this should be a warning that we're trying to make the box twice in the same frame?
		log_warning("trying to make ui box '%.*s' multiple times in the same frame\n", (int)box->string.len, box->string.ptr);
	}

	//NOTE: setup per-frame state
	box->frameCounter = ui->frameCounter;
	box->string = str8_push_copy(&ui->frameArena, string);
	box->flags = flags;

	//NOTE: create style and setup non-inherited attributes to default values
	box->targetStyle = mem_arena_alloc_type(&ui->frameArena, ui_style);
	ui_apply_style_with_mask(box->targetStyle, &UI_STYLE_DEFAULTS, ~0ULL);

	//NOTE: set tags, before rules and last box
	box->tags = ui->nextBoxTags;
	ui->nextBoxTags = (list_info){0};

	box->beforeRules = ui->nextBoxBeforeRules;
	for_list(&box->beforeRules, rule, ui_style_rule, boxElt)
	{
		rule->owner = box;
	}
	ui->nextBoxBeforeRules = (list_info){0};

	box->afterRules = ui->nextBoxAfterRules;
	for_list(&box->afterRules, rule, ui_style_rule, boxElt)
	{
		rule->owner = box;
	}
	ui->nextBoxAfterRules = (list_info){0};


	//NOTE: set scroll
	if(ui_box_hovering(box, ui_mouse_position()))
	{
		vec2 wheel = ui_mouse_wheel();
		if(box->flags & UI_FLAG_SCROLL_WHEEL_X)
		{
			box->scroll.x += wheel.x;
		}
		if(box->flags & UI_FLAG_SCROLL_WHEEL_Y)
		{
			box->scroll.y += wheel.y;
		}
	}
	return(box);
}

ui_box* ui_box_begin_str8(str8 string, ui_flags flags)
{
	ui_context* ui = ui_get_context();
	ui_box* box = ui_box_make_str8(string, flags);
	ui_box_push(box);
	return(box);
}

ui_box* ui_box_end(void)
{
	ui_context* ui = ui_get_context();
	ui_box* box = ui_box_top();
	DEBUG_ASSERT(box, "box stack underflow");

	ui_box_pop();

	return(box);
}

void ui_box_set_draw_proc(ui_box* box, ui_box_draw_proc proc, void* data)
{
	box->drawProc = proc;
	box->drawData = data;
}

void ui_box_set_closed(ui_box* box, bool closed)
{
	box->closed = closed;
}

bool ui_box_closed(ui_box* box)
{
	return(box->closed);
}

void ui_box_activate(ui_box* box)
{
	box->active = true;
}

void ui_box_deactivate(ui_box* box)
{
	box->active = false;
}

bool ui_box_active(ui_box* box)
{
	return(box->active);
}

void ui_box_set_hot(ui_box* box, bool hot)
{
	box->hot = hot;
}

bool ui_box_hot(ui_box* box)
{
	return(box->hot);
}

ui_sig ui_box_sig(ui_box* box)
{
	//NOTE: compute input signals
	ui_sig sig = {0};

	ui_context* ui = ui_get_context();
	mp_input_state* input = &ui->input;

	sig.box = box;

	if(!box->closed && !box->parentClosed)
	{
		vec2 mousePos = ui_mouse_position();

		sig.hovering = ui_box_hovering(box, mousePos);

		if(box->flags & UI_FLAG_CLICKABLE)
		{
			if(sig.hovering)
			{
				sig.pressed = mp_mouse_pressed(input, MP_MOUSE_LEFT);
				if(sig.pressed)
				{
					box->dragging = true;
				}
				sig.doubleClicked = mp_mouse_double_clicked(input, MP_MOUSE_LEFT);
				sig.rightPressed = mp_mouse_pressed(input, MP_MOUSE_RIGHT);
			}

			sig.released = mp_mouse_released(input, MP_MOUSE_LEFT);
			if(sig.released)
			{
				if(box->dragging && sig.hovering)
				{
					sig.clicked = true;
				}
			}

			if(!mp_mouse_down(input, MP_MOUSE_LEFT))
			{
				box->dragging = false;
			}

			sig.dragging = box->dragging;
		}

		sig.mouse = (vec2){mousePos.x - box->rect.x, mousePos.y - box->rect.y};
		sig.delta = ui_mouse_delta();
		sig.wheel = ui_mouse_wheel();
	}
	return(sig);
}

bool ui_box_hidden(ui_box* box)
{
	return(box->closed || box->parentClosed);
}

//-----------------------------------------------------------------------------
// Auto-layout
//-----------------------------------------------------------------------------

void ui_animate_f32(ui_context* ui, f32* value, f32 target, f32 animationTime)
{
	if(  animationTime < 1e-6
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
		f32 alpha = 3/animationTime;
		f32 dt = ui->lastFrameDuration;

		*value += (target - *value)*alpha*dt;
	}
}

void ui_animate_color(ui_context* ui, mg_color* color, mg_color target, f32 animationTime)
{
	for(int i=0; i<4; i++)
	{
		ui_animate_f32(ui, &color->c[i], target.c[i], animationTime);
	}
}

void ui_animate_ui_size(ui_context* ui, ui_size* size, ui_size target, f32 animationTime)
{
	size->kind = target.kind;
	ui_animate_f32(ui, &size->value, target.value, animationTime);
	ui_animate_f32(ui, &size->relax, target.relax, animationTime);
}

void ui_box_animate_style(ui_context* ui, ui_box* box)
{
	ui_style* targetStyle = box->targetStyle;
	DEBUG_ASSERT(targetStyle);

	f32 animationTime = targetStyle->animationTime;

	//NOTE: interpolate based on transition values
	ui_style_mask mask = box->targetStyle->animationMask;

	if(box->fresh)
	{
		box->style = *targetStyle;
	}
	else
	{
		if(mask & UI_STYLE_SIZE_WIDTH)
		{
			ui_animate_ui_size(ui, &box->style.size.c[UI_AXIS_X], targetStyle->size.c[UI_AXIS_X], animationTime);
		}
		else
		{
			box->style.size.c[UI_AXIS_X] = targetStyle->size.c[UI_AXIS_X];
		}

		if(mask & UI_STYLE_SIZE_HEIGHT)
		{
			ui_animate_ui_size(ui, &box->style.size.c[UI_AXIS_Y], targetStyle->size.c[UI_AXIS_Y], animationTime);
		}
		else
		{
			box->style.size.c[UI_AXIS_Y] = targetStyle->size.c[UI_AXIS_Y];
		}

		if(mask & UI_STYLE_COLOR)
		{
			ui_animate_color(ui, &box->style.color, targetStyle->color, animationTime);
		}
		else
		{
			box->style.color = targetStyle->color;
		}


		if(mask & UI_STYLE_BG_COLOR)
		{
			ui_animate_color(ui, &box->style.bgColor, targetStyle->bgColor, animationTime);
		}
		else
		{
			box->style.bgColor = targetStyle->bgColor;
		}

		if(mask & UI_STYLE_BORDER_COLOR)
		{
			ui_animate_color(ui, &box->style.borderColor, targetStyle->borderColor, animationTime);
		}
		else
		{
			box->style.borderColor = targetStyle->borderColor;
		}

		if(mask & UI_STYLE_FONT_SIZE)
		{
			ui_animate_f32(ui, &box->style.fontSize, targetStyle->fontSize, animationTime);
		}
		else
		{
			box->style.fontSize = targetStyle->fontSize;
		}

		if(mask & UI_STYLE_BORDER_SIZE)
		{
			ui_animate_f32(ui, &box->style.borderSize, targetStyle->borderSize, animationTime);
		}
		else
		{
			box->style.borderSize = targetStyle->borderSize;
		}

		if(mask & UI_STYLE_ROUNDNESS)
		{
			ui_animate_f32(ui, &box->style.roundness, targetStyle->roundness, animationTime);
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

void ui_apply_style_with_mask(ui_style* dst, ui_style* src, ui_style_mask mask)
{
	if(mask & UI_STYLE_SIZE_WIDTH)
	{
		dst->size.c[UI_AXIS_X] = src->size.c[UI_AXIS_X];
	}
	if(mask & UI_STYLE_SIZE_HEIGHT)
	{
		dst->size.c[UI_AXIS_Y] = src->size.c[UI_AXIS_Y];
	}
	if(mask & UI_STYLE_LAYOUT_AXIS)
	{
		dst->layout.axis = src->layout.axis;
	}
	if(mask & UI_STYLE_LAYOUT_ALIGN_X)
	{
		dst->layout.align.x = src->layout.align.x;
	}
	if(mask & UI_STYLE_LAYOUT_ALIGN_Y)
	{
		dst->layout.align.y = src->layout.align.y;
	}
	if(mask & UI_STYLE_LAYOUT_SPACING)
	{
		dst->layout.spacing = src->layout.spacing;
	}
	if(mask & UI_STYLE_LAYOUT_MARGIN_X)
	{
		dst->layout.margin.x = src->layout.margin.x;
	}
	if(mask & UI_STYLE_LAYOUT_MARGIN_Y)
	{
		dst->layout.margin.y = src->layout.margin.y;
	}
	if(mask & UI_STYLE_FLOAT_X)
	{
		dst->floating.c[UI_AXIS_X] = src->floating.c[UI_AXIS_X];
		dst->floatTarget.x = src->floatTarget.x;
	}
	if(mask & UI_STYLE_FLOAT_Y)
	{
		dst->floating.c[UI_AXIS_Y] = src->floating.c[UI_AXIS_Y];
		dst->floatTarget.y = src->floatTarget.y;
	}
	if(mask & UI_STYLE_COLOR)
	{
		dst->color = src->color;
	}
	if(mask & UI_STYLE_BG_COLOR)
	{
		dst->bgColor = src->bgColor;
	}
	if(mask & UI_STYLE_BORDER_COLOR)
	{
		dst->borderColor = src->borderColor;
	}
	if(mask & UI_STYLE_BORDER_SIZE)
	{
		dst->borderSize = src->borderSize;
	}
	if(mask & UI_STYLE_ROUNDNESS)
	{
		dst->roundness = src->roundness;
	}
	if(mask & UI_STYLE_FONT)
	{
		dst->font = src->font;
	}
	if(mask & UI_STYLE_FONT_SIZE)
	{
		dst->fontSize = src->fontSize;
	}
	if(mask & UI_STYLE_ANIMATION_TIME)
	{
		dst->animationTime = src->animationTime;
	}
	if(mask & UI_STYLE_ANIMATION_MASK)
	{
		dst->animationMask = src->animationMask;
	}
}


bool ui_style_selector_match(ui_box* box, ui_style_rule* rule, ui_selector* selector)
{
	bool res = false;
	switch(selector->kind)
	{
		case UI_SEL_ANY:
			res = true;
			break;

		case UI_SEL_OWNER:
			res = (box == rule->owner);
			break;

		case UI_SEL_TEXT:
			res = !str8_cmp(box->string, selector->text);
			break;

		case UI_SEL_TAG:
		{
			for_list(&box->tags, elt, ui_tag_elt, listElt)
			{
				if(elt->tag.hash == selector->tag.hash)
				{
					res = true;
					break;
				}
			}
		} break;

		case UI_SEL_STATUS:
		{
			res = true;
			if(selector->status & UI_HOVER)
			{
				res = res && ui_box_hovering(box, ui_mouse_position());
			}
			if(selector->status & UI_ACTIVE)
			{
				res = res && box->active;
			}
			if(selector->status & UI_DRAGGING)
			{
				res = res && box->dragging;
			}
		} break;

		case UI_SEL_KEY:
			res = ui_key_equal(box->key, selector->key);
		default:
			break;
	}
	return(res);
}

void ui_style_rule_match(ui_context* ui, ui_box* box, ui_style_rule* rule, list_info* buildList, list_info* tmpList)
{
	ui_selector* selector = list_first_entry(&rule->pattern.l, ui_selector, listElt);
	bool match = ui_style_selector_match(box, rule, selector);

	selector = list_next_entry(&rule->pattern.l, selector, ui_selector, listElt);
	while(match && selector && selector->op == UI_SEL_AND)
	{
		match = match && ui_style_selector_match(box, rule, selector);
		selector = list_next_entry(&rule->pattern.l, selector, ui_selector, listElt);
	}

	if(match)
	{
		if(!selector)
		{
			ui_apply_style_with_mask(box->targetStyle, rule->style, rule->mask);
		}
		else
		{
			//NOTE create derived rule if there's more than one selector
			ui_style_rule* derived = mem_arena_alloc_type(&ui->frameArena, ui_style_rule);
			derived->mask = rule->mask;
			derived->style = rule->style;
			derived->pattern.l = (list_info){&selector->listElt, rule->pattern.l.last};

			list_append(buildList, &derived->buildElt);
			list_append(tmpList, &derived->tmpElt);
		}
	}
}

void ui_styling_prepass(ui_context* ui, ui_box* box, list_info* before, list_info* after)
{
	//NOTE: inherit style from parent
	if(box->parent)
	{
		ui_apply_style_with_mask(box->targetStyle,
		                         box->parent->targetStyle,
		                         UI_STYLE_MASK_INHERITED);
	}


	//NOTE: append box before rules to before and tmp
	list_info tmpBefore = {0};
	for_list(&box->beforeRules, rule, ui_style_rule, boxElt)
	{
		list_append(before, &rule->buildElt);
		list_append(&tmpBefore, &rule->tmpElt);
	}
	//NOTE: match before rules
	for_list(before, rule, ui_style_rule, buildElt)
	{
		ui_style_rule_match(ui, box, rule, before, &tmpBefore);
	}

	//NOTE: prepend box after rules to after and append them to tmp
	list_info tmpAfter = {0};
	for_list_reverse(&box->afterRules, rule, ui_style_rule, boxElt)
	{
		list_push(after, &rule->buildElt);
		list_append(&tmpAfter, &rule->tmpElt);
	}

	//NOTE: match after rules
	for_list(after, rule, ui_style_rule, buildElt)
	{
		ui_style_rule_match(ui, box, rule, after, &tmpAfter);
	}

	//NOTE: compute static sizes
	ui_box_animate_style(ui, box);

	if(ui_box_hidden(box))
	{
		return;
	}

	ui_style* style = &box->style;

	mp_rect textBox = {0};
	ui_size desiredSize[2] = {box->style.size.c[UI_AXIS_X],
	                          box->style.size.c[UI_AXIS_Y]};

	if( desiredSize[UI_AXIS_X].kind == UI_SIZE_TEXT
	  ||desiredSize[UI_AXIS_Y].kind == UI_SIZE_TEXT)
	{
		textBox = mg_text_bounding_box(style->font, style->fontSize, box->string);
	}

	for(int i=0; i<UI_AXIS_COUNT; i++)
	{
		ui_size size = desiredSize[i];

		if(size.kind == UI_SIZE_TEXT)
		{
			f32 margin = style->layout.margin.c[i];
			box->rect.c[2+i] = textBox.c[2+i] + margin*2;
		}
		else if(size.kind == UI_SIZE_PIXELS)
		{
			box->rect.c[2+i] = size.value;
		}
	}

	//NOTE: descend in children
	for_list(&box->children, child, ui_box, listElt)
	{
		ui_styling_prepass(ui, child, before, after);
	}

	//NOTE: remove temporary rules
	for_list(&tmpBefore, rule, ui_style_rule, tmpElt)
	{
		list_remove(before, &rule->buildElt);
	}
	for_list(&tmpAfter, rule, ui_style_rule, tmpElt)
	{
		list_remove(after, &rule->buildElt);
	}
}

bool ui_layout_downward_dependency(ui_box* child, int axis)
{
	return(   !ui_box_hidden(child)
	       && !child->style.floating.c[axis]
	       && child->style.size.c[axis].kind != UI_SIZE_PARENT
	       && child->style.size.c[axis].kind != UI_SIZE_PARENT_MINUS_PIXELS);
}

void ui_layout_downward_dependent_size(ui_context* ui, ui_box* box, int axis)
{
	//NOTE: layout children and compute spacing
	f32 count = 0;
	for_list(&box->children, child, ui_box, listElt)
	{
		if(!ui_box_hidden(child))
		{
			ui_layout_downward_dependent_size(ui, child, axis);

			if(  box->style.layout.axis == axis
				 && !child->style.floating.c[axis])
			{
				count++;
			}
		}
	}
	box->spacing[axis] = maximum(0, count-1)*box->style.layout.spacing;

	ui_size* size = &box->style.size.c[axis];
	if(size->kind == UI_SIZE_CHILDREN)
	{
		//NOTE: if box is dependent on children, compute children's size. If we're in the layout
		//      axis this is the sum of each child size, otherwise it is the maximum child size
		f32 sum = 0;

		if(box->style.layout.axis == axis)
		{
			for_list(&box->children, child, ui_box, listElt)
			{
				if(ui_layout_downward_dependency(child, axis))
				{
					sum += child->rect.c[2+axis];
				}
			}
		}
		else
		{
			for_list(&box->children, child, ui_box, listElt)
			{
				if(ui_layout_downward_dependency(child, axis))
				{
					sum = maximum(sum, child->rect.c[2+axis]);
				}
			}
		}
		f32 margin = box->style.layout.margin.c[axis];
		box->rect.c[2+axis] = sum + box->spacing[axis] + 2*margin;
	}
}

void ui_layout_upward_dependent_size(ui_context* ui, ui_box* box, int axis)
{
	//NOTE: re-compute/set size of children that depend on box's size

	f32 margin = box->style.layout.margin.c[axis];
	f32 availableSize = maximum(0, box->rect.c[2+axis] - box->spacing[axis] - 2*margin);

	for_list(&box->children, child, ui_box, listElt)
	{
		ui_size* size = &child->style.size.c[axis];
		if(size->kind == UI_SIZE_PARENT)
		{
			child->rect.c[2+axis] = availableSize * size->value;
		}
		else if(size->kind == UI_SIZE_PARENT_MINUS_PIXELS)
		{
			child->rect.c[2+axis] = maximum(0, availableSize - size->value);
		}
	}

	//NOTE: solve downard conflicts
	int overflowFlag = (UI_FLAG_ALLOW_OVERFLOW_X << axis);
	f32 sum = 0;

	if(box->style.layout.axis == axis)
	{
		//NOTE: if we're solving in the layout axis, first compute total sum of children and
		//      total slack available
		f32 slack = 0;

		for_list(&box->children, child, ui_box, listElt)
		{
			if(  !ui_box_hidden(child)
			  && !child->style.floating.c[axis])
			{
				sum += child->rect.c[2+axis];
				slack += child->rect.c[2+axis] * child->style.size.c[axis].relax;
			}
		}

		if(!(box->flags & overflowFlag))
		{
			//NOTE: then remove excess proportionally to each box slack, and recompute children sum.
			f32 totalContents = sum + box->spacing[axis] + 2*box->style.layout.margin.c[axis];
			f32 excess = ClampLowBound(totalContents - box->rect.c[2+axis], 0);
			f32 alpha = Clamp(excess / slack, 0, 1);

			sum = 0;
			for_list(&box->children, child, ui_box, listElt)
			{
				f32 relax = child->style.size.c[axis].relax;
				child->rect.c[2+axis] -= alpha * child->rect.c[2+axis] * relax;
				sum += child->rect.c[2+axis];
			}
		}
	}
	else
	{
		//NOTE: if we're solving on the secondary axis, we remove excess to each box individually
		//      according to its own slack. Children sum is the maximum child size.

		for_list(&box->children, child, ui_box, listElt)
		{
			if(!ui_box_hidden(child) && !child->style.floating.c[axis])
			{
				if(!(box->flags & overflowFlag))
				{
					f32 totalContents = child->rect.c[2+axis] + 2*box->style.layout.margin.c[axis];
					f32 excess = ClampLowBound(totalContents - box->rect.c[2+axis], 0);
					f32 relax = child->style.size.c[axis].relax;
					child->rect.c[2+axis] -= minimum(excess, child->rect.c[2+axis]*relax);
				}
				sum = maximum(sum, child->rect.c[2+axis]);
			}
		}
	}

	box->childrenSum[axis] = sum;

	//NOTE: recurse in children
	for_list(&box->children, child, ui_box, listElt)
	{
		ui_layout_upward_dependent_size(ui, child, axis);
	}
}

void ui_layout_compute_rect(ui_context* ui, ui_box* box, vec2 pos)
{
	if(ui_box_hidden(box))
	{
		return;
	}

	box->rect.x = pos.x;
	box->rect.y = pos.y;
	box->z = ui->z;
	ui->z++;

	ui_axis layoutAxis = box->style.layout.axis;
	ui_axis secondAxis = (layoutAxis == UI_AXIS_X) ? UI_AXIS_Y : UI_AXIS_X;
	f32 spacing = box->style.layout.spacing;

	ui_align* align = box->style.layout.align.c;

	vec2 origin = {box->rect.x,
	               box->rect.y};
	vec2 currentPos = origin;

	vec2 margin = {box->style.layout.margin.x,
	               box->style.layout.margin.y};

	currentPos.x += margin.x;
	currentPos.y += margin.y;

	for(int i=0; i<UI_AXIS_COUNT; i++)
	{
		if(align[i] == UI_ALIGN_END)
		{
			currentPos.c[i] = origin.c[i] + box->rect.c[2+i] - (box->childrenSum[i] + box->spacing[i] + margin.c[i]);
		}
	}
	if(align[layoutAxis] == UI_ALIGN_CENTER)
	{
		currentPos.c[layoutAxis] = origin.c[layoutAxis]
		                         + 0.5*(box->rect.c[2+layoutAxis]
		                         - (box->childrenSum[layoutAxis] + box->spacing[layoutAxis]));
	}

	currentPos.x -= box->scroll.x;
	currentPos.y -= box->scroll.y;

	for_list(&box->children, child, ui_box, listElt)
	{
		if(align[secondAxis] == UI_ALIGN_CENTER)
		{
			currentPos.c[secondAxis] = origin.c[secondAxis] + 0.5*(box->rect.c[2+secondAxis] - child->rect.c[2+secondAxis]);
		}

		vec2 childPos = currentPos;
		for(int i=0; i<UI_AXIS_COUNT; i++)
		{
			if(child->style.floating.c[i])
			{
				ui_style* style = child->targetStyle;
				if((child->targetStyle->animationMask & (UI_STYLE_FLOAT_X << i))
				  && !child->fresh)
				{
					ui_animate_f32(ui, &child->floatPos.c[i], child->style.floatTarget.c[i], style->animationTime);
				}
				else
				{
					child->floatPos.c[i] = child->style.floatTarget.c[i];
				}
				childPos.c[i] = origin.c[i] + child->floatPos.c[i];
			}
		}

		ui_layout_compute_rect(ui, child, childPos);

		if(!child->style.floating.c[layoutAxis])
		{
			currentPos.c[layoutAxis] += child->rect.c[2+layoutAxis] + spacing;
		}
	}
}

void ui_layout_find_next_hovered_recursive(ui_context* ui, ui_box* box, vec2 p)
{
	if(ui_box_hidden(box))
	{
		return;
	}

	bool hit = ui_rect_hit(box->rect, p);
	if(hit && (box->flags & UI_FLAG_BLOCK_MOUSE))
	{
		ui->hovered = box;
	}
	if(hit || !(box->flags & UI_FLAG_CLIP))
	{
		for_list(&box->children, child, ui_box, listElt)
		{
			ui_layout_find_next_hovered_recursive(ui, child, p);
		}
	}
}

void ui_layout_find_next_hovered(ui_context* ui, vec2 p)
{
	ui->hovered = 0;
	ui_layout_find_next_hovered_recursive(ui, ui->root, p);
}

void ui_solve_layout(ui_context* ui)
{
	list_info beforeRules = {0};
	list_info afterRules = {0};

	//NOTE: style and compute static sizes
	ui_styling_prepass(ui, ui->root, &beforeRules, &afterRules);

	//NOTE: reparent overlay boxes
	for_list(&ui->overlayList, box, ui_box, overlayElt)
	{
		if(box->parent)
		{
			list_remove(&box->parent->children, &box->listElt);
			list_append(&ui->overlay->children, &box->listElt);
		}
	}

	//NOTE: compute layout
	for(int axis=0; axis<UI_AXIS_COUNT; axis++)
	{
		ui_layout_downward_dependent_size(ui, ui->root, axis);
		ui_layout_upward_dependent_size(ui, ui->root, axis);
	}
	ui_layout_compute_rect(ui, ui->root, (vec2){0, 0});

	vec2 p = ui_mouse_position();
	ui_layout_find_next_hovered(ui, p);
}

//-----------------------------------------------------------------------------
// Drawing
//-----------------------------------------------------------------------------

void ui_rectangle_fill(mp_rect rect, f32 roundness)
{
	if(roundness)
	{
		mg_rounded_rectangle_fill(rect.x, rect.y, rect.w, rect.h, roundness);
	}
	else
	{
		mg_rectangle_fill(rect.x, rect.y, rect.w, rect.h);
	}
}

void ui_rectangle_stroke(mp_rect rect, f32 roundness)
{
	if(roundness)
	{
		mg_rounded_rectangle_stroke(rect.x, rect.y, rect.w, rect.h, roundness);
	}
	else
	{
		mg_rectangle_stroke(rect.x, rect.y, rect.w, rect.h);
	}
}

void ui_draw_box(ui_box* box)
{
	if(ui_box_hidden(box))
	{
		return;
	}

	ui_style* style = &box->style;

	if(box->flags & UI_FLAG_CLIP)
	{
		mg_clip_push(box->rect.x, box->rect.y, box->rect.w, box->rect.h);
	}

	if(box->flags & UI_FLAG_DRAW_BACKGROUND)
	{
		mg_set_color(style->bgColor);
		ui_rectangle_fill(box->rect, style->roundness);
	}

	if((box->flags & UI_FLAG_DRAW_PROC) && box->drawProc)
	{
		box->drawProc(box, box->drawData);
	}

	for_list(&box->children, child, ui_box, listElt)
	{
		ui_draw_box(child);
	}

	if(box->flags & UI_FLAG_DRAW_TEXT)
	{
		mp_rect textBox = mg_text_bounding_box(style->font, style->fontSize, box->string);

		f32 x = 0;
		f32 y = 0;
		switch(style->layout.align.x)
		{
			case UI_ALIGN_START:
				x = box->rect.x + style->layout.margin.x;
				break;

			case UI_ALIGN_END:
				x = box->rect.x + box->rect.w - style->layout.margin.x - textBox.w;
				break;

			case UI_ALIGN_CENTER:
				x = box->rect.x + 0.5*(box->rect.w - textBox.w);
				break;
		}

		switch(style->layout.align.y)
		{
			case UI_ALIGN_START:
				y = box->rect.y + style->layout.margin.y - textBox.y;
				break;

			case UI_ALIGN_END:
				y = box->rect.y + box->rect.h - style->layout.margin.y - textBox.h + textBox.y;
				break;

			case UI_ALIGN_CENTER:
				y = box->rect.y + 0.5*(box->rect.h - textBox.h) - textBox.y;
				break;
		}

		mg_set_font(style->font);
		mg_set_font_size(style->fontSize);
		mg_set_color(style->color);

		mg_move_to(x, y);
		mg_text_outlines(box->string);
		mg_fill();
	}

	if(box->flags & UI_FLAG_CLIP)
	{
		mg_clip_pop();
	}

	if(box->flags & UI_FLAG_DRAW_BORDER)
	{
		mg_set_width(style->borderSize);
		mg_set_color(style->borderColor);
		ui_rectangle_stroke(box->rect, style->roundness);
	}
}

void ui_draw()
{
	ui_context* ui = ui_get_context();

	//NOTE: draw
	bool oldTextFlip = mg_get_text_flip();
	mg_set_text_flip(false);

	ui_draw_box(ui->root);

	mg_set_text_flip(oldTextFlip);
}

//-----------------------------------------------------------------------------
// frame begin/end
//-----------------------------------------------------------------------------

void ui_begin_frame(vec2 size, ui_style* defaultStyle, ui_style_mask defaultMask)
{
	ui_context* ui = ui_get_context();

	mem_arena_clear(&ui->frameArena);

	ui->frameCounter++;
	f64 time = mp_get_time(MP_CLOCK_MONOTONIC);
	ui->lastFrameDuration = time - ui->frameTime;
	ui->frameTime = time;

	ui->clipStack = 0;
	ui->z = 0;

	defaultMask &= UI_STYLE_COLOR
	             | UI_STYLE_BG_COLOR
	             | UI_STYLE_BORDER_COLOR
	             | UI_STYLE_FONT
	             | UI_STYLE_FONT_SIZE;

	ui_style_match_before(ui_pattern_all(), defaultStyle, defaultMask);
	ui_style_next(&(ui_style){.size.width = {UI_SIZE_PIXELS, size.x},
	                          .size.height = {UI_SIZE_PIXELS, size.y}},
	              UI_STYLE_SIZE);

	ui->root = ui_box_begin("_root_", 0);

	ui_style_mask contentStyleMask = UI_STYLE_SIZE
	                               | UI_STYLE_LAYOUT
	                               | UI_STYLE_FLOAT;

	ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
	                          .size.height = {UI_SIZE_PARENT, 1},
	                          .layout = {UI_AXIS_Y, UI_ALIGN_START, UI_ALIGN_START},
	                          .floating = {true, true},
	                          .floatTarget = {0, 0}},
	              contentStyleMask);

	ui_box* contents = ui_box_make("_contents_", 0);

	ui_style_next(&(ui_style){.layout = {UI_AXIS_Y, UI_ALIGN_START, UI_ALIGN_START},
	                          .floating = {true, true},
	                          .floatTarget = {0, 0}},
	              UI_STYLE_LAYOUT | UI_STYLE_FLOAT_X | UI_STYLE_FLOAT_Y);

	ui->overlay = ui_box_make("_overlay_", 0);
	ui->overlayList = (list_info){0};

	ui->nextBoxBeforeRules = (list_info){0};
	ui->nextBoxAfterRules = (list_info){0};
	ui->nextBoxTags = (list_info){0};

	ui_box_push(contents);
}

void ui_end_frame(void)
{
	ui_context* ui = ui_get_context();

	ui_box_pop();

	ui_box* box = ui_box_end();
	DEBUG_ASSERT(box == ui->root, "unbalanced box stack");

	//TODO: check balancing of style stacks

	//NOTE: layout
	ui_solve_layout(ui);

	//NOTE: prune unused boxes
	for(int i=0; i<UI_BOX_MAP_BUCKET_COUNT; i++)
	{
		for_list_safe(&ui->boxMap[i], box, ui_box, bucketElt)
		{
			if(box->frameCounter < ui->frameCounter)
			{
				list_remove(&ui->boxMap[i], &box->bucketElt);
			}
		}
	}

	mp_input_next_frame(&ui->input);
}

//-----------------------------------------------------------------------------
// Init / cleanup
//-----------------------------------------------------------------------------
void ui_init(ui_context* ui)
{
	__uiCurrentContext = &__uiThreadContext;

	memset(ui, 0, sizeof(ui_context));
	mem_arena_init(&ui->frameArena);
	mem_pool_init(&ui->boxPool, sizeof(ui_box));
	ui->init = true;

	ui_set_context(ui);
}

void ui_cleanup(void)
{
	ui_context* ui = ui_get_context();
	mem_arena_release(&ui->frameArena);
	mem_pool_release(&ui->boxPool);
	ui->init = false;
}


//-----------------------------------------------------------------------------
// label
//-----------------------------------------------------------------------------

ui_sig ui_label_str8(str8 label)
{
	ui_style_next(&(ui_style){.size.width = {UI_SIZE_TEXT, 0, 0},
	                          .size.height = {UI_SIZE_TEXT, 0, 0}},
	              UI_STYLE_SIZE_WIDTH | UI_STYLE_SIZE_HEIGHT);

	ui_flags flags = UI_FLAG_CLIP
	               | UI_FLAG_DRAW_TEXT;
	ui_box* box = ui_box_make_str8(label, flags);

	ui_sig sig = ui_box_sig(box);
	return(sig);
}

ui_sig ui_label(const char* label)
{
	return(ui_label_str8(STR8((char*)label)));
}

//------------------------------------------------------------------------------
// button
//------------------------------------------------------------------------------

ui_sig ui_button_behavior(ui_box* box)
{
	ui_sig sig = ui_box_sig(box);

	if(sig.hovering)
	{
		ui_box_set_hot(box, true);
		if(sig.dragging)
		{
			ui_box_activate(box);
		}
	}
	else
	{
		ui_box_set_hot(box, false);
	}
	if(!sig.dragging)
	{
		ui_box_deactivate(box);
	}
	return(sig);
}

ui_sig ui_button_str8(str8 label)
{
	ui_context* ui = ui_get_context();

	ui_style defaultStyle = {.size.width = {UI_SIZE_TEXT},
	                         .size.height = {UI_SIZE_TEXT},
	                         .layout.align.x = UI_ALIGN_CENTER,
	                         .layout.align.y = UI_ALIGN_CENTER,
	                         .layout.margin.x = 5,
	                         .layout.margin.y = 5,
	                         .bgColor = {0.5, 0.5, 0.5, 1},
	                         .borderColor = {0.2, 0.2, 0.2, 1},
	                         .borderSize = 1,
	                         .roundness = 10};

	ui_style_mask defaultMask = UI_STYLE_SIZE_WIDTH
	                          | UI_STYLE_SIZE_HEIGHT
	                          | UI_STYLE_LAYOUT_MARGIN_X
	                          | UI_STYLE_LAYOUT_MARGIN_Y
	                          | UI_STYLE_LAYOUT_ALIGN_X
	                          | UI_STYLE_LAYOUT_ALIGN_Y
	                          | UI_STYLE_BG_COLOR
	                          | UI_STYLE_BORDER_COLOR
	                          | UI_STYLE_BORDER_SIZE
	                          | UI_STYLE_ROUNDNESS;

	ui_style_next(&defaultStyle, defaultMask);

	ui_style activeStyle = {.bgColor = {0.3, 0.3, 0.3, 1},
	                        .borderColor = {0.2, 0.2, 0.2, 1},
	                        .borderSize = 2};
	ui_style_mask activeMask = UI_STYLE_BG_COLOR
	                         | UI_STYLE_BORDER_COLOR
	                         | UI_STYLE_BORDER_SIZE;
	ui_pattern activePattern = {0};
	ui_pattern_push(&ui->frameArena,
	                &activePattern,
	                (ui_selector){.kind = UI_SEL_STATUS,
	                              .status = UI_ACTIVE|UI_HOVER});
	ui_style_match_before(activePattern, &activeStyle, activeMask);

	ui_flags flags = UI_FLAG_CLICKABLE
	               | UI_FLAG_CLIP
	               | UI_FLAG_DRAW_BACKGROUND
	               | UI_FLAG_DRAW_BORDER
	               | UI_FLAG_DRAW_TEXT
	               | UI_FLAG_HOT_ANIMATION
	               | UI_FLAG_ACTIVE_ANIMATION;

	ui_box* box = ui_box_make_str8(label, flags);
	ui_tag_box(box, "button");

	ui_sig sig = ui_button_behavior(box);
	return(sig);
}

ui_sig ui_button(const char* label)
{
	return(ui_button_str8(STR8((char*)label)));
}

void ui_checkbox_draw(ui_box* box, void* data)
{
	bool checked = *(bool*)data;
	if(checked)
	{
		mg_move_to(box->rect.x + 0.2*box->rect.w, box->rect.y + 0.5*box->rect.h);
		mg_line_to(box->rect.x + 0.4*box->rect.w, box->rect.y + 0.75*box->rect.h);
		mg_line_to(box->rect.x + 0.8*box->rect.w, box->rect.y + 0.2*box->rect.h);

		mg_set_color(box->style.color);
		mg_set_width(0.2*box->rect.w);
		mg_set_joint(MG_JOINT_MITER);
		mg_set_max_joint_excursion(0.2 * box->rect.h);
		mg_stroke();
	}
}

ui_sig ui_checkbox(const char* name, bool* checked)
{
	ui_context* ui = ui_get_context();

	ui_style defaultStyle = {.size.width = {UI_SIZE_PIXELS, 20},
	                         .size.height = {UI_SIZE_PIXELS, 20},
	                         .bgColor = {1, 1, 1, 1},
	                         .color = {0, 0, 0, 1},
	                         .borderColor = {0.2, 0.2, 0.2, 1},
	                         .borderSize = 1,
	                         .roundness = 5};

	ui_style_mask defaultMask = UI_STYLE_SIZE_WIDTH
	                          | UI_STYLE_SIZE_HEIGHT
	                          | UI_STYLE_BG_COLOR
	                          | UI_STYLE_COLOR
	                          | UI_STYLE_BORDER_COLOR
	                          | UI_STYLE_BORDER_SIZE
	                          | UI_STYLE_ROUNDNESS;

	ui_style_next(&defaultStyle, defaultMask);

	ui_style activeStyle = {.bgColor = {0.5, 0.5, 0.5, 1},
	                        .borderColor = {0.2, 0.2, 0.2, 1},
	                        .borderSize = 2};
	ui_style_mask activeMask = UI_STYLE_BG_COLOR
	                         | UI_STYLE_BORDER_COLOR
	                         | UI_STYLE_BORDER_SIZE;
	ui_pattern activePattern = {0};
	ui_pattern_push(&ui->frameArena,
	                &activePattern,
	                (ui_selector){.kind = UI_SEL_STATUS,
	                              .status = UI_ACTIVE|UI_HOVER});
	ui_style_match_before(activePattern, &activeStyle, activeMask);

	ui_flags flags = UI_FLAG_CLICKABLE
	               | UI_FLAG_CLIP
	               | UI_FLAG_DRAW_BACKGROUND
	               | UI_FLAG_DRAW_PROC
	               | UI_FLAG_DRAW_BORDER
	               | UI_FLAG_HOT_ANIMATION
	               | UI_FLAG_ACTIVE_ANIMATION;

	ui_box* box = ui_box_make(name, flags);
	ui_tag_box(box, "checkbox");

	ui_sig sig = ui_button_behavior(box);
	if(sig.clicked)
	{
		*checked = !*checked;
	}
	ui_box_set_draw_proc(box, ui_checkbox_draw, checked);

	return(sig);
}

//------------------------------------------------------------------------------
// slider / scrollbar
//------------------------------------------------------------------------------
ui_box* ui_slider(const char* label, f32 thumbRatio, f32* scrollValue)
{
	ui_style_match_before(ui_pattern_all(), &(ui_style){0}, UI_STYLE_LAYOUT);
	ui_box* frame = ui_box_begin(label, 0);
	{
		f32 beforeRatio = (*scrollValue) * (1. - thumbRatio);
		f32 afterRatio = (1. - *scrollValue) * (1. - thumbRatio);

		ui_axis trackAxis = (frame->rect.w > frame->rect.h) ? UI_AXIS_X : UI_AXIS_Y;
		ui_axis secondAxis = (trackAxis == UI_AXIS_Y) ? UI_AXIS_X : UI_AXIS_Y;
		f32 roundness = 0.5*frame->rect.c[2+secondAxis];
		f32 animationTime = 0.5;

		ui_style trackStyle = {.size.width = {UI_SIZE_PARENT, 1},
		                       .size.height = {UI_SIZE_PARENT, 1},
		                       .layout.axis = trackAxis,
		                       .layout.align.x = UI_ALIGN_START,
		                       .layout.align.y = UI_ALIGN_START,
		                       .bgColor = {0.5, 0.5, 0.5, 1},
		                       .roundness = roundness};

		ui_style beforeStyle = trackStyle;
		beforeStyle.size.c[trackAxis] = (ui_size){UI_SIZE_PARENT, beforeRatio};

		ui_style afterStyle = trackStyle;
		afterStyle.size.c[trackAxis] = (ui_size){UI_SIZE_PARENT, afterRatio};

		ui_style thumbStyle = trackStyle;
		thumbStyle.size.c[trackAxis] = (ui_size){UI_SIZE_PARENT, thumbRatio};
		thumbStyle.bgColor = (mg_color){0.3, 0.3, 0.3, 1};

		ui_style_mask styleMask = UI_STYLE_SIZE_WIDTH
		                        | UI_STYLE_SIZE_HEIGHT
		                        | UI_STYLE_LAYOUT
		                        | UI_STYLE_BG_COLOR
		                        | UI_STYLE_ROUNDNESS;

		ui_flags trackFlags = UI_FLAG_CLIP
	                    	| UI_FLAG_DRAW_BACKGROUND
	                    	| UI_FLAG_HOT_ANIMATION
	                    	| UI_FLAG_ACTIVE_ANIMATION;

		ui_style_next(&trackStyle, styleMask);
		ui_box* track = ui_box_begin("track", trackFlags);

			ui_style_next(&beforeStyle, UI_STYLE_SIZE_WIDTH|UI_STYLE_SIZE_HEIGHT);
			ui_box* beforeSpacer = ui_box_make("before", 0);


			ui_flags thumbFlags = UI_FLAG_CLICKABLE
		                    	| UI_FLAG_DRAW_BACKGROUND
		                    	| UI_FLAG_HOT_ANIMATION
		                    	| UI_FLAG_ACTIVE_ANIMATION;

			ui_style_next(&thumbStyle, styleMask);
			ui_box* thumb = ui_box_make("thumb", thumbFlags);


			ui_style_next(&afterStyle, UI_STYLE_SIZE_WIDTH|UI_STYLE_SIZE_HEIGHT);
			ui_box* afterSpacer = ui_box_make("after", 0);

		ui_box_end();

		//NOTE: interaction
		ui_sig thumbSig = ui_box_sig(thumb);
		if(thumbSig.dragging)
		{
			f32 trackExtents = track->rect.c[2+trackAxis] - thumb->rect.c[2+trackAxis];
			f32 delta = thumbSig.delta.c[trackAxis]/trackExtents;
			f32 oldValue = *scrollValue;

			*scrollValue += delta;
			*scrollValue = Clamp(*scrollValue, 0, 1);
		}

		ui_sig trackSig = ui_box_sig(track);

		if(ui_box_active(frame))
		{
			//NOTE: activated from outside
			ui_box_set_hot(track, true);
			ui_box_set_hot(thumb, true);
			ui_box_activate(track);
			ui_box_activate(thumb);
		}

		if(trackSig.hovering)
		{
			ui_box_set_hot(track, true);
			ui_box_set_hot(thumb, true);
		}
		else if(thumbSig.wheel.c[trackAxis] == 0)
		{
			ui_box_set_hot(track, false);
			ui_box_set_hot(thumb, false);
		}

		if(thumbSig.dragging)
		{
			ui_box_activate(track);
			ui_box_activate(thumb);
		}
		else if(thumbSig.wheel.c[trackAxis] == 0)
		{
			ui_box_deactivate(track);
			ui_box_deactivate(thumb);
			ui_box_deactivate(frame);
		}

	} ui_box_end();

	return(frame);
}

//------------------------------------------------------------------------------
// panels
//------------------------------------------------------------------------------
void ui_panel_begin(const char* str, ui_flags flags)
{
	flags = flags
	      | UI_FLAG_CLIP
	      | UI_FLAG_BLOCK_MOUSE
	      | UI_FLAG_ALLOW_OVERFLOW_X
	      | UI_FLAG_ALLOW_OVERFLOW_Y
	      | UI_FLAG_SCROLL_WHEEL_X
	      | UI_FLAG_SCROLL_WHEEL_Y;

	ui_box_begin(str, flags);
}

void ui_panel_end(void)
{
	ui_box* panel = ui_box_top();
	ui_sig sig = ui_box_sig(panel);

	f32 contentsW = ClampLowBound(panel->childrenSum[0], panel->rect.w);
	f32 contentsH = ClampLowBound(panel->childrenSum[1], panel->rect.h);

	contentsW = ClampLowBound(contentsW, 1);
	contentsH = ClampLowBound(contentsH, 1);

	ui_box* scrollBarX = 0;
	ui_box* scrollBarY = 0;

	bool needsScrollX = contentsW > panel->rect.w;
	bool needsScrollY = contentsH > panel->rect.h;

	if(needsScrollX)
	{
		f32 thumbRatioX = panel->rect.w / contentsW;
		f32 sliderX = panel->scroll.x /(contentsW - panel->rect.w);

		ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1., 0},
		                          .size.height = {UI_SIZE_PIXELS, 10, 0},
		                          .floating.x = true,
		                          .floating.y = true,
		                          .floatTarget = {0, panel->rect.h - 10}},
		              UI_STYLE_SIZE
		              |UI_STYLE_FLOAT);

		scrollBarX = ui_slider("scrollerX", thumbRatioX, &sliderX);

		panel->scroll.x = sliderX * (contentsW - panel->rect.w);
		if(sig.hovering)
		{
			ui_box_activate(scrollBarX);
		}
	}

	if(needsScrollY)
	{
		f32 thumbRatioY = panel->rect.h / contentsH;
		f32 sliderY = panel->scroll.y /(contentsH - panel->rect.h);

		f32 spacerSize = needsScrollX ? 10 : 0;

		ui_style_next(&(ui_style){.size.width = {UI_SIZE_PIXELS, 10, 0},
		                          .size.height = {UI_SIZE_PARENT_MINUS_PIXELS, spacerSize, 0},
		                          .floating.x = true,
		                          .floating.y = true,
		                          .floatTarget = {panel->rect.w - 10, 0}},
		               UI_STYLE_SIZE
		              |UI_STYLE_FLOAT);

		scrollBarY = ui_slider("scrollerY", thumbRatioY, &sliderY);

		panel->scroll.y = sliderY * (contentsH - panel->rect.h);
		if(sig.hovering)
		{
			ui_box_activate(scrollBarY);
		}
	}
	panel->scroll.x = Clamp(panel->scroll.x, 0, contentsW - panel->rect.w);
	panel->scroll.y = Clamp(panel->scroll.y, 0, contentsH - panel->rect.h);

	ui_box_end();
}

//------------------------------------------------------------------------------
// tooltips
//------------------------------------------------------------------------------

ui_sig ui_tooltip_begin(const char* name)
{
	ui_context* ui = ui_get_context();

	vec2 p = ui_mouse_position();

	ui_style style = {.size.width = {UI_SIZE_CHILDREN},
	                  .size.height = {UI_SIZE_CHILDREN},
	                  .floating.x = true,
	                  .floating.y = true,
	                  .floatTarget = {p.x, p.y}};
	ui_style_mask mask = UI_STYLE_SIZE | UI_STYLE_FLOAT;

	ui_style_next(&style, mask);

	ui_flags flags = UI_FLAG_OVERLAY
	               | UI_FLAG_DRAW_BACKGROUND
	               | UI_FLAG_DRAW_BORDER;

	ui_box* tooltip = ui_box_make(name, flags);
	ui_box_push(tooltip);

	return(ui_box_sig(tooltip));
}

void ui_tooltip_end(void)
{
	ui_box_pop(); // tooltip
}

//------------------------------------------------------------------------------
// Menus
//------------------------------------------------------------------------------

void ui_menu_bar_begin(const char* name)
{
	ui_style style = {.size.width = {UI_SIZE_PARENT, 1, 0},
	                  .size.height = {UI_SIZE_CHILDREN},
	                  .layout.axis = UI_AXIS_X,
	                  .layout.spacing = 20,};
	ui_style_mask mask = UI_STYLE_SIZE
	                   | UI_STYLE_LAYOUT_AXIS
	                   | UI_STYLE_LAYOUT_SPACING;

	ui_style_next(&style, mask);
	ui_box* bar = ui_box_begin(name, UI_FLAG_DRAW_BACKGROUND);

	ui_sig sig = ui_box_sig(bar);
	ui_context* ui = ui_get_context();
	if(!sig.hovering && mp_mouse_released(&ui->input, MP_MOUSE_LEFT))
	{
		ui_box_deactivate(bar);
	}
}

void ui_menu_bar_end(void)
{
	ui_box_end(); // menu bar
}

void ui_menu_begin(const char* label)
{
	ui_box* container = ui_box_make(label, 0);
	ui_box_push(container);

	ui_style_next(&(ui_style){.size.width = {UI_SIZE_TEXT},
	                          .size.height = {UI_SIZE_TEXT}},
	             UI_STYLE_SIZE);

	ui_box* button = ui_box_make(label, UI_FLAG_CLICKABLE | UI_FLAG_DRAW_TEXT);
	ui_box* bar = container->parent;

	ui_sig sig = ui_box_sig(button);
	ui_sig barSig = ui_box_sig(bar);

	ui_context* ui = ui_get_context();

	ui_style style = {.size.width = {UI_SIZE_CHILDREN},
	                  .size.height = {UI_SIZE_CHILDREN},
	                  .floating.x = true,
	                  .floating.y = true,
	                  .floatTarget = {button->rect.x,
	                                  button->rect.y + button->rect.h},
	                  .layout.axis = UI_AXIS_Y,
	                  .layout.spacing = 5,
	                  .layout.margin.x = 0,
	                  .layout.margin.y = 5,
	                  .bgColor = {0.2, 0.2, 0.2, 1}};

	ui_style_mask mask = UI_STYLE_SIZE
	                   | UI_STYLE_FLOAT
	                   | UI_STYLE_LAYOUT
	                   | UI_STYLE_BG_COLOR;

	ui_flags flags = UI_FLAG_OVERLAY
	               | UI_FLAG_DRAW_BACKGROUND
	               | UI_FLAG_DRAW_BORDER;

	ui_style_next(&style, mask);
	ui_box* menu = ui_box_make("panel", flags);

	if(ui_box_active(bar))
	{
		if(sig.hovering)
		{
			ui_box_activate(button);
		}
		else if(barSig.hovering)
		{
			ui_box_deactivate(button);
		}
	}
	else
	{
		ui_box_deactivate(button);
		if(sig.pressed)
		{
			ui_box_activate(bar);
			ui_box_activate(button);
		}
	}

	ui_box_set_closed(menu, !ui_box_active(button));
	ui_box_push(menu);
}

void ui_menu_end(void)
{
	ui_box_pop(); // menu
	ui_box_pop(); // container
}

ui_sig ui_menu_button(const char* name)
{
	ui_context* ui = ui_get_context();

	ui_style_next(&(ui_style){.size.width = {UI_SIZE_TEXT},
	                          .size.height = {UI_SIZE_TEXT},
	                          .layout.margin.x = 5,
	                          .bgColor = {0, 0, 0, 0}},
	               UI_STYLE_SIZE
	              |UI_STYLE_LAYOUT_MARGIN_X
	              |UI_STYLE_BG_COLOR);

	ui_pattern pattern = {0};
	ui_pattern_push(&ui->frameArena, &pattern, (ui_selector){.kind = UI_SEL_STATUS, .status = UI_HOVER});

	ui_style style = {.bgColor = {0, 0, 1, 1}};
	ui_style_mask mask = UI_STYLE_BG_COLOR;
	ui_style_match_before(pattern, &style, mask);

	ui_flags flags = UI_FLAG_CLICKABLE
	               | UI_FLAG_CLIP
	               | UI_FLAG_DRAW_TEXT
	               | UI_FLAG_DRAW_BACKGROUND;

	ui_box* box = ui_box_make(name, flags);
	ui_sig sig = ui_box_sig(box);
	return(sig);
}

void ui_select_popup_draw_arrow(ui_box* box, void* data)
{
	f32 r = minimum(box->parent->style.roundness, box->rect.w);
	f32 cr = r*4*(sqrt(2)-1)/3;

	mg_move_to(box->rect.x, box->rect.y);
	mg_line_to(box->rect.x + box->rect.w - r, box->rect.y);
	mg_cubic_to(box->rect.x + box->rect.w - cr, box->rect.y,
	            box->rect.x + box->rect.w, box->rect.y + cr,
	            box->rect.x + box->rect.w, box->rect.y + r);
	mg_line_to(box->rect.x + box->rect.w, box->rect.y + box->rect.h - r);
	mg_cubic_to(box->rect.x + box->rect.w, box->rect.y + box->rect.h - cr,
	            box->rect.x + box->rect.w - cr, box->rect.y + box->rect.h,
	            box->rect.x + box->rect.w - r, box->rect.y + box->rect.h);
	mg_line_to(box->rect.x, box->rect.y + box->rect.h);

	mg_set_color(box->style.bgColor);
	mg_fill();

	mg_move_to(box->rect.x + 0.25*box->rect.w, box->rect.y + 0.45*box->rect.h);
	mg_line_to(box->rect.x + 0.5*box->rect.w, box->rect.y + 0.75*box->rect.h);
	mg_line_to(box->rect.x + 0.75*box->rect.w, box->rect.y + 0.45*box->rect.h);
	mg_close_path();

	mg_set_color(box->style.color);
	mg_fill();
}

ui_select_popup_info ui_select_popup(const char* name, ui_select_popup_info* info)
{
	ui_select_popup_info result = *info;

	ui_context* ui = ui_get_context();

	ui_container(name, 0)
	{
		ui_box* button = ui_box_make("button",
	                             	UI_FLAG_CLICKABLE
	                             	|UI_FLAG_DRAW_BACKGROUND
	                             	|UI_FLAG_DRAW_BORDER
	                             	|UI_FLAG_ALLOW_OVERFLOW_X
	                             	|UI_FLAG_CLIP);

		f32 maxOptionWidth = 0;
		f32 lineHeight = 0;
		mp_rect bbox = {0};
		for(int i=0; i<info->optionCount; i++)
		{
			bbox = mg_text_bounding_box(button->style.font, button->style.fontSize, info->options[i]);
			maxOptionWidth = maximum(maxOptionWidth, bbox.w);
		}
		f32 buttonWidth = maxOptionWidth + 2*button->style.layout.margin.x + button->rect.h;

		ui_style_box_before(button,
	                    	ui_pattern_owner(),
	                    	&(ui_style){.size.width = {UI_SIZE_PIXELS, buttonWidth},
	                                	.size.height = {UI_SIZE_CHILDREN},
	                                	.layout.margin.x = 5,
	                                	.layout.margin.y = 1,
	                                	.roundness = 5,
	                                	.borderSize = 1,
	                                	.borderColor = {0.3, 0.3, 0.3, 1}},
	                    	UI_STYLE_SIZE
	                    	|UI_STYLE_LAYOUT_MARGIN_X
	                    	|UI_STYLE_LAYOUT_MARGIN_Y
	                    	|UI_STYLE_ROUNDNESS
	                    	|UI_STYLE_BORDER_SIZE
	                    	|UI_STYLE_BORDER_COLOR);
		ui_box_push(button);
		{
			ui_label_str8(info->options[info->selectedIndex]);

			ui_style_next(&(ui_style){.size.width = {UI_SIZE_PIXELS, button->rect.h},
		                          	.size.height = {UI_SIZE_PIXELS, button->rect.h},
		                          	.floating.x = true,
		                          	.floating.y = true,
		                          	.floatTarget = {button->rect.w - button->rect.h, 0},
		                          	.color = {0, 0, 0, 1},
		                          	.bgColor = {0.7, 0.7, 0.7, 1}},
		               	UI_STYLE_SIZE
		              	|UI_STYLE_FLOAT
		              	|UI_STYLE_COLOR
		              	|UI_STYLE_BG_COLOR);

			ui_box* arrow = ui_box_make("arrow", UI_FLAG_DRAW_PROC);
			ui_box_set_draw_proc(arrow, ui_select_popup_draw_arrow, 0);

		} ui_box_pop();

		//panel
		ui_box* panel = ui_box_make("panel",
	                             	UI_FLAG_DRAW_BACKGROUND
	                            	|UI_FLAG_BLOCK_MOUSE
	                            	|UI_FLAG_OVERLAY);

		//TODO: set width to max(button.w, max child...)
		f32 containerWidth = maximum(maxOptionWidth + 2*panel->style.layout.margin.x,
	                             	button->rect.w);

		ui_style_box_before(panel,
	                    	ui_pattern_owner(),
	                    	&(ui_style){.size.width = {UI_SIZE_PIXELS, containerWidth},
	                          	.size.height = {UI_SIZE_CHILDREN},
	                          	.floating.x = true,
	                          	.floating.y = true,
	                          	.floatTarget = {button->rect.x,
	                                          	button->rect.y + button->rect.h},
	                          	.layout.axis = UI_AXIS_Y,
	                          	.layout.margin.x = 0,
	                          	.layout.margin.y = 5,
	                          	.bgColor = {0.2, 0.2, 0.2, 1}},
	              	UI_STYLE_SIZE
	             	|UI_STYLE_FLOAT
	             	|UI_STYLE_LAYOUT
	             	|UI_STYLE_BG_COLOR);

		ui_box_push(panel);
		{
			for(int i=0; i<info->optionCount; i++)
			{
				ui_style_next(&(ui_style){.size.width = {UI_SIZE_PARENT, 1},
			                          	.size.height = {UI_SIZE_TEXT},
			                          	.layout.axis = UI_AXIS_Y,
			                          	.layout.align.x = UI_ALIGN_START,
			                          	.layout.margin.x = 5,
			                          	.layout.margin.y = 2.5},
			               	UI_STYLE_SIZE
			              	|UI_STYLE_LAYOUT_AXIS
			              	|UI_STYLE_LAYOUT_ALIGN_X
			              	|UI_STYLE_LAYOUT_MARGIN_X
			              	|UI_STYLE_LAYOUT_MARGIN_Y);


				ui_pattern pattern = {0};
				ui_pattern_push(&ui->frameArena, &pattern, (ui_selector){.kind = UI_SEL_STATUS, .status = UI_HOVER});
				ui_style_match_before(pattern, &(ui_style){.bgColor = {0, 0, 1, 1}}, UI_STYLE_BG_COLOR);

				ui_box* box = ui_box_make_str8(info->options[i],
			                                	UI_FLAG_DRAW_TEXT
			                               	|UI_FLAG_CLICKABLE
			                               	|UI_FLAG_DRAW_BACKGROUND);
				ui_sig sig = ui_box_sig(box);
				if(sig.pressed)
				{
					result.selectedIndex = i;
				}
			}
		}
		ui_box_pop();

		ui_context* ui = ui_get_context();
		if(ui_box_active(panel) && mp_mouse_pressed(&ui->input, MP_MOUSE_LEFT))
		{
			ui_box_deactivate(panel);
		}
		else if(ui_box_sig(button).pressed)
		{
			ui_box_activate(panel);
		}
		ui_box_set_closed(panel, !ui_box_active(panel));
	}
	return(result);
}

//------------------------------------------------------------------------------
// text box
//------------------------------------------------------------------------------
str32 ui_edit_replace_selection_with_codepoints(ui_context* ui, str32 codepoints, str32 input)
{
	u32 start = minimum(ui->editCursor, ui->editMark);
	u32 end = maximum(ui->editCursor, ui->editMark);

	str32 before = str32_slice(codepoints, 0, start);
	str32 after = str32_slice(codepoints, end, codepoints.len);

	str32_list list = {0};
	str32_list_push(&ui->frameArena, &list, before);
	str32_list_push(&ui->frameArena, &list, input);
	str32_list_push(&ui->frameArena, &list, after);

	codepoints = str32_list_join(&ui->frameArena, list);

	ui->editCursor = start + input.len;
	ui->editMark = ui->editCursor;
	return(codepoints);
}

str32 ui_edit_delete_selection(ui_context* ui, str32 codepoints)
{
	return(ui_edit_replace_selection_with_codepoints(ui, codepoints, (str32){0}));
}

void ui_edit_copy_selection_to_clipboard(ui_context* ui, str32 codepoints)
{
	#if !PLATFORM_ORCA
	if(ui->editCursor == ui->editMark)
	{
		return;
	}
	u32 start = minimum(ui->editCursor, ui->editMark);
	u32 end = maximum(ui->editCursor, ui->editMark);
	str32 selection = str32_slice(codepoints, start, end);
	str8 string = utf8_push_from_codepoints(&ui->frameArena, selection);

	mp_clipboard_clear();
	mp_clipboard_set_string(string);
	#endif
}

str32 ui_edit_replace_selection_with_clipboard(ui_context* ui, str32 codepoints)
{
	#if PLATFORM_ORCA
	str32 result = {0};
	#else
	str8 string = mp_clipboard_get_string(&ui->frameArena);
	str32 input = utf8_push_to_codepoints(&ui->frameArena, string);
	str32 result = ui_edit_replace_selection_with_codepoints(ui, codepoints, input);
	#endif
	return(result);
}

typedef enum {
	UI_EDIT_MOVE,
	UI_EDIT_SELECT,
	UI_EDIT_SELECT_EXTEND,
	UI_EDIT_DELETE,
	UI_EDIT_CUT,
	UI_EDIT_COPY,
	UI_EDIT_PASTE,
	UI_EDIT_SELECT_ALL } ui_edit_op;

typedef enum {
	UI_EDIT_MOVE_NONE = 0,
	UI_EDIT_MOVE_ONE,
	UI_EDIT_MOVE_WORD,
	UI_EDIT_MOVE_LINE } ui_edit_move;

typedef struct ui_edit_command
{
	mp_key_code key;
	mp_keymod_flags mods;

	ui_edit_op operation;
	ui_edit_move move;
	int direction;

} ui_edit_command;

const ui_edit_command UI_EDIT_COMMANDS[] = {
	//NOTE(martin): move one left
	{
		.key = MP_KEY_LEFT,
		.operation = UI_EDIT_MOVE,
		.move = UI_EDIT_MOVE_ONE,
		.direction = -1
	},
	//NOTE(martin): move one right
	{
		.key = MP_KEY_RIGHT,
		.operation = UI_EDIT_MOVE,
		.move = UI_EDIT_MOVE_ONE,
		.direction = 1
	},
	//NOTE(martin): move start
	{
		.key = MP_KEY_Q,
		.mods = MP_KEYMOD_CTRL,
		.operation = UI_EDIT_MOVE,
		.move = UI_EDIT_MOVE_LINE,
		.direction = -1
	},
	{
		.key = MP_KEY_UP,
		.operation = UI_EDIT_MOVE,
		.move = UI_EDIT_MOVE_LINE,
		.direction = -1
	},
	//NOTE(martin): move end
	{
		.key = MP_KEY_E,
		.mods = MP_KEYMOD_CTRL,
		.operation = UI_EDIT_MOVE,
		.move = UI_EDIT_MOVE_LINE,
		.direction = 1
	},
	{
		.key = MP_KEY_DOWN,
		.operation = UI_EDIT_MOVE,
		.move = UI_EDIT_MOVE_LINE,
		.direction = 1
	},
	//NOTE(martin): select one left
	{
		.key = MP_KEY_LEFT,
		.mods = MP_KEYMOD_SHIFT,
		.operation = UI_EDIT_SELECT,
		.move = UI_EDIT_MOVE_ONE,
		.direction = -1
	},
	//NOTE(martin): select one right
	{
		.key = MP_KEY_RIGHT,
		.mods = MP_KEYMOD_SHIFT,
		.operation = UI_EDIT_SELECT,
		.move = UI_EDIT_MOVE_ONE,
		.direction = 1
	},
	//NOTE(martin): extend select to start
	{
		.key = MP_KEY_Q,
		.mods = MP_KEYMOD_CTRL | MP_KEYMOD_SHIFT,
		.operation = UI_EDIT_SELECT_EXTEND,
		.move = UI_EDIT_MOVE_LINE,
		.direction = -1
	},
	{
		.key = MP_KEY_UP,
		.mods = MP_KEYMOD_SHIFT,
		.operation = UI_EDIT_SELECT_EXTEND,
		.move = UI_EDIT_MOVE_LINE,
		.direction = -1
	},
	//NOTE(martin): extend select to end
	{
		.key = MP_KEY_E,
		.mods = MP_KEYMOD_CTRL | MP_KEYMOD_SHIFT,
		.operation = UI_EDIT_SELECT_EXTEND,
		.move = UI_EDIT_MOVE_LINE,
		.direction = 1
	},
	{
		.key = MP_KEY_DOWN,
		.mods = MP_KEYMOD_SHIFT,
		.operation = UI_EDIT_SELECT_EXTEND,
		.move = UI_EDIT_MOVE_LINE,
		.direction = 1
	},
	//NOTE(martin): select all
	{
		.key = MP_KEY_Q,
		.mods = MP_KEYMOD_MAIN_MODIFIER,
		.operation = UI_EDIT_SELECT_ALL,
		.move = UI_EDIT_MOVE_NONE
	},
	//NOTE(martin): delete
	{
		.key = MP_KEY_DELETE,
		.operation = UI_EDIT_DELETE,
		.move = UI_EDIT_MOVE_ONE,
		.direction = 1
	},
	//NOTE(martin): backspace
	{
		.key = MP_KEY_BACKSPACE,
		.operation = UI_EDIT_DELETE,
		.move = UI_EDIT_MOVE_ONE,
		.direction = -1
	},
	//NOTE(martin): cut
	{
		.key = MP_KEY_X,
		.mods = MP_KEYMOD_MAIN_MODIFIER,
		.operation = UI_EDIT_CUT,
		.move = UI_EDIT_MOVE_NONE
	},
	//NOTE(martin): copy
	{
		.key = MP_KEY_C,
		.mods = MP_KEYMOD_MAIN_MODIFIER,
		.operation = UI_EDIT_COPY,
		.move = UI_EDIT_MOVE_NONE
	},
	//NOTE(martin): paste
	{
		.key = MP_KEY_V,
		.mods = MP_KEYMOD_MAIN_MODIFIER,
		.operation = UI_EDIT_PASTE,
		.move = UI_EDIT_MOVE_NONE
	}
};

const u32 UI_EDIT_COMMAND_COUNT = sizeof(UI_EDIT_COMMANDS)/sizeof(ui_edit_command);

void ui_edit_perform_move(ui_context* ui, ui_edit_move move, int direction, u32 textLen)
{
	switch(move)
	{
		case UI_EDIT_MOVE_NONE:
			break;

		case UI_EDIT_MOVE_ONE:
		{
			if(direction < 0 && ui->editCursor > 0)
			{
				ui->editCursor--;
			}
			else if(direction > 0 && ui->editCursor < textLen)
			{
				ui->editCursor++;
			}
		} break;

		case UI_EDIT_MOVE_LINE:
		{
			if(direction < 0)
			{
				ui->editCursor = 0;
			}
			else if(direction > 0)
			{
				ui->editCursor = textLen;
			}
		} break;

		case UI_EDIT_MOVE_WORD:
			DEBUG_ASSERT(0, "not implemented yet");
			break;
	}
}

str32 ui_edit_perform_operation(ui_context* ui, ui_edit_op operation, ui_edit_move move, int direction, str32 codepoints)
{
	switch(operation)
	{
		case UI_EDIT_MOVE:
		{
			//NOTE(martin): we place the cursor on the direction-most side of the selection
			//              before performing the move
			u32 cursor = direction < 0 ?
				         minimum(ui->editCursor, ui->editMark) :
				         maximum(ui->editCursor, ui->editMark);
			ui->editCursor = cursor;

			if(ui->editCursor == ui->editMark || move != UI_EDIT_MOVE_ONE)
			{
				//NOTE: we special case move-one when there is a selection
				//      (just place the cursor at begining/end of selection)
				ui_edit_perform_move(ui, move, direction, codepoints.len);
			}
			ui->editMark = ui->editCursor;
		} break;

		case UI_EDIT_SELECT:
		{
			ui_edit_perform_move(ui, move, direction, codepoints.len);
		} break;

		case UI_EDIT_SELECT_EXTEND:
		{
			if((direction > 0) != (ui->editCursor > ui->editMark))
			{
				u32 tmp = ui->editCursor;
				ui->editCursor = ui->editMark;
				ui->editMark = tmp;
			}
			ui_edit_perform_move(ui, move, direction, codepoints.len);
		} break;

		case UI_EDIT_DELETE:
		{
			if(ui->editCursor == ui->editMark)
			{
				ui_edit_perform_move(ui, move, direction, codepoints.len);
			}
			codepoints = ui_edit_delete_selection(ui, codepoints);
			ui->editMark = ui->editCursor;
		} break;

		case UI_EDIT_CUT:
		{
			ui_edit_copy_selection_to_clipboard(ui, codepoints);
			codepoints = ui_edit_delete_selection(ui, codepoints);
		} break;

		case UI_EDIT_COPY:
		{
			ui_edit_copy_selection_to_clipboard(ui, codepoints);
		} break;

		case UI_EDIT_PASTE:
		{
			codepoints = ui_edit_replace_selection_with_clipboard(ui, codepoints);
		} break;

		case UI_EDIT_SELECT_ALL:
		{
			ui->editCursor = 0;
			ui->editMark = codepoints.len;
		} break;
	}
	ui->editCursorBlinkStart = ui->frameTime;

	return(codepoints);
}

void ui_text_box_render(ui_box* box, void* data)
{
	str32 codepoints = *(str32*)data;
	ui_context* ui = ui_get_context();

	u32 firstDisplayedChar = 0;
	if(ui_box_active(box))
	{
		firstDisplayedChar = ui->editFirstDisplayedChar;
	}

	ui_style* style = &box->style;
	mg_font_extents extents = mg_font_get_scaled_extents(style->font, style->fontSize);
	f32 lineHeight = extents.ascent + extents.descent;

	str32 before = str32_slice(codepoints, 0, firstDisplayedChar);
	mp_rect beforeBox = mg_text_bounding_box_utf32(style->font, style->fontSize, before);

	f32 textMargin = 5; //TODO: make that configurable

	f32 textX = textMargin + box->rect.x - beforeBox.w;
	f32 textTop = box->rect.y + 0.5*(box->rect.h - lineHeight);
	f32 textY = textTop + extents.ascent ;

	if(box->active)
	{
		u32 selectStart = minimum(ui->editCursor, ui->editMark);
		u32 selectEnd = maximum(ui->editCursor, ui->editMark);

		str32 beforeSelect = str32_slice(codepoints, 0, selectStart);
		mp_rect beforeSelectBox = mg_text_bounding_box_utf32(style->font, style->fontSize, beforeSelect);
		beforeSelectBox.x += textX;
		beforeSelectBox.y += textY;

		if(selectStart != selectEnd)
		{
			str32 select = str32_slice(codepoints, selectStart, selectEnd);
			str32 afterSelect = str32_slice(codepoints, selectEnd, codepoints.len);
			mp_rect selectBox = mg_text_bounding_box_utf32(style->font, style->fontSize, select);
			mp_rect afterSelectBox = mg_text_bounding_box_utf32(style->font, style->fontSize, afterSelect);

			selectBox.x += beforeSelectBox.x + beforeSelectBox.w;
			selectBox.y += textY;

			mg_set_color_rgba(0, 0, 1, 1);
			mg_rectangle_fill(selectBox.x, selectBox.y, selectBox.w, lineHeight);

			mg_set_font(style->font);
			mg_set_font_size(style->fontSize);
			mg_set_color(style->color);

			mg_move_to(textX, textY);
			mg_codepoints_outlines(beforeSelect);
			mg_fill();

			mg_set_color_rgba(1, 1, 1, 1);
			mg_codepoints_outlines(select);
			mg_fill();

			mg_set_color(style->color);
			mg_codepoints_outlines(afterSelect);
			mg_fill();
		}
		else
		{
			if(!((u64)(2*(ui->frameTime - ui->editCursorBlinkStart)) & 1))
			{
				f32 caretX = box->rect.x + textMargin - beforeBox.w + beforeSelectBox.w;
				f32 caretY = textTop;
				mg_set_color(style->color);
				mg_rectangle_fill(caretX, caretY, 1, lineHeight);
			}
			mg_set_font(style->font);
			mg_set_font_size(style->fontSize);
			mg_set_color(style->color);

			mg_move_to(textX, textY);
			mg_codepoints_outlines(codepoints);
			mg_fill();
		}
	}
	else
	{
		mg_set_font(style->font);
		mg_set_font_size(style->fontSize);
		mg_set_color(style->color);

		mg_move_to(textX, textY);
		mg_codepoints_outlines(codepoints);
		mg_fill();
	}
}

ui_text_box_result ui_text_box(const char* name, mem_arena* arena, str8 text)
{
	ui_context* ui = ui_get_context();

	ui_text_box_result result = {.text = text};

	ui_flags frameFlags = UI_FLAG_CLICKABLE
	                    | UI_FLAG_DRAW_BACKGROUND
	                    | UI_FLAG_DRAW_BORDER
	                    | UI_FLAG_CLIP
	                    | UI_FLAG_DRAW_PROC;

	ui_box* frame = ui_box_make(name, frameFlags);
	ui_style* style = &frame->style;
	f32 textMargin = 5; //TODO parameterize this margin! must be the same as in ui_text_box_render

	mg_font_extents extents = mg_font_get_scaled_extents(style->font, style->fontSize);

	ui_sig sig = ui_box_sig(frame);

	if(sig.hovering)
	{
		ui_box_set_hot(frame, true);

		if(sig.pressed)
		{
			if(!ui_box_active(frame))
			{
				ui_box_activate(frame);

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
			vec2 pos = ui_mouse_position();
			f32 cursorX = pos.x - frame->rect.x - textMargin;

			str32 codepoints = utf8_push_to_codepoints(&ui->frameArena, text);
			i32 newCursor = codepoints.len;
			f32 x = 0;
			for(int i = ui->editFirstDisplayedChar; i<codepoints.len; i++)
			{
				mp_rect bbox = mg_text_bounding_box_utf32(style->font, style->fontSize, str32_slice(codepoints, i, i+1));
				if(x + 0.5*bbox.w > cursorX)
				{
					newCursor = i;
					break;
				}
				x += bbox.w;
			}
			//NOTE: put cursor the closest to new cursor (this maximizes the resulting selection,
			//      and seems to be the standard behaviour across a number of text editor)
			if(abs(newCursor - ui->editCursor) > abs(newCursor - ui->editMark))
			{
				i32 tmp = ui->editCursor;
				ui->editCursor = ui->editMark;
				ui->editMark = tmp;
			}
			//NOTE: set the new cursor, and set or leave the mark depending on mode
			ui->editCursor = newCursor;
			if(sig.pressed && !(mp_key_mods(&ui->input) & MP_KEYMOD_SHIFT))
			{
				ui->editMark = ui->editCursor;
			}
		}
	}
	else
	{
		ui_box_set_hot(frame, false);

		if(sig.pressed)
		{
			if(ui_box_active(frame))
			{
				ui_box_deactivate(frame);

				//NOTE loose focus
				ui->focus = 0;
			}
		}
	}

	if(ui_box_active(frame))
	{
		str32 oldCodepoints = utf8_push_to_codepoints(&ui->frameArena, text);
		str32 codepoints = oldCodepoints;
		ui->editCursor = Clamp(ui->editCursor, 0, codepoints.len);
		ui->editMark = Clamp(ui->editMark, 0, codepoints.len);

		//NOTE replace selection with input codepoints
		str32 input = mp_input_text_utf32(&ui->input, &ui->frameArena);
		if(input.len)
		{
			codepoints = ui_edit_replace_selection_with_codepoints(ui, codepoints, input);
			ui->editCursorBlinkStart = ui->frameTime;
		}

		//NOTE handle shortcuts
		mp_keymod_flags mods = mp_key_mods(&ui->input);

		for(int i=0; i<UI_EDIT_COMMAND_COUNT; i++)
		{
			const ui_edit_command* command = &(UI_EDIT_COMMANDS[i]);

			if( (mp_key_pressed(&ui->input, command->key) || mp_key_repeated(&ui->input, command->key))
			  && mods == command->mods)
			{
				codepoints = ui_edit_perform_operation(ui, command->operation, command->move, command->direction, codepoints);
				break;
			}
		}

		//NOTE(martin): check changed/accepted
		if(oldCodepoints.ptr != codepoints.ptr)
		{
			result.changed = true;
			result.text = utf8_push_from_codepoints(arena, codepoints);
		}

		if(mp_key_pressed(&ui->input, MP_KEY_ENTER))
		{
			//TODO(martin): extract in gui_edit_complete() (and use below)
			result.accepted = true;
			ui_box_deactivate(frame);
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
				str32 firstToCursor = str32_slice(codepoints, firstDisplayedChar, ui->editCursor);
				mp_rect firstToCursorBox = mg_text_bounding_box_utf32(style->font, style->fontSize, firstToCursor);

				while(firstToCursorBox.w > (frame->rect.w - 2*textMargin))
				{
					firstDisplayedChar++;
					firstToCursor = str32_slice(codepoints, firstDisplayedChar, ui->editCursor);
					firstToCursorBox = mg_text_bounding_box_utf32(style->font, style->fontSize, firstToCursor);
				}

				ui->editFirstDisplayedChar = firstDisplayedChar;
			}
		}

		//NOTE: set renderer
		str32* renderCodepoints = mem_arena_alloc_type(&ui->frameArena, str32);
		*renderCodepoints = str32_push_copy(&ui->frameArena, codepoints);
		ui_box_set_draw_proc(frame, ui_text_box_render, renderCodepoints);
	}
	else
	{
		//NOTE: set renderer
		str32* renderCodepoints = mem_arena_alloc_type(&ui->frameArena, str32);
		*renderCodepoints = utf8_push_to_codepoints(&ui->frameArena, text);
		ui_box_set_draw_proc(frame, ui_text_box_render, renderCodepoints);
	}

	return(result);
}
