#pragma once

#include "rfcommon/String.hpp"
#include "rfcommon/GameNumber.hpp"
#include "rfcommon/SetNumber.hpp"

namespace rfcommon {
    class SetFormat;
}

namespace rfapp {

class ActiveSessionManagerListener
{
public:
    // We re-propagate all Session frame and meta-data events because
    // ActiveGameSessionManager allows you to change these properties even when
    // there is no active session
    virtual void onActiveSessionManagerPlayerNameChanged(int player, const rfcommon::SmallString<15>& name) = 0;
    virtual void onActiveSessionManagerSetNumberChanged(rfcommon::SetNumber number) = 0;
    virtual void onActiveSessionManagerGameNumberChanged(rfcommon::GameNumber number) = 0;
    virtual void onActiveSessionManagerFormatChanged(const rfcommon::SetFormat& format) = 0;
    virtual void onActiveSessionManagerWinnerChanged(int winner) = 0;
    virtual void onActiveSessionManagerTrainingSessionNumberChanged(rfcommon::GameNumber number) = 0;
};

}
