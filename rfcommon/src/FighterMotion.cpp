#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
FighterMotion FighterMotion::fromHexString(const char* hex)
{
    uint64_t value = 0;

    if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X'))
        hex += 2;

    for (; *hex; ++hex)
    {
        value <<= 4;
        if (*hex >= '0' && *hex <= '9')
            value |= *hex - '0';
        else if (*hex >= 'A' && *hex <= 'F')
            value |= *hex - 'A' + 10;
        else if (*hex >= 'a' && *hex <= 'f')
            value |= *hex - 'a' + 10;
        else
            return makeInvalid();
    }

    // limit to 40 bit
    if (value & 0xFFFFFF0000000000)
        return makeInvalid();

    return FighterMotion(value);
}

// ----------------------------------------------------------------------------
FighterMotion FighterMotion::fromValue(Type value)
{
    NOPROFILE();

    return FighterMotion(value);
}

// ----------------------------------------------------------------------------
FighterMotion FighterMotion::fromParts(uint8_t upper, uint32_t lower)
{
    NOPROFILE();

    return FighterMotion((static_cast<Type>(upper) << 32) | lower);
}

// ----------------------------------------------------------------------------
FighterMotion FighterMotion::makeInvalid()
{
    NOPROFILE();

    return FighterMotion(0);
}

// ----------------------------------------------------------------------------
FighterMotion::~FighterMotion()
{}

// ----------------------------------------------------------------------------
FighterMotion::FighterMotion(Type value)
    : value_(value)
{}

}
