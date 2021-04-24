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
                    std::vector<FighterID>&& playerFighterIDs,
                    std::vector<std::string>&& playerTags,
                    StageID stageID);

    void setPlayerName(int index, const std::string& name);
    void setGameNumber(GameNumber number);
    void setSetNumber(SetNumber number);
    void setFormat(const SetFormat& format);
    void addPlayerState(int index, PlayerState&& state);
};

}
