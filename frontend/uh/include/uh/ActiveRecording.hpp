#pragma once

#include "uh/Recording.hpp"

namespace uh {

class RecordingListener;
class PlayerState;

class ActiveRecording : public Recording
{
public:
    ActiveRecording(MappingInfo&& mapping,
                    std::initializer_list<uint8_t> playerFighterIDs,
                    std::initializer_list<std::string> playerTags,
                    uint16_t stageID);

    void setPlayerName(int index, const std::string& name);
    void setGameNumber(int number);
    void setSetNumber(int number);
    void setFormat(const SetFormat& format);
    void addPlayerState(int index, PlayerState&& state);
};

}
