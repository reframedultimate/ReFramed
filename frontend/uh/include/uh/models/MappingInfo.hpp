#pragma once

#include "uh/models/FighterStatusMapping.hpp"
#include "uh/models/FighterIDMapping.hpp"
#include "uh/models/StageIDMapping.hpp"

namespace uh {

class MappingInfo
{
public:
    FighterStatusMapping fighterStatus;
    FighterIDMapping fighterID;
    StageIDMapping stageID;
};

}
