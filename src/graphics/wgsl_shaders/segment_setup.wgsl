
//------------------------------------------------------------------------------------------------
// Segment setup
//------------------------------------------------------------------------------------------------

@group(0) @binding(0) var<storage, read> pathBuffer : array<oc_path>;
@group(0) @binding(1) var<storage, read> eltBuffer : array<oc_path_elt>;
@group(0) @binding(2) var<uniform> eltCount : u32;
@group(0) @binding(3) var<storage, read_write> segmentBuffer : array<oc_segment>;
@group(0) @binding(4) var<storage, read_write> segmentCount : atomic<u32>;
@group(0) @binding(5) var<storage, read_write> pathBins : array<oc_path_bin>;
@group(0) @binding(6) var<storage, read_write> binQueues : array<oc_bin_queue_atomic>;
@group(0) @binding(7) var<storage, read_write> tileOpBuffer : array<oc_tile_op>;
@group(0) @binding(8) var<storage, read_write> tileOpCount : atomic<u32>;
@group(0) @binding(9) var<uniform> tileSize : u32;

fn push_segment(p: array<vec2f, 4>, kind : i32, pathIndex : i32) -> u32
{
    let segIndex = atomicAdd(&segmentCount, 1u);

    if(segIndex < arrayLength(&segmentBuffer))
    {
        var s : vec2f;
        var e : vec2f;
        var c : vec2f;

        switch(kind)
        {
            case OC_SEG_LINE, default:
            {
                s = p[0];
                c = p[0];
                e = p[1];
            }

            case OC_SEG_QUADRATIC:
            {
                s = p[0];
                c = p[1];
                e = p[2];
            }

            case OC_SEG_CUBIC:
            {
                s = p[0];
                let sqrNorm0 : f32 = dot(p[1] - p[0], p[1] - p[0]);
                let sqrNorm1 : f32 = dot(p[3] - p[2], p[3] - p[2]);
                if(sqrNorm0 < sqrNorm1)
                {
                    c = p[2];
                }
                else
                {
                    c = p[1];
                }
                e = p[3];
            }
        }

        let seg = &segmentBuffer[segIndex];

        let goingUp : bool = e.y >= s.y;
        let goingRight : bool = e.x >= s.x;

        (*seg).kind = kind;
        (*seg).pathIndex = pathIndex;

        if(goingUp)
        {
            (*seg).windingIncrement = 1;
        }
        else
        {
            (*seg).windingIncrement = -1;
        }

        (*seg).box = vec4f(
            min(s.x, e.x),
            min(s.y, e.y),
            max(s.x, e.x),
            max(s.y, e.y)
        );

        let dx = c.x - (*seg).box.x;
        let dy = c.y - (*seg).box.y;
        let alpha = ((*seg).box.w - (*seg).box.y) / ((*seg).box.z - (*seg).box.x);
        let ofs = (*seg).box.w - (*seg).box.y;

        if(goingUp == goingRight)
        {
            if(kind == OC_SEG_LINE)
            {
                (*seg).config = OC_SEG_BR;
            }
            else if(dy > alpha * dx)
            {
                (*seg).config = OC_SEG_TL;
            }
            else
            {
                (*seg).config = OC_SEG_BR;
            }
        }
        else
        {
            if(kind == OC_SEG_LINE)
            {
                (*seg).config = OC_SEG_TR;
            }
            else if(dy < ofs - alpha * dx)
            {
                (*seg).config = OC_SEG_BL;
            }
            else
            {
                (*seg).config = OC_SEG_TR;
            }
        }
    }
    return(segIndex);
}

