
struct oc_path
{
    uvTransform : mat3x3f,
    colors : array<vec4f, 4>,
    box: vec4f,
    clip: vec4f,
    cmd : u32,
    textureID : i32,
    hasGradient : i32,
};

struct oc_path_elt
{
    pathIndex: i32,
    kind : i32,
    p : array<vec2f, 4>,
};

struct oc_segment
{
    kind : i32,
    pathIndex : i32,
    windingIncrement : i32,
    config : i32,
    box : vec4f,
    implicitMatrix : mat3x3f,
    hullVertex : vec2f,
    sign : f32,
    debugID : i32,
};

struct oc_tile_op
{
    kind : i32,
    next : i32,
    index : u32,
    windingOffsetOrCrossRight : i32,
};

struct oc_bin_queue
{
    windingOffset : i32,
    first : i32,
    last : i32,
};

struct oc_bin_queue_atomic
{
    windingOffset : atomic<i32>,
    first : atomic<i32>,
    last : i32,
};

struct oc_path_bin
{
    area : vec4i,
    binQueues : i32,
};

struct oc_chunk
{
    first : i32,
    last : i32,
};

struct oc_chunk_elt
{
    next : i32,
    pathIndex : u32,
};

struct oc_tile_queue
{
    tileCoord : vec2u,
    first : i32,
};

struct oc_debug_display_options
{
    showTileBorders : u32,
    showPathArea : u32,
    showClip : u32,
    textureOff: u32,
    debugTileQueues : u32,
};

const OC_CMD_FILL : u32 = 0;
const OC_CMD_STROKE : u32 = 1;

const OC_SEG_BL : i32 = 0; // curve on bottom left
const OC_SEG_BR : i32 = 1; // curve on bottom right
const OC_SEG_TL : i32 = 2; // curve on top left
const OC_SEG_TR : i32 = 3; // curve on top right

const OC_SEG_LINE : i32 = 1;
const OC_SEG_QUADRATIC : i32 = 2;
const OC_SEG_CUBIC : i32 = 3;

const OC_OP_START : i32 = 0;
const OC_OP_CLIP_FILL : i32 = 1;
const OC_OP_FILL : i32 = 2;
const OC_OP_SEGMENT : i32 = 3;
const OC_OP_END : i32 = 4;

fn squaref(x : f32) -> f32
{
    return(x*x);
}

fn cubef(x : f32) -> f32
{
    return(x*x*x);
}

fn ccw(a : vec2f, b : vec2f, c : vec2f) -> f32
{
    return ((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x));
}

fn side_of_segment(p : vec2f, seg : oc_segment) -> i32
{
    var side : i32 = 0;
    if(p.y > seg.box.w || p.y <= seg.box.y)
    {
        if(p.x > seg.box.x && p.x <= seg.box.z)
        {
            if(p.y > seg.box.w)
            {
                if(seg.config == OC_SEG_TL || seg.config == OC_SEG_BR)
                {
                    side = -1;
                }
                else
                {
                    side = 1;
                }
            }
            else
            {
                if(seg.config == OC_SEG_TL || seg.config == OC_SEG_BR)
                {
                    side = 1;
                }
                else
                {
                    side = -1;
                }
            }
        }
    }
    else if(p.x > seg.box.z)
    {
        side = 1;
    }
    else if(p.x <= seg.box.x)
    {
        side = -1;
    }
    else
    {
        var a : vec2f;
        var b : vec2f;
        var c : vec2f;

        switch(seg.config)
        {
            case OC_SEG_TL:
            {
                a = seg.box.xy;
                b = seg.box.zw;
            }

            case OC_SEG_BR:
            {
                a = seg.box.zw;
                b = seg.box.xy;
            }

            case OC_SEG_TR:
            {
                a = seg.box.xw;
                b = seg.box.zy;
            }

            case OC_SEG_BL, default:
            {
                a = seg.box.zy;
                b = seg.box.xw;
            }
        }
        c = seg.hullVertex;

        if(ccw(a, b, p) < 0)
        {
            // other side of the diagonal
            if(seg.config == OC_SEG_BR || seg.config == OC_SEG_TR)
            {
                side = -1;
            }
            else
            {
                side = 1;
            }
        }
        else if(ccw(b, c, p) < 0 || ccw(c, a, p) < 0)
        {
            // same side of the diagonal, but outside curve hull
            if(seg.config == OC_SEG_BL || seg.config == OC_SEG_TL)
            {
                side = -1;
            }
            else
            {
                side = 1;
            }
        }
        else
        {
            switch(seg.kind)
            {
                case OC_SEG_LINE, default:
                {
                    side = 1;
                }

                case OC_SEG_QUADRATIC:
                {
                    let ph : vec3f = vec3f(p.x, p.y, 1);
                    let klm : vec3f = seg.implicitMatrix * ph;
                    let test : f32 = (klm.x * klm.x - klm.y) * klm.z;
                    if(test < 0)
                    {
                        side = -1;
                    }
                    else
                    {
                        side = 1;
                    }
                }

                case OC_SEG_CUBIC:
                {
                    let ph : vec3f = vec3f(p.x, p.y, 1);
                    let klm : vec3f = seg.implicitMatrix * ph;
                    let test : f32 = seg.sign * (klm.x * klm.x * klm.x - klm.y * klm.z);
                    if(test < 0)
                    {
                        side = -1;
                    }
                    else
                    {
                        side = 1;
                    }
                }
            }
        }
    }
    return (side);
}
