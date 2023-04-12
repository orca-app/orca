/************************************************************//**
*
*	@file: wasm_main.cpp
*	@author: Martin Fouilleul
*	@date: 14/08/2022
*	@revision:
*
*****************************************************************/
#include"typedefs.h"
#include"keys.h"
#include"macro_helpers.h"
#include"GLES3/gl32.h"

typedef struct str8
{
	unsigned long long len;
	char* ptr;
} str8;

#define str8_lit(s) ((str8){.len = sizeof(s)-1, .ptr = (char*)(s)})

#define M_PI 3.14159265358979323846

extern void log_string_flat(unsigned long long len, char* ptr);
extern void log_int(int i);
extern float cosf(float x);
extern float sinf(float x);

void log_string(str8 string)
{
	log_string_flat(string.len, string.ptr);
}

unsigned int program;

const char* vshaderSource =
	"#version 300 es\n"
	"precision mediump float;"
	"layout(location=0) in vec4 vPosition;\n"
	"layout(location=1) in vec4 aColor;\n"
	"out vec4 vColor;\n"
	"uniform mat4 transform;\n"
	"void main()\n"
	"{\n"
	"   gl_Position = transform*vPosition;\n"
	"	vColor = aColor;\n"
	"}\n";

const char* fshaderSource =
	"#version 300 es\n"
	"precision mediump float;\n"
	"out vec4 fragColor;\n"
	"in vec4 vColor;\n"
	"void main()\n"
	"{\n"
	"    fragColor = vColor;\n"
	"}\n";

void compile_shader(GLuint shader, const char* source)
{
	glShaderSource(shader, 1, &source, 0);
	glCompileShader(shader);

	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(success == GL_FALSE)
	{
		char message[1024];
		int length = 0;
		glGetShaderInfoLog(shader, 1024, &length, message);
		log_string(str8_lit("gl shader error: \n"));
		log_string_flat(length, message);
	}
}

void OnInit(void)
{
	//log_string(str8_lit("init procedure\n"));

	unsigned int vshader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fshader = glCreateShader(GL_FRAGMENT_SHADER);
	program = glCreateProgram();

	compile_shader(vshader, vshaderSource);
	compile_shader(fshader, fshaderSource);

	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);
	glUseProgram(program);
}

const vec4 paddleColor = {1, 0, 0, 1};
mp_rect paddle = {200, 40, 200, 40};

const vec4 ballColor = {1, 1, 0, 1};
mp_rect ball = {200, 200, 60, 60};

vec2 velocity = {10, 10};

vec2 frameSize = {100, 100};
float rotationDir = 1;

bool leftDown = false;
bool rightDown = false;

void OnFrameResize(u32 width, u32 height)
{
	log_string(str8_lit("frame resize "));
	log_int(width);
	log_int(height);
	log_string(str8_lit("\n"));

	frameSize.x = width;
	frameSize.y = height;

	paddle.x = width/2. - paddle.w/2.;
	ball.x = width/2. - ball.w/2.;
	ball.y = height/2. - ball.h/2.;
}

void OnMouseDown(int button)
{
	rotationDir *= -1;
}

void OnKeyDown(int key)
{
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
	if(key == KEY_LEFT)
	{
		leftDown = false;
	}
	if(key == KEY_RIGHT)
	{
		rightDown = false;
	}
}

void debug_draw_rotating_triangle()
{
 	static float alpha = 0;
	f32 aspect = frameSize.x/frameSize.y;

    GLfloat matrix[] = {cosf(alpha)/aspect, sinf(alpha), 0, 0,
    	                   -sinf(alpha)/aspect, cosf(alpha), 0, 0,
    	                   0, 0, 1, 0,
    	                   0, 0, 0, 1};
    alpha += rotationDir*2*M_PI/120;

    glUniformMatrix4fv(0, 1, false, matrix);

	GLfloat vertices[] = {
		-0.866/2, -0.5/2, 0,
		0.866/2, -0.5/2, 0,
		0, 0.5, 0};

	GLfloat colors[] = {
		1, 0, 0, 1,
	    0, 1, 0, 1,
	    0, 0, 1, 1};

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, colors);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void draw_rect(mp_rect rect, vec4 color)
{
	GLfloat vertices[6*3] = {
		rect.x, rect.y, 0,
		rect.x, rect.y + rect.h, 0,
		rect.x+rect.w, rect.y + rect.h, 0,
		rect.x, rect.y, 0,
		rect.x+rect.w, rect.y + rect.h, 0,
		rect.x+rect.w, rect.y, 0};

	GLfloat colors[6*4];
	for(int i=0; i<6*4; i+=4)
	{
		colors[i] = color.x;
		colors[i+1] = color.y;
		colors[i+2] = color.z;
		colors[i+3] = color.w;
	}

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, colors);
    glEnableVertexAttribArray(1);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void OnFrameRefresh(void)
{
	//log_string(str8_lit("frame procedure\n"));

	glClearColor(0, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	f32 aspect = frameSize.x/frameSize.y;

    GLfloat matrix[] = {2./frameSize.x, 0, 0, 0,
    	                0, 2./frameSize.y, 0, 0,
    	                0, 0, 1, 0,
    	                -1, -1, 0, 1};

    glUniformMatrix4fv(0, 1, false, matrix);

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
    }

    if(ball.y <= 0)
    {
		ball.x = frameSize.x/2. - ball.w;
		ball.y = frameSize.y/2. - ball.h;
	}

    draw_rect(paddle, paddleColor);
    draw_rect(ball, ballColor);
}
