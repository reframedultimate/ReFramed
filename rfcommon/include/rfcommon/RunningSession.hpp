#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Session.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API RunningSession : virtual public Session
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
    virtual void addPlayerState(int playerIdx, PlayerState&& state);

protected:
    uint8_t frameUniqueBits_;
};

}
