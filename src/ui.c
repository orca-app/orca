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

//-----------------------------------------------------------------------------
// context
//-----------------------------------------------------------------------------

const u32 UI_MAX_INPUT_CHAR_PER_FRAME = 64;

typedef struct ui_input_text
{
	u8 count;
	utf32 codePoints[UI_MAX_INPUT_CHAR_PER_FRAME];

} ui_input_text;

typedef struct ui_style_attr
{
	ui_style_tag tag;
	ui_style_selector selector;
	union
	{
		ui_size size;
		mg_color color;
		mg_font font;
		f32 value;
	};
} ui_style_attr;

typedef struct ui_stack_elt ui_stack_elt;
struct ui_stack_elt
{
	ui_stack_elt* parent;
	union
	{
		ui_box* box;
		ui_size size;
		mp_rect clip;
		ui_style_attr attr;
	};
};

const u64 UI_BOX_MAP_BUCKET_COUNT = 1024;

typedef struct ui_context
{
	bool init;

	f32 width;
	f32 height;

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

	ui_stack_elt* sizeStack[UI_AXIS_COUNT];
	ui_stack_elt* bgColorStack;
	ui_stack_elt* fgColorStack;
	ui_stack_elt* borderColorStack;
	ui_stack_elt* fontColorStack;
	ui_stack_elt* fontStack;
	ui_stack_elt* fontSizeStack;
	ui_stack_elt* borderSizeStack;
	ui_stack_elt* roundnessStack;
	ui_stack_elt* animationTimeStack;

	u32 z;
	ui_box* hovered;

	ui_box* focus;
	u32 editCursor;
	u32 editMark;
	u32 editFirstDisplayedChar;
	f64 editCursorBlinkStart;

} ui_context;

__thread ui_context __uiContext = {0};

