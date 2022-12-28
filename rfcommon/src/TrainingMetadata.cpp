#include "rfcommon/TrainingMetadata.hpp"
#include "rfcommon/MetadataListener.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
TrainingMetadata::TrainingMetadata(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<Costume, 2>&& costumes,
        SmallVector<String, 2>&& tags)
    : Metadata(timeStarted, timeEnded, stageID, std::move(fighterIDs), std::move(costumes), std::move(tags))
    , sessionNumber_(SessionNumber::fromValue(1))
{}

// ----------------------------------------------------------------------------
TrainingMetadata::~TrainingMetadata()
{}

// ----------------------------------------------------------------------------
void TrainingMetadata::setSessionNumber(SessionNumber sessionNumber)
{
    PROFILE(TrainingMetadata, setSessionNumber);

    bool notify = (sessionNumber_ != sessionNumber);
    sessionNumber_ = sessionNumber;
    if (notify)
        dispatcher.dispatch(&MetadataListener::onMetadataTrainingSessionNumberChanged, sessionNumber);
}

}