fn bin_to_tiles(segIndex : u32)
{
    //NOTE: add segment index to the queues of tiles it overlaps with
    let seg : oc_segment = segmentBuffer[segIndex];
    let pathBin : oc_path_bin = pathBins[seg.pathIndex];

    let pathArea : vec4i  = pathBin.area;
    let coveredTiles : vec4i = vec4i(seg.box) / i32(tileSize);
    let xMin : i32 = max(0, coveredTiles.x - pathArea.x);
    let yMin : i32 = max(0, coveredTiles.y - pathArea.y);
    let xMax : i32 = min(coveredTiles.z - pathArea.x, pathArea.z - 1);
    let yMax : i32 = min(coveredTiles.w - pathArea.y, pathArea.w - 1);

    for(var y : i32 = yMin; y <= yMax; y++)
    {
        for(var x : i32 = xMin; x <= xMax; x++)
        {
            let tileBox = vec4f(f32(x + pathArea.x),
                                f32(y + pathArea.y),
                                f32(x + pathArea.x + 1),
                                f32(y + pathArea.y + 1))
                         * f32(tileSize);

            let bl = vec2f(tileBox.x, tileBox.y);
            let br = vec2f(tileBox.z, tileBox.y);
            let tr = vec2f(tileBox.z, tileBox.w);
            let tl = vec2f(tileBox.x, tileBox.w);

            let sbl : i32 = side_of_segment(bl, seg);
            let sbr : i32 = side_of_segment(br, seg);
            let str : i32 = side_of_segment(tr, seg);
            let stl : i32 = side_of_segment(tl, seg);

            let crossL : bool = (stl * sbl < 0);
            let crossR : bool = (str * sbr < 0);
            let crossT : bool = (stl * str < 0);
            let crossB : bool = (sbl * sbr < 0);

            var s0 : vec2f;
            var s1 : vec2f;

            if(seg.config == OC_SEG_TL || seg.config == OC_SEG_BR)
            {
                s0 = seg.box.xy;
                s1 = seg.box.zw;
            }
            else
            {
                s0 = seg.box.xw;
                s1 = seg.box.zy;
            }
            var s0Inside : bool = s0.x >= tileBox.x
                               && s0.x < tileBox.z
                               && s0.y >= tileBox.y
                               && s0.y < tileBox.w;

            var s1Inside : bool = s1.x >= tileBox.x
                               && s1.x < tileBox.z
                               && s1.y >= tileBox.y
                               && s1.y < tileBox.w;

            if(crossL || crossR || crossT || crossB || s0Inside || s1Inside)
            {
                let tileOpIndex : u32 = atomicAdd(&tileOpCount, 1u);

                if(tileOpIndex < arrayLength(&tileOpBuffer))
                {
                    tileOpBuffer[tileOpIndex].kind = OC_OP_SEGMENT;
                    tileOpBuffer[tileOpIndex].index = segIndex;
                    tileOpBuffer[tileOpIndex].windingOffsetOrCrossRight = 0;
                    tileOpBuffer[tileOpIndex].next = -1;

                    let binQueueIndex = i32(pathBin.binQueues) + y * pathArea.z + x;

                    tileOpBuffer[tileOpIndex].next = atomicExchange(&binQueues[binQueueIndex].first, i32(tileOpIndex));
                    if(tileOpBuffer[tileOpIndex].next == -1)
                    {
                        binQueues[binQueueIndex].last = i32(tileOpIndex);
                    }

                    //NOTE: if the segment crosses the tile's bottom boundary, update the tile's winding offset
                    if(crossB)
                    {
                        atomicAdd(&binQueues[binQueueIndex].windingOffset, seg.windingIncrement);
                    }

                    //NOTE: if the segment crosses the right boundary, mark it.
                    if(crossR)
                    {
                        tileOpBuffer[tileOpIndex].windingOffsetOrCrossRight = 1;
                    }
                }
            }
        }
    }
}


//--------------------------------------------------------------
//NOTE: Lines
//--------------------------------------------------------------

fn line_setup(p : array<vec2f, 4>, pathIndex : i32)
{
    var segIndex = push_segment(p, OC_SEG_LINE, pathIndex);
    if(segIndex < arrayLength(&segmentBuffer))
    {
        segmentBuffer[segIndex].hullVertex = p[0];
        segmentBuffer[segIndex].debugID = 0;

        bin_to_tiles(segIndex);
    }
}

//--------------------------------------------------------------
//NOTE: Quadratics
//--------------------------------------------------------------

fn quadratic_blossom(p : array<vec2f, 4>, u : f32, v : f32) -> vec2f
{
    let b10 : vec2f = u * p[1] + (1 - u) * p[0];
    let b11 : vec2f = u * p[2] + (1 - u) * p[1];
    let b20 : vec2f = v * b11 + (1 - v) * b10;
    return (b20);
}

