#include "rfcommon/TrainingMetaData.hpp"
#include "rfcommon/MetaDataListener.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
TrainingMetaData::TrainingMetaData(
    TimeStamp timeStarted,
    TimeStamp timeEnded,
    StageID stageID,
    SmallVector<FighterID, 2>&& fighterIDs,
    SmallVector<String, 2>&& tags,
    GameNumber sessionNumber)
    : MetaData(timeStarted, timeEnded, stageID, std::move(fighterIDs), std::move(tags))
    , sessionNumber_(sessionNumber)
{}

// ----------------------------------------------------------------------------
MetaData::Type TrainingMetaData::type() const
{
    NOPROFILE();

    return TRAINING;
}

// ----------------------------------------------------------------------------
const String& TrainingMetaData::name(int playerIdx) const
{
    NOPROFILE();

    return tag(playerIdx);
}

// ----------------------------------------------------------------------------
const String& TrainingMetaData::sponsor(int playerIdx) const
{
    NOPROFILE();

    return "";
}

// ----------------------------------------------------------------------------
FighterID TrainingMetaData::playerFighterID() const
{
    NOPROFILE();

    return fighterID(0);
}

// ----------------------------------------------------------------------------
FighterID TrainingMetaData::cpuFighterID() const
{
    NOPROFILE();

    return fighterID(1);
}

// ----------------------------------------------------------------------------
GameNumber TrainingMetaData::sessionNumber() const
{
    NOPROFILE();

    return sessionNumber_;
}

// ----------------------------------------------------------------------------
void TrainingMetaData::setSessionNumber(GameNumber sessionNumber)
{
    PROFILE(TrainingMetaData, setSessionNumber);

    bool notify = (sessionNumber_ != sessionNumber);
    sessionNumber_ = sessionNumber;
    if (notify)
        dispatcher.dispatch(&MetaDataListener::onMetaDataTrainingSessionNumberChanged, sessionNumber);
}

// ----------------------------------------------------------------------------
void TrainingMetaData::resetSessionNumber()
{
    PROFILE(TrainingMetaData, resetSessionNumber);

    setSessionNumber(GameNumber::fromValue(1));
}

}
