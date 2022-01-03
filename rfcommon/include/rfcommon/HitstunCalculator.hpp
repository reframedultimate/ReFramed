#pragma once

#include "rfcommon/config.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API HitstunCalculator
{
public:
    int hitstun(double knockback);
};

}
