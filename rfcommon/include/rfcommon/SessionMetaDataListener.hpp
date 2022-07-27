#pragma once

#include "rfcommon/Types.hpp"
#include "rfcommon/SetFormat.hpp"

namespace rfcommon {

class Frame;

class SessionMetaDataListener
{
public:
    virtual void onSessionMetaDataPlayerNameChanged(int fighterIdx, const SmallString<15>& name) = 0;
    virtual void onSessionMetaDataSetNumberChanged(SetNumber number) = 0;
    virtual void onSessionMetaDataGameNumberChanged(GameNumber number) = 0;
    virtual void onSessionMetaDataFormatChanged(const SetFormat& format) = 0;
    virtual void onSessionMetaDataWinnerChanged(int winnerPlayerIdx) = 0;
};

}
