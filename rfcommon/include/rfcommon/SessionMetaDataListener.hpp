#pragma once

#include "rfcommon/GameNumber.hpp"
#include "rfcommon/SetNumber.hpp"

namespace rfcommon {

class Frame;
class SetFormat;

class SessionMetaDataListener
{
public:
    virtual void onSessionMetaDataPlayerNameChanged(int fighterIdx, const SmallString<15>& name) = 0;
    virtual void onSessionMetaDataSetNumberChanged(SetNumber number) = 0;
    virtual void onSessionMetaDataGameNumberChanged(GameNumber number) = 0;
    virtual void onSessionMetaDataSetFormatChanged(const SetFormat& format) = 0;
    virtual void onSessionMetaDataWinnerChanged(int winnerPlayerIdx) = 0;

    // In training mode this increments every time a new training room is loaded
    virtual void onSessionMetaDataSessionNumberChanged(GameNumber number) = 0;
};

}
