#pragma once

#include "rfcommon/String.hpp"
#include "rfcommon/DeltaTime.hpp"
#include "rfcommon/GameNumber.hpp"
#include "rfcommon/SetNumber.hpp"
#include "rfcommon/TimeStamp.hpp"

namespace rfcommon {
    class Session;
    class SetFormat;
}

namespace rfapp {

class ActiveSessionManagerListener
{
public:
    virtual void onActiveSessionManagerConnected(const char* ip, uint16_t port) = 0;
    virtual void onActiveSessionManagerDisconnected() = 0;

    virtual void onActiveSessionManagerGameStarted(rfcommon::Session* game) = 0;
    virtual void onActiveSessionManagerGameEnded(rfcommon::Session* game) = 0;
    virtual void onActiveSessionManagerTrainingStarted(rfcommon::Session* training) = 0;
    virtual void onActiveSessionManagerTrainingEnded(rfcommon::Session* training) = 0;

    virtual void onActiveSessionManagerTimeRemainingChanged(double seconds) = 0;
    virtual void onActiveSessionManagerFighterStateChanged(int fighterIdx, float damage, int stocks) = 0;

    // We re-propagate all Session meta-data events because
    // ActiveGameSessionManager allows you to change these properties even when
    // there is no active session
    virtual void onActiveSessionManagerTimeStartedChanged(rfcommon::TimeStamp timeStarted) = 0;
    virtual void onActiveSessionManagerTimeEndedChanged(rfcommon::TimeStamp timeEnded) = 0;

    virtual void onActiveSessionManagerPlayerNameChanged(int fighterIdx, const char* name) = 0;
    virtual void onActiveSessionManagerSetNumberChanged(rfcommon::SetNumber number) = 0;
    virtual void onActiveSessionManagerGameNumberChanged(rfcommon::GameNumber number) = 0;
    virtual void onActiveSessionManagerSetFormatChanged(const rfcommon::SetFormat& format) = 0;
    virtual void onActiveSessionManagerWinnerChanged(int winnerPlayerIdx) = 0;

    virtual void onActiveSessionManagerTrainingSessionNumberChanged(rfcommon::GameNumber number) = 0;
};

}
