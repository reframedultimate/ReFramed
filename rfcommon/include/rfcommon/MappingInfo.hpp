#pragma once

#include "rfcommon/FighterStatusMapping.hpp"
#include "rfcommon/HitStatusMapping.hpp"
#include "rfcommon/FighterIDMapping.hpp"
#include "rfcommon/StageIDMapping.hpp"

namespace rfcommon {

class MappingInfo
{
public:
    FighterStatusMapping fighterStatus;
    HitStatusMapping hitStatus;
    FighterIDMapping fighterID;
    StageIDMapping stageID;
};

}
