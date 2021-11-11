#include "uh/GameSession.hpp"
#include "uh/PlayerState.hpp"
#include "uh/RunningGameSessionListener.hpp"
#include "uh/time.h"
#include "uh/StreamBuffer.hpp"
#include "nlohmann/json.hpp"
#include "cpp-base64/base64.h"
#include "zlib.h"
#include <cassert>
#include <unordered_set>

namespace uh {

// ----------------------------------------------------------------------------
GameSession::GameSession(MappingInfo&& mapping,
                         SmallVector<FighterID, 8>&& playerFighterIDs,
                         SmallVector<SmallString<15>, 8>&& playerTags,
                         SmallVector<SmallString<15>, 8>&& playerNames,
                         StageID stageID)
    : Session(std::move(mapping), std::move(playerFighterIDs), std::move(playerTags), stageID)
    , playerNames_(std::move(playerNames))
{
    assert(playerTags_.count() == playerNames_.count());
    assert(playerTags_.count() == playerFighterIDs_.count());
    assert(playerTags_.count() == playerStates_.count());
}

// ----------------------------------------------------------------------------
uint64_t GameSession::timeStampStartedMs() const
{
    return playerStates_[0][0].timeStampMs();
}

// ----------------------------------------------------------------------------
int GameSession::findWinner() const
{
    // The winner is the player with most stocks and least damage
    int winneridx = 0;
    for (int i = 0; i != playerCount(); ++i)
    {
        if (playerStates_[i].count() == 0 || playerStates_[winneridx].count() == 0)
            continue;

        const auto& current = playerStates_[i][playerStates_[i].count() - 1];
        const auto& winner = playerStates_[winneridx][playerStates_[winneridx].count() - 1];

        if (current.stocks() > winner.stocks())
            winneridx = i;
        else if (current.stocks() == winner.stocks())
            if (current.damage() < winner.damage())
                winneridx = i;
    }

    return winneridx;
}

}
