//------------------------------------------------------------------------------------------------
// Blit
//------------------------------------------------------------------------------------------------

@group(0) @binding(0) var outTexture : texture_2d<f32>;

/*
struct vertex_out
{
    @builtin(position) pos : vec4f,
    @location(0) uv : vec2f,
};
*/
@vertex fn vs(@builtin(vertex_index) vertexIndex : u32) -> @builtin(position) vec4f
{
    var pos = vec2f(f32((vertexIndex << 1) & 2), f32(vertexIndex & 2));

    var out : vec4f = vec4f(0, 0, 0, 1);

    out = vec4f(pos * vec2f(2, 2) + vec2f(-1, -1), 0.0, 1.0);
    return out;
}

@fragment fn fs(@builtin(position) pos: vec4f) -> @location(0) vec4f
{
    var color = textureLoad(outTexture, vec2u(pos.xy), 0);
    color = pow(color, vec4f(0.454545, 0.454545, 0.454545, 1));
    return (color);
}
