#pragma once

#include "uh/Plugin.hpp"

namespace uh {

class GameSession;
class TrainingSession;

class TrainingModePlugin : public Plugin
{
public:
    virtual void onGameSessionStarted(GameSession* session) = 0;
    virtual void onGameSessionEnded(GameSession* session) = 0;
    virtual void onTrainingSessionStarted(TrainingSession* session) = 0;
    virtual void onTrainingSessionEnded(TrainingSession* session) = 0;
};

}
