#include "rfcommon/GameSession.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
GameSession::GameSession(
        SmallVector<SmallString<15>, 2>&& playerNames,
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
const SmallString<15>& GameSession::name(int playerIdx) const
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

}
