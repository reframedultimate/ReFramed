#pragma once

#include "uh/config.hpp"
#include "uh/RunningSession.hpp"
#include "uh/GameSession.hpp"

namespace uh {

class UH_PUBLIC_API RunningGameSession : public RunningSession, public GameSession
{
public:
    RunningGameSession(
            MappingInfo&& mapping,
            StageID stageID,
            SmallVector<FighterID, 8>&& playerFighterIDs,
            SmallVector<SmallString<15>, 8>&& playerTags,
            SmallVector<SmallString<15>, 8>&& playerNames
    );

    bool save(const String& fileName);

    void setPlayerName(int index, const SmallString<15>& name);
    void setGameNumber(GameNumber number);
    void setSetNumber(SetNumber number);
    void setFormat(const SetFormat& format);

    void addPlayerState(int playerIdx, PlayerState&& state) override;
    int winner() const override
        { return currentWinner_; }

    // Make MSVC shut up about dominance
    const SmallString<15>& playerName(int playerIdx) const override
        { return GameSession::playerName(playerIdx); }
    TimeStampMS timeStampStartedMs() const override
        { return GameSession::timeStampStartedMs(); }

private:
    int currentWinner_ = 0;
};

}
