#include "modules.h"

/* clips values to Interval [l_limit, u_limit] */
float clip(float x, float l_limit, float u_limit)
{
    if (x > u_limit) return u_limit;
    else if (x < l_limit) return l_limit;
    else return x;
}

int clip(int x, int l_limit, int u_limit)
{
    if (x > u_limit) return u_limit;
    else if (x < l_limit) return l_limit;
    else return x;
}

