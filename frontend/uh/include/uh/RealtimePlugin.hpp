#pragma once

#include "uh/Plugin.hpp"

namespace uh {

class RunningGameSession;
class RunningTrainingSession;

class UH_PUBLIC_API RealtimePlugin : public Plugin
{
public:
    virtual void onGameSessionStarted(RunningGameSession* session) = 0;
    virtual void onGameSessionEnded(RunningGameSession* session) = 0;
    virtual void onTrainingSessionStarted(RunningTrainingSession* session) = 0;
    virtual void onTrainingSessionEnded(RunningTrainingSession* session) = 0;
};

}
