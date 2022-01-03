#include "rfcommon/HitstunCalculator.hpp"

namespace rfcommon {

int HitstunCalculator::hitstun(double knockback)
{
    return static_cast<int>(knockback * 0.4 - 1.0);
}

}
