#pragma once

#include "rfcommon/GameNumber.hpp"
#include "rfcommon/SetNumber.hpp"
#include "rfcommon/TimeStamp.hpp"
#include "rfcommon/String.hpp"

namespace rfcommon {

class SetFormat;

class MetaDataListener
{
public:
    virtual void onMetaDataTimeStartedChanged(TimeStamp timeStarted) = 0;
    virtual void onMetaDataTimeEndedChanged(TimeStamp timeEnded) = 0;

    // Game related events
    virtual void onMetaDataPlayerNameChanged(int fighterIdx, const char* name) = 0;
    virtual void onMetaDataSponsorChanged(int fighterIdx, const char* sponsor) = 0;
    virtual void onMetaDataTournamentNameChanged(const char* name) = 0;
    virtual void onMetaDataEventNameChanged(const char* name) = 0;
    virtual void onMetaDataRoundNameChanged(const char* name) = 0;
    virtual void onMetaDataCommentatorsChanged(const SmallVector<String, 2>& names) = 0;
    virtual void onMetaDataSetNumberChanged(SetNumber number) = 0;
    virtual void onMetaDataGameNumberChanged(GameNumber number) = 0;
    virtual void onMetaDataSetFormatChanged(const SetFormat& format) = 0;
    virtual void onMetaDataWinnerChanged(int winnerPlayerIdx) = 0;

    // In training mode this increments every time a new training room is loaded
    virtual void onMetaDataTrainingSessionNumberChanged(GameNumber number) = 0;
};

}
