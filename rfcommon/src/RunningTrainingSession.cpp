#include "rfcommon/RunningTrainingSession.hpp"
#include "rfcommon/SessionListener.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
RunningTrainingSession::RunningTrainingSession(
        MappingInfo&& mapping,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<SmallString<15>, 2>&& tags)
    : Session(std::move(mapping), stageID, std::move(fighterIDs), std::move(tags))
{
}

}
