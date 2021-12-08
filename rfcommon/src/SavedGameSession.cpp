#include "rfcommon/SavedGameSession.hpp"
#include "rfcommon/PlayerState.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
SavedGameSession::SavedGameSession(
        MappingInfo&& mapping,
        uint16_t stageID,
        SmallVector<FighterID, 8>&& playerFighterIDs,
        SmallVector<SmallString<15>, 8>&& playerTags,
        SmallVector<SmallString<15>, 8>&& playerNames,
        GameNumber gameNumber,
        SetNumber setNumber,
        SetFormat setFormat)
    : Session(std::move(mapping), stageID, std::move(playerFighterIDs), std::move(playerTags))
    , SavedSession()
    , GameSession(std::move(playerNames), gameNumber, setNumber, setFormat)
{
}

}
