#pragma once

#include "uh/config.hpp"
#include "uh/Recording.hpp"

namespace uh {

class RecordingListener;
class PlayerState;

class UH_PUBLIC_API ActiveRecording : public Recording
{
public:
    ActiveRecording(MappingInfo&& mapping,
                    SmallVector<FighterID, 8>&& playerFighterIDs,
                    SmallVector<SmallString<15>, 8>&& playerTags,
                    StageID stageID);

    void setPlayerName(int index, const SmallString<15>& name);
    void setGameNumber(GameNumber number);
    void setSetNumber(SetNumber number);
    void setFormat(const SetFormat& format);
    void addPlayerState(int index, PlayerState&& state);
};

}
