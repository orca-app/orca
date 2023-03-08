/************************************************************//**
*
*	@file: ui.c
*	@author: Martin Fouilleul
*	@date: 08/08/2022
*	@revision:
*
*****************************************************************/
#include"memory.h"
#include"hash.h"
#include"platform_clock.h"
#include"ui.h"

#define LOG_SUBSYSTEM "UI"


static ui_style UI_STYLE_DEFAULTS =
{
	.size.width = {.kind = UI_SIZE_CHILDREN,
	               .value = 0,
	               .strictness = 0},
	.size.height = {.kind = UI_SIZE_CHILDREN,
	                .value = 0,
	                .strictness = 0},

	.layout = {.axis = UI_AXIS_Y,
	           .align = {UI_ALIGN_START,
	                     UI_ALIGN_START}},
	.color = {0, 0, 0, 1},
	.fontSize = 16,
};
//-----------------------------------------------------------------------------
// context
//-----------------------------------------------------------------------------

const u32 UI_MAX_INPUT_CHAR_PER_FRAME = 64;

typedef struct ui_input_text
{
	u8 count;
	utf32 codePoints[UI_MAX_INPUT_CHAR_PER_FRAME];

} ui_input_text;

typedef struct ui_stack_elt ui_stack_elt;
struct ui_stack_elt
{
	ui_stack_elt* parent;
	union
	{
		ui_box* box;
		ui_size size;
		mp_rect clip;
	};
};

typedef struct ui_tag_elt
{
	list_elt listElt;
	ui_tag tag;
} ui_tag_elt;

const u64 UI_BOX_MAP_BUCKET_COUNT = 1024;

typedef struct ui_context
{
	bool init;

	u64 frameCounter;
	f64 frameTime;
	f64 lastFrameDuration;

	mem_arena frameArena;
	mem_pool boxPool;
	list_info boxMap[UI_BOX_MAP_BUCKET_COUNT];

	ui_box* root;
	ui_box* overlay;
	ui_stack_elt* boxStack;
	ui_stack_elt* clipStack;

	list_info nextBoxBeforeRules;
	list_info nextBoxAfterRules;
	list_info nextBoxTags;

	u32 z;
	ui_box* hovered;

	ui_box* focus;
	i32 editCursor;
	i32 editMark;
	i32 editFirstDisplayedChar;
	f64 editCursorBlinkStart;

} ui_context;

__thread ui_context __uiThreadContext = {0};
__thread ui_context* __uiCurrentContext = 0;

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
		LOG_ERROR("ui stack underflow\n");
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
	ui_tag tag = {.hash = mp_hash_aes_string(string)};
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

	ui_key key = {};
	key.hash = mp_hash_aes_string_seed(string, seed);
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
		seed = mp_hash_aes_string_seed(elt->string, seed);
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

vec2 ui_mouse_position(void)
{
	vec2 mousePos = mp_mouse_position();
	return(mousePos);
}

vec2 ui_mouse_delta(void)
{
	ui_context* ui = ui_get_context();
	vec2 delta = mp_mouse_delta();
	return(delta);
}

