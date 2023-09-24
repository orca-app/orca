#include "libm.h"

float __math_xflowf(uint32_t sign, float y)
{
    // NOTE(orca): no fp barriers
    // return eval_as_double(fp_barrier(sign ? -y : y) * y);
    return eval_as_float((sign ? -y : y) * y);
}
