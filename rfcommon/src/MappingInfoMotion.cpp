#include "rfcommon/MappingInfoMotion.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
MappingInfoMotion::MappingInfoMotion()
{}

// ----------------------------------------------------------------------------
MappingInfoMotion::~MappingInfoMotion()
{}

// ----------------------------------------------------------------------------
String MappingInfoMotion::toLabel(FighterMotion motion)
{
    return "";
}

// ----------------------------------------------------------------------------
const char* MappingInfoMotion::toLabel(FighterMotion motion, const char* fallback)
{
    return "";
}

// ----------------------------------------------------------------------------
String MappingInfoMotion::toUserLabel(FighterMotion motion)
{
    return "";
}

// ----------------------------------------------------------------------------
const char* MappingInfoMotion::toUserLabel(FighterMotion motion, const char* fallback)
{
    return "";
}

// ----------------------------------------------------------------------------
FighterMotion MappingInfoMotion::fromLabel(const char* label)
{
    return FighterMotion::fromValue(0);
}

// ----------------------------------------------------------------------------
SmallVector<FighterMotion, 4> MappingInfoMotion::fromUserLabel(const char* userLabel, FighterID fighterID)
{
    return {};
}

}
