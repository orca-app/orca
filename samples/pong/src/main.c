#include <keys.h>
#include <graphics.h>

#include <orca.h>

extern float cosf(float);
extern float sinf(float);
extern float sqrtf(float);

#define NUM_BLOCKS_PER_ROW 7
#define NUM_BLOCKS 42 // 7 * 6

#define BLOCKS_WIDTH 810.0f
#define BLOCK_HEIGHT 30.0f
#define BLOCKS_PADDING 15.0f
#define BLOCKS_BOTTOM 300.0f
const f32 BLOCK_WIDTH = (BLOCKS_WIDTH - ((NUM_BLOCKS_PER_ROW + 1) * BLOCKS_PADDING)) / NUM_BLOCKS_PER_ROW;

#define PADDLE_MAX_LAUNCH_ANGLE 0.7f

const mg_color paddleColor = {1, 0, 0, 1};
mp_rect paddle = {BLOCKS_WIDTH/2 - 100, 40, 200, 40};

const mg_color ballColor = {1, 1, 0, 1};
mp_rect ball = {200, 200, 20, 20};

vec2 velocity = {5, 5};

// This is upside down from how it will actually be drawn.
int blockHealth[NUM_BLOCKS] = {
    0, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3,
};

vec2 frameSize = {100, 100};

bool leftDown = false;
bool rightDown = false;

mg_surface surface;
mg_canvas canvas;
mg_image waterImage;
mg_image ballImage;
mg_image paddleImage;
mg_font pongFont;

// TODO(ben): Why is this here? Why isn't it forward-declared by some header?
mg_surface mg_surface_main(void);

f32 lerp(f32 a, f32 b, f32 t);
mp_rect blockRect(int i);
int checkCollision(mp_rect block);
mg_mat2x3 flipYAt(vec2 pos);

str8 loadFile(mem_arena* arena, str8 filename) {
    file_handle file = file_open(filename, FILE_ACCESS_READ, 0);
    if(file_last_error(file) != IO_OK)
    {
        log_error("Couldn't open file %s\n", str8_to_cstring(mem_scratch(), filename));
    }
    u64 size = file_size(file);
    char* buffer = mem_arena_alloc(arena, size);
    file_read(file, size, buffer);
    file_close(file);
    return str8_from_buffer(size, buffer);
}

ORCA_EXPORT void OnInit(void)
{
    surface = mg_surface_main();
    canvas = mg_canvas_create();

    waterImage = mg_image_create_from_data(surface, loadFile(mem_scratch(), STR8("/underwater.jpg")), false);
    ballImage = mg_image_create_from_data(surface, loadFile(mem_scratch(), STR8("/ball.png")), false);
    paddleImage = mg_image_create_from_data(surface, loadFile(mem_scratch(), STR8("/wall.png")), false);

    str8 fontStr = loadFile(mem_scratch(), STR8("/Literata-SemiBoldItalic.ttf"));
    unicode_range ranges[5] = {UNICODE_RANGE_BASIC_LATIN,
                               UNICODE_RANGE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
                               UNICODE_RANGE_LATIN_EXTENDED_A,
                               UNICODE_RANGE_LATIN_EXTENDED_B,
                               UNICODE_RANGE_SPECIALS};
    // NOTE(ben): Weird that images are "create from data" but fonts are "create from memory"
    // TODO: Decide whether we're using strings or explicit pointer + length
    pongFont = mg_font_create_from_memory(fontStr.len, (byte*)fontStr.ptr, 5, ranges);

    mem_arena_clear(mem_scratch());
}

ORCA_EXPORT void OnFrameResize(u32 width, u32 height)
{
    log_info("frame resize %u, %u", width, height);
    frameSize.x = width;
    frameSize.y = height;
}

ORCA_EXPORT void OnMouseDown(int button)
{
    log_info("mouse down!");
}

ORCA_EXPORT void OnKeyDown(int key)
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

ORCA_EXPORT void OnKeyUp(int key)
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

