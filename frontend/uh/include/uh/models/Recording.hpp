#pragma once

#include "uh/listeners/ListenerDispatcher.hpp"
#include "uh/models/MappingInfo.hpp"
#include "uh/models/GameInfo.hpp"
#include "uh/models/PlayerInfo.hpp"
#include "uh/models/PlayerState.hpp"
#include <QSharedData>
#include <QDir>

namespace uh {

class RecordingListener;

class Recording : public QSharedData
{
public:
    Recording(const MappingInfo& mapping,
              const GameInfo& gameInfo,
              QVector<PlayerInfo>&& playerInfos);

    static Recording* load(const QString& fileName);

    /*!
     * \brief Saves this recording into the specified directory. A filename is
     * automatically created based on metadata.
     * \param path The path to save to.
     * \return Returns true if the file was saved successfully, false if otherwise.
     */
    bool saveTo(const QDir& path);

    /*!
     * \brief Composes a filename based on metadata, and makes sure the filename
     * does not exist in the specified path. A game number will be inserted
     * into the filename to ensure it can be unique.
     * \param path A path where you'd like to save the recording to.
     * \return Returns the absolve path + filename to a file that does not exist
     * yet.
     */
    QString findNonExistingFileName(const QDir& path);

    void addPlayerState(int index, PlayerState&& state);

    MappingInfo& mappingInfo() { return mappingInfo_; }
    GameInfo& gameInfo() { return gameInfo_; }
    PlayerInfo& playerInfo(int index) { return playerInfo_[index]; }
    int playerCount() const { return (int)playerInfo_.size(); }

    ListenerDispatcher<RecordingListener> dispatcher;

private:
    MappingInfo mappingInfo_;
    GameInfo gameInfo_;
    QVector<PlayerInfo> playerInfo_;
    QVector<QVector<PlayerState>> playerStates_;
};

}
