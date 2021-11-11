#include "uh/Session.hpp"

namespace uh {

// ----------------------------------------------------------------------------
Session::Session(MappingInfo&& mapping,
                 SmallVector<FighterID, 8>&& playerFighterIDs,
                 SmallVector<SmallString<15>, 8>&& playerTags,
                 StageID stageID)
    : mappingInfo_(std::move(mapping))
    , playerFighterIDs_(std::move(playerFighterIDs))
    , playerTags_(std::move(playerTags))
    , stageID_(stageID)
{
    assert(playerTags_.count() == playerFighterIDs_.count());
}

}
