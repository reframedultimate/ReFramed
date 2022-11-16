#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/BracketType.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/Round.hpp"
#include "rfcommon/ScoreCount.hpp"
#include "rfcommon/SetFormat.hpp"
#include "rfcommon/StageID.hpp"
#include "rfcommon/TimeStamp.hpp"
#include <cstdio>

namespace rfcommon {

class MetaDataListener;
class GameMetaData;
class TrainingMetaData;

class RFCOMMON_PUBLIC_API MetaData : public RefCounted
{
public:
    enum Type
    {
        GAME,
        TRAINING
    };

protected:
    MetaData(
            TimeStamp timeStarted,
            TimeStamp timeEnded,
            StageID stageID,
            SmallVector<FighterID, 2>&& fighterIDs,
            SmallVector<String, 2>&& tags);

public:
    /*!
     * \brief Constructs a new GameMetaData object from the specified
     * parameters.
     *
     * - RoundNumber is set to "Free Play" and 1.
     * - ScoreCount is set to 0-0
     * - SetFormat is set to "Freeplay"
     * - TimeStarted and TimeEnded are set to the current time
     */
    static MetaData* newActiveGameSession(
            StageID stageID,
            SmallVector<FighterID, 2>&& fighterIDs,
            SmallVector<String, 2>&& tags);

    /*!
     * \brief Constructs a new TrainingMetaData object from the specified
     * parameters.
     *
     * - TimeStarted and TimeEnded are set to the current time
     */
    static MetaData* newActiveTrainingSession(
            StageID stageID,
            SmallVector<FighterID, 2>&& fighterIDs,
            SmallVector<String, 2>&& tags);

    static MetaData* newSavedGameSession(
            TimeStamp timeStarted,
            TimeStamp timeEnded,
            StageID stageID,
            BracketType bracketType,
            Round round,
            SetFormat format,
            ScoreCount score,
            SmallVector<FighterID, 2>&& fighterIDs,
            SmallVector<String, 2>&& tags,
            SmallVector<String, 2>&& names,
            SmallVector<String, 2>&& sponsors,
            SmallVector<String, 2>&& pronouns,
            int winner);

    static MetaData* newSavedTrainingSession(
            TimeStamp timeStarted,
            TimeStamp timeEnded,
            StageID stageID,
            SessionNumber sessionNumber,
            SmallVector<FighterID, 2>&& fighterIDs,
            SmallVector<String, 2>&& tags);

    virtual ~MetaData();

    virtual Type type() const = 0;
    GameMetaData* asGame();
    const GameMetaData* asGame() const;
    TrainingMetaData* asTraining();
    const TrainingMetaData* asTraining() const;

    static MetaData* load(const void*, uint32_t size);
    uint32_t save(FILE* fp) const;

    /*!
     * \brief Gets the number of fighters in this session. Usually 2, but can
     * go up to 8.
     */
    int fighterCount() const { return fighterIDs_.count(); }

    /*!
     * \brief Gets the tag used by the player. This is the string that appears
     * above the player in-game and is created when the player sets their controls.
     * \param fighterIdx Which player to get
     */
    const String& playerTag(int fighterIdx) const { return tags_[fighterIdx]; }

    /*!
     * \brief Gets the fighter ID being used by the specified player. The ID
     * can be used to look up the character's type or name by using the
     * MappingInfo structure.
     * \param fighterIdx The fighter index, from 0 to fighterCount() - 1.
     */
    FighterID playerFighterID(int fighterIdx) const { return fighterIDs_[fighterIdx]; }

    /*!
     * \brief Gets the stage ID being played on. The ID can be used to look up
     * the stage name by using the MappingInfo structure.
     */
    StageID stageID() const { return stageID_; }

    /*!
     * \brief Gets the absolute time of when the session started in unix time
     * (milli-seconds since Jan 1 1970). May be slightly off by 1 second or so
     * depending on latency.
     *
     * In the case of a game session, this marks the first frame of gameplay,
     * immediately after the 3-2-1-Go countdown completes.
     *
     * In the case of a training session, this marks the first frame that was
     * received (may not be the first frame of training mode depending on when
     * the user connected).
     *
     * @note If no frames were received yet, then TimeStamp::isValid() will be
     * false. If you really depend on a valid timestamp then you will need
     * to register as a listener to be notified when a valid timestamp is
     * received.
     */
    TimeStamp timeStarted() const { return timeStarted_; }

    void setTimeStarted(TimeStamp timeStamp);

    /*!
     * \brief Gets the absolute time of when the last timestamp was received in
     * unix time (milli-seconds since Jan 1 1970). May be slightly off by 1
     * second or so depending on latency.
     *
     * In the case of a running session, this will be the timestamp of the most
     * recent frame received.
     *
     * In the case of a saved session, this will be the timestamp of the last
     * frame received.
     *
     * @note If no frames were received yet, then TimeStamp::isValid() will be
     * false. If you really depend on a valid timestamp then you will need
     * to register as a listener to be notified when a valid timestamp is
     * received.
     */
    TimeStamp timeEnded() const { return timeEnded_; }

    void setTimeEnded(TimeStamp timeStamp);

    /*!
     * \brief Returns the length of the session. This is equivalent to
     * timeStampEnded() - timeStampStarted().
     */
    DeltaTime length() const;

    ListenerDispatcher<MetaDataListener> dispatcher;

protected:
    TimeStamp timeStarted_;
    TimeStamp timeEnded_;
    SmallVector<FighterID, 2> fighterIDs_;
    SmallVector<String, 2> tags_;
    StageID stageID_;
};

}
