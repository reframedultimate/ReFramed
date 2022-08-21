#include "rfcommon/Vec2.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
Vec2 Vec2::fromValues(float x, float y)
{
    NOPROFILE();

    return Vec2(x, y);
}

// ----------------------------------------------------------------------------
Vec2 Vec2::makeZero()
{
    NOPROFILE();

    return Vec2(0, 0);
}

// ----------------------------------------------------------------------------
Vec2::Vec2(float x, float y)
    : x_(x)
    , y_(y)
{}

// ----------------------------------------------------------------------------
Vec2::~Vec2()
{}

}
