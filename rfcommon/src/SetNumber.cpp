#include "rfcommon/SetNumber.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
SetNumber SetNumber::fromValue(Type value)
{
    NOPROFILE();

    return SetNumber(value);
}

// ----------------------------------------------------------------------------
SetNumber::~SetNumber()
{}

// ----------------------------------------------------------------------------
SetNumber::SetNumber(Type value)
    : value_(value)
{}

}
