#pragma once

#include "uh/config.hpp"
#include "uh/Session.hpp"
#include "uh/Types.hpp"

namespace uh {

class TrainingSession : virtual public Session
{
protected:
    TrainingSession();

public:
    FighterID playerFighterID() const;
    FighterID cpuFighterID() const;

private:
    const TimeStampMS timeStarted_;
};

}