fn quadratic_slice(p : array<vec2f, 4>, s0 : f32, s1 : f32, sp : ptr<function, array<vec2f ,4>>)
{
    /*NOTE: using blossoms to compute sub-curve control points ensure that the fourth point
	        of sub-curve (s0, s1) and the first point of sub-curve (s1, s3) match.
	        However, due to numerical errors, the evaluation of B(s=0) might not be equal to
	        p[0] (and likewise, B(s=1) might not equal p[3]).
	        We handle that case explicitly to ensure that we don't create gaps in the paths.
	*/
	if(s0 == 0)
	{
        (*sp)[0] =  p[0];
    }
    else
    {
        (*sp)[0] = quadratic_blossom(p, s0, s0);
    }
    (*sp)[1] = quadratic_blossom(p, s0, s1);

    if(s1 == 1)
    {
        (*sp)[2] = p[2];
    }
    else
    {
        (*sp)[2] = quadratic_blossom(p, s1, s1);
    }
}

fn quadratic_monotonize(p : array<vec2f, 4>, splits : ptr<function, array<f32,4>>) -> i32
{
    //NOTE: compute split points
    var count : i32 = 0;
    (*splits)[0] = 0;
    count++;

    var r : vec2f = (p[0] - p[1]) / (p[2] - 2 * p[1] + p[0]);
    if(r.x > r.y)
    {
        let tmp : f32 = r.x;
        r.x = r.y;
        r.y = tmp;
    }
    if(r.x > 0 && r.x < 1)
    {
        (*splits)[count] = r.x;
        count++;
    }
    if(r.y > 0 && r.y < 1)
    {
        (*splits)[count] = r.y;
        count++;
    }
    (*splits)[count] = 1;
    count++;
    return (count);
}

fn quadratic_emit(p : array<vec2f, 4>, pathIndex : i32, debugID : i32)
{
    var segIndex : u32 = push_segment(p, OC_SEG_QUADRATIC, pathIndex);

    if(segIndex < arrayLength(&segmentBuffer))
    {
        let seg = &segmentBuffer[segIndex];

        //NOTE: compute implicit equation matrix
        let det : f32 = p[0].x * (p[1].y - p[2].y) + p[1].x * (p[2].y - p[0].y) + p[2].x * (p[0].y - p[1].y);

        let a : f32 = p[0].y - p[1].y + 0.5 * (p[2].y - p[0].y);
        let b : f32 = p[1].x - p[0].x + 0.5 * (p[0].x - p[2].x);
        let c : f32 = p[0].x * p[1].y - p[1].x * p[0].y + 0.5 * (p[2].x * p[0].y - p[0].x * p[2].y);
        let d : f32 = p[0].y - p[1].y;
        let e : f32 = p[1].x - p[0].x;
        let f : f32 = p[0].x * p[1].y - p[1].x * p[0].y;

        var flip : f32;
        if(  (*seg).config == OC_SEG_TL
          || (*seg).config == OC_SEG_BL)
        {
            flip = -1;
        }
        else
        {
            flip = 1;
        }

        let g : f32 = flip * (p[2].x * (p[0].y - p[1].y) + p[0].x * (p[1].y - p[2].y) + p[1].x * (p[2].y - p[0].y));

        (*seg).implicitMatrix = (1 / det) * mat3x3f(a, d, 0., b, e, 0., c, f, g);
        (*seg).hullVertex = p[1];

        (*seg).debugID = debugID;
        bin_to_tiles(segIndex);
    }
}

fn quadratic_setup(p : array<vec2f, 4>, pathIndex : i32)
{
    var splits : array<f32, 4>;
    var splitCount : i32 = quadratic_monotonize(p, &splits);

    //NOTE: produce bézier curve for each consecutive pair of roots
    for(var sliceIndex : i32 = 0; sliceIndex < splitCount - 1; sliceIndex++)
    {
        var sp : array<vec2f, 4>;
        quadratic_slice(p, splits[sliceIndex], splits[sliceIndex + 1], &sp);
        quadratic_emit(sp, pathIndex, sliceIndex);
    }
}

//--------------------------------------------------------------
//NOTE: Cubics
//--------------------------------------------------------------

fn quadratic_roots_with_det(a : f32, b : f32, c : f32, det : f32, r : ptr<function, array<f32, 2>>) -> i32
{
    var count : i32 = 0;

    if(a == 0)
    {
        if(b != 0)
        {
            count = 1;
            (*r)[0] = -c / b;
        }
    }
    else
    {
        let B : f32 = b/2.0;

        if(det >= 0)
        {
            if(det == 0)
            {
                count = 1;
            }
            else
            {
                count = 2;
            }

            if(B > 0)
            {
                let q : f32 = B + sqrt(det);
                (*r)[0] = -c / q;
                (*r)[1] = -q / a;
            }
            else if(B < 0)
            {
                let q : f32 = -B + sqrt(det);
                (*r)[0] = q / a;
                (*r)[1] = c / q;
            }
            else
            {
                let q : f32 = sqrt(-a * c);
                if(abs(a) >= abs(c))
                {
                    (*r)[0] = q / a;
                    (*r)[1] = -q / a;
                }
                else
                {
                    (*r)[0] = -c / q;
                    (*r)[1] = c / q;
                }
            }
        }
    }

    if(count > 1 && (*r)[0] > (*r)[1])
    {
        let tmp : f32 = (*r)[0];
        (*r)[0] = (*r)[1];
        (*r)[1] = tmp;
    }
    return (count);
}

fn quadratic_roots(a : f32, b : f32, c : f32, r : ptr<function, array<f32, 2>>) -> i32
{
    let det : f32 = squaref(b) / 4. - a * c;
    return (quadratic_roots_with_det(a, b, c, det, r));
}


const CUBIC_ERROR : i32 = 0;
const CUBIC_SERPENTINE : i32 = 1;
const CUBIC_CUSP : i32 = 2;
const CUBIC_CUSP_INFINITY : i32 = 3;
const CUBIC_LOOP : i32 = 4;
const CUBIC_DEGENERATE_QUADRATIC : i32 = 5;
const CUBIC_DEGENERATE_LINE : i32 = 6;

struct cubic_info
{
    kind : i32,
    K : mat4x4f,
    ts : array<vec2f, 2>,
    d1 : f32,
    d2 : f32,
    d3 : f32,
};

