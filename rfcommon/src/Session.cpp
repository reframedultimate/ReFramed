#include "rfcommon/Session.hpp"
#include "rfcommon/SessionListener.hpp"
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
void Session::replayUniqueStateEvents(SessionListener* listener)
{
    SmallVector<int, 8> stateIdx({0, 0, 0, 0, 0, 0, 0, 0});

    Frame currentFrame = 1;
    bool atEnd;
    do
    {
        for (int p = 0; p != playerCount(); ++p)
            if (stateIdx[p] < playerStateCount(p))
                if (playerStateAt(p, stateIdx[p]).frame() <= currentFrame)
                {
                    listener->onRunningSessionNewUniquePlayerState(p, playerStateAt(p, stateIdx[p]));
                    ++stateIdx[p];
                }

        atEnd = true;
        for (int p = 0; p != playerCount(); ++p)
            if (stateIdx[p] < playerStateCount(p))
                atEnd = false;

        currentFrame++;
    } while (!atEnd);
}

// ----------------------------------------------------------------------------
void Session::replayUniqueFrameEvents(SessionListener* listener)
{
    SmallVector<int, 8> stateIdx({0, 0, 0, 0, 0, 0, 0, 0});

    // All players must have at least one state
    for (int p = 0; p != playerCount(); ++p)
        if (playerStateCount(p) == 0)
            return;

    Frame currentFrame = 1;
    bool atEnd;
    do
    {
        rfcommon::SmallVector<PlayerState, 8> states;
        for (int p = 0; p != playerCount(); ++p)
        {
            states.push(playerStateAt(p, stateIdx[p]));
            if (stateIdx[p] < playerStateCount(p) - 1)
            {
                if (playerStateAt(p, stateIdx[p]).frame() <= currentFrame)
                    ++stateIdx[p];
            }
        }

        listener->onRunningSessionNewUniqueFrame(states);

        atEnd = true;
        for (int p = 0; p != playerCount(); ++p)
            if (stateIdx[p] < playerStateCount(p) - 1)
                atEnd = false;

        currentFrame++;
    } while (!atEnd);
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
