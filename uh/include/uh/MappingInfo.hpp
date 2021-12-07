#pragma once

#include "uh/FighterStatusMapping.hpp"
#include "uh/HitStatusMapping.hpp"
#include "uh/FighterIDMapping.hpp"
#include "uh/StageIDMapping.hpp"

namespace uh {

class MappingInfo
{
public:
    FighterStatusMapping fighterStatus;
    HitStatusMapping hitStatus;
    FighterIDMapping fighterID;
    StageIDMapping stageID;
};

}
