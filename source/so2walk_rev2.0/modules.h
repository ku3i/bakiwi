#ifndef BAKIWI_MODULES_H
#define BAKIWI_MODULES_H

#include <math.h>

/* clips values to Interval [l_limit, u_limit] */
float clip(float x, float l_limit, float u_limit);
//int clip(int x, int l_limit, int u_limit);
/*
template <typename T>
T clip(T value, T min, T max)
{
    if (value > max) return value = max;
    else if (value < min) return value = min;
    else return value;
}

template<class T>
constexpr const T& clip( const T& v, const T& lo, const T& hi )
{
   // assert( !(hi < lo) );
    return (v < lo) ? lo : (hi < v) ? hi : v;
}
*/
#endif
