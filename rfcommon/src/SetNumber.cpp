#include "rfcommon/SetNumber.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
SetNumber SetNumber::fromValue(Type value)
{
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