ui_context* ui_get_context()
{
	return(&__uiContext);
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

mp_rect ui_clip_top()
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

void ui_clip_pop()
{
	ui_context* ui = ui_get_context();
	ui_stack_pop(&ui->clipStack);
}

ui_box* ui_box_top()
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

void ui_box_pop()
{
	ui_context* ui = ui_get_context();
	ui_box* box = ui_box_top();
	if(box->flags & UI_FLAG_CLIP)
	{
		ui_clip_pop();
	}

	ui_stack_pop(&ui->boxStack);
}

ui_style_attr* ui_push_style_attr(ui_context* ui, ui_stack_elt** stack)
{
	ui_stack_elt* elt = ui_stack_push(ui, stack);
	return(&elt->attr);
}

void ui_push_size(ui_axis axis, ui_size_kind kind, f32 value, f32 strictness)
{
	ui_push_size_ext(UI_STYLE_TAG_ANY, UI_STYLE_SEL_ANY, axis, kind, value, strictness);

}

void ui_push_size_ext(ui_style_tag tag, ui_style_selector selector, ui_axis axis, ui_size_kind kind, f32 value, f32 strictness)
{
	ui_context* ui = ui_get_context();
	ui_style_attr* attr = ui_push_style_attr(ui, &ui->sizeStack[axis]);
	attr->tag = tag;
	attr->selector = selector;
	attr->size = (ui_size){kind, value, strictness};
}

void ui_pop_size(ui_axis axis)
{
	ui_context* ui = ui_get_context();
	ui_stack_pop(&ui->sizeStack[axis]);
}

#define UI_STYLE_STACK_OP_DEF(name, stack, type, arg) \
void _cat3_(ui_push_, name, _ext)(ui_style_tag tag, ui_style_selector selector, type arg) \
{ \
	ui_context* ui = ui_get_context(); \
	ui_style_attr* attr = ui_push_style_attr(ui, &ui->stack); \
	attr->tag = tag; \
	attr->selector = selector; \
	attr->arg = arg; \
} \
void _cat2_(ui_push_, name)(type arg) \
{ \
	_cat3_(ui_push_, name, _ext)(UI_STYLE_TAG_ANY, UI_STYLE_SEL_ANY, arg); \
} \
void _cat2_(ui_pop_, name)() \
{ \
	ui_context* ui = ui_get_context(); \
	ui_stack_pop(&ui->stack); \
}

UI_STYLE_STACK_OP_DEF(bg_color, bgColorStack, mg_color, color)
UI_STYLE_STACK_OP_DEF(fg_color, fgColorStack, mg_color, color)
UI_STYLE_STACK_OP_DEF(border_color, borderColorStack, mg_color, color)
UI_STYLE_STACK_OP_DEF(font_color, fontColorStack, mg_color, color)
UI_STYLE_STACK_OP_DEF(font, fontStack, mg_font, font)
UI_STYLE_STACK_OP_DEF(font_size, fontSizeStack, f32, value)
UI_STYLE_STACK_OP_DEF(border_size, borderSizeStack, f32, value)
UI_STYLE_STACK_OP_DEF(roundness, roundnessStack, f32, value)
UI_STYLE_STACK_OP_DEF(animation_time, animationTimeStack, f32, value)

ui_style_attr* ui_style_attr_query(ui_stack_elt* stack, ui_style_tag tag, ui_style_selector selector)
{
	ui_style_attr* result = 0;
	while(stack)
	{
		ui_style_attr* attr = &stack->attr;
		bool matchTag = (attr->tag == UI_STYLE_TAG_ANY)
		              ||(attr->tag == tag);

		bool matchSel = attr->selector & selector;

		if(matchTag && matchSel)
		{
			result = attr;
			break;
		}
		stack = stack->parent;
	}
	return(result);
}

mg_color ui_style_color_query(ui_stack_elt* stack, ui_style_tag tag, ui_style_selector selector)
{
	ui_style_attr* attr = ui_style_attr_query(stack, tag, selector);
	if(attr)
	{
		return(attr->color);
	}
	else
	{
		return((mg_color){});
	}
}

mg_font ui_style_font_query(ui_stack_elt* stack, ui_style_tag tag, ui_style_selector selector)
{
	ui_style_attr* attr = ui_style_attr_query(stack, tag, selector);
	if(attr)
	{
		return(attr->font);
	}
	else
	{
		return(mg_font_nil());
	}
}

f32 ui_style_float_query(ui_stack_elt* stack, ui_style_tag tag, ui_style_selector selector)
{
	ui_style_attr* attr = ui_style_attr_query(stack, tag, selector);
	if(attr)
	{
		return(attr->value);
	}
	else
	{
		return(0);
	}
}

ui_size ui_style_size_query(ui_stack_elt* stack, ui_style_tag tag, ui_style_selector selector)
{
	ui_style_attr* attr = ui_style_attr_query(stack, tag, selector);
	if(attr)
	{
		return(attr->size);
	}
	else
	{
		return((ui_size){});
	}
}

ui_style* ui_collate_style(ui_context* context, ui_style_tag tag, ui_style_selector selector)
{
	ui_style* style = mem_arena_alloc_type(&context->frameArena, ui_style);

	style->size[UI_AXIS_X] = ui_style_size_query(context->sizeStack[UI_AXIS_X], tag, selector);
	style->size[UI_AXIS_Y] = ui_style_size_query(context->sizeStack[UI_AXIS_Y], tag, selector);
	style->bgColor = ui_style_color_query(context->bgColorStack, tag, selector);
	style->fgColor = ui_style_color_query(context->fgColorStack, tag, selector);
	style->borderColor = ui_style_color_query(context->borderColorStack, tag, selector);
	style->fontColor = ui_style_color_query(context->fontColorStack, tag, selector);
	style->font = ui_style_font_query(context->fontStack, tag, selector);
	style->fontSize = ui_style_float_query(context->fontSizeStack, tag, selector);
	style->borderSize = ui_style_float_query(context->borderSizeStack, tag, selector);
	style->roundness = ui_style_float_query(context->roundnessStack, tag, selector);
	style->animationTime = ui_style_float_query(context->animationTimeStack, tag, selector);
	return(style);
}

//-----------------------------------------------------------------------------
// key hashing and caching
//-----------------------------------------------------------------------------
ui_key ui_key_from_string(str8 string)
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

bool ui_key_equal(ui_key a, ui_key b)
{
	return(a.hash == b.hash);
}

void ui_box_cache(ui_context* ui, ui_box* box)
{
	u64 index = box->key.hash & (UI_BOX_MAP_BUCKET_COUNT-1);
	ListAppend(&(ui->boxMap[index]), &box->bucketElt);
}

ui_box* ui_box_lookup(ui_context* ui, ui_key key)
{
	u64 index = key.hash & (UI_BOX_MAP_BUCKET_COUNT-1);

	for_each_in_list(&ui->boxMap[index], box, ui_box, bucketElt)
	{
		if(ui_key_equal(key, box->key))
		{
			return(box);
		}
	}
	return(0);
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

vec2 ui_mouse_position()
{
	ui_context* ui = ui_get_context();

	vec2 mousePos = mp_input_mouse_position();
	mousePos.y = ui->height - mousePos.y;
	return(mousePos);
}

vec2 ui_mouse_delta()
{
	ui_context* ui = ui_get_context();
	vec2 delta = mp_input_mouse_delta();
	delta.y *= -1.;
	return(delta);
}

vec2 ui_mouse_wheel()
{
	ui_context* ui = ui_get_context();
	vec2 delta = mp_input_mouse_wheel();
	delta.y *= -1.;
	return(delta);
}

void ui_box_compute_signals(ui_context* ui, ui_box* box)
{
	ui_sig* sig = mem_arena_alloc_type(&ui->frameArena, ui_sig);
	memset(sig, 0, sizeof(ui_sig));

	if(!box->closed && !box->parentClosed)
	{
		vec2 mousePos = ui_mouse_position();

		sig->hovering = ui_box_hovering(box, mousePos);

		if(box->flags & UI_FLAG_CLICKABLE)
		{
			if(sig->hovering)
			{
				sig->pressed = mp_input_mouse_pressed(MP_MOUSE_LEFT);
				if(sig->pressed)
				{
					box->dragging = true;
				}

				sig->clicked = mp_input_mouse_clicked(MP_MOUSE_LEFT);
				sig->doubleClicked = mp_input_mouse_clicked(MP_MOUSE_LEFT);
			}

			sig->released = mp_input_mouse_released(MP_MOUSE_LEFT);
			if(sig->released)
			{
				if(box->dragging && sig->hovering)
				{
					sig->triggered = true;
				}
			}

			if(!mp_input_mouse_down(MP_MOUSE_LEFT))
			{
				box->dragging = false;
			}

			sig->dragging = box->dragging;
		}

		sig->mouse = (vec2){mousePos.x - box->rect.x, mousePos.y - box->rect.y};
		sig->delta = ui_mouse_delta();
		sig->wheel = ui_mouse_wheel();
	}
	box->sig = sig;
}

ui_box* ui_box_make_str8(str8 string, ui_flags flags)
{
	ui_context* ui = ui_get_context();

	ui_key key = ui_key_from_string(string);
	ui_box* box = ui_box_lookup(ui, key);

	if(!box)
	{
		box = mem_pool_alloc_type(&ui->boxPool, ui_box);
		memset(box, 0, sizeof(ui_box));

		box->key = key;
		ui_box_cache(ui, box);
	}

	//NOTE: setup hierarchy
	ListInit(&box->children);
	box->parent = ui_box_top();
	if(box->parent)
	{
		ListAppend(&box->parent->children, &box->listElt);
		box->parentClosed = box->parent->closed || box->parent->parentClosed;
	}

	//NOTE: setup per-frame state
	box->frameCounter = ui->frameCounter;
	box->string = str8_push_copy(&ui->frameArena, string);
	box->flags = flags;

	box->floating[UI_AXIS_X] = false;
	box->floating[UI_AXIS_Y] = false;
	box->layout = box->parent ? box->parent->layout : (ui_layout){0};

	//NOTE: compute input signals
	ui_box_compute_signals(ui, box);

	//NOTE: compute style
	ui_style_selector selector = UI_STYLE_SEL_NORMAL;
	if(box->hot)
	{
		selector = UI_STYLE_SEL_HOT;
	}
	if(box->active)
	{
		selector = UI_STYLE_SEL_ACTIVE;
	}
	box->targetStyle = ui_collate_style(ui, box->tag, selector);

	return(box);
}

ui_box* ui_box_make(const char* cstring, ui_flags flags)
{
	str8 string = str8_from_cstring((char*)cstring);
	return(ui_box_make_str8(string, flags));
}

ui_box* ui_box_begin_str8(str8 string, ui_flags flags)
{
	ui_context* ui = ui_get_context();
	ui_box* box = ui_box_make_str8(string, flags);
	ui_box_push(box);
	return(box);
}

ui_box* ui_box_begin(const char* cstring, ui_flags flags)
{
	str8 string = str8_from_cstring((char*)cstring);
	return(ui_box_begin_str8(string, flags));
}

ui_box* ui_box_end()
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

void ui_box_set_layout(ui_box* box, ui_axis axis, ui_align alignX, ui_align alignY)
{
	box->layout = (ui_layout){axis, {alignX, alignY}};
}

void ui_box_set_size(ui_box* box, ui_axis axis, ui_size_kind kind, f32 value, f32 strictness)
{
	box->targetStyle->size[axis] = (ui_size){kind, value, strictness};
}

void ui_box_set_floating(ui_box* box, ui_axis axis, f32 pos)
{
	box->floating[axis] = true;
	box->rect.c[axis] = pos;
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

void ui_box_set_tag(ui_box* box, ui_style_tag tag)
{
	box->tag = tag;
}

ui_sig ui_box_sig(ui_box* box)
{
	return(*box->sig);
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

void ui_box_compute_styling(ui_context* ui, ui_box* box)
{
	ui_style* targetStyle = box->targetStyle;
	DEBUG_ASSERT(targetStyle);

	f32 animationTime = targetStyle->animationTime;

	box->computedStyle.size[UI_AXIS_X] = targetStyle->size[UI_AXIS_X];
	box->computedStyle.size[UI_AXIS_Y] = targetStyle->size[UI_AXIS_Y];

	//TODO: interpolate based on transition values
	ui_animate_color(ui, &box->computedStyle.bgColor, targetStyle->bgColor, animationTime);
	ui_animate_color(ui, &box->computedStyle.fgColor, targetStyle->fgColor, animationTime);
	ui_animate_color(ui, &box->computedStyle.borderColor, targetStyle->borderColor, animationTime);
	ui_animate_color(ui, &box->computedStyle.fontColor, targetStyle->fontColor, animationTime);

	box->computedStyle.font = targetStyle->font;
	ui_animate_f32(ui, &box->computedStyle.fontSize, targetStyle->fontSize, animationTime);
	ui_animate_f32(ui, &box->computedStyle.borderSize, targetStyle->borderSize, animationTime);
	ui_animate_f32(ui, &box->computedStyle.roundness, targetStyle->roundness, animationTime);
}

void ui_layout_prepass(ui_context* ui, ui_box* box)
{
	if(ui_box_hidden(box))
	{
		return;
	}

	//NOTE: compute styling and static sizes
	ui_box_compute_styling(ui, box);

	ui_style* style = &box->computedStyle;

	mp_rect textBox = {};
	ui_size desiredSize[2] = {box->computedStyle.size[UI_AXIS_X],
	                          box->computedStyle.size[UI_AXIS_Y]};

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
			box->rect.c[2+i] = textBox.c[2+i] + desiredSize[i].value*2;
		}
		else if(size.kind == UI_SIZE_PIXELS)
		{
			box->rect.c[2+i] = size.value;
		}
	}

	for_each_in_list(&box->children, child, ui_box, listElt)
	{
		ui_layout_prepass(ui, child);
	}
}

void ui_layout_upward_dependent_size(ui_context* ui, ui_box* box, int axis)
{
	if(ui_box_hidden(box))
	{
		return;
	}

	ui_size* size = &box->computedStyle.size[axis];

	if(size->kind == UI_SIZE_PARENT_RATIO)
	{
		ui_box* parent = box->parent;
		if(  parent
		  && parent->computedStyle.size[axis].kind != UI_SIZE_CHILDREN)
		{
			box->rect.c[2+axis] = parent->rect.c[2+axis] * size->value;
		}
		//TODO else?
	}

	for_each_in_list(&box->children, child, ui_box, listElt)
	{
		ui_layout_upward_dependent_size(ui, child, axis);
	}
}

void ui_layout_downward_dependent_size(ui_context* ui, ui_box* box, int axis)
{
	f32 sum = 0;
	if(box->layout.axis == axis)
	{
		for_each_in_list(&box->children, child, ui_box, listElt)
		{
			if(!ui_box_hidden(child))
			{
				ui_layout_downward_dependent_size(ui, child, axis);
				if(!child->floating[axis])
				{
					sum += child->rect.c[2+axis];
				}
			}
		}
	}
	else
	{
		for_each_in_list(&box->children, child, ui_box, listElt)
		{
			if(!ui_box_hidden(child))
			{
				ui_layout_downward_dependent_size(ui, child, axis);
				if(!child->floating[axis])
				{
					sum = maximum(sum, child->rect.c[2+axis]);
				}
			}
		}
	}

	box->childrenSum[axis] = sum;

	ui_size* size = &box->computedStyle.size[axis];
	if(size->kind == UI_SIZE_CHILDREN)
	{
		box->rect.c[2+axis] = sum  + size->value*2;
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

	ui_axis layoutAxis = box->layout.axis;
	ui_axis secondAxis = (layoutAxis == UI_AXIS_X) ? UI_AXIS_Y : UI_AXIS_X;
	ui_align* align = box->layout.align;

	vec2 origin = {box->rect.x - box->scroll.x,
	               box->rect.y - box->scroll.y};
	vec2 currentPos = origin;

	vec2 contentsSize = {maximum(box->rect.w, box->childrenSum[UI_AXIS_X]),
	                     maximum(box->rect.h, box->childrenSum[UI_AXIS_Y])};

	for(int i=0; i<UI_AXIS_COUNT; i++)
	{
		if(align[i] == UI_ALIGN_END)
		{
			currentPos.c[i] += contentsSize.c[i] - box->childrenSum[i];
		}
	}
	if(align[layoutAxis] == UI_ALIGN_CENTER)
	{
		currentPos.c[layoutAxis] += 0.5*(contentsSize.c[layoutAxis] - box->childrenSum[layoutAxis]);
	}

	for_each_in_list(&box->children, child, ui_box, listElt)
	{
		if(align[secondAxis] == UI_ALIGN_CENTER)
		{
			currentPos.c[secondAxis] = origin.c[secondAxis] + 0.5*(contentsSize.c[secondAxis] - child->rect.c[2+secondAxis]);
		}

		vec2 childPos = currentPos;
		for(int i=0; i<UI_AXIS_COUNT; i++)
		{
			if(child->floating[i])
			{
				childPos.c[i] = origin.c[i] + child->rect.c[i];
			}
		}

		ui_layout_compute_rect(ui, child, childPos);

		if(!child->floating[layoutAxis])
		{
			currentPos.c[layoutAxis] += child->rect.c[2+layoutAxis];
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
		for_each_in_list(&box->children, child, ui_box, listElt)
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
	ui_layout_prepass(ui, ui->root);

	for(int axis=0; axis<UI_AXIS_COUNT; axis++)
	{
		ui_layout_upward_dependent_size(ui, ui->root, axis);
		ui_layout_downward_dependent_size(ui, ui->root, axis);
		ui_layout_solve_conflicts(ui, ui->root, axis);
	}
	ui_layout_compute_rect(ui, ui->root, (vec2){0, 0});

	vec2 p = ui_mouse_position();
	ui_layout_find_next_hovered(ui, p);
}

//-----------------------------------------------------------------------------
// Drawing
//-----------------------------------------------------------------------------

void ui_rectangle_fill(mg_canvas canvas, mp_rect rect, f32 roundness)
{
	if(roundness)
	{
		mg_rounded_rectangle_fill(canvas, rect.x, rect.y, rect.w, rect.h, roundness);
	}
	else
	{
		mg_rectangle_fill(canvas, rect.x, rect.y, rect.w, rect.h);
	}
}

void ui_rectangle_stroke(mg_canvas canvas, mp_rect rect, f32 roundness)
{
	if(roundness)
	{
		mg_rounded_rectangle_stroke(canvas, rect.x, rect.y, rect.w, rect.h, roundness);
	}
	else
	{
		mg_rectangle_stroke(canvas, rect.x, rect.y, rect.w, rect.h);
	}
}

void ui_draw_box(mg_canvas canvas, ui_box* box)
{
	if(ui_box_hidden(box))
	{
		return;
	}

	ui_style* style = &box->computedStyle;

	if(box->flags & UI_FLAG_CLIP)
	{
		mg_clip_push(canvas, box->rect.x, box->rect.y, box->rect.w, box->rect.h);
	}

	if(box->flags & UI_FLAG_DRAW_BACKGROUND)
	{
		mg_set_color(canvas, style->bgColor);
		ui_rectangle_fill(canvas, box->rect, style->roundness);
	}

	for_each_in_list(&box->children, child, ui_box, listElt)
	{
		ui_draw_box(canvas, child);
	}

	if(box->flags & UI_FLAG_DRAW_FOREGROUND)
	{
		mg_set_color(canvas, style->fgColor);
		ui_rectangle_fill(canvas, box->rect, style->roundness);
	}

	if(box->flags & UI_FLAG_DRAW_TEXT)
	{
		mp_rect textBox = mg_text_bounding_box(style->font, style->fontSize, box->string);

		f32 x = box->rect.x + 0.5*(box->rect.w - textBox.w);
		f32 y = box->rect.y + 0.5*(box->rect.h - textBox.h) - textBox.y;

		mg_set_font(canvas, style->font);
		mg_set_font_size(canvas, style->fontSize);
		mg_set_color(canvas, style->fontColor);

		mg_move_to(canvas, x, y);
		mg_text_outlines(canvas, box->string);
		mg_fill(canvas);
	}

	if((box->flags & UI_FLAG_DRAW_RENDER_PROC) && box->renderProc)
	{
		box->renderProc(canvas, box, box->renderData);
	}

	if(box->flags & UI_FLAG_CLIP)
	{
		mg_clip_pop(canvas);
	}

	if(box->flags & UI_FLAG_DRAW_BORDER)
	{
		mg_set_width(canvas, style->borderSize);
		mg_set_color(canvas, style->borderColor);
		ui_rectangle_stroke(canvas, box->rect, style->roundness);
	}
}

void ui_draw(mg_canvas canvas)
{
	ui_context* ui = ui_get_context();

	//NOTE: draw
	mg_mat2x3 transform = {1, 0, 0,
	                       0, -1, ui->height};

	bool oldTextFlip = mg_get_text_flip(canvas);
	mg_set_text_flip(canvas, true);

	mg_matrix_push(canvas, transform);
	ui_draw_box(canvas, ui->root);
	mg_matrix_pop(canvas);

	mg_set_text_flip(canvas, oldTextFlip);

	//TODO: restore flip??
}

//-----------------------------------------------------------------------------
// frame begin/end
//-----------------------------------------------------------------------------

void ui_begin_frame(u32 width, u32 height, ui_style defaultStyle)
{
	ui_context* ui = ui_get_context();

	mem_arena_clear(&ui->frameArena);

	ui->width = width;
	ui->height = height;
	ui->frameCounter++;
	f64 time = mp_get_time(MP_CLOCK_MONOTONIC);
	ui->lastFrameDuration = time - ui->frameTime;
	ui->frameTime = time;

	ui->boxStack = 0;
	ui->sizeStack[UI_AXIS_X] = 0;
	ui->sizeStack[UI_AXIS_Y] = 0;
	ui->bgColorStack = 0;
	ui->fgColorStack = 0;
	ui->borderColorStack = 0;
	ui->fontColorStack = 0;
	ui->fontStack = 0;
	ui->fontSizeStack = 0;
	ui->borderSizeStack = 0;
	ui->roundnessStack = 0;


	ui->clipStack = 0;
	ui->z = 0;

	for(int i=0; i<UI_AXIS_COUNT; i++)
	{
		ui_push_size(i,
		             defaultStyle.size[i].kind,
		             defaultStyle.size[i].value,
		             defaultStyle.size[i].strictness);
	}
	ui_push_bg_color(defaultStyle.bgColor);
	ui_push_fg_color(defaultStyle.fgColor);
	ui_push_border_color(defaultStyle.borderColor);
	ui_push_font_color(defaultStyle.fontColor);
	ui_push_font(defaultStyle.font);
	ui_push_font_size(defaultStyle.fontSize);
	ui_push_border_size(defaultStyle.borderSize);
	ui_push_roundness(defaultStyle.roundness);


	ui_push_size(UI_AXIS_X, UI_SIZE_PIXELS, width, 0);
	ui_push_size(UI_AXIS_Y, UI_SIZE_PIXELS, height, 0);

	ui->root = ui_box_begin("_root_", 0);

	ui_box* contents = ui_box_make("_contents_", 0);
	ui_box_set_floating(contents, UI_AXIS_X, 0);
	ui_box_set_floating(contents, UI_AXIS_Y, 0);
	ui_box_set_layout(contents, UI_AXIS_Y, UI_ALIGN_START, UI_ALIGN_START);

	ui->overlay = ui_box_make("_overlay_", 0);
	ui_box_set_floating(ui->overlay, UI_AXIS_X, 0);
	ui_box_set_floating(ui->overlay, UI_AXIS_Y, 0);

	ui_pop_size(UI_AXIS_X);
	ui_pop_size(UI_AXIS_Y);

	ui_box_push(contents);
}

void ui_end_frame()
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
		for_each_in_list_safe(&ui->boxMap[i], box, ui_box, bucketElt)
		{
			if(box->frameCounter < ui->frameCounter)
			{
				ListRemove(&ui->boxMap[i], &box->bucketElt);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Init / cleanup
//-----------------------------------------------------------------------------
void ui_init()
{
	ui_context* ui = ui_get_context();
	if(!ui->init)
	{
		memset(ui, 0, sizeof(ui_context));
		mem_arena_init(&ui->frameArena);
		mem_pool_init(&ui->boxPool, sizeof(ui_box));
		ui->init = true;
	}
}

void ui_cleanup()
{
	ui_context* ui = ui_get_context();
	mem_arena_release(&ui->frameArena);
	mem_pool_release(&ui->boxPool);
	ui->init = false;
}


//-----------------------------------------------------------------------------
// Basic helpers
//-----------------------------------------------------------------------------

ui_sig ui_label(const char* label)
{
	ui_flags flags = UI_FLAG_CLIP
	               | UI_FLAG_DRAW_TEXT;
	ui_box* box = ui_box_make(label, flags);
	ui_box_set_size(box, UI_AXIS_X, UI_SIZE_TEXT, 0, 0);
	ui_box_set_size(box, UI_AXIS_Y, UI_SIZE_TEXT, 0, 0);

	ui_sig sig = ui_box_sig(box);
	return(sig);
}

ui_sig ui_button(const char* label)
{
	ui_flags flags = UI_FLAG_CLICKABLE
	               | UI_FLAG_CLIP
	               | UI_FLAG_DRAW_FOREGROUND
	               | UI_FLAG_DRAW_BORDER
	               | UI_FLAG_DRAW_TEXT
	               | UI_FLAG_HOT_ANIMATION
	               | UI_FLAG_ACTIVE_ANIMATION;

	ui_box* box = ui_box_make(label, flags);
	ui_box_set_tag(box, UI_STYLE_TAG_BUTTON);
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

ui_box* ui_scrollbar(const char* label, f32 thumbRatio, f32* scrollValue)
{
	ui_box* frame = ui_box_begin(label, 0);
	{
		ui_axis trackAxis = (frame->rect.w > frame->rect.h) ? UI_AXIS_X : UI_AXIS_Y;
		ui_axis secondAxis = (trackAxis == UI_AXIS_Y) ? UI_AXIS_X : UI_AXIS_Y;

		f32 roundness = 0.5*frame->rect.c[2+secondAxis];
		f32 animationTime = 0.5;

		ui_push_bg_color((mg_color){0, 0, 0, 0});
		ui_push_fg_color((mg_color){0, 0, 0, 0});
		ui_push_roundness(roundness);

		ui_push_bg_color_ext(UI_STYLE_TAG_ANY, UI_STYLE_SEL_HOT|UI_STYLE_SEL_ACTIVE, (mg_color){0, 0, 0, 0.5});
		ui_push_fg_color_ext(UI_STYLE_TAG_ANY, UI_STYLE_SEL_HOT|UI_STYLE_SEL_ACTIVE, (mg_color){0, 0, 0, 0.7});
		ui_push_roundness_ext(UI_STYLE_TAG_ANY, UI_STYLE_SEL_HOT|UI_STYLE_SEL_ACTIVE, roundness);
		ui_push_animation_time_ext(UI_STYLE_TAG_ANY, UI_STYLE_SEL_ANY, 1.);

		ui_flags trackFlags = UI_FLAG_CLIP
	                    	| UI_FLAG_DRAW_BACKGROUND
	                    	| UI_FLAG_HOT_ANIMATION
	                    	| UI_FLAG_ACTIVE_ANIMATION;

		ui_box* track = ui_box_begin("track", trackFlags);

			ui_box_set_size(track, trackAxis, UI_SIZE_PARENT_RATIO, 1., 0);
			ui_box_set_size(track, secondAxis, UI_SIZE_PARENT_RATIO, 1., 0);
			ui_box_set_layout(track, trackAxis, UI_ALIGN_START, UI_ALIGN_START);

			f32 beforeRatio = (*scrollValue) * (1. - thumbRatio);
			f32 afterRatio = (1. - *scrollValue) * (1. - thumbRatio);

			ui_box* beforeSpacer = ui_box_make("before", 0);
			ui_box_set_size(beforeSpacer, trackAxis, UI_SIZE_PARENT_RATIO, beforeRatio, 0);
			ui_box_set_size(beforeSpacer, secondAxis, UI_SIZE_PARENT_RATIO, 1., 0);

			ui_flags thumbFlags = UI_FLAG_CLICKABLE
		                    	| UI_FLAG_DRAW_FOREGROUND
		                    	| UI_FLAG_HOT_ANIMATION
		                    	| UI_FLAG_ACTIVE_ANIMATION;

			ui_box* thumb = ui_box_make("thumb", thumbFlags);
			ui_box_set_size(thumb, trackAxis, UI_SIZE_PARENT_RATIO, thumbRatio, 0);
			ui_box_set_size(thumb, secondAxis, UI_SIZE_PARENT_RATIO, 1., 0);

			ui_box* afterSpacer = ui_box_make("after", 0);
			ui_box_set_size(afterSpacer, trackAxis, UI_SIZE_PARENT_RATIO, afterRatio, 0);
			ui_box_set_size(afterSpacer, secondAxis, UI_SIZE_PARENT_RATIO, 1., 0);

		ui_box_end();

		ui_pop_bg_color();
		ui_pop_fg_color();
		ui_pop_roundness();
		ui_pop_bg_color();
		ui_pop_fg_color();
		ui_pop_roundness();
		ui_pop_animation_time();

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
		ui_box_set_size(scrollBarX, UI_AXIS_X, UI_SIZE_PARENT_RATIO, 1., 0);
		ui_box_set_size(scrollBarX, UI_AXIS_Y, UI_SIZE_PIXELS, 10, 0);

		panel->scroll.x = sliderX * (contentsW - panel->rect.w);
		if(sig.hovering)
		{
			panel->scroll.x += sig.wheel.x;
			ui_box_activate(scrollBarX);
		}
		panel->scroll.x = Clamp(panel->scroll.x, 0, contentsW - panel->rect.w);
	}

	if(contentsH > panel->rect.h)
	{
		f32 thumbRatioY = panel->rect.h / contentsH;
		f32 sliderY = panel->scroll.y /(contentsH - panel->rect.h);

		scrollBarY = ui_scrollbar("scrollerY", thumbRatioY, &sliderY);
		ui_box_set_size(scrollBarY, UI_AXIS_X, UI_SIZE_PIXELS, 10, 0);
		ui_box_set_size(scrollBarY, UI_AXIS_Y, UI_SIZE_PARENT_RATIO, 1., 0);

		panel->scroll.y = sliderY * (contentsH - panel->rect.h);
		if(sig.hovering)
		{
			panel->scroll.y += sig.wheel.y;
			ui_box_activate(scrollBarY);
		}
		panel->scroll.y = Clamp(panel->scroll.y, 0, contentsH - panel->rect.h);
	}

	if(scrollBarX)
	{
		ui_box_set_floating(scrollBarX, UI_AXIS_X, panel->scroll.x);
		ui_box_set_floating(scrollBarX, UI_AXIS_Y, panel->scroll.y + panel->rect.h - 12);
	}

	if(scrollBarY)
	{
		ui_box_set_floating(scrollBarY, UI_AXIS_X, panel->scroll.x + panel->rect.w - 12);
		ui_box_set_floating(scrollBarY, UI_AXIS_Y, panel->scroll.y);
	}

	ui_box_end();
}

ui_sig ui_tooltip_begin(const char* name)
{
	ui_context* ui = ui_get_context();
	ui_box_push(ui->overlay);

	vec2 p = ui_mouse_position();

	ui_flags flags = UI_FLAG_DRAW_BACKGROUND
	               | UI_FLAG_DRAW_BORDER;

	ui_box* tooltip = ui_box_make(name, flags);
	ui_box_set_size(tooltip, UI_AXIS_X, UI_SIZE_CHILDREN, 0, 0);
	ui_box_set_size(tooltip, UI_AXIS_Y, UI_SIZE_CHILDREN, 0, 0);
	ui_box_set_floating(tooltip, UI_AXIS_X, p.x);
	ui_box_set_floating(tooltip, UI_AXIS_Y, p.y);
	ui_box_push(tooltip);

	return(ui_box_sig(tooltip));
}

void ui_tooltip_end()
{
	ui_box_pop(); // tooltip
	ui_box_pop(); // ui->overlay
}

void ui_menu_bar_begin(const char* name)
{
	ui_box* bar = ui_box_begin(name, UI_FLAG_DRAW_BACKGROUND);
	ui_box_set_size(bar, UI_AXIS_X, UI_SIZE_PARENT_RATIO, 1., 0);
	ui_box_set_size(bar, UI_AXIS_Y, UI_SIZE_CHILDREN, 0, 0);
	ui_box_set_layout(bar, UI_AXIS_X, UI_ALIGN_START, UI_ALIGN_START);

	ui_push_size(UI_AXIS_X, UI_SIZE_TEXT, 0, 0);
	ui_push_size(UI_AXIS_Y, UI_SIZE_TEXT, 0, 0);

	ui_sig sig = ui_box_sig(bar);
	if(!sig.hovering && mp_input_mouse_released(MP_MOUSE_LEFT))
	{
		ui_box_deactivate(bar);
	}
}

void ui_menu_bar_end()
{
	ui_pop_size(UI_AXIS_X);
	ui_pop_size(UI_AXIS_Y);
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
	ui_box_set_size(menu, UI_AXIS_X, UI_SIZE_CHILDREN, 0, 0);
	ui_box_set_size(menu, UI_AXIS_Y, UI_SIZE_CHILDREN, 0, 0);
	ui_box_set_floating(menu, UI_AXIS_X, button->rect.x);
	ui_box_set_floating(menu, UI_AXIS_Y, button->rect.y + button->rect.h);
	ui_box_set_layout(menu, UI_AXIS_Y, UI_ALIGN_START, UI_ALIGN_START);

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

void ui_menu_end()
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
	mp_key_mods mods;

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

void ui_edit_perform_move(ui_context* ui, ui_edit_move move, int direction, u32 textLen, u32 cursor)
{
	switch(move)
	{
		case UI_EDIT_MOVE_NONE:
			break;

		case UI_EDIT_MOVE_ONE:
		{
			if(direction < 0 && cursor > 0)
			{
				cursor--;
			}
			else if(direction > 0 && cursor < textLen)
			{
				cursor++;
			}
		} break;

		case UI_EDIT_MOVE_LINE:
		{
			if(direction < 0)
			{
				cursor = 0;
			}
			else if(direction > 0)
			{
				cursor = textLen;
			}
		} break;

		case UI_EDIT_MOVE_WORD:
			DEBUG_ASSERT(0, "not implemented yet");
			break;
	}
	ui->editCursor = cursor;
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

			if(ui->editCursor == ui->editMark || move != UI_EDIT_MOVE_ONE)
			{
				//NOTE: we special case move-one when there is a selection
				//      (just place the cursor at begining/end of selection)
				ui_edit_perform_move(ui, move, direction, codepoints.len, cursor);
			}
			ui->editMark = ui->editCursor;
		} break;

		case UI_EDIT_SELECT:
		{
			ui_edit_perform_move(ui, move, direction, codepoints.len, ui->editCursor);
		} break;

		case UI_EDIT_SELECT_EXTEND:
		{
			u32 cursor = direction > 0 ?
			            (ui->editCursor > ui->editMark ? ui->editCursor : ui->editMark) :
				        (ui->editCursor < ui->editMark ? ui->editCursor : ui->editMark);

			ui_edit_perform_move(ui, move, direction, codepoints.len, cursor);
		} break;

		case UI_EDIT_DELETE:
		{
			if(ui->editCursor == ui->editMark)
			{
				ui_edit_perform_move(ui, move, direction, codepoints.len, ui->editCursor);
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

void ui_text_box_render(mg_canvas canvas, ui_box* box, void* data)
{
	str32 codepoints = *(str32*)data;
	ui_context* ui = ui_get_context();

	u32 firstDisplayedChar = 0;
	if(ui_box_active(box))
	{
		firstDisplayedChar = ui->editFirstDisplayedChar;
	}

	ui_style* style = &box->computedStyle;
	mg_font_extents extents = mg_font_get_scaled_extents(style->font, style->fontSize);
	f32 lineHeight = extents.ascent + extents.descent;

	str32 before = str32_slice(codepoints, 0, firstDisplayedChar);
	mp_rect beforeBox = mg_text_bounding_box_utf32(style->font, style->fontSize, before);

	f32 textMargin = 5;

	f32 textX = textMargin + box->rect.x - beforeBox.w;
	f32 textTop = box->rect.y + 0.5*(box->rect.h - lineHeight);
	f32 textY = textTop + extents.ascent ;

	if(box->active && !((u64)(2*(ui->frameTime - ui->editCursorBlinkStart)) & 1))
	{
		str32 beforeCaret = str32_slice(codepoints, 0, ui->editCursor);
		mp_rect beforeCaretBox = mg_text_bounding_box_utf32(style->font, style->fontSize, beforeCaret);

		f32 caretX = box->rect.x + textMargin - beforeBox.w + beforeCaretBox.w;
		f32 caretY = textTop;

		mg_set_color_rgba(canvas, 0, 0, 0, 1);
		mg_rectangle_fill(canvas, caretX, caretY, 2, lineHeight);
	}

	mg_set_font(canvas, style->font);
	mg_set_font_size(canvas, style->fontSize);
	mg_set_color(canvas, style->fontColor);

	mg_move_to(canvas, textX, textY);
	mg_codepoints_outlines(canvas, codepoints);
	mg_fill(canvas);
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
	ui_style* style = &frame->computedStyle;
	mg_font_extents extents = mg_font_get_scaled_extents(style->font, style->fontSize);
	ui_box_set_size(frame, UI_AXIS_Y, UI_SIZE_PIXELS, extents.ascent+extents.descent+10, 1);

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
				ui->editCursorBlinkStart = ui->frameTime;
			}
			//TODO: set cursor on mouse pos

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
		str32 codepoints = utf8_push_to_codepoints(&ui->frameArena, text);
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
		mp_key_mods mods = mp_input_key_mods();

		for(int i=0; i<UI_EDIT_COMMAND_COUNT; i++)
		{
			const ui_edit_command* command = &(UI_EDIT_COMMANDS[i]);
			if(mp_input_key_pressed(command->key) && mods == command->mods)
			{
				codepoints = ui_edit_perform_operation(ui, command->operation, command->move, command->direction, codepoints);
				break;
			}
		}

		//NOTE(martin): text box focus shortcuts
		if(mp_input_key_pressed(MP_KEY_ENTER))
		{
			//TODO(martin): extract in gui_edit_complete() (and use below)
			ui_box_deactivate(frame);
			ui->focus = 0;
		}

		result.text = utf8_push_from_codepoints(arena, codepoints);

		//TODO slide contents

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
