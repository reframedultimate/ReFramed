#pragma once

#include "uh/config.hpp"

namespace uh {

class TrainingModeContextListener
{
public:
    virtual void onTrainingModeContextNewPlayerState(int player, const PlayerState& state) = 0;
};

}

