#ifndef JCL_MODULES_H
#define JCL_MODULES_H

#include <math.h>

namespace jcl {

/* clips values to Interval [l_limit, u_limit] */
float clip(float x, float l_limit, float u_limit);

/* generates a random value within interval [a,b] */
float random_value(float a, float b);

} /* namespace jcl */

#endif /* JCL_MODULES_H */
