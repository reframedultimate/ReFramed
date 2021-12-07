#pragma once

#include "uh/config.hpp"

namespace uh {

class KnockbackCalculator;

class UH_PUBLIC_API HitstunCalculator
{
public:
    int hitstun(double knockback)
        { return static_cast<int>(knockback * 0.4 - 1.0); }
};

}