fn cubic_classify(c : array<vec2f, 4>) -> cubic_info
{
    var F : mat4x4f;
    var result : cubic_info;
    result.kind = CUBIC_ERROR;

    /*NOTE(martin):
		now, compute determinants d0, d1, d2, d3, which gives the coefficients of the
	        inflection points polynomial:

		I(t, s) = d0*t^3 - 3*d1*t^2*s + 3*d2*t*s^2 - d3*s^3

		The roots of this polynomial are the inflection points of the parametric curve, in homogeneous
		coordinates (ie we can have an inflection point at inifinity with s=0).

		         |x3 y3 w3|              |x3 y3 w3|             |x3 y3 w3|              |x2 y2 w2|
		d0 = det |x2 y2 w2|    d1 = -det |x2 y2 w2|    d2 = det |x1 y1 w1|    d3 = -det |x1 y1 w1|
		         |x1 y1 w1|              |x0 y0 w0|             |x0 y0 w0|              |x0 y0 w0|

		In our case, the pi.w equal 1 (no point at infinity), so _in_the_power_basis_, w1 = w2 = w3 = 0 and w0 = 1
		(which also means d0 = 0)

		//WARN: there seems to be a mismatch between the signs of the d_i and the orientation test in the Loop-Blinn paper?
		//      flipping the sign of the d_i doesn't change the roots (and the implicit matrix), but it does change the orientation.
		//      Keeping the signs of the paper puts the interior on the left of parametric travel, unlike what's stated in the paper.
		//      this may very well be an error on my part that's cancelled by flipping the signs of the d_i though!
	*/

    let d1 : f32 = -(c[3].y * c[2].x - c[3].x * c[2].y);
    let d2 : f32 = -(c[3].x * c[1].y - c[3].y * c[1].x);
    let d3 : f32 = -(c[2].y * c[1].x - c[2].x * c[1].y);

    result.d1 = d1;
    result.d2 = d2;
    result.d3 = d3;

    //NOTE(martin): compute the second factor of the discriminant discr(I) = d1^2*(3*d2^2 - 4*d3*d1)
    let discrFactor2 : f32 = 3.0 * squaref(d2) - 4.0 * d3 * d1;

    //NOTE(martin): each following case gives the number of roots, hence the category of the parametric curve
    if(abs(d1) <= 1e-6 && abs(d2) <= 1e-6 && abs(d3) > 1e-6)
    {
        //NOTE(martin): quadratic degenerate case
        //NOTE(martin): compute quadratic curve control point, which is at p0 + 1.5*(p1-p0) = 1.5*p1 - 0.5*p0
        result.kind = CUBIC_DEGENERATE_QUADRATIC;
    }
    else if((discrFactor2 > 0 && abs(d1) > 1e-6)
            || (discrFactor2 == 0 && abs(d1) > 1e-6))
    {
        //NOTE(martin): serpentine curve or cusp with inflection at infinity
        //              (these two cases are handled the same way).
        //NOTE(martin): compute the solutions (tl, sl), (tm, sm), and (tn, sn) of the inflection point equation
        var tmtl : array<f32, 2>;
        quadratic_roots_with_det(1, -2 * d2, (4. / 3. * d1 * d3), (1. / 3.) * discrFactor2, &tmtl);

        var tm : f32 = tmtl[0];
        var sm : f32 = 2 * d1;
        var tl : f32 = tmtl[1];
        var sl : f32 = 2 * d1;

        var invNorm : f32 = 1 / sqrt(squaref(tm) + squaref(sm));
        tm *= invNorm;
        sm *= invNorm;

        invNorm = 1 / sqrt(squaref(tl) + squaref(sl));
        tl *= invNorm;
        sl *= invNorm;

        /*NOTE(martin):
			the power basis coefficients of points k,l,m,n are collected into the rows of the 4x4 matrix F:

				| tl*tm            tl^3        tm^3        1 |
				| -sm*tl - sl*tm   -3sl*tl^2   -3*sm*tm^2  0 |
				| sl*sm            3*sl^2*tl   3*sm^2*tm   0 |
				| 0                -sl^3       -sm^3       0 |
		*/
		if(discrFactor2 > 0 && d1 != 0)
		{
            result.kind = CUBIC_SERPENTINE;
        }
        else
        {
            result.kind = CUBIC_CUSP;
        }

        F = mat4x4f(tl * tm, -sm * tl - sl * tm, sl * sm, 0,
                    cubef(tl), -3 * sl * squaref(tl), 3 * squaref(sl) * tl, -cubef(sl),
                    cubef(tm), -3 * sm * squaref(tm), 3 * squaref(sm) * tm, -cubef(sm),
                    1, 0, 0, 0);

        result.ts[0] = vec2f(tm, sm);
        result.ts[1] = vec2f(tl, sl);
    }
    else if(discrFactor2 < 0 && abs(d1) > 1e-6)
    {
        //NOTE(martin): loop curve
        result.kind = CUBIC_LOOP;

        var tetd : array<f32, 2>;
        quadratic_roots_with_det(1, -2 * d2, 4 * (squaref(d2) - d1 * d3), -discrFactor2, &tetd);

        var td : f32 = tetd[1];
        var sd : f32 = 2 * d1;
        var te : f32 = tetd[0];
        var se : f32 = 2 * d1;

        var invNorm : f32 = 1 / sqrt(squaref(td) + squaref(sd));
        td *= invNorm;
        sd *= invNorm;

        invNorm = 1 / sqrt(squaref(te) + squaref(se));
        te *= invNorm;
        se *= invNorm;

        /*NOTE(martin):
			the power basis coefficients of points k,l,m,n are collected into the rows of the 4x4 matrix F:

				| td*te            td^2*te                 td*te^2                1 |
				| -se*td - sd*te   -se*td^2 - 2sd*te*td    -sd*te^2 - 2*se*td*te  0 |
				| sd*se            te*sd^2 + 2*se*td*sd    td*se^2 + 2*sd*te*se   0 |
				| 0                -sd^2*se                -sd*se^2               0 |
		*/
        F = mat4x4f(td * te, -se * td - sd * te, sd * se, 0,
                    squaref(td) * te, -se * squaref(td) - 2 * sd * te * td, te * squaref(sd) + 2 * se * td * sd, -squaref(sd) * se,
                    td * squaref(te), -sd * squaref(te) - 2 * se * td * te, td * squaref(se) + 2 * sd * te * se, -sd * squaref(se),
                    1, 0, 0, 0);

        result.ts[0] = vec2f(td, sd);
        result.ts[1] = vec2f(te, se);
    }
    else if(d2 != 0)
    {
        //NOTE(martin): cusp with cusp at infinity
        var tl : f32 = d3;
        var sl : f32 = 3 * d2;

        let invNorm : f32 = 1 / sqrt(squaref(tl) + squaref(sl));
        tl *= invNorm;
        sl *= invNorm;

        /*NOTE(martin):
			the power basis coefficients of points k,l,m,n are collected into the rows of the 4x4 matrix F:

				| tl    tl^3        1  1 |
				| -sl   -3sl*tl^2   0  0 |
				| 0     3*sl^2*tl   0  0 |
				| 0     -sl^3       0  0 |
		*/
        result.kind = CUBIC_CUSP_INFINITY;

        F = mat4x4f(tl, -sl, 0, 0,
                    cubef(tl), -3 * sl * squaref(tl), 3 * squaref(sl) * tl, -cubef(sl),
                    1, 0, 0, 0,
                    1, 0, 0, 0);

        result.ts[0] = vec2f(tl, sl);
        result.ts[1] = vec2f(0, 0);
    }
    else
    {
        //NOTE(martin): line or point degenerate case
        result.kind = CUBIC_DEGENERATE_LINE;
    }

    /*
			F is then multiplied by M3^(-1) on the left which yelds the bezier coefficients k, l, m, n
			at the control points.

			               | 1  0   0   0 |
				M3^(-1) =  | 1  1/3 0   0 |
				           | 1  2/3 1/3 0 |
					       | 1  1   1   1 |
	*/
    let invM3 = mat4x4f(1, 1, 1, 1,
                        0, 1. / 3., 2. / 3., 1,
                        0, 0, 1. / 3., 1,
                        0, 0, 0, 1);

    result.K = transpose(invM3 * F);

    return (result);
}

