#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/RunningSession.hpp"
#include "rfcommon/GameSession.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API RunningGameSession : public RunningSession, public GameSession
{
public:
    RunningGameSession(
            MappingInfo&& mapping,
            StageID stageID,
            SmallVector<FighterID, 2>&& fighterIDs,
            SmallVector<SmallString<15>, 2>&& tags,
            SmallVector<SmallString<15>, 2>&& playerNames
    );

    void setPlayerName(int fighterIdx, const SmallString<15>& name);
    void setGameNumber(GameNumber number);
    void setSetNumber(SetNumber number);
    void setFormat(const SetFormat& format);

    void addFrame(SmallVector<FighterFrame, 2>&& frame) override;

    int winner() const override
        { assert(currentWinner_ != -1); return currentWinner_; }

private:
    int currentWinner_ = -1;
};

}
