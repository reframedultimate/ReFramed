#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/GameNumber.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/StageID.hpp"
#include "rfcommon/SetFormat.hpp"
#include "rfcommon/SetNumber.hpp"
#include "rfcommon/TimeStamp.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API GameMetaData : public MetaData
{
    GameMetaData(
        const char* tournamentName,
        const char* eventName,
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<String, 2>&& tags,
        SmallVector<String, 2>&& names,
        SmallVector<String, 2>&& sponsors,
        SmallVector<String, 2>&& commentators,
        GameNumber gameNumber,
        SetNumber setNumber,
        SetFormat setFormat,
        const char* roundName,
        int winner);

public:
    Type type() const override;

    /*!
     * \brief Gets the name of the player. By default this will be the same as
     * the tag, but many players like to create tags that are shorter or a
     * variation of their real name. This string is their real name.
     * Unlike tags, there is also no character limit to a player's name.
     * \note If training mode, this will always be the same as the tag.
     * \param fighterIdx Which player to get
     */
    const String& name(int fighterIdx) const override;

    void setName(int fighterIdx, const char* name);

    const String& sponsor(int fighterIdx) const override;
    void setSponsor(int fighterIdx, const char* sponsor);

    const String& tournamentName() const;
    void setTournamentName(const char* name);

    const String& eventName() const;
    void setEventName(const char* name);

    const String& roundName() const;
    void setRoundName(const char* name);

    const SmallVector<String, 2>& commentators() const;
    void setCommentators(SmallVector<String, 2>&& names);

    /*!
     * \brief Gets the current game number. Starts at 1 and counts upwards as
     * sets progress.
     */
    GameNumber gameNumber() const;

    /*!
     * \brief Sets the current game number. Should start at 1.
     */
    void setGameNumber(GameNumber gameNumber);

    /*!
     * \brief Resets the game number to 1
     */
    void resetGameNumber();

    /*!
     * \brief Gets the set number. Usually 1. This number is used to disambiguate
     * sets where the same two players play the same characters on the same day.
     */
    SetNumber setNumber() const;

    /*!
     * \brief Sets the current set number. Should start at 1.
     */
    void setSetNumber(SetNumber setNumber);

    /*!
     * \brief Resets the current set number to 1.
     */
    void resetSetNumber();

    /*!
     * \brief Gets the format of the set, \see SetFormat
     */
    SetFormat setFormat() const;

    /*!
     * \brief Sets the format of the set, \see SetFormat
     */
    void setSetFormat(SetFormat format);

    /*!
     * \brief Gets the index of the player who won the game, or is currently in
     * the lead in the case of an on-going session.
     * \note If there is no winner, for example, if this is a training session,
     * or if the session has no frames, then -1 is returned.
     * \return Returns the player index. Can return -1 if no winner exists.
     */
    int winner() const;

    /*!
     * \brief Set the index of the player who won the game, or is currently in
     * the lead in the case of an on-going session.
     */
    void setWinner(int fighterIdx);

private:
    friend class MetaData;

    struct PlayerData
    {
        String name;
        String sponsor;
    };

    SmallVector<PlayerData, 2> players_;
    SmallVector<String, 2> commentators_;
    String tournamentName_;
    String eventName_;
    String roundName_;
    GameNumber gameNumber_;
    SetNumber setNumber_;
    SetFormat setFormat_;
    int winner_;
};

}
