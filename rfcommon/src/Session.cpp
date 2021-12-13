#include "rfcommon/Session.hpp"
#include "rfcommon/PlayerState.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
Session::Session(
        MappingInfo&& mapping,
        StageID stageID,
        SmallVector<FighterID, 8>&& playerFighterIDs,
        SmallVector<SmallString<15>, 8>&& playerTags)
    : mappingInfo_(std::move(mapping))
    , stageID_(stageID)
    , playerFighterIDs_(std::move(playerFighterIDs))
    , playerTags_(std::move(playerTags))
    , playerStates_(playerFighterIDs_.count())
{
    assert(playerFighterIDs_.count() == playerTags_.count());
    assert(playerFighterIDs_.count() == playerStates_.count());
}

// ----------------------------------------------------------------------------
Session::~Session()
{
}

// ----------------------------------------------------------------------------
int Session::playerCount() const
{
    return static_cast<int>(playerTags_.count());
}

// ----------------------------------------------------------------------------
const SmallString<15>& Session::playerTag(int index) const
{
    return playerTags_[index];
}

// ----------------------------------------------------------------------------
FighterID Session::playerFighterID(int index) const
{
    return playerFighterIDs_[index];
}

// ----------------------------------------------------------------------------
int Session::playerStateCount(int playerIdx) const
{
    return static_cast<int>(playerStates_[playerIdx].count());
}

// ----------------------------------------------------------------------------
const PlayerState& Session::playerStateAt(int playerIdx, int stateIdx) const
{
    return playerStates_[playerIdx][stateIdx];
}

// ----------------------------------------------------------------------------
const PlayerState& Session::firstPlayerState(int playerIdx) const
{
    return playerStates_[playerIdx][0];
}

// ----------------------------------------------------------------------------
const PlayerState& Session::lastPlayerState(int playerIdx) const
{
    return playerStateAt(playerIdx, playerStateCount(playerIdx) - 1);
}

// ----------------------------------------------------------------------------
const PlayerState* Session::playerStatesBegin(int playerIdx) const
{
    return playerStates_[playerIdx].data();
}

// ----------------------------------------------------------------------------
const PlayerState* Session::playerStatesEnd(int playerIdx) const
{
    return playerStatesBegin(playerIdx) + playerStateCount(playerIdx);
}

// ----------------------------------------------------------------------------
int Session::findWinner() const
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
