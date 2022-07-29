#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/FighterStatus.hpp"
#include "rfcommon/Hash40Strings.hpp"
#include "rfcommon/HashMap.hpp"
#include "rfcommon/String.hpp"

namespace rfcommon {

class Hash40Strings;

class RFCOMMON_PUBLIC_API MappingInfoMotion
{
public:
    MappingInfoMotion();
    ~MappingInfoMotion();

    String toLabel(FighterMotion);
    const char* toLabel(FighterMotion motion, const char* fallback);
    String toUserLabel(FighterMotion motion);
    const char* toUserLabel(FighterMotion motion, const char* fallback);
    FighterMotion fromLabel(const char* label);
    SmallVector<FighterMotion, 4> fromUserLabel(const char* userLabel, FighterID fighterID);

private:
    Hash40Strings motionLabels_;
};

}
