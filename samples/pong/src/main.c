#include <math.h>

#include <orca.h>

#define NUM_BLOCKS_PER_ROW 7
#define NUM_BLOCKS 42 // 7 * 6

#define BLOCKS_WIDTH 810.0f
#define BLOCK_HEIGHT 30.0f
#define BLOCKS_PADDING 15.0f
#define BLOCKS_BOTTOM 300.0f
const f32 BLOCK_WIDTH = (BLOCKS_WIDTH - ((NUM_BLOCKS_PER_ROW + 1) * BLOCKS_PADDING)) / NUM_BLOCKS_PER_ROW;

#define PADDLE_MAX_LAUNCH_ANGLE 0.7f

const oc_color paddleColor = { 1, 0, 0, 1 };
oc_rect paddle = { 300, 50, 200, 24 };

const oc_color ballColor = { 1, 1, 0, 1 };
oc_rect ball = { 200, 200, 20, 20 };

oc_vec2 velocity = { 5, 5 };

// This is upside down from how it will actually be drawn.
int blockHealth[NUM_BLOCKS] = {
    0, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3
};

oc_vec2 frameSize = { 100, 100 };

bool leftDown = false;
bool rightDown = false;

oc_surface surface;
oc_canvas canvas;
oc_image waterImage;
oc_image ballImage;
oc_image paddleImage;
oc_font pongFont;

f32 lerp(f32 a, f32 b, f32 t);
oc_rect blockRect(int i);
int checkCollision(oc_rect block);
oc_mat2x3 flipY(oc_rect r);
oc_mat2x3 flipYAt(oc_vec2 pos);

oc_str8 loadFile(oc_arena* arena, oc_str8 filename)
{
    oc_file file = oc_file_open(filename, OC_FILE_ACCESS_READ, 0);
    if(oc_file_last_error(file) != OC_IO_OK)
    {
        oc_log_error("Couldn't open file %s\n", oc_str8_to_cstring(oc_scratch(), filename));
    }
    u64 size = oc_file_size(file);
    char* buffer = oc_arena_push(arena, size);
    oc_file_read(file, size, buffer);
    oc_file_close(file);
    return oc_str8_from_buffer(size, buffer);
}

ORCA_EXPORT void oc_on_init(void)
{
    surface = oc_surface_canvas();
    canvas = oc_canvas_create();

    waterImage = oc_image_create_from_memory(surface, loadFile(oc_scratch(), OC_STR8("/underwater.jpg")), false);
    ballImage = oc_image_create_from_memory(surface, loadFile(oc_scratch(), OC_STR8("/ball.png")), false);
    paddleImage = oc_image_create_from_memory(surface, loadFile(oc_scratch(), OC_STR8("/wall.png")), false);

    if(oc_image_is_nil(waterImage))
    {
        oc_log_error("couldn't load water image\n");
    }
    if(oc_image_is_nil(ballImage))
    {
        oc_log_error("couldn't load ball image\n");
    }
    if(oc_image_is_nil(paddleImage))
    {
        oc_log_error("couldn't load paddle image\n");
    }

    oc_str8 fontStr = loadFile(oc_scratch(), OC_STR8("/Literata-SemiBoldItalic.ttf"));
    oc_unicode_range ranges[5] = { OC_UNICODE_BASIC_LATIN,
                                   OC_UNICODE_C1_CONTROLS_AND_LATIN_1_SUPPLEMENT,
                                   OC_UNICODE_LATIN_EXTENDED_A,
                                   OC_UNICODE_LATIN_EXTENDED_B,
                                   OC_UNICODE_SPECIALS };
    // NOTE(ben): Weird that images are "create from data" but fonts are "create from memory"
    // TODO: Decide whether we're using strings or explicit pointer + length
    pongFont = oc_font_create_from_memory(fontStr, 5, ranges);

    oc_arena_clear(oc_scratch());
}

ORCA_EXPORT void oc_on_resize(u32 width, u32 height)
{
    oc_log_info("frame resize %u, %u", width, height);
    frameSize.x = width;
    frameSize.y = height;
}

ORCA_EXPORT void oc_on_mouse_down(int button)
{
    oc_log_info("mouse down!");
}

ORCA_EXPORT void oc_on_key_down(int key)
{
    if(key == OC_KEY_SPACE)
    {
        oc_log_error("(this is just for testing errors)");
        return;
    }
    if(key == OC_KEY_ENTER)
    {
        oc_log_warning("(this is just for testing warning)");
        return;
    }

    oc_log_info("key down: %i", key);
    if(key == OC_KEY_LEFT)
    {
        leftDown = true;
    }
    if(key == OC_KEY_RIGHT)
    {
        rightDown = true;
    }
}

