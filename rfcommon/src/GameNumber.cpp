#include "rfcommon/GameNumber.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
GameNumber GameNumber::fromValue(Type value)
{
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
