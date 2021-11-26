#pragma once

#include "uh/config.hpp"
#include "uh/GameSession.hpp"
#include "uh/SavedSession.hpp"

namespace uh {

class UH_PUBLIC_API SavedGameSession : public SavedSession, public GameSession
{
private:
    SavedGameSession(
            MappingInfo&& mapping,
            StageID stageID,
            SmallVector<FighterID, 8>&& playerFighterIDs,
            SmallVector<SmallString<15>, 8>&& playerTags,
            SmallVector<SmallString<15>, 8>&& playerNames,
            GameNumber gameNumber=1,
            SetNumber setNumber=1,
            SetFormat setFormat=SetFormat::FRIENDLIES
    );

    friend class SavedSession;

public:
    // Make MSVC shut up about dominance
    const SmallString<15>& playerName(int playerIdx) const override
        { return GameSession::playerName(playerIdx); }
    TimeStampMS timeStampStartedMs() const override
        { return GameSession::timeStampStartedMs(); }
    int winner() const override
        { return SavedSession::winner(); }
};

}
