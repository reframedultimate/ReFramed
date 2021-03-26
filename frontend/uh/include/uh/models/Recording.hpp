#pragma once

#include "uh/models/MappingInfo.hpp"
#include "uh/models/GameInfo.hpp"
#include "uh/models/PlayerInfo.hpp"
#include "uh/models/PlayerState.hpp"
#include <QSharedData>
#include <QScopedPointer>

namespace uh {

class Recording : public QSharedData
{
public:
    Recording(const MappingInfo& mapping);

    void setGameInfo(const GameInfo& gameInfo);
    void addPlayer(const PlayerInfo& playerInfo);
    void addPlayerState(int index, const PlayerState& state);

    const MappingInfo& mappingInfo() const { return mappingInfo_; }
    const GameInfo& gameInfo() const { return gameInfo_; }
    const PlayerInfo& playerInfo(int index) const { return playerInfo_[index]; }
    int playerCount() const { return (int)playerInfo_.size(); }

private:
    MappingInfo mappingInfo_;
    GameInfo gameInfo_;
    QVector<PlayerInfo> playerInfo_;
    QVector<QVector<PlayerState>> playerStates_;
};

}
