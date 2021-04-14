#pragma once

#include "uh/models/FighterStatusMapping.hpp"
#include "uh/models/HitStatusMapping.hpp"
#include "uh/models/FighterIDMapping.hpp"
#include "uh/models/StageIDMapping.hpp"

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
