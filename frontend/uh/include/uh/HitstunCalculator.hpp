#pragma once

namespace uh {

class KnockbackCalculator;

class HitstunCalculator
{
public:
    int hitstun(double knockback)
        { return static_cast<int>(knockback * 0.4 - 1.0); }
};

}