ORCA_EXPORT void OnFrameRefresh(void)
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

    if (ball.x + ball.w >= frameSize.x) {
        velocity.x = -velocity.x;
    }
    if (ball.x <= 0) {
        velocity.x = -velocity.x;
    }
    if (ball.y + ball.h >= frameSize.y) {
        velocity.y = -velocity.y;
    }

    if (
        ball.y <= paddle.y + paddle.h
        && ball.x+ball.w >= paddle.x
        && ball.x <= paddle.x + paddle.w
        && velocity.y < 0
    ) {
        f32 t = ((ball.x + ball.w/2) - paddle.x) / paddle.w;
        f32 launchAngle = lerp(-PADDLE_MAX_LAUNCH_ANGLE, PADDLE_MAX_LAUNCH_ANGLE, t);
        f32 speed = sqrtf(velocity.x*velocity.x + velocity.y*velocity.y);
        velocity = (vec2){
            sinf(launchAngle) * speed,
            cosf(launchAngle) * speed,
        };
        ball.y = paddle.y + paddle.h;

        log_info("PONG!");
    }

    if (ball.y <= 0) {
        ball.x = frameSize.x/2. - ball.w;
        ball.y = frameSize.y/2. - ball.h;
    }

    for (int i = 0; i < NUM_BLOCKS; i++) {
        if (blockHealth[i] <= 0) {
            continue;
        }

        mp_rect r = blockRect(i);
        int result = checkCollision(r);
        if (result) {
            log_info("Collision! direction=%d", result);
            blockHealth[i] -= 1;

            f32 vx = velocity.x;
            f32 vy = velocity.y;

            switch (result) {
            case 1:
            case 5:
                velocity.y = -vy;
                break;
            case 3:
            case 7:
                velocity.x = -vx;
                break;
            case 2:
            case 6:
                velocity.x = -vy;
                velocity.y = -vx;
                break;
            case 4:
            case 8:
                velocity.x = vy;
                velocity.y = vx;
                break;
            }
        }

    }

    mg_canvas_set_current(canvas);

    mg_set_color_rgba(10.0f/255.0f, 31.0f/255.0f, 72.0f/255.0f, 1);
    mg_clear();

    mg_mat2x3 yUp = {
        1, 0, 0,
        0, -1, frameSize.y,
    };

    mg_matrix_push(yUp);
    {
        for (int i = 0; i < NUM_BLOCKS; i++) {
            if (blockHealth[i] <= 0) {
                continue;
            }

            mp_rect r = blockRect(i);
            mg_set_color_rgba(0.9, 0.9, 0.9, 1);
            mg_rounded_rectangle_fill(r.x, r.y, r.w, r.h, 4);

            // TODO: measure and center text
            vec2 textPos = {r.x + r.w/2 - 5, r.y + 9};

            mg_set_color_rgba(0, 0, 0, 1);
            mg_set_font(pongFont);
            mg_set_font_size(18);
            mg_move_to(textPos.x, textPos.y);
            mg_matrix_push(flipYAt(textPos));
            {
                str8 text = str8_pushf(mem_scratch(),
                    "%d", blockHealth[i]
                );
                mg_text_outlines(text);
                mg_fill();
            }
            mg_matrix_pop();
        }

        mg_image_draw(paddleImage, paddle);
        mg_image_draw(ballImage, ball);
    }
    mg_matrix_pop();

    mg_surface_prepare(surface);
    mg_render(surface, canvas);
    mg_surface_present(surface);
}

mp_rect blockRect(int i) {
    int row = i / NUM_BLOCKS_PER_ROW;
    int col = i % NUM_BLOCKS_PER_ROW;
    return (mp_rect){
        BLOCKS_PADDING + (BLOCKS_PADDING + BLOCK_WIDTH) * col,
        BLOCKS_BOTTOM + (BLOCKS_PADDING + BLOCK_HEIGHT) * row,
        BLOCK_WIDTH,
        BLOCK_HEIGHT
    };
}

