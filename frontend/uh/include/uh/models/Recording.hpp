#pragma once

#include "uh/listeners/ListenerDispatcher.hpp"
#include "uh/models/MappingInfo.hpp"
#include "uh/models/SetFormat.hpp"
#include <QSharedData>
#include <QVector>
#include <QDateTime>

namespace uh {

class PlayerState;
class RecordingListener;

class Recording : public QSharedData
{
public:
    Recording(MappingInfo&& mapping,
              QVector<uint8_t>&& playerFighterIDs,
              QVector<QString>&& playerTags,
              uint16_t stageID);

    bool saveAs(const QString& fileName);

    /*!
     * \brief Returns information on how to map fighter/stage/state IDs to
     * strings.
     */
    const MappingInfo& mappingInfo() const { return mappingInfo_; }

    /*!
     * \brief Gets the number of players
     */
    int playerCount() const { return static_cast<int>(playerTags_.size()); }

    /*!
     * \brief Gets the tag used by the player. This is the string that appears
     * above the player in-game and is created when the player sets their controls.
     * \param index Which player to get
     */
    const QString& playerTag(int index) const { return playerTags_[index]; }

    /*!
     * \brief Gets the name of the player. By default this will be the same as
     * the tag, but many players like to create tags that are shorter or a
     * variation of their real name. This string is their real name.
     * Unlike tags, there is also no character limit to a player's name.
     * \param index Which player to get
     */
    const QString& playerName(int index) const { return playerNames_[index]; }

    /*!
     * \brief Gets the fighter ID being used by the specified player.
     * \param index The player to get
     */
    uint8_t playerFighterID(int index) const { return playerFighterIDs_[index]; }

    /*!
     * \brief Gets the stage ID being played on.
     */
    uint16_t stageID() const { return stageID_; }

    /*!
     * \brief Gets the current game number. Starts at 1 and counts upwards as
     * sets progress.
     */
    int gameNumber() const { return gameNumber_; }

    /*!
     * \brief Gets the set number. Usually 1. This number is used to disambiguate
     * sets where the same two players play the same characters on the same day.
     */
    int setNumber() const { return setNumber_; }

    /*!
     * \brief Returns the player index of the player that won.
     */
    int winner() const { return winner_; }

    /*!
     * \brief Gets the format of the set, @see Recording::Format
     */
    SetFormat format() const { return format_; }

    /*!
     * \brief Gets the datetime of when the match started. This marks the first
     * frame of gameplay, immediately after the 3-2-1-Go countdown completes.
     * May be slightly off by a few frames depending on latency.
     */
    const QDateTime& timeStarted() const { return timeStarted_; }

    const QVector<PlayerState>& playerStates(int player) const;

    ListenerDispatcher<RecordingListener> dispatcher;

protected:
    MappingInfo mappingInfo_;
    QDateTime timeStarted_;
    QVector<QString> playerTags_;
    QVector<QString> playerNames_;
    QVector<uint8_t> playerFighterIDs_;
    QVector<QVector<PlayerState>> playerStates_;
    SetFormat format_;
    int gameNumber_ = 1;
    int setNumber_ = 1;
    int winner_ = 0;
    uint16_t stageID_;
};

}
