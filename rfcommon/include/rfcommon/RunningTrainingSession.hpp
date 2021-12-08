#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/RunningSession.hpp"
#include "rfcommon/TrainingSession.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API RunningTrainingSession : public RunningSession, public TrainingSession
{
public:
    RunningTrainingSession(
            MappingInfo&& mapping,
            StageID stageID,
            SmallVector<FighterID, 8>&& playerFighterIDs,
            SmallVector<SmallString<15>, 8>&& playerTags
    );

    void resetTraining();

    const SmallString<15>& playerName(int playerIdx) const
        { return playerTag(playerIdx); }

    int winner() const override
        { return -1; }

    // Make MSVC shut up about dominance
    TimeStampMS timeStampStartedMs() const override
        { return TrainingSession::timeStampStartedMs(); }
};

}
