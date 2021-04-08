#pragma once

#include "uh/listeners/ListenerDispatcher.hpp"
#include "uh/models/Recording.hpp"

namespace uh {

class ActiveRecordingListener;
class PlayerState;

class ActiveRecording : public Recording
{
public:
    ActiveRecording(const MappingInfo& mapping,
                    QVector<uint8_t>&& playerFighterIDs,
                    QVector<QString>&& playerTags,
                    uint16_t stageID);

    void setPlayerName(int index, const QString& name);
    void setGameNumber(int number);
    void setSetNumber(int number);
    void setFormat(SetFormat format, const QString& otherFormatDesc="");
    void addPlayerState(int index, PlayerState&& state);

    ListenerDispatcher<ActiveRecordingListener> dispatcher;
};

}
