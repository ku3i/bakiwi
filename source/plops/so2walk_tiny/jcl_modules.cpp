#include "jcl_modules.h"

namespace jcl {

/* clips values to Interval [l_limit, u_limit] */
float clip(float x, float l_limit, float u_limit)
{
    if (x > u_limit) return u_limit;
    else if (x < l_limit) return l_limit;
    else return x;
}

} /* namespace jcl */
