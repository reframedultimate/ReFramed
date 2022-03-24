#include "rfcommon/SavedGameSession.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
SavedGameSession::SavedGameSession(
        MappingInfo&& mapping,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<SmallString<15>, 2>&& tags,
        SmallVector<SmallString<15>, 2>&& playerNames,
        GameNumber gameNumber,
        SetNumber setNumber,
        SetFormat setFormat)
    : Session(std::move(mapping), stageID, std::move(fighterIDs), std::move(tags))
    , SavedSession()
    , GameSession(std::move(playerNames), gameNumber, setNumber, setFormat)
{
}

}