ORCA_EXPORT void oc_on_key_up(int key)
{
    if(key == OC_KEY_ENTER || key == OC_KEY_SPACE)
    {
        return;
    }

    oc_log_info("key up: %i", key);
    if(key == OC_KEY_LEFT)
    {
        leftDown = false;
    }
    if(key == OC_KEY_RIGHT)
    {
        rightDown = false;
    }
}

ORCA_EXPORT void oc_on_frame_refresh(void)
{
    f32 aspect = frameSize.x / frameSize.y;

    if(leftDown)
    {
        paddle.x -= 10;
    }
    else if(rightDown)
    {
        paddle.x += 10;
    }
    paddle.x = oc_clamp(paddle.x, 0, frameSize.x - paddle.w);

    ball.x += velocity.x;
    ball.y += velocity.y;
    ball.x = oc_clamp(ball.x, 0, frameSize.x - ball.w);
    ball.y = oc_clamp(ball.y, 0, frameSize.y - ball.h);

    if(ball.x + ball.w >= frameSize.x)
    {
        velocity.x = -velocity.x;
    }
    if(ball.x <= 0)
    {
        velocity.x = -velocity.x;
    }
    if(ball.y + ball.h >= frameSize.y)
    {
        velocity.y = -velocity.y;
    }

    if(
        ball.y <= paddle.y + paddle.h && ball.x + ball.w >= paddle.x && ball.x <= paddle.x + paddle.w && velocity.y < 0)
    {
        f32 t = ((ball.x + ball.w / 2) - paddle.x) / paddle.w;
        f32 launchAngle = lerp(-PADDLE_MAX_LAUNCH_ANGLE, PADDLE_MAX_LAUNCH_ANGLE, t);
        f32 speed = sqrtf(velocity.x * velocity.x + velocity.y * velocity.y);
        velocity = (oc_vec2){
            sinf(launchAngle) * speed,
            cosf(launchAngle) * speed,
        };
        ball.y = paddle.y + paddle.h;

        oc_log_info("PONG!");
    }

    if(ball.y <= 0)
    {
        ball.x = frameSize.x / 2. - ball.w;
        ball.y = frameSize.y / 2. - ball.h;
    }

    for(int i = 0; i < NUM_BLOCKS; i++)
    {
        if(blockHealth[i] <= 0)
        {
            continue;
        }

        oc_rect r = blockRect(i);
        int result = checkCollision(r);
        if(result)
        {
            oc_log_info("Collision! direction=%d", result);
            blockHealth[i] -= 1;

            f32 vx = velocity.x;
            f32 vy = velocity.y;

            switch(result)
            {
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

    oc_canvas_set_current(canvas);

    oc_set_color_rgba(10.0f / 255.0f, 31.0f / 255.0f, 72.0f / 255.0f, 1);
    oc_clear();

    oc_image_draw(waterImage, (oc_rect){ 0, 0, frameSize.x, frameSize.y });

    oc_mat2x3 yUp = {
        1, 0, 0,
        0, -1, frameSize.y
    };

    oc_matrix_push(yUp);
    {
        for(int i = 0; i < NUM_BLOCKS; i++)
        {
            if(blockHealth[i] <= 0)
            {
                continue;
            }

            oc_rect r = blockRect(i);
            oc_set_color_rgba(0, 0, 0, 0.2);
            oc_rounded_rectangle_fill(r.x, r.y - 2, r.w, r.h, 4);
            oc_set_color_rgba(0.9, 0.9, 0.9, 1);
            oc_rounded_rectangle_fill(r.x, r.y, r.w, r.h, 4);

            int fontSize = 18;
            oc_str8 text = oc_str8_pushf(oc_scratch(),
                                         "%d", blockHealth[i]);
            oc_rect textRect = oc_text_bounding_box(pongFont, fontSize, text);

            oc_vec2 textPos = {
                r.x + r.w / 2 - textRect.w / 2,
                r.y + 9, // TODO: oc_text_bounding_box is returning extremely wack results for height.
            };

            oc_set_color_rgba(0, 0, 0, 1);
            oc_set_font(pongFont);
            oc_set_font_size(18);
            oc_move_to(textPos.x, textPos.y);
            oc_matrix_push(flipYAt(textPos));
            {
                oc_text_outlines(text);
                oc_fill();
            }
            oc_matrix_pop();
        }

        oc_set_color_rgba(0.9, 0.9, 0.9, 1);
        oc_rounded_rectangle_fill(paddle.x, paddle.y, paddle.w, paddle.h, 4);

        oc_matrix_push(flipY(ball));
        {
            oc_image_draw(ballImage, ball);
        }
        oc_matrix_pop();
    }
    oc_matrix_pop();

    oc_surface_select(surface);
    oc_render(surface, canvas);
    oc_surface_present(surface);
}

oc_rect blockRect(int i)
{
    int row = i / NUM_BLOCKS_PER_ROW;
    int col = i % NUM_BLOCKS_PER_ROW;
    return (oc_rect){
        BLOCKS_PADDING + (BLOCKS_PADDING + BLOCK_WIDTH) * col,
        BLOCKS_BOTTOM + (BLOCKS_PADDING + BLOCK_HEIGHT) * row,
        BLOCK_WIDTH,
        BLOCK_HEIGHT
    };
}

// Returns a cardinal direction 1-8 for the collision with the block, or zero
// if no collision. 1 is straight up and directions proceed clockwise.
int checkCollision(oc_rect block)
{
    // Note that all the logic for this game has the origin in the bottom left.

    f32 ballx2 = ball.x + ball.w;
    f32 bally2 = ball.y + ball.h;
    f32 blockx2 = block.x + block.w;
    f32 blocky2 = block.y + block.h;

    if(ballx2 < block.x || blockx2 < ball.x || bally2 < block.y || blocky2 < ball.y)
    {
        // Ball is fully outside block
        return 0;
    }

    // if ((block.x <= ball.x && ballx2 <= blockx2)
    //     && (block.y <= ball.y && bally2 <= blocky2))
    // {
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

    oc_vec2 ballCenter = (oc_vec2){ ball.x + ball.w / 2, ball.y + ball.h / 2 };
    oc_vec2 blockCenter = (oc_vec2){ block.x + block.w / 2, block.y + block.h / 2 };

    // Moving right
    if(velocity.x > 0)
    {
        // Ball's top right corner
        if(ballCenter.x <= block.x && block.x <= ballx2 && ballCenter.y <= block.y && block.y <= bally2)
        {
            return 2;
        }

        // Ball's bottom right corner
        if(ballCenter.x <= block.x && block.x <= ballx2 && ball.y <= blocky2 && blocky2 <= ballCenter.y)
        {
            return 4;
        }

        // Ball's right edge
        if(ballCenter.x <= block.x && block.x <= ballx2)
        {
            return 3;
        }
    }

    // Moving up
    if(velocity.y > 0)
    {
        // Ball's top left corner
        if(ball.x <= blockx2 && blockx2 <= ballCenter.x && ballCenter.y <= block.y && block.y <= bally2)
        {
            return 8;
        }

        // Ball's top right corner
        if(ballCenter.x <= block.x && block.x <= ballx2 && ballCenter.y <= block.y && block.y <= bally2)
        {
            return 2;
        }

        // Ball's top edge
        if(ballCenter.y <= block.y && block.y <= bally2)
        {
            return 1;
        }
    }

    // Moving left
    if(velocity.x < 0)
    {
        // Ball's bottom left corner
        if(ball.x <= blockx2 && blockx2 <= ballCenter.x && ball.y <= blocky2 && blocky2 <= ballCenter.y)
        {
            return 6;
        }

        // Ball's top left corner
        if(ball.x <= blockx2 && blockx2 <= ballCenter.x && ballCenter.y <= block.y && block.y <= bally2)
        {
            return 8;
        }

        // Ball's left edge
        if(ball.x <= blockx2 && blockx2 <= ballCenter.x)
        {
            return 7;
        }
    }

    // Moving down
    if(velocity.y < 0)
    {
        // Ball's bottom right corner
        if(ballCenter.x <= block.x && block.x <= ballx2 && ball.y <= blocky2 && blocky2 <= ballCenter.y)
        {
            return 4;
        }

        // Ball's bottom left corner
        if(ball.x <= blockx2 && blockx2 <= ballCenter.x && ball.y <= blocky2 && blocky2 <= ballCenter.y)
        {
            return 6;
        }

        // Ball's bottom edge
        if(ball.y <= blocky2 && blocky2 <= ballCenter.y)
        {
            return 5;
        }
    }

    return 0;
}

f32 lerp(f32 a, f32 b, f32 t)
{
    return (1 - t) * a + t * b;
}

oc_mat2x3 flipY(oc_rect r)
{
    return (oc_mat2x3){
        1, 0, 0,
        0, -1, 2 * r.y + r.h
    };
}

oc_mat2x3 flipYAt(oc_vec2 pos)
{
    return (oc_mat2x3){
        1, 0, 0,
        0, -1, 2 * pos.y
    };
}
