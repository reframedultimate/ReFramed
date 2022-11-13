#include "rfcommon/TrainingMetaData.hpp"
#include "rfcommon/MetaDataListener.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
TrainingMetaData::TrainingMetaData(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SessionNumber sessionNumber,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<String, 2>&& tags)
    : MetaData(timeStarted, timeEnded, stageID, std::move(fighterIDs), std::move(tags))
    , sessionNumber_(sessionNumber)
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
