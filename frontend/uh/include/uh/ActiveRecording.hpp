#pragma once

#include "uh/Recording.hpp"

namespace uh {

class RecordingListener;
class PlayerState;

class ActiveRecording : public Recording
{
public:
    ActiveRecording(MappingInfo&& mapping,
                    QVector<uint8_t>&& playerFighterIDs,
                    QVector<QString>&& playerTags,
                    uint16_t stageID);

    void setPlayerName(int index, const QString& name);
    void setGameNumber(int number);
    void setSetNumber(int number);
    void setFormat(const SetFormat& format);
    void addPlayerState(int index, PlayerState&& state);
};

}
