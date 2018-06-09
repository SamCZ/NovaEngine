#ifndef EX_CORE_MATH_H
#define EX_CORE_MATH_H

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef M_PI
#undef M_PI
#define M_PI 3.14159265359f
#endif // M_PI

#ifdef M_PI_2
#undef M_PI_2
#define M_PI_2 (M_PI * 2)
#endif // M_PI


#define INV_PI (1.0f / M_PI)
#define INV_PI_2 (1.0f / M_PI_2)
#define HALF_PI (M_PI * 0.5f)

#endif // !EX_CORE_MATH_H
