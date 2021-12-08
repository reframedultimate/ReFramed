#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/Types.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API TrainingSession : virtual public Session
{
protected:
    TrainingSession();

public:
    FighterID playerFighterID() const;
    FighterID cpuFighterID() const;

    TimeStampMS timeStampStartedMs() const override
        { return timeStarted_; }

private:
    const TimeStampMS timeStarted_;
};

}
