#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/EventType.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/Round.hpp"
#include "rfcommon/ScoreCount.hpp"
#include "rfcommon/SetFormat.hpp"
#include "rfcommon/StageID.hpp"
#include "rfcommon/TimeStamp.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API GameMetaData : public MetaData
{
    GameMetaData(
            TimeStamp timeStarted,
            TimeStamp timeEnded,
            StageID stageID,
            EventType eventType,
            Round round,
            SetFormat format,
            ScoreCount score,
            SmallVector<FighterID, 2>&& fighterIDs,
            SmallVector<String, 2>&& tags,
            SmallVector<String, 2>&& names,
            SmallVector<String, 2>&& sponsors,
            SmallVector<String, 2>&& pronouns,
            int winner);

public:
    ~GameMetaData();

    Type type() const override final { return GAME; }

    const String& tournamentName() const { return tournamentName_; }
    void setTournamentName(const char* name);

    const String& tournamentWebsite() const { return tournamentURL_; }
    void setTournamentWebsite(const char* url);

    int tournamentOrganizerCount() const { return organizers_.count(); }
    const String& tournamentOrganizerName(int toIdx) const { return organizers_[toIdx].name; }
    const String& tournamentOrganizerSocial(int toIdx) const { return organizers_[toIdx].social; }
    const String& tournamentOrganizerPronouns(int toIdx) const { return organizers_[toIdx].pronouns; }
    void addTournamentOrganizer(const char* name, const char* social, const char* pronouns="he/him");
    void removeTournamentOrganizer(int toIdx);

    int sponsorCount() const { return sponsors_.count(); }
    const String& sponsorName(int idx) const { return sponsors_[idx].name; }
    const String& sponsorWebsite(int idx) const { return sponsors_[idx].website; }
    void addSponsor(const char* name, const char* website);
    void removeSponsor(int idx);

    int commentatorCount() const { return commentators_.count(); }
    const String& commentatorName(int idx) const { return commentators_[idx].name; }
    const String& commentatorSocial(int idx) const { return commentators_[idx].social; }
    const String& commentatorPronouns(int idx) const { return commentators_[idx].pronouns; }
    void addCommentator(const char* name, const char* social, const char* pronouns="he/him");
    void removeCommentator(int idx);

    const EventType eventType() const { return eventType_; }
    const String& eventURL() const { return eventURL_; }
    void setEventType(EventType type);
    void setEventURL(const char* url);

    Round round() const { return round_; }

    void setRound(Round round);

    /*!
     * \brief Gets the format of the set, \see SetFormat
     */
    SetFormat setFormat() const { return setFormat_; }

    /*!
     * \brief Sets the format of the set, \see SetFormat
     */
    void setSetFormat(SetFormat format);

    /*!
     * \brief Gets the current score.
     */
    ScoreCount score() const { return score_; }

    /*!
     * \brief Sets the current game number. Should start at 1.
     */
    void setScore(ScoreCount score);

    /*!
     * \brief Gets the name of the player. By default this will be the same as
     * the tag, but many players like to create tags that are shorter or a
     * variation of their real name. This string is their real name.
     * Unlike tags, there is also no character limit to a player's name.
     * \note If training mode, this will always be the same as the tag.
     * \param fighterIdx Which player to get
     */
    const String& playerName(int fighterIdx) const { return players_[fighterIdx].name; }
    void setPlayerName(int fighterIdx, const char* name);

    /*!
     * \brief Gets the sponsor prefix. Usually a 2-3 letter word
     * \param fighterIdx Index of the fighter to get the sponsor for.
     */
    const String& playerSponsor(int fighterIdx) const { return players_[fighterIdx].sponsor; }
    void setPlayerSponsor(int fighterIdx, const char* sponsor);

    /*!
     * \brief Gets the player's social contact info (twitter, discord, etc.)
     * \param fighterIdx Index of the fighter.
     */
    const String& playerSocial(int fighterIdx) const { return players_[fighterIdx].social; }
    void setPlayerSocial(int fighterIdx, const char* social);

    /*!
     * \brief Gets the pronouns of the player (e.g. "he/him").
     * \param fighterIdx Index of the fighter.
     */
    const String& playerPronouns(int fighterIdx) const { return players_[fighterIdx].pronouns; }
    void setPlayerPronouns(int fighterIdx, const char* pronouns);

    bool playerIsLoserSide(int fighterIdx) const { return players_[fighterIdx].isLoser; }
    void setPlayerIsLoserSide(int fighterIdx, bool isLoser);

    /*!
     * \brief Gets the index of the player who won the game, or is currently in
     * the lead in the case of an on-going session.
     * \note If there is no winner, for example, if this is a training session,
     * or if the session has no frames, then -1 is returned.
     * \return Returns the player index. Can return -1 if no winner exists.
     */
    int winner() const { return winner_; }

    /*!
     * \brief Set the index of the player who won the game, or is currently in
     * the lead in the case of an on-going session.
     */
    void setWinner(int fighterIdx);

private:
    friend class MetaData;

    struct Person
    {
        String name;
        String social;
        String pronouns;
    };

    struct Sponsor
    {
        String name;
        String website;
    };

    struct Player
    {
        String name;
        String sponsor;
        String social;
        String pronouns;
        bool isLoser;
    };

    SmallVector<Person, 2> organizers_;
    SmallVector<Person, 2> commentators_;
    SmallVector<Sponsor, 2> sponsors_;
    SmallVector<Player, 2> players_;
    String tournamentName_, tournamentURL_;
    String eventURL_;
    EventType eventType_;
    Round round_;
    SetFormat setFormat_;
    ScoreCount score_;
    int winner_;
};

}
