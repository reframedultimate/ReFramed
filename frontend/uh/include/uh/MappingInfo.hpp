#pragma once

#include "application/models/FighterStatusMapping.hpp"
#include "application/models/HitStatusMapping.hpp"
#include "application/models/FighterIDMapping.hpp"
#include "application/models/StageIDMapping.hpp"

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
