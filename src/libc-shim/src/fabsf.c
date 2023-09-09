#include <math.h>
#include <stdint.h>

float fabsf(float x)
{
    union
    {
        float f;
        uint32_t i;
    } u = { x };

    u.i &= -1U / 2;
    return u.f;
}
