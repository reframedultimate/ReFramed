#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/SessionNumber.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API TrainingMetaData : public MetaData
{
    TrainingMetaData(
            TimeStamp timeStarted,
            TimeStamp timeEnded,
            StageID stageID,
            SessionNumber sessionNumber,
            SmallVector<FighterID, 2>&& fighterIDs,
            SmallVector<String, 2>&& tags);

public:
    ~TrainingMetaData();

    Type type() const override final { return TRAINING; }

    FighterID humanFighterID() const { return FighterID::fromValue(0); }
    FighterID cpuFighterID() const { return FighterID::fromValue(1); }

    SessionNumber sessionNumber() const { return sessionNumber_; }
    void setSessionNumber(SessionNumber sessionNumber);

private:
    friend class MetaData;

    SessionNumber sessionNumber_;
};

}