fn cubic_blossom(p : array<vec2f, 4>, u : f32, v : f32, w : f32) -> vec2f
{
    let b10 : vec2f = u * p[1] + (1 - u) * p[0];
    let b11 : vec2f = u * p[2] + (1 - u) * p[1];
    let b12 : vec2f = u * p[3] + (1 - u) * p[2];
    let b20 : vec2f = v * b11 + (1 - v) * b10;
    let b21 : vec2f = v * b12 + (1 - v) * b11;
    let b30 : vec2f = w * b21 + (1 - w) * b20;
    return (b30);
}

fn cubic_slice(p : array<vec2f, 4>, s0 : f32, s1 : f32, sp : ptr<function, array<vec2f, 4>>)
{
    /*NOTE: using blossoms to compute sub-curve control points ensure that the fourth point
	        of sub-curve (s0, s1) and the first point of sub-curve (s1, s3) match.
	        However, due to numerical errors, the evaluation of B(s=0) might not be equal to
	        p[0] (and likewise, B(s=1) might not equal p[3]).
	        We handle that case explicitly to ensure that we don't create gaps in the paths.
	*/
	if(s0 == 0)
	{
        (*sp)[0] =  p[0];
    }
    else
    {
        (*sp)[0] = cubic_blossom(p, s0, s0, s0);
    }

    (*sp)[1] = cubic_blossom(p, s0, s0, s1);
    (*sp)[2] = cubic_blossom(p, s0, s1, s1);

    if(s1 == 1)
    {
        (*sp)[3] = p[3];
    }
    else
    {
        (*sp)[3] = cubic_blossom(p, s1, s1, s1);
    }
}

fn barycentric_matrix(v0 : vec2f, v1 : vec2f, v2 : vec2f) -> mat3x3f
{
    let det : f32 = v0.x * (v1.y - v2.y) + v1.x * (v2.y - v0.y) + v2.x * (v0.y - v1.y);
    var B = mat3x3f(v1.y - v2.y, v2.y - v0.y, v0.y - v1.y,
                    v2.x - v1.x, v0.x - v2.x, v1.x - v0.x,
                    v1.x * v2.y - v2.x * v1.y, v2.x * v0.y - v0.x * v2.y, v0.x * v1.y - v1.x * v0.y);
    B *= (1 / det);
    return (B);
}

