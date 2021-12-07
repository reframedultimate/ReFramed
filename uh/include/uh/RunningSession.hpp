#pragma once

#include "uh/config.hpp"
#include "uh/Session.hpp"

namespace uh {

class UH_PUBLIC_API RunningSession : virtual public Session
{
protected:
    RunningSession();
    RunningSession(
            MappingInfo&& mapping,
            StageID stageID,
            SmallVector<FighterID, 8>&& playerFighterIDs,
            SmallVector<SmallString<15>, 8>&& playerTags
    );

public:
    virtual void addPlayerState(int PlayerIdx, PlayerState&& state);
};

}
