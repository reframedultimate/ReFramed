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
            SmallVector<FighterID, 2>&& fighterIDs,
            SmallVector<SmallString<15>, 2>&& tags
    );

    const SmallString<15>& name(int fighterIdx) const override
        { return tag(fighterIdx); }

    int winner() const override
        { return -1; }
};

}
