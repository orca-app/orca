#include "graphics.h"
#include "graphics_common.h"

void oc_text_draw_run(oc_glyph_run* run, f32 fontSize)
{
    oc_canvas_context_data* context = oc_currentCanvasContext;
    if(!context)
    {
        return;
    }
    oc_font_data* fontData = oc_font_data_from_handle(run->font);
    if(!fontData)
    {
        return;
    }

    oc_arena_scope scratch = oc_scratch_begin();

    f32 scale = oc_font_get_scale_for_em_pixels(run->font, fontSize);
    oc_vec2 start = context->subPathLastPoint;
    bool flipY = context->textFlip;

    u32 eltCount = 0;
    oc_path_elt* elements = oc_harfbuzz_get_curves(scratch.arena,
                                                   fontData->harfbuzzHandle,
                                                   run,
                                                   context->subPathLastPoint,
                                                   scale,
                                                   flipY,
                                                   &eltCount);

    for(u32 i = 0; i < eltCount; i++)
    {
        oc_path_elt* elt = &elements[i];

        switch(elt->type)
        {
            case OC_PATH_MOVE:
                oc_move_to(elt->p[0].x, elt->p[0].y);
                break;
            case OC_PATH_LINE:
                oc_line_to(elt->p[0].x, elt->p[0].y);
                break;
            case OC_PATH_QUADRATIC:
                oc_quadratic_to(elt->p[0].x, elt->p[0].y, elt->p[1].x, elt->p[1].y);
                break;
            case OC_PATH_CUBIC:
                oc_cubic_to(elt->p[0].x, elt->p[0].y, elt->p[1].x, elt->p[1].y, elt->p[2].x, elt->p[2].y);
                break;
        }
    }
    oc_fill_rule oldRule = context->attributes.fillRule;
    context->attributes.fillRule = OC_FILL_NON_ZERO;
    oc_fill();
    context->attributes.fillRule = oldRule;

    oc_scratch_end(scratch);
}
