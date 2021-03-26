#include "uh/models/Recording.hpp"
#include "uh/models/PlayerState.hpp"

namespace uh {

// ----------------------------------------------------------------------------
Recording::Recording(const MappingInfo& mapping)
    : mappingInfo_(mapping)
{
}

// ----------------------------------------------------------------------------
void Recording::setGameInfo(const GameInfo& gameInfo)
{
    gameInfo_ = gameInfo;
}

// ----------------------------------------------------------------------------
void Recording::addPlayer(const PlayerInfo& playerInfo)
{
    playerInfo_.push_back(playerInfo);
    playerStates_.push_back(QVector<PlayerState>());
}

// ----------------------------------------------------------------------------
void Recording::addPlayerState(int index, const PlayerState& state)
{
    playerStates_[index].push_back(state);
}

}
