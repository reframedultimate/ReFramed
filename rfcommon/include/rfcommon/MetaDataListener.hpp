#pragma once

#include "rfcommon/GameNumber.hpp"
#include "rfcommon/SetNumber.hpp"
#include "rfcommon/TimeStamp.hpp"

namespace rfcommon {

class SetFormat;

class MetaDataListener
{
public:
    virtual void onMetaDataTimeStartedChanged(TimeStamp timeStarted) = 0;
    virtual void onMetaDataTimeEndedChanged(TimeStamp timeEnded) = 0;

    // Game related events
    virtual void onMetaDataPlayerNameChanged(int fighterIdx, const String& name) = 0;
    virtual void onMetaDataSetNumberChanged(SetNumber number) = 0;
    virtual void onMetaDataGameNumberChanged(GameNumber number) = 0;
    virtual void onMetaDataSetFormatChanged(const SetFormat& format) = 0;
    virtual void onMetaDataWinnerChanged(int winnerPlayerIdx) = 0;

    // In training mode this increments every time a new training room is loaded
    virtual void onMetaDataTrainingSessionNumberChanged(GameNumber number) = 0;
};

}
