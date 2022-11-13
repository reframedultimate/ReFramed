#include "rfcommon/SessionNumber.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
SessionNumber SessionNumber::fromValue(Type value)
{
    NOPROFILE();

    return SessionNumber(value);
}

// ----------------------------------------------------------------------------
SessionNumber::~SessionNumber()
{}

// ----------------------------------------------------------------------------
SessionNumber::SessionNumber(Type value)
    : value_(value)
{}

}
