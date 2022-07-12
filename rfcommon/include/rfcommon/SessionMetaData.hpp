#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/SetFormat.hpp"
#include "rfcommon/RefCounted.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API SessionMetaData : public RefCounted
{
public:
    enum Type
    {
        GAME,
        TRAINING
    };

protected:
    SessionMetaData(
            StageID stageID,
            SmallVector<FighterID, 2>&& fighterIDs,
            SmallVector<SmallString<15>, 2>&& tags);

public:
    virtual ~SessionMetaData();

    virtual Type type() const = 0;

    bool save(const String& fileName);

    /*!
     * \brief Gets the number of fighters in this session. Usually 2, but can
     * go up to 8.
     */
    int fighterCount() const
        { return fighterIDs_.count(); }

    /*!
     * \brief Gets the tag used by the player. This is the string that appears
     * above the player in-game and is created when the player sets their controls.
     * \param fighterIdx Which player to get
     */
    const SmallString<15>& tag(int fighterIdx) const
        { return tags_[fighterIdx]; }

    /*!
     * \brief Gets the name of the player. By default this will be the same as
     * the tag, but many players like to create tags that are shorter or a
     * variation of their real name. This string is their real name.
     * Unlike tags, there is also no character limit to a player's name.
     * \note If training mode, this will always be the same as the tag.
     * \param fighterIdx Which player to get
     */
    virtual const SmallString<15>& name(int fighterIdx) const = 0;

    /*!
     * \brief Gets the fighter ID being used by the specified player. The ID
     * can be used to look up the character's type or name by using the
     * MappingInfo structure.
     * \param fighterIdx The fighter index, from 0 to fighterCount() - 1.
     */
    FighterID fighterID(int fighterIdx) const
        { return fighterIDs_[fighterIdx]; }

    /*!
     * \brief Gets the stage ID being played on. The ID can be used to look up
     * the stage name by using the MappingInfo structure.
     */
    StageID stageID() const
        { return stageID_; }

    /*!
     * \brief Gets the index of the player who won the game, or is currently in
     * the lead in the case of an on-going session.
     * \note If there is no winner, for example, if this is a training session,
     * or if the session has no frames, then -1 is returned.
     * \return Returns the player index. Can return -1 if no winner exists.
     */
    int winner() const;

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
     * If no frames were received yet, then this will return "0", which is the
     * timestamp for Jan 1 1970.
     */
    TimeStampMS timeStampStartedMs() const
        { return timeStarted_; }

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
     * If no frames were received yet, then this will return "0", which is the
     * timestamp for Jan 1 1970.
     */
    TimeStampMS timeStampEndedMs() const
        { return timeEnded_; }

    /*!
     * \brief Returns the length of the session. This is equivalent to
     * timeStampEndedMs() - timeStampStartedMs().
     */
    DeltaTimeMS lengthMs() const
        { return timeStampStartedMs() - timeStampEndedMs(); }

protected:
    TimeStampMS timeStarted_;
    TimeStampMS timeEnded_;
    SmallVector<FighterID, 2> fighterIDs_;
    SmallVector<SmallString<15>, 2> tags_;
    StageID stageID_;
};

class RFCOMMON_PUBLIC_API GameSessionMetaData : public SessionMetaData
{
public:
    GameSessionMetaData(
            StageID stageID,
            SmallVector<FighterID, 2>&& fighterIDs,
            SmallVector<SmallString<15>, 2>&& tags,
            SmallVector<SmallString<15>, 2>&& names,
            GameNumber gameNumber=1,
            SetNumber setNumber=1,
            SetFormat setFormat=SetFormat::FRIENDLIES);

    Type type() const override
        { return GAME; }

    /*!
     * \brief Gets the name of the player. By default this will be the same as
     * the tag, but many players like to create tags that are shorter or a
     * variation of their real name. This string is their real name.
     * Unlike tags, there is also no character limit to a player's name.
     * \note If training mode, this will always be the same as the tag.
     * \param fighterIdx Which player to get
     */
    const SmallString<15>& name(int playerIdx) const override;

    /*!
     * \brief Gets the current game number. Starts at 1 and counts upwards as
     * sets progress.
     */
    GameNumber gameNumber() const;

    /*!
     * \brief Gets the set number. Usually 1. This number is used to disambiguate
     * sets where the same two players play the same characters on the same day.
     */
    SetNumber setNumber() const;

    /*!
     * \brief Gets the format of the set, @see Recording::Format
     */
    SetFormat setFormat() const;
};

class RFCOMMON_PUBLIC_API TrainingSessionMetaData : public SessionMetaData
{
public:
    TrainingSessionMetaData(
            StageID stageID,
            SmallVector<FighterID, 2>&& fighterIDs,
            SmallVector<SmallString<15>, 2>&& tags);

    FighterID playerFighterID() const;
    FighterID cpuFighterID() const;
};

}
