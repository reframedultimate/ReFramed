#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/GameNumber.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API TrainingMetaData : public MetaData
{
    TrainingMetaData(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<String, 2>&& tags,
        GameNumber sessionNumber);

public:
    Type type() const override;

    const String& name(int playerIdx) const override;
    const String& sponsor(int playerIdx) const override;

    FighterID playerFighterID() const;
    FighterID cpuFighterID() const;

    GameNumber sessionNumber() const;
    void setSessionNumber(GameNumber sessionNumber);
    void resetSessionNumber();

private:
    friend class MetaData;

    GameNumber sessionNumber_;
};

}
