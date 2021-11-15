#pragma once

#include "uh/config.hpp"
#include "uh/Session.hpp"

namespace uh {

class RunningSession : virtual public Session
{
protected:
    RunningSession();

public:
    virtual void addPlayerState(int PlayerIdx, PlayerState&& state);
};

}