fn select_hull_vertex(p0 : vec2f, p1 : vec2f, p2 : vec2f, p3 : vec2f) -> vec2f
{
    /*NOTE: check intersection of lines (p1-p0) and (p3-p2)
		P = p0 + u(p1-p0)
		P = p2 + w(p3-p2)

		control points are inside a right triangle so we should always find an intersection
	*/
    var pm : vec2f;

    let det : f32 = (p1.x - p0.x) * (p3.y - p2.y) - (p1.y - p0.y) * (p3.x - p2.x);
    let sqrNorm0 : f32 = dot(p1 - p0, p1 - p0);
    let sqrNorm1 : f32 = dot(p2 - p3, p2 - p3);

    if(abs(det) < 1e-3 || sqrNorm0 < 0.1 || sqrNorm1 < 0.1)
    {
        if(sqrNorm0 < sqrNorm1)
        {
            pm = p2;
        }
        else
        {
            pm = p1;
        }
    }
    else
    {
        let u : f32 = ((p0.x - p2.x) * (p2.y - p3.y) - (p0.y - p2.y) * (p2.x - p3.x)) / det;
        pm = p0 + u * (p1 - p0);
    }
    return (pm);
}

fn cubic_emit(curve : cubic_info, p : array<vec2f, 4>, s0 : f32, s1 : f32, sp : array<vec2f, 4>, pathIndex : i32, debugID : i32)
{
    let segIndex : u32  = push_segment(sp, OC_SEG_CUBIC, pathIndex);

    if(segIndex < arrayLength(&segmentBuffer))
    {
        var v0 : vec2f = p[0];
        var v1 : vec2f = p[3];
        var v2 : vec2f;
        var K : mat3x3f;

        //TODO: haul that up in caller
        let sqrNorm0 : f32 = dot(p[1] - p[0], p[1] - p[0]);
        let sqrNorm1 : f32 = dot(p[2] - p[3], p[2] - p[3]);

        if(dot(p[0] - p[3], p[0] - p[3]) > 1e-5)
        {
            if(sqrNorm0 >= sqrNorm1)
            {
                v2 = p[1];
                K = mat3x3f(curve.K[0].xyz, curve.K[3].xyz, curve.K[1].xyz);
            }
            else
            {
                v2 = p[2];
                K = mat3x3f(curve.K[0].xyz, curve.K[3].xyz, curve.K[2].xyz);
            }
        }
        else
        {
            v1 = p[1];
            v2 = p[2];
            K = mat3x3f(curve.K[0].xyz, curve.K[1].xyz, curve.K[2].xyz);
        }
        //NOTE: set matrices

        //TODO: should we compute matrix relative to a base point to avoid loss of precision
        //      when computing barycentric matrix?

        let B : mat3x3f = barycentric_matrix(v0, v1, v2);

        segmentBuffer[segIndex].implicitMatrix = K * B;
        segmentBuffer[segIndex].hullVertex = select_hull_vertex(sp[0], sp[1], sp[2], sp[3]);

        //NOTE: compute sign flip
        segmentBuffer[segIndex].sign = 1;

        if(curve.kind == CUBIC_SERPENTINE
           || curve.kind == CUBIC_CUSP)
        {
            if(curve.d1 < 0)
            {
                segmentBuffer[segIndex].sign =  -1;
            }
            else
            {
                segmentBuffer[segIndex].sign = 1;
            }
        }
        else if(curve.kind == CUBIC_LOOP)
        {
            let d1 : f32 = curve.d1;
            let d2 : f32 = curve.d2;
            let d3 : f32 = curve.d3;

            let H0 : f32 = d3 * d1 - squaref(d2) + d1 * d2 * s0 - squaref(d1) * squaref(s0);
            let H1 : f32 = d3 * d1 - squaref(d2) + d1 * d2 * s1 - squaref(d1) * squaref(s1);
            var H : f32;

            if(abs(H0) > abs(H1))
            {
                H = H0;
            }
            else
            {
                H = H1;
            }

            if(H * d1 > 0)
            {
                segmentBuffer[segIndex].sign = -1;
            }
            else
            {
                 segmentBuffer[segIndex].sign = 1;
            }
        }

        if(sp[3].y > sp[0].y)
        {
            segmentBuffer[segIndex].sign *= -1;
        }

        segmentBuffer[segIndex].debugID = debugID;

        bin_to_tiles(segIndex);
    }
}

