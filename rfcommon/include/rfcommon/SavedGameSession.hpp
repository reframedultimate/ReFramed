#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/GameSession.hpp"
#include "rfcommon/SavedSession.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API SavedGameSession : public SavedSession, public GameSession
{
private:
    SavedGameSession(
            MappingInfo&& mapping,
            StageID stageID,
            SmallVector<FighterID, 2>&& fighterIDs,
            SmallVector<SmallString<15>, 2>&& tags,
            SmallVector<SmallString<15>, 2>&& playerNames,
            GameNumber gameNumber=1,
            SetNumber setNumber=1,
            SetFormat setFormat=SetFormat::FRIENDLIES
    );

    friend class SavedSession;
};

}
