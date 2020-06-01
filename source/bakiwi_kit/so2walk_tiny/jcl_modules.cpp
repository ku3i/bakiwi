#include "jcl_modules.h"

namespace jcl {

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
