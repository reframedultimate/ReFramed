#include "rfcommon/Costume.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
Costume Costume::fromValue(Type value)
{
    NOPROFILE();

    return Costume(value);
}

// ----------------------------------------------------------------------------
Costume Costume::makeDefault()
{
    NOPROFILE();

    return Costume(0);
}

// ----------------------------------------------------------------------------
Costume::~Costume()
{}

// ----------------------------------------------------------------------------
Costume::Costume(Type value)
    : value_(value)
{}

}
