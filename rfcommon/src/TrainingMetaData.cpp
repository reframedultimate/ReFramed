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
        SmallVector<Costume, 2>&& costumes,
        SmallVector<String, 2>&& tags)
    : MetaData(timeStarted, timeEnded, stageID, std::move(fighterIDs), std::move(costumes), std::move(tags))
    , sessionNumber_(SessionNumber::fromValue(1))
{}

// ----------------------------------------------------------------------------
TrainingMetaData::~TrainingMetaData()
{}

// ----------------------------------------------------------------------------
void TrainingMetaData::setSessionNumber(SessionNumber sessionNumber)
{
    PROFILE(TrainingMetaData, setSessionNumber);

    bool notify = (sessionNumber_ != sessionNumber);
    sessionNumber_ = sessionNumber;
    if (notify)
        dispatcher.dispatch(&MetaDataListener::onMetaDataTrainingSessionNumberChanged, sessionNumber);
}

}