fn cubic_setup(p : array<vec2f, 4>, pathIndex : i32)
{
    /*NOTE(martin): first convert the control points to power basis, multiplying by M3

		     | 1  0  0  0|      |p0|      |c0|
		M3 = |-3  3  0  0|, B = |p1|, C = |c1| = M3*B
		     | 3 -6  3  0|      |p2|      |c2|
		     |-1  3 -3  1|      |p3|      |c3|
	*/
    let c = array<vec2f, 4>(
        p[0],
        3.0 * (p[1] - p[0]),
        3.0 * (p[0] + p[2] - 2 * p[1]),
        3.0 * (p[1] - p[2]) + p[3] - p[0]
    );

    //NOTE: get classification, implicit matrix, double points and inflection points
    let curve : cubic_info = cubic_classify(c);

    if(curve.kind == CUBIC_DEGENERATE_LINE)
    {
        let l = array<vec2f, 4>(p[0], p[3], vec2f(0, 0), vec2f(0, 0));
        line_setup(l, pathIndex);
        return;
    }
    else if(curve.kind == CUBIC_DEGENERATE_QUADRATIC)
    {
        let quadPoint = vec2f(1.5 * p[1].x - 0.5 * p[0].x, 1.5 * p[1].y - 0.5 * p[0].y);
        let q = array<vec2f, 4>(p[0], quadPoint, p[3], vec2(0));
        quadratic_setup(q, pathIndex);
        return;
    }


    //NOTE: get the roots of B'(s) = 3.c3.s^2 + 2.c2.s + c1
    var rootsX : array<f32, 2>;
    let rootCountX : i32 = quadratic_roots(3 * c[3].x, 2 * c[2].x, c[1].x, &rootsX);

    var rootsY : array<f32, 2>;
    let rootCountY : i32 = quadratic_roots(3 * c[3].y, 2 * c[2].y, c[1].y, &rootsY);

    var roots : array<f32, 6>;
    for(var i : i32 = 0; i < rootCountX; i++)
    {
        roots[i] = rootsX[i];
    }
    for(var i : i32 = 0; i < rootCountY; i++)
    {
        roots[i + rootCountX] = rootsY[i];
    }

    //NOTE: add double points and inflection points to roots if finite
    var rootCount : i32 = rootCountX + rootCountY;
    for(var i : i32 = 0; i < 2; i++)
    {
        if(curve.ts[i].y != 0)
        {
            roots[rootCount] = curve.ts[i].x / curve.ts[i].y;
            rootCount++;
        }
    }

    //NOTE: sort roots
    for(var i : i32 = 1; i < rootCount; i++)
    {
        let tmp : f32 = roots[i];
        var j : i32 = i - 1;
        while(j >= 0 && roots[j] > tmp)
        {
            roots[j + 1] = roots[j];
            j--;
        }
        roots[j + 1] = tmp;
    }

    //NOTE: compute split points
    var splits : array<f32, 8>;
    var splitCount : i32 = 0;
    splits[0] = 0;
    splitCount++;
    for(var i : i32 = 0; i < rootCount; i++)
    {
        if(roots[i] > 0 && roots[i] < 1)
        {
            splits[splitCount] = roots[i];
            splitCount++;
        }
    }
    splits[splitCount] = 1;
    splitCount++;

    //NOTE: for each monotonic segment, compute hull matrix and sign, and emit segment
    for(var sliceIndex : i32 = 0; sliceIndex < splitCount - 1; sliceIndex++)
    {
        let s0 : f32 = splits[sliceIndex];
        let s1 : f32 = splits[sliceIndex + 1];
        var sp : array<vec2f, 4>;
        cubic_slice(p, s0, s1, &sp);
        cubic_emit(curve, p, s0, s1, sp, pathIndex, sliceIndex);
    }
}

//--------------------------------------------------------------
//NOTE: entry point, dispatch to setup procs
//--------------------------------------------------------------

@compute @workgroup_size(16, 16) fn segment_setup(@builtin(workgroup_id) workGroupID : vec3u,
                                                @builtin(local_invocation_id) localID : vec3u)
{
    let eltIndex : u32 = workGroupID.x * (16*16) + localID.y * 16 + localID.x;

    if(eltIndex >= eltCount)
    {
        return;
    }

    let elt = &eltBuffer[eltIndex];

    let p = array<vec2f, 4>(
        (*elt).p[0],
        (*elt).p[1],
        (*elt).p[2],
        (*elt).p[3]);

    switch((*elt).kind)
    {
        case OC_SEG_LINE, default:
        {
            line_setup(p, (*elt).pathIndex);
        }

        case OC_SEG_QUADRATIC:
        {
            quadratic_setup(p, (*elt).pathIndex);
        }

        case OC_SEG_CUBIC:
        {
            cubic_setup(p, (*elt).pathIndex);
        }
    }
}
