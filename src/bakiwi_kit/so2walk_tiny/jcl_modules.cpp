#include "jcl_modules.h"
#include <stdlib.h>

namespace jcl {

/* clips values to Interval [l_limit, u_limit] */
float clip(float x, float l_limit, float u_limit)
{
    if (x > u_limit) return u_limit;
    else if (x < l_limit) return l_limit;
    else return x;
}

float
random_value(float a, float b)
{
    /* switch interval borders if necessary */
    float temp;
    if (b < a) {
        temp = a;
        a = b;
        b = temp;
    }
    /* generate random value for the given interval */
    return (b - a) * ((float) rand()) / RAND_MAX + a;
}

} /* namespace jcl */
