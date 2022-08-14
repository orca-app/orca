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

typedef struct ui_stack_elt ui_stack_elt;
struct ui_stack_elt
{
	ui_stack_elt* parent;
	union
	{
		ui_box* box;
		ui_size size;
		mp_rect clip;
		ui_style style;
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
	ui_stack_elt* sizeStack[UI_AXIS_COUNT];
	ui_stack_elt* styleStack[UI_STYLE_SELECTOR_COUNT];
	ui_stack_elt* clipStack;

	u32 z;
	ui_box* hovered;

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

void ui_size_push(ui_axis axis, ui_size_kind kind, f32 value, f32 strictness)
{
	ui_context* ui = ui_get_context();
	ui_stack_elt* elt = ui_stack_push(ui, &ui->sizeStack[axis]);
	elt->size = (ui_size){.kind = kind, .value = value, .strictness = strictness};
}

void ui_size_pop(ui_axis axis)
{
	ui_context* ui = ui_get_context();
	ui_stack_pop(&ui->sizeStack[axis]);
}

ui_size ui_size_top(ui_axis axis)
{
	ui_context* ui = ui_get_context();
	ui_size size = {0};
	if(ui->sizeStack[axis])
	{
		size = ui->sizeStack[axis]->size;
	}
	return(size);
}

ui_stack_elt** ui_style_stack_select(ui_context* ui, ui_style_selector selector)
{
	DEBUG_ASSERT(selector < UI_STYLE_SELECTOR_COUNT);
	ui_stack_elt** stack = &ui->styleStack[selector];
	return(stack);
}

void ui_style_push(ui_style_selector selector, ui_style style)
{
	ui_context* ui = ui_get_context();
	ui_stack_elt** stack = ui_style_stack_select(ui, selector);
	ui_stack_elt* elt = ui_stack_push(ui, stack);
	elt->style = style;
}

void ui_style_pop(ui_style_selector selector)
{
	ui_context* ui = ui_get_context();
	ui_stack_elt** stack = ui_style_stack_select(ui, selector);
	ui_stack_pop(stack);
}

ui_style* ui_style_top(ui_style_selector selector)
{
	ui_context* ui = ui_get_context();
	ui_stack_elt* stack = *ui_style_stack_select(ui, selector);
	ui_style* style = 0;
	if(stack)
	{
		style = &stack->style;
	}
	return(style);
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

	box->desiredSize[UI_AXIS_X] = ui_size_top(UI_AXIS_X);
	box->desiredSize[UI_AXIS_Y] = ui_size_top(UI_AXIS_Y);
	box->layout = box->parent ? box->parent->layout : (ui_layout){0};

	for(int i=0; i<UI_STYLE_SELECTOR_COUNT; i++)
	{
		box->styles[i] = ui_style_top(i);
	}
	box->styleSelector = UI_STYLE_NORMAL;

	//NOTE: compute input signals
	ui_box_compute_signals(ui, box);

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

void ui_box_set_layout(ui_box* box, ui_axis axis, ui_align alignX, ui_align alignY)
{
	box->layout = (ui_layout){axis, {alignX, alignY}};
}

void ui_box_set_size(ui_box* box, ui_axis axis, ui_size_kind kind, f32 value, f32 strictness)
{
	box->desiredSize[axis] = (ui_size){kind, value, strictness};
}

void ui_box_set_floating(ui_box* box, ui_axis axis, f32 pos)
{
	box->floating[axis] = true;
	box->rect.c[axis] = pos;
}

void ui_box_set_style_selector(ui_box* box, ui_style_selector selector)
{
	box->styleSelector = selector;
}

void ui_box_set_closed(ui_box* box, bool closed)
{
	box->closed = closed;
}

bool ui_box_closed(ui_box* box)
{
	return(box->closed);
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
	ui_style* targetStyle = box->styles[box->styleSelector];
	if(!targetStyle)
	{
		targetStyle = box->styles[UI_STYLE_NORMAL];
	}
	DEBUG_ASSERT(targetStyle);

	f32 animationTime = targetStyle->animationTime;

	/*
	f32 transitionValue = 0;

	//NOTE: update transition values
	if(box->styleSelector == UI_STYLE_ACTIVE)
	{
		//NOTE: transition from normal or hot to active
		if(box->hotTransition)
		{
			//NOTE: transition from hot to active
			baseStyle = box->styles[UI_STYLE_HOT];
		}
		else
		{
			//NOTE: transition from normal to active
			baseStyle = box->styles[UI_STYLE_NORMAL];
		}

		box->hotTransition = ui_update_transition(ui, box->activeTransition, 1, animationTime);
		box->activeTransition = ui_update_transition(ui, box->activeTransition, 1, animationTime);
		transitionValue = box->activeTransition;
	}
	else if(box->styleSelector == UI_STYLE_HOT)
	{
		box->activeTransition = ui_update_transition(ui, box->activeTransition, 0, animationTime);
		box->hotTransition = ui_update_transition(ui, box->hotTransition, 1, animationTime);

		if(box->activeTransition)
		{
			//NOTE: transition from active to hot
			transitionValue = box->activeTransition;
			baseStyle = box->styles[UI_STYLE_ACTIVE];
		}
		else
		{
			//NOTE: transition from normal to hot
			transitionValue = box->hotTransition;
			baseStyle = box->styles[UI_STYLE_NORMAL];
		}
	}
	else
	{
		//NOTE: transition from hot or active to normal
		box->activeTransition = 0;
		box->hotTransition = ui_update_transition(ui, box->hotTransition, 0, animationTime);
		transitionValue = box->hotTransition;
	}
	*/

	//TODO: interpolate based on transition values
	ui_animate_color(ui, &box->computedStyle.backgroundColor, targetStyle->backgroundColor, animationTime);
	ui_animate_color(ui, &box->computedStyle.foregroundColor, targetStyle->foregroundColor, animationTime);
	ui_animate_color(ui, &box->computedStyle.borderColor, targetStyle->borderColor, animationTime);
	ui_animate_color(ui, &box->computedStyle.textColor, targetStyle->textColor, animationTime);

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
	if( box->desiredSize[UI_AXIS_X].kind == UI_SIZE_TEXT
	  ||box->desiredSize[UI_AXIS_Y].kind == UI_SIZE_TEXT)
	{
		textBox = mg_text_bounding_box(style->font, style->fontSize, box->string);
	}

	for(int i=0; i<UI_AXIS_COUNT; i++)
	{
		ui_size size = box->desiredSize[i];

		if(size.kind == UI_SIZE_TEXT)
		{
			box->rect.c[2+i] = textBox.c[2+i];
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

	ui_size* size = &box->desiredSize[axis];

	if(size->kind == UI_SIZE_PARENT_RATIO)
	{
		ui_box* parent = box->parent;
		if(  parent
		  && parent->desiredSize[axis].kind != UI_SIZE_CHILDREN)
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

	ui_size* size = &box->desiredSize[axis];
	if(size->kind == UI_SIZE_CHILDREN)
	{
		box->rect.c[2+axis] = sum;
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
		mg_set_color(canvas, style->backgroundColor);
		ui_rectangle_fill(canvas, box->rect, style->roundness);
	}

	for_each_in_list(&box->children, child, ui_box, listElt)
	{
		ui_draw_box(canvas, child);
	}

	if(box->flags & UI_FLAG_DRAW_FOREGROUND)
	{
		mg_set_color(canvas, style->foregroundColor);
		ui_rectangle_fill(canvas, box->rect, style->roundness);
	}

	if(box->flags & UI_FLAG_DRAW_TEXT)
	{
		mp_rect textBox = mg_text_bounding_box(style->font, style->fontSize, box->string);

		f32 x = box->rect.x + 0.5*(box->rect.w - textBox.w);
		f32 y = box->rect.y + 0.5*(box->rect.h - textBox.h) - textBox.y;

		mg_set_font(canvas, style->font);
		mg_set_font_size(canvas, style->fontSize);
		mg_set_color(canvas, style->textColor);

		mg_move_to(canvas, x, y);
		mg_text_outlines(canvas, box->string);
		mg_fill(canvas);
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
	for(int i=0; i<UI_STYLE_SELECTOR_COUNT; i++)
	{
		ui->styleStack[i] = 0;
	}

	ui->clipStack = 0;
	ui->z = 0;

	ui_style_push(UI_STYLE_NORMAL, defaultStyle);

	ui_size_push(UI_AXIS_X, UI_SIZE_PIXELS, width, 0);
	ui_size_push(UI_AXIS_Y, UI_SIZE_PIXELS, height, 0);

	ui->root = ui_box_begin("_root_", 0);

	ui_box* contents = ui_box_make("_contents_", 0);
	ui_box_set_floating(contents, UI_AXIS_X, 0);
	ui_box_set_floating(contents, UI_AXIS_Y, 0);
	ui_box_set_layout(contents, UI_AXIS_Y, UI_ALIGN_START, UI_ALIGN_START);

	ui->overlay = ui_box_make("_overlay_", 0);
	ui_box_set_floating(ui->overlay, UI_AXIS_X, 0);
	ui_box_set_floating(ui->overlay, UI_AXIS_Y, 0);

	ui_size_pop(UI_AXIS_X);
	ui_size_pop(UI_AXIS_Y);

	ui_box_push(contents);
}

void ui_end_frame()
{
	ui_context* ui = ui_get_context();

	ui_box_pop();

	ui_box* box = ui_box_end();
	DEBUG_ASSERT(box == ui->root, "unbalanced box stack");

	ui_style_pop(UI_STYLE_NORMAL);

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
	ui_sig sig = ui_box_sig(box);

	if(sig.hovering)
	{
		ui_box_set_style_selector(box, UI_STYLE_HOT);
		if(sig.dragging)
		{
			ui_box_set_style_selector(box, UI_STYLE_ACTIVE);
		}
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

		ui_style style[UI_STYLE_SELECTOR_COUNT] = {
			[UI_STYLE_NORMAL] = {.backgroundColor = {0, 0, 0, 0},
			                 	 .foregroundColor = {0, 0, 0, 0},
			                 	 .roundness = roundness,
			                 	 .animationTime = animationTime},
			[UI_STYLE_HOT] = {.backgroundColor = {0, 0, 0, 0.5},
			              	  .foregroundColor = {0, 0, 0, 0.7},
			              	  .roundness = roundness,
			              	  .animationTime = animationTime},
			[UI_STYLE_ACTIVE] = {.backgroundColor = {0, 0, 0, 0.5},
			                 	 .foregroundColor = {0, 0, 0, 0.7},
			                 	 .roundness = roundness,
			                 	 .animationTime = animationTime}};

		for(int i=0; i<UI_STYLE_SELECTOR_COUNT; i++)
		{
			ui_style_push(i, style[i]);
		}

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

		for(int i=0; i<UI_STYLE_SELECTOR_COUNT; i++)
		{
			ui_style_pop(i);
		}

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
		if(trackSig.hovering)
		{
			ui_box_set_style_selector(track, UI_STYLE_HOT);
			ui_box_set_style_selector(thumb, UI_STYLE_HOT);
		}
		if(thumbSig.dragging)
		{
			ui_box_set_style_selector(track, UI_STYLE_ACTIVE);
			ui_box_set_style_selector(thumb, UI_STYLE_ACTIVE);
		}

	} ui_box_end();

	return(frame);
}

void ui_panel_begin(const char* name)
{
	ui_flags panelFlags = UI_FLAG_DRAW_BACKGROUND
	                    | UI_FLAG_DRAW_BORDER
	                    | UI_FLAG_CLIP
	                    | UI_FLAG_BLOCK_MOUSE;
	ui_box* panel = ui_box_begin(name, panelFlags);

	ui_box* innerView = ui_box_begin(name, 0);
}

void ui_panel_end()
{
	ui_box* innerView = ui_box_top();
	ui_box_end();

	ui_box* panel = ui_box_top();

	f32 contentsW = ClampLowBound(innerView->childrenSum[0], innerView->rect.w);
	f32 contentsH = ClampLowBound(innerView->childrenSum[1], innerView->rect.h);

	contentsW = ClampLowBound(contentsW, 1);
	contentsH = ClampLowBound(contentsH, 1);

	if(contentsW > innerView->rect.w)
	{
		f32 thumbRatioX = innerView->rect.w / contentsW;
		f32 sliderX = innerView->scroll.x /(contentsW - innerView->rect.w);

		ui_box* scrollBarX = ui_scrollbar("scrollerX", thumbRatioX, &sliderX);
		ui_box_set_size(scrollBarX, UI_AXIS_X, UI_SIZE_PARENT_RATIO, 1., 0);
		ui_box_set_size(scrollBarX, UI_AXIS_Y, UI_SIZE_PIXELS, 10, 0);
		ui_box_set_floating(scrollBarX, UI_AXIS_X, 0);
		ui_box_set_floating(scrollBarX, UI_AXIS_Y, panel->rect.h - 12);

		innerView->scroll.x = sliderX * (contentsW - innerView->rect.w);
	}

	if(contentsH > innerView->rect.h)
	{
		f32 thumbRatioY = innerView->rect.h / contentsH;
		f32 sliderY = innerView->scroll.y /(contentsH - innerView->rect.h);

		ui_box* scrollBarY = ui_scrollbar("scrollerY", thumbRatioY, &sliderY);
		ui_box_set_size(scrollBarY, UI_AXIS_X, UI_SIZE_PIXELS, 10, 0);
		ui_box_set_size(scrollBarY, UI_AXIS_Y, UI_SIZE_PARENT_RATIO, 1., 0);
		ui_box_set_floating(scrollBarY, UI_AXIS_X, panel->rect.w - 12);
		ui_box_set_floating(scrollBarY, UI_AXIS_Y, 0);

		innerView->scroll.y = sliderY * (contentsH - innerView->rect.h);
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

void ui_menu_bar_begin(const char* name)
{
	ui_box* bar = ui_box_begin(name, UI_FLAG_DRAW_BACKGROUND);
	ui_box_set_size(bar, UI_AXIS_X, UI_SIZE_PARENT_RATIO, 1., 0);
	ui_box_set_size(bar, UI_AXIS_Y, UI_SIZE_CHILDREN, 0, 0);
	ui_box_set_layout(bar, UI_AXIS_X, UI_ALIGN_START, UI_ALIGN_START);

	ui_size_push(UI_AXIS_X, UI_SIZE_TEXT, 0, 0);
	ui_size_push(UI_AXIS_Y, UI_SIZE_TEXT, 0, 0);


	ui_sig sig = ui_box_sig(bar);
	if(!sig.hovering && mp_input_mouse_released(MP_MOUSE_LEFT))
	{
		ui_box_deactivate(bar);
	}
}

void ui_menu_bar_end()
{
	ui_size_pop(UI_AXIS_X);
	ui_size_pop(UI_AXIS_Y);
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




#undef LOG_SUBSYSTEM
