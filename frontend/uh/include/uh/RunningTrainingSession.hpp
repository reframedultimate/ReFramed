#pragma once

#include "uh/config.hpp"
#include "uh/RunningSession.hpp"
#include "uh/TrainingSession.hpp"

namespace uh {

class RunningTrainingSession : public RunningSession, public TrainingSession
{
public:
    RunningTrainingSession(
            MappingInfo&& mapping,
            StageID stageID,
            SmallVector<FighterID, 8>&& playerFighterIDs,
            SmallVector<SmallString<15>, 8>&& playerTags
    );
};

}
