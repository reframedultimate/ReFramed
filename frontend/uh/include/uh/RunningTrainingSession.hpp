#pragma once

#include "uh/config.hpp"
#include "uh/RunningSession.hpp"
#include "uh/TrainingSession.hpp"

namespace uh {

class UH_PUBLIC_API RunningTrainingSession : public RunningSession, public TrainingSession
{
public:
    RunningTrainingSession(
            MappingInfo&& mapping,
            StageID stageID,
            SmallVector<FighterID, 8>&& playerFighterIDs,
            SmallVector<SmallString<15>, 8>&& playerTags
    );

    const SmallString<15>& playerName(int playerIdx) const
        { return playerTag(playerIdx); }

    int winner() const override
        { return -1; }
};

}
