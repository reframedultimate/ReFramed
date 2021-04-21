#pragma once

#include "uh/Recording.hpp"

namespace uh {

class RecordingListener;
class PlayerState;

class ActiveRecording : public Recording
{
public:
    ActiveRecording(MappingInfo&& mapping,
                    std::vector<FighterID>&& playerFighterIDs,
                    std::vector<std::string>&& playerTags,
                    StageID stageID);

    void setPlayerName(int index, const std::string& name);
    void setGameNumber(int number);
    void setSetNumber(int number);
    void setFormat(const SetFormat& format);
    void addPlayerState(int index, PlayerState&& state);
};

}
