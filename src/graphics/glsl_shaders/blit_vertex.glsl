
precision mediump float;

out vec2 uv;

void main()
{
    /* generate (0, 0) (1, 0) (1, 1) (1, 1) (0, 1) (0, 0)*/

    float x = float(((uint(gl_VertexID) + 2u) / 3u) % 2u);
    float y = float(((uint(gl_VertexID) + 1u) / 3u) % 2u);

    gl_Position = vec4(-1.0f + x * 2.0f, -1.0f + y * 2.0f, 0.0f, 1.0f);
    uv = vec2(x, 1 - y);
}
