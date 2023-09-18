#include "libm.h"

float __math_divzerof(uint32_t sign)
{
	// NOTE(orca): no fp barriers
	return (sign ? -1.0f : 1.0f) / 0.0f;
}
