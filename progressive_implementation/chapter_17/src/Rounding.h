#ifndef ROUNDING_H
#define ROUNDING_H

#include <string>
#include <mutex>

namespace Rounding
{
    // Rounds x up to the nearest multiple of n.
    inline int roundAwayFromZero(int n, int x)
    {
        if (x % n == 0) // x is already a multiple of n
            return x;

        if (x < 0)
            return x - n - (x % n);

        return x + n - (x % n);
    }
};

#endif