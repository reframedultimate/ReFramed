#include "rfcommon/HitstunCalculator.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

int HitstunCalculator::hitstun(double knockback)
{
    NOPROFILE();

    return static_cast<int>(knockback * 0.4 - 1.0);
}

}