// Returns a cardinal direction 1-8 for the collision with the block, or zero
// if no collision. 1 is straight up and directions proceed clockwise.
int checkCollision(mp_rect block) {
    // Note that all the logic for this game has the origin in the bottom left.

    f32 ballx2 = ball.x + ball.w;
    f32 bally2 = ball.y + ball.h;
    f32 blockx2 = block.x + block.w;
    f32 blocky2 = block.y + block.h;

    if (
        ballx2 < block.x
        || blockx2 < ball.x
        || bally2 < block.y
        || blocky2 < ball.y
    ) {
        // Ball is fully outside block
        return 0;
    }

    // if (
    //     (block.x <= ball.x && ballx2 <= blockx2)
    //     && (block.y <= ball.y && bally2 <= blocky2)
    // ) {
    //     // Ball is fully inside block; do not consider as a collision
    //     return 0;
    // }

    // If moving right, the ball can bounce off its top right corner, right
    // side, or bottom right corner. Corner bounces occur if the block's bottom
    // left corner is in the ball's top right quadrant, or if the block's top
    // left corner is in the ball's bottom left quadrant. Otherwise, an edge
    // bounce occurs if the block's left edge falls in either of the ball's
    // right quadrants.
    //
    // This logic generalizes to other directions.
    //
    // We assume significant tunneling can't happen.

    vec2 ballCenter = (vec2){ball.x + ball.w/2, ball.y + ball.h/2};
    vec2 blockCenter = (vec2){block.x + block.w/2, block.y + block.h/2};

    // Moving right
    if (velocity.x > 0) {
        // Ball's top right corner
        if (
            ballCenter.x <= block.x && block.x <= ballx2
            && ballCenter.y <= block.y && block.y <= bally2
        ) { return 2; }

        // Ball's bottom right corner
        if (
            ballCenter.x <= block.x && block.x <= ballx2
            && ball.y <= blocky2 && blocky2 <= ballCenter.y
        ) { return 4; }

        // Ball's right edge
        if (
            ballCenter.x <= block.x && block.x <= ballx2
        ) { return 3; }
    }

    // Moving up
    if (velocity.y > 0) {
        // Ball's top left corner
        if (
            ball.x <= blockx2 && blockx2 <= ballCenter.x
            && ballCenter.y <= block.y && block.y <= bally2
        ) { return 8; }

        // Ball's top right corner
        if (
            ballCenter.x <= block.x && block.x <= ballx2
            && ballCenter.y <= block.y && block.y <= bally2
        ) { return 2; }

        // Ball's top edge
        if (
            ballCenter.y <= block.y && block.y <= bally2
        ) { return 1; }
    }

    // Moving left
    if (velocity.x < 0) {
        // Ball's bottom left corner
        if (
            ball.x <= blockx2 && blockx2 <= ballCenter.x
            && ball.y <= blocky2 && blocky2 <= ballCenter.y
        ) { return 6; }

        // Ball's top left corner
        if (
            ball.x <= blockx2 && blockx2 <= ballCenter.x
            && ballCenter.y <= block.y && block.y <= bally2
        ) { return 8; }

        // Ball's left edge
        if (
            ball.x <= blockx2 && blockx2 <= ballCenter.x
        ) { return 7; }
    }

    // Moving down
    if (velocity.y < 0) {
        // Ball's bottom right corner
        if (
            ballCenter.x <= block.x && block.x <= ballx2
            && ball.y <= blocky2 && blocky2 <= ballCenter.y
        ) { return 4; }

        // Ball's bottom left corner
        if (
            ball.x <= blockx2 && blockx2 <= ballCenter.x
            && ball.y <= blocky2 && blocky2 <= ballCenter.y
        ) { return 6; }

        // Ball's bottom edge
        if (
            ball.y <= blocky2 && blocky2 <= ballCenter.y
        ) { return 5; }
    }

    return 0;
}

f32 lerp(f32 a, f32 b, f32 t) {
    return (1 - t) * a + t * b;
}

mg_mat2x3 flipYAt(vec2 pos) {
    return (mg_mat2x3){
        1, 0, 0,
        0, -1, 2 * pos.y,
    };
}
