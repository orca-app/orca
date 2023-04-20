/************************************************************//**
*
*	@file: wasm_main.cpp
*	@author: Martin Fouilleul
*	@date: 14/08/2022
*	@revision:
*
*****************************************************************/

#include"keys.h"
#include"graphics.h"

#include"orca.h"

#define M_PI 3.14159265358979323846

extern float cosf(float x);
extern float sinf(float x);

const g_color paddleColor = {1, 0, 0, 1};
mp_rect paddle = {200, 40, 200, 40};

const g_color ballColor = {1, 1, 0, 1};
mp_rect ball = {200, 200, 60, 60};

vec2 velocity = {10, 10};

vec2 frameSize = {100, 100};
float rotationDir = 1;

bool leftDown = false;
bool rightDown = false;

g_font font;

mem_arena arena;

void OnInit(void)
{
	mem_arena_init(&arena);
	font = g_font_create_default();
}

void OnFrameResize(u32 width, u32 height)
{
	log_info("frame resize %u, %u", width, height);

	frameSize.x = width;
	frameSize.y = height;
/*
	paddle.x = width/2. - paddle.w/2.;
	ball.x = width/2. - ball.w/2.;
	ball.y = height/2. - ball.h/2.;
*/
}

void OnMouseDown(int button)
{
	log_info("mouse down!");
	rotationDir *= -1;
}

void OnKeyDown(int key)
{
	if(key == KEY_SPACE)
	{
		log_error("(this is just for testing errors)");
		return;
	}
	if(key == KEY_ENTER)
	{
		log_warning("(this is just for testing warning)");
		return;
	}

	log_info("key down: %i", key);
	if(key == KEY_LEFT)
	{
		leftDown = true;
	}
	if(key == KEY_RIGHT)
	{
		rightDown = true;
	}
}

void OnKeyUp(int key)
{
	if(key == KEY_ENTER || key == KEY_SPACE)
	{
		return;
	}

	log_info("key up: %i", key);
	if(key == KEY_LEFT)
	{
		leftDown = false;
	}
	if(key == KEY_RIGHT)
	{
		rightDown = false;
	}
}

void OnFrameRefresh(void)
{
	char* tmp = mem_arena_alloc(&arena, 512);

	f32 aspect = frameSize.x/frameSize.y;

    if(leftDown)
    {
		paddle.x -= 10;
    }
    else if(rightDown)
    {
		paddle.x += 10;
    }
    paddle.x = Clamp(paddle.x, 0, frameSize.x - paddle.w);

    ball.x += velocity.x;
    ball.y += velocity.y;
    ball.x = Clamp(ball.x, 0, frameSize.x - ball.w);
    ball.y = Clamp(ball.y, 0, frameSize.y - ball.h);

    if(ball.x + ball.w >= frameSize.x)
    {
		velocity.x = -10;
    }
    if(ball.x <= 0)
    {
		velocity.x = +10;
    }
    if(ball.y + ball.h >= frameSize.y)
    {
		velocity.y = -10;
    }

    if(ball.y <= paddle.y + paddle.h
       && ball.x+ball.w >= paddle.x
       && ball.x <= paddle.x + paddle.w
       && velocity.y < 0)
    {
		velocity.y *= -1;
		ball.y = paddle.y + paddle.h;

		log_info("PONG!");
    }

    if(ball.y <= 0)
    {
		ball.x = frameSize.x/2. - ball.w;
		ball.y = frameSize.y/2. - ball.h;
	}

	g_set_color_rgba(0, 1, 1, 1);
	g_clear();

	g_mat2x3 transform = {1, 0, 0,
	                       0, -1, frameSize.y};

	g_matrix_push(transform);

	g_set_color(paddleColor);
	g_rectangle_fill(paddle.x, paddle.y, paddle.w, paddle.h);

	g_set_color(ballColor);
	g_circle_fill(ball.x+ball.w/2, ball.y + ball.w/2, ball.w/2.);


	g_set_font(font);
	g_set_font_size(16);
	g_set_color_rgba(0, 0, 0, 1);
	g_set_text_flip(true);

	str8 str = {.len = 13, .ptr = (char*)"Hello, world!"};
	g_move_to(10, 10);
	g_text_outlines(str);
	g_fill();

    g_matrix_pop();

    mem_arena_clear(&arena);
}
