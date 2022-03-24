#include "rfcommon/RunningGameSession.hpp"
#include "rfcommon/SessionListener.hpp"
#include "rfcommon/StreamBuffer.hpp"
#include "nlohmann/json.hpp"
#include "cpp-base64/base64.h"
#include "zlib.h"
#include <cassert>

using nlohmann::json;

namespace rfcommon {

// ----------------------------------------------------------------------------
RunningGameSession::RunningGameSession(
        MappingInfo&& mapping,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<SmallString<15>, 2>&& tags,
        SmallVector<SmallString<15>, 2>&& playerNames)
    : Session(std::move(mapping), stageID, std::move(fighterIDs), std::move(tags))
    , RunningSession()
    , GameSession(std::move(playerNames))
{
}

// ----------------------------------------------------------------------------
void RunningGameSession::setPlayerName(int index, const SmallString<15>& name)
{
    assert(name.length() > 0);
    playerNames_[index] = name;
    dispatcher.dispatch(&SessionListener::onRunningGameSessionPlayerNameChanged, index, name);
}

// ----------------------------------------------------------------------------
void RunningGameSession::setGameNumber(GameNumber number)
{
    gameNumber_ = number;
    dispatcher.dispatch(&SessionListener::onRunningGameSessionGameNumberChanged, number);
}

// ----------------------------------------------------------------------------
void RunningGameSession::setSetNumber(SetNumber number)
{
    setNumber_ = number;
    dispatcher.dispatch(&SessionListener::onRunningGameSessionSetNumberChanged, number);
}

// ----------------------------------------------------------------------------
void RunningGameSession::setFormat(const SetFormat& format)
{
    format_ = format;
    dispatcher.dispatch(&SessionListener::onRunningGameSessionFormatChanged, format);
}

// ----------------------------------------------------------------------------
void RunningGameSession::addFrame(Frame&& frame)
{
    RunningSession::addFrame(std::move(frame));

    // Winner might have changed
    int winner = findWinner();
    if (currentWinner_ != winner)
    {
        currentWinner_ = winner;
        dispatcher.dispatch(&SessionListener::onRunningGameSessionWinnerChanged, currentWinner_);
    }
}

}
