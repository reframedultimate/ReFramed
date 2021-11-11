#pragma once

#include "uh/config.hpp"
#include "uh/GameSession.hpp"
#include "uh/ListenerDispatcher.hpp"

namespace uh {

class RunningGameSessionListener;

class UH_PUBLIC_API RunningGameSession : public GameSession
{
public:
    RunningGameSession(MappingInfo&& mapping,
                       SmallVector<FighterID, 8>&& playerFighterIDs,
                       SmallVector<SmallString<15>, 8>&& playerTags,
                       SmallVector<SmallString<15>, 8>&& playerNames,
                       StageID stageID);

    bool save(const String& fileName);

    void setPlayerName(int index, const SmallString<15>& name);
    void setGameNumber(GameNumber number);
    void setSetNumber(SetNumber number);
    void setFormat(const SetFormat& format);
    void addPlayerState(int index, PlayerState&& state);

    ListenerDispatcher<RunningGameSessionListener> dispatcher;

private:
    int currentWinner_ = 0;
};

}
