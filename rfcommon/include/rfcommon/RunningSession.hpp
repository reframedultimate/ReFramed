#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Session.hpp"

namespace rfcommon {

class Frame;

class RFCOMMON_PUBLIC_API RunningSession : virtual public Session
{
protected:
    RunningSession();
    RunningSession(
            MappingInfo&& mapping,
            StageID stageID,
            SmallVector<FighterID, 2>&& fighterIDs,
            SmallVector<SmallString<15>, 2>&& tags
    );

public:
    virtual void addFrame(Frame&& frame);
};

}
