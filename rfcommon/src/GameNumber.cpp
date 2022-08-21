#include "rfcommon/GameNumber.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
GameNumber GameNumber::fromValue(Type value)
{
    NOPROFILE();

    return GameNumber(value);
}

// ----------------------------------------------------------------------------
GameNumber::~GameNumber()
{}

// ----------------------------------------------------------------------------
GameNumber::GameNumber(Type value)
    : value_(value)
{}

}
