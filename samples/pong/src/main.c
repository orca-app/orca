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

const mg_color paddleColor = {1, 0, 0, 1};
mp_rect paddle = {200, 40, 200, 40};

const mg_color ballColor = {1, 1, 0, 1};
mp_rect ball = {200, 200, 60, 60};

vec2 velocity = {10, 10};

vec2 frameSize = {100, 100};

bool leftDown = false;
bool rightDown = false;

mg_canvas canvas;
mg_surface surface;

mg_surface mg_surface_main(void);

void OnInit(void)
{
	//TODO create surface for main window
	surface = mg_surface_main();
	canvas = mg_canvas_create();

	file_handle file = file_open(STR8("/test_write.txt")	, IO_OPEN_CREATE | IO_OPEN_WRITE);

	str8 string = STR8("Hello, file!\n");
	file_write(file, string.len, string.ptr);
	file_close(file);

	file = file_open(STR8("/dir1/test_read.txt")	, IO_OPEN_READ);
	u64 size = file_size(file);
	char* buffer = mem_arena_alloc(mem_scratch(), size);
	file_read(file, size, buffer);
	file_close(file);

	log_info("read file: %.*s", (int)size, buffer);
}

void OnFrameResize(u32 width, u32 height)
{
	log_info("frame resize %u, %u", width, height);
	frameSize.x = width;
	frameSize.y = height;
}

void OnMouseDown(int button)
{
	log_info("mouse down!");
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

	mg_canvas_set_current(canvas);

	mg_set_color_rgba(0, 1, 1, 1);
	mg_clear();

	mg_mat2x3 transform = {1, 0, 0,
	                       0, -1, frameSize.y};

	mg_matrix_push(transform);

	mg_set_color(paddleColor);
	mg_rectangle_fill(paddle.x, paddle.y, paddle.w, paddle.h);

	mg_set_color(ballColor);
	mg_circle_fill(ball.x+ball.w/2, ball.y + ball.w/2, ball.w/2.);

    mg_matrix_pop();

    mg_surface_prepare(surface);
    mg_render(surface, canvas);
    mg_surface_present(surface);
}
