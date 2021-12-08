#pragma once

#include "rfcommon/config.hpp"

namespace rfcommon {

class KnockbackCalculator;

class RFCOMMON_PUBLIC_API HitstunCalculator
{
public:
    int hitstun(double knockback)
        { return static_cast<int>(knockback * 0.4 - 1.0); }
};

}
