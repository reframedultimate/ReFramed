#include "rfcommon/GameSession.hpp"
#include "rfcommon/PlayerState.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
GameSession::GameSession(
        SmallVector<SmallString<15>, 8>&& playerNames,
        GameNumber gameNumber,
        SetNumber setNumber,
        SetFormat setFormat)
    : playerNames_(std::move(playerNames))
    , gameNumber_(gameNumber)
    , setNumber_(setNumber)
    , format_(setFormat)
{
}

// ----------------------------------------------------------------------------
const SmallString<15>& GameSession::playerName(int playerIdx) const
{
    return playerNames_[playerIdx];
}

// ----------------------------------------------------------------------------
GameNumber GameSession::gameNumber() const
{
    return gameNumber_;
}

// ----------------------------------------------------------------------------
SetNumber GameSession::setNumber() const
{
    return setNumber_;
}

// ----------------------------------------------------------------------------
SetFormat GameSession::format() const
{
    return format_;
}

// ----------------------------------------------------------------------------
TimeStampMS GameSession::timeStampStartedMs() const
{
    return playerStates_[0][0].timeStampMs();
}

}
