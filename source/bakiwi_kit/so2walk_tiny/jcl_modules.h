#ifndef JCL_MODULES_H
#define JCL_MODULES_H

#include <stdlib.h>

namespace jcl {

/* clamps values to Interval [lo, hi] */
template<class T>
constexpr const T& clamp( const T& v, const T& lo, const T& hi )
{
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

/* generates a random value within interval [a,b] */
float random_value(float a, float b);


template <unsigned N>
unsigned random_index(void)
{
  static_assert(N > 0);
  return rand() % N;
}


} /* namespace jcl */

#endif /* JCL_MODULES_H */
