#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/Metadata.hpp"
#include "rfcommon/SessionNumber.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API TrainingMetadata : public Metadata
{
    TrainingMetadata(
            TimeStamp timeStarted,
            TimeStamp timeEnded,
            StageID stageID,
            SmallVector<FighterID, 2>&& fighterIDs,
            SmallVector<Costume, 2>&& costumes,
            SmallVector<String, 2>&& tags);

public:
    ~TrainingMetadata();

    Type type() const override final { return TRAINING; }

    FighterID humanFighterID() const { return FighterID::fromValue(0); }
    FighterID cpuFighterID() const { return FighterID::fromValue(1); }

    SessionNumber sessionNumber() const { return sessionNumber_; }
    void setSessionNumber(SessionNumber sessionNumber);

private:
    friend class Metadata;

    SessionNumber sessionNumber_;
};

}
