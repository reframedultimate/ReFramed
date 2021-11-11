#pragma once

#include "uh/config.hpp"

namespace uh {

class PlayerState;

class SessionListener
{
public:
    virtual void onSessionNewPlayerState(int playerIdx, const PlayerState& state) = 0;
    virtual void onSessionNewUniquePlayerState(int playerIdx, const PlayerState& state) = 0;
};

}