vec2 ui_mouse_wheel(void)
{
	ui_context* ui = ui_get_context();
	vec2 delta = mp_mouse_wheel();
	return(delta);
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
	}
	else
	{
		//maybe this should be a warning that we're trying to make the box twice in the same frame?
		LOG_WARNING("trying to make ui box '%.*s' multiple times in the same frame\n", (int)box->string.len, box->string.ptr);
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

void ui_box_set_render_proc(ui_box* box, ui_box_render_proc proc, void* data)
{
	box->renderProc = proc;
	box->renderData = data;
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

	sig.box = box;

	if(!box->closed && !box->parentClosed)
	{
		vec2 mousePos = ui_mouse_position();

		sig.hovering = ui_box_hovering(box, mousePos);

		if(box->flags & UI_FLAG_CLICKABLE)
		{
			if(sig.hovering)
			{
				sig.pressed = mp_mouse_pressed(MP_MOUSE_LEFT);
				if(sig.pressed)
				{
					box->dragging = true;
				}
				sig.doubleClicked = mp_mouse_double_clicked(MP_MOUSE_LEFT);
				sig.rightPressed = mp_mouse_pressed(MP_MOUSE_RIGHT);
			}

			sig.released = mp_mouse_released(MP_MOUSE_LEFT);
			if(sig.released)
			{
				if(box->dragging && sig.hovering)
				{
					sig.clicked = true;
				}
			}

			if(!mp_mouse_down(MP_MOUSE_LEFT))
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
	ui_animate_f32(ui, &size->strictness, target.strictness, animationTime);
}

void ui_box_compute_styling(ui_context* ui, ui_box* box)
{
	ui_style* targetStyle = box->targetStyle;
	DEBUG_ASSERT(targetStyle);

	f32 animationTime = targetStyle->animationTime;

	//NOTE: interpolate based on transition values
	u32 flags = box->targetStyle->animationFlags;

	if(box->fresh)
	{
		box->style = *targetStyle;
	}
	else
	{
		if(flags & UI_STYLE_ANIMATE_SIZE_WIDTH)
		{
			ui_animate_ui_size(ui, &box->style.size.c[UI_AXIS_X], targetStyle->size.c[UI_AXIS_X], animationTime);
		}
		else
		{
			box->style.size.c[UI_AXIS_X] = targetStyle->size.c[UI_AXIS_X];
		}

		if(flags & UI_STYLE_ANIMATE_SIZE_HEIGHT)
		{
			ui_animate_ui_size(ui, &box->style.size.c[UI_AXIS_Y], targetStyle->size.c[UI_AXIS_Y], animationTime);
		}
		else
		{
			box->style.size.c[UI_AXIS_Y] = targetStyle->size.c[UI_AXIS_Y];
		}

		if(flags & UI_STYLE_ANIMATE_COLOR)
		{
			ui_animate_color(ui, &box->style.color, targetStyle->color, animationTime);
		}
		else
		{
			box->style.color = targetStyle->color;
		}


		if(flags & UI_STYLE_ANIMATE_BG_COLOR)
		{
			ui_animate_color(ui, &box->style.bgColor, targetStyle->bgColor, animationTime);
		}
		else
		{
			box->style.bgColor = targetStyle->bgColor;
		}

		if(flags & UI_STYLE_ANIMATE_BORDER_COLOR)
		{
			ui_animate_color(ui, &box->style.borderColor, targetStyle->borderColor, animationTime);
		}
		else
		{
			box->style.borderColor = targetStyle->borderColor;
		}

		if(flags & UI_STYLE_ANIMATE_FONT_SIZE)
		{
			ui_animate_f32(ui, &box->style.fontSize, targetStyle->fontSize, animationTime);
		}
		else
		{
			box->style.fontSize = targetStyle->fontSize;
		}

		if(flags & UI_STYLE_ANIMATE_BORDER_SIZE)
		{
			ui_animate_f32(ui, &box->style.borderSize, targetStyle->borderSize, animationTime);
		}
		else
		{
			box->style.borderSize = targetStyle->borderSize;
		}

		if(flags & UI_STYLE_ANIMATE_ROUNDNESS)
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
		dst->layout.margin.x = src->layout.margin.y;
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
	if(mask & UI_STYLE_ANIMATION_FLAGS)
	{
		dst->animationFlags = src->animationFlags;
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
				res = res && box->hot;
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

	//NOTE: match rules
	list_info tmpBefore = {0};
	for_list(before, rule, ui_style_rule, buildElt)
	{
		ui_style_rule_match(ui, box, rule, before, &tmpBefore);
	}
	for_list(&box->beforeRules, rule, ui_style_rule, boxElt)
	{
		list_append(before, &rule->buildElt);
		list_append(&tmpBefore, &rule->tmpElt);
		ui_style_rule_match(ui, box, rule, before, &tmpBefore);
	}

	list_info tmpAfter = {0};
	for_list(after, rule, ui_style_rule, buildElt)
	{
		ui_style_rule_match(ui, box, rule, after, &tmpAfter);
	}
	for_list(&box->afterRules, rule, ui_style_rule, boxElt)
	{
		list_append(after, &rule->buildElt);
		list_append(&tmpAfter, &rule->tmpElt);
		ui_style_rule_match(ui, box, rule, after, &tmpAfter);
	}

	//NOTE: compute static sizes
	ui_box_compute_styling(ui, box);

	if(ui_box_hidden(box))
	{
		return;
	}

	ui_style* style = &box->style;

	mp_rect textBox = {};
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

void ui_layout_downward_dependent_size(ui_context* ui, ui_box* box, int axis)
{
	f32 sum = 0;
	f32 count = 0;

	if(box->style.layout.axis == axis)
	{
		int count = 0;
		for_list(&box->children, child, ui_box, listElt)
		{
			if(!ui_box_hidden(child))
			{
				ui_layout_downward_dependent_size(ui, child, axis);
				if(!child->style.floating.c[axis])
				{
					//TODO: maybe log an error if child is dependant on parent
					sum += child->rect.c[2+axis];
					count++;
				}
			}
		}
		box->spacing[axis] = maximum(0, count-1)*box->style.layout.spacing;
	}
	else
	{
		for_list(&box->children, child, ui_box, listElt)
		{
			if(!ui_box_hidden(child))
			{
				ui_layout_downward_dependent_size(ui, child, axis);
				if(!child->style.floating.c[axis])
				{
					sum = maximum(sum, child->rect.c[2+axis]);
				}
			}
		}
		box->spacing[axis] = 0;
	}

	box->childrenSum[axis] = sum;

	ui_size* size = &box->style.size.c[axis];
	if(size->kind == UI_SIZE_CHILDREN)
	{
		f32 margin = box->style.layout.margin.c[axis];
		box->rect.c[2+axis] = sum + box->spacing[axis] + 2*margin;
	}
}

void ui_layout_upward_dependent_size(ui_context* ui, ui_box* box, int axis)
{
	if(ui_box_hidden(box))
	{
		return;
	}

	ui_size* size = &box->style.size.c[axis];

	if(size->kind == UI_SIZE_PARENT)
	{
		ui_box* parent = box->parent;
		if(parent)
		{
			f32 margin = parent->style.layout.margin.c[axis];
			box->rect.c[2+axis] = maximum(0, parent->rect.c[2+axis] - parent->spacing[axis] - 2*margin) * size->value;
		}
		//TODO else?
	}

	for_list(&box->children, child, ui_box, listElt)
	{
		ui_layout_upward_dependent_size(ui, child, axis);
	}
}

void ui_layout_solve_conflicts(ui_context* ui, ui_box* box, int axis)
{
	//TODO
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
				if((child->targetStyle->animationFlags & UI_STYLE_ANIMATE_POS)
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

	ui_styling_prepass(ui, ui->root, &beforeRules, &afterRules);

	for(int axis=0; axis<UI_AXIS_COUNT; axis++)
	{
		ui_layout_downward_dependent_size(ui, ui->root, axis);
		ui_layout_upward_dependent_size(ui, ui->root, axis);
		ui_layout_solve_conflicts(ui, ui->root, axis);
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

	if((box->flags & UI_FLAG_DRAW_RENDER_PROC) && box->renderProc)
	{
		box->renderProc(box, box->renderData);
	}

	for_list(&box->children, child, ui_box, listElt)
	{
		ui_draw_box(child);
	}

	if(box->flags & UI_FLAG_DRAW_TEXT)
	{
		mp_rect textBox = mg_text_bounding_box(style->font, style->fontSize, box->string);

		f32 x = box->rect.x + 0.5*(box->rect.w - textBox.w);
		f32 y = box->rect.y + 0.5*(box->rect.h - textBox.h) - textBox.y;

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

void ui_begin_frame()
{
	ui_context* ui = ui_get_context();

	mem_arena_clear(&ui->frameArena);

	ui->frameCounter++;
	f64 time = mp_get_time(MP_CLOCK_MONOTONIC);
	ui->lastFrameDuration = time - ui->frameTime;
	ui->frameTime = time;

	ui->clipStack = 0;
	ui->z = 0;

	vec2 size = mg_canvas_size();

	ui_style defaultStyle = {0};
	defaultStyle.size.c[UI_AXIS_X] = (ui_size){UI_SIZE_PIXELS, size.x};
	defaultStyle.size.c[UI_AXIS_Y] = (ui_size){UI_SIZE_PIXELS, size.y};

	ui->root = ui_box_begin("_root_", 0);
	*ui->root->targetStyle = defaultStyle;

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
}

//-----------------------------------------------------------------------------
// Init / cleanup
//-----------------------------------------------------------------------------
void ui_init(void)
{
	ui_context* ui = &__uiThreadContext;
	if(!ui->init)
	{
		__uiCurrentContext = &__uiThreadContext;

		memset(ui, 0, sizeof(ui_context));
		mem_arena_init(&ui->frameArena);
		mem_pool_init(&ui->boxPool, sizeof(ui_box));
		ui->init = true;
	}
}

void ui_cleanup(void)
{
	ui_context* ui = ui_get_context();
	mem_arena_release(&ui->frameArena);
	mem_pool_release(&ui->boxPool);
	ui->init = false;
}


//-----------------------------------------------------------------------------
// Basic helpers
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

ui_sig ui_button_str8(str8 label)
{
	ui_context* ui = ui_get_context();

	ui_style defaultStyle = {.size.width = {UI_SIZE_TEXT},
	                         .size.height = {UI_SIZE_TEXT},
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

ui_sig ui_button(const char* label)
{
	return(ui_button_str8(STR8((char*)label)));
}


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

/*
ui_box* ui_panel_begin(const char* name)
{
	ui_flags panelFlags = UI_FLAG_DRAW_BACKGROUND
	                    | UI_FLAG_DRAW_BORDER
	                    | UI_FLAG_CLIP
	                    | UI_FLAG_BLOCK_MOUSE;
	ui_box* panel = ui_box_begin(name, panelFlags);

	return(panel);
}

void ui_panel_end()
{
	ui_box* panel = ui_box_top();
	ui_sig sig = ui_box_sig(panel);

	f32 contentsW = ClampLowBound(panel->childrenSum[0], panel->rect.w);
	f32 contentsH = ClampLowBound(panel->childrenSum[1], panel->rect.h);

	contentsW = ClampLowBound(contentsW, 1);
	contentsH = ClampLowBound(contentsH, 1);

	ui_box* scrollBarX = 0;
	ui_box* scrollBarY = 0;

	if(contentsW > panel->rect.w)
	{
		f32 thumbRatioX = panel->rect.w / contentsW;
		f32 sliderX = panel->scroll.x /(contentsW - panel->rect.w);

		scrollBarX = ui_scrollbar("scrollerX", thumbRatioX, &sliderX);
//		ui_box_set_size(scrollBarX, UI_AXIS_X, UI_SIZE_PARENT, 1., 0);
//		ui_box_set_size(scrollBarX, UI_AXIS_Y, UI_SIZE_PIXELS, 10, 0);

		panel->scroll.x = sliderX * (contentsW - panel->rect.w);
		if(sig.hovering)
		{
			panel->scroll.x += sig.wheel.x;
			ui_box_activate(scrollBarX);
		}
	}

	if(contentsH > panel->rect.h)
	{
		f32 thumbRatioY = panel->rect.h / contentsH;
		f32 sliderY = panel->scroll.y /(contentsH - panel->rect.h);

		scrollBarY = ui_scrollbar("scrollerY", thumbRatioY, &sliderY);
//		ui_box_set_size(scrollBarY, UI_AXIS_X, UI_SIZE_PIXELS, 10, 0);
//		ui_box_set_size(scrollBarY, UI_AXIS_Y, UI_SIZE_PARENT, 1., 0);

		panel->scroll.y = sliderY * (contentsH - panel->rect.h);
		if(sig.hovering)
		{
			panel->scroll.y += sig.wheel.y;
			ui_box_activate(scrollBarY);
		}
	}

	panel->scroll.x = Clamp(panel->scroll.x, 0, contentsW - panel->rect.w);
	panel->scroll.y = Clamp(panel->scroll.y, 0, contentsH - panel->rect.h);

	if(scrollBarX)
	{
//		ui_box_set_floating(scrollBarX, UI_AXIS_X, panel->scroll.x);
//		ui_box_set_floating(scrollBarX, UI_AXIS_Y, panel->scroll.y + panel->rect.h - 12);
	}

	if(scrollBarY)
	{
//		ui_box_set_floating(scrollBarY, UI_AXIS_X, panel->scroll.x + panel->rect.w - 12);
//		ui_box_set_floating(scrollBarY, UI_AXIS_Y, panel->scroll.y);
	}

	ui_box_end();
}
*/

ui_sig ui_tooltip_begin(const char* name)
{
	ui_context* ui = ui_get_context();
	ui_box_push(ui->overlay);

	vec2 p = ui_mouse_position();

	ui_flags flags = UI_FLAG_DRAW_BACKGROUND
	               | UI_FLAG_DRAW_BORDER;

	ui_box* tooltip = ui_box_make(name, flags);
//	ui_box_set_size(tooltip, UI_AXIS_X, UI_SIZE_CHILDREN, 0, 0);
//	ui_box_set_size(tooltip, UI_AXIS_Y, UI_SIZE_CHILDREN, 0, 0);
//	ui_box_set_floating(tooltip, UI_AXIS_X, p.x);
//	ui_box_set_floating(tooltip, UI_AXIS_Y, p.y);
	ui_box_push(tooltip);

	return(ui_box_sig(tooltip));
}

void ui_tooltip_end(void)
{
	ui_box_pop(); // tooltip
	ui_box_pop(); // ui->overlay
}

void ui_menu_bar_begin(const char* name)
{
	ui_box* bar = ui_box_begin(name, UI_FLAG_DRAW_BACKGROUND);
//	ui_box_set_size(bar, UI_AXIS_X, UI_SIZE_PARENT, 1., 0);
//	ui_box_set_size(bar, UI_AXIS_Y, UI_SIZE_CHILDREN, 0, 0);
	//ui_box_set_layout(bar, UI_AXIS_X, UI_ALIGN_START, UI_ALIGN_START);
/*
	ui_push_size(UI_AXIS_X, UI_SIZE_TEXT, 0, 0);
	ui_push_size(UI_AXIS_Y, UI_SIZE_TEXT, 0, 0);
*/
	ui_sig sig = ui_box_sig(bar);
	if(!sig.hovering && mp_mouse_released(MP_MOUSE_LEFT))
	{
		ui_box_deactivate(bar);
	}
}

void ui_menu_bar_end(void)
{
/*
	ui_pop_size(UI_AXIS_X);
	ui_pop_size(UI_AXIS_Y);
*/
	ui_box_end(); // menu bar
}

void ui_menu_begin(const char* label)
{
	ui_box* button = ui_box_make(label, UI_FLAG_CLICKABLE | UI_FLAG_DRAW_TEXT);
	ui_box* bar = button->parent;

	ui_sig sig = ui_box_sig(button);
	ui_sig barSig = ui_box_sig(bar);

	ui_context* ui = ui_get_context();
	ui_box_push(ui->overlay);

	ui_flags flags = UI_FLAG_DRAW_BACKGROUND
	               | UI_FLAG_DRAW_BORDER;

	ui_box* menu = ui_box_make(label, flags);
//	ui_box_set_size(menu, UI_AXIS_X, UI_SIZE_CHILDREN, 0, 0);
//	ui_box_set_size(menu, UI_AXIS_Y, UI_SIZE_CHILDREN, 0, 0);
//	ui_box_set_floating(menu, UI_AXIS_X, button->rect.x);
//	ui_box_set_floating(menu, UI_AXIS_Y, button->rect.y + button->rect.h);
	//ui_box_set_layout(menu, UI_AXIS_Y, UI_ALIGN_START, UI_ALIGN_START);

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
	ui_box_pop(); // overlay;
}

////////////////////////////////////////////////////////////////////////////////////////////////

/*
void ui_edit_set_cursor_from_mouse(ui_context* gui, ui_style* style, f32 textStartX)
{
	mp_graphics_font_extents fontExtents;
	mp_graphics_font_get_extents(gui->graphics, style->font, &fontExtents);
	f32 fontScale = mp_graphics_font_get_scale_for_em_pixels(gui->graphics, style->font, style->fontSize);

	//NOTE(martin): find cursor position from mouse position
	gui->editCursor = gui->editBufferSize;

	mp_graphics_text_extents glyphExtents[UI_EDIT_BUFFER_MAX_SIZE];
	mp_string32 codePoints = {gui->editBufferSize, gui->editBuffer};
	mp_string32 glyphs = mp_graphics_font_push_glyph_indices(gui->graphics, style->font, mem_scratch(), codePoints);
	mp_graphics_font_get_glyph_extents(gui->graphics, style->font, glyphs, glyphExtents);

	mp_graphics_set_font(gui->graphics, style->font);
	vec2 dimToFirst = mp_graphics_get_glyphs_dimensions(gui->graphics, mp_string32_slice(glyphs, 0, gui->firstDisplayedChar));

	ui_transform tr = ui_transform_top(gui);
	f32 x = gui->input.mouse.x + tr.x - textStartX;
	f32 glyphX = -dimToFirst.x;

	for(int i=0; i<gui->editBufferSize; i++)
	{
		if(x < glyphX + glyphExtents[i].xAdvance*fontScale/2)
		{
			gui->editCursor = i;
			return;
		}
		glyphX += glyphExtents[i].xAdvance*fontScale;
	}
	return;
}
*/

str32 ui_edit_replace_selection_with_codepoints(ui_context* ui, str32 codepoints, str32 input)
{
	u32 start = minimum(ui->editCursor, ui->editMark);
	u32 end = maximum(ui->editCursor, ui->editMark);

	str32 before = str32_slice(codepoints, 0, start);
	str32 after = str32_slice(codepoints, end, codepoints.len);

	str32_list list = {};
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
}

str32 ui_edit_replace_selection_with_clipboard(ui_context* ui, str32 codepoints)
{
	str8 string = mp_clipboard_get_string(&ui->frameArena);
	str32 input = utf8_push_to_codepoints(&ui->frameArena, string);
	str32 result = ui_edit_replace_selection_with_codepoints(ui, codepoints, input);
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
		.mods = MP_KEYMOD_CMD,
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
		.mods = MP_KEYMOD_CMD,
		.operation = UI_EDIT_CUT,
		.move = UI_EDIT_MOVE_NONE
	},
	//NOTE(martin): copy
	{
		.key = MP_KEY_C,
		.mods = MP_KEYMOD_CMD,
		.operation = UI_EDIT_COPY,
		.move = UI_EDIT_MOVE_NONE
	},
	//NOTE(martin): paste
	{
		.key = MP_KEY_V,
		.mods = MP_KEYMOD_CMD,
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
				mg_set_color_rgba(0, 0, 0, 1);
				mg_rectangle_fill(caretX, caretY, 2, lineHeight);
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
	                    | UI_FLAG_DRAW_RENDER_PROC
	                    | UI_FLAG_SCROLLABLE;

	ui_box* frame = ui_box_make(name, frameFlags);
	ui_style* style = &frame->style;
	f32 textMargin = 5; //TODO parameterize this margin! must be the same as in ui_text_box_render

	mg_font_extents extents = mg_font_get_scaled_extents(style->font, style->fontSize);
//	ui_box_set_size(frame, UI_AXIS_Y, UI_SIZE_PIXELS, extents.ascent+extents.descent+10, 1);

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
			if(sig.pressed && !(mp_key_mods() & MP_KEYMOD_SHIFT))
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
		str32 input = mp_input_text_utf32(&ui->frameArena);
		if(input.len)
		{
			codepoints = ui_edit_replace_selection_with_codepoints(ui, codepoints, input);
			ui->editCursorBlinkStart = ui->frameTime;
		}

		//NOTE handle shortcuts
		mp_keymod_flags mods = mp_key_mods();

		for(int i=0; i<UI_EDIT_COMMAND_COUNT; i++)
		{
			const ui_edit_command* command = &(UI_EDIT_COMMANDS[i]);

			if( (mp_key_pressed(command->key) || mp_key_repeated(command->key))
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

		if(mp_key_pressed(MP_KEY_ENTER))
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
		ui_box_set_render_proc(frame, ui_text_box_render, renderCodepoints);
	}
	else
	{
		//NOTE: set renderer
		str32* renderCodepoints = mem_arena_alloc_type(&ui->frameArena, str32);
		*renderCodepoints = utf8_push_to_codepoints(&ui->frameArena, text);
		ui_box_set_render_proc(frame, ui_text_box_render, renderCodepoints);
	}

	return(result);
}


#undef LOG_SUBSYSTEM
