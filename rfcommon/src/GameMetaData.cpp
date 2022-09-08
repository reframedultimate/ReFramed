#include "rfcommon/GameMetaData.hpp"
#include "rfcommon/MetaDataListener.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
GameMetaData::GameMetaData(
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
        int winner)
    : MetaData(timeStarted, timeEnded, stageID, std::move(fighterIDs), std::move(tags))
    , commentators_(std::move(commentators))
    , tournamentName_(tournamentName)
    , eventName_(eventName)
    , roundName_(roundName)
    , gameNumber_(gameNumber)
    , setNumber_(setNumber)
    , setFormat_(setFormat)
    , winner_(winner)
{
    assert(names.count() == sponsors.count());
    for (int i = 0; i != names.count(); ++i)
        players_.push({ std::move(names[i]), std::move(sponsors[i]) });
}

// ----------------------------------------------------------------------------
MetaData::Type GameMetaData::type() const
{
    NOPROFILE();

    return GAME;
}

// ----------------------------------------------------------------------------
const String& GameMetaData::name(int playerIdx) const
{
    NOPROFILE();

    return players_[playerIdx].name;
}

// ----------------------------------------------------------------------------
void GameMetaData::setName(int fighterIdx, const char* name)
{
    PROFILE(GameMetaData, setName);

    if (!players_[fighterIdx].name.replaceWith(name))
        dispatcher.dispatch(&MetaDataListener::onMetaDataPlayerNameChanged, fighterIdx, name);
}

// ----------------------------------------------------------------------------
const String& GameMetaData::sponsor(int fighterIdx) const
{
    return players_[fighterIdx].sponsor;
}

// ----------------------------------------------------------------------------
void GameMetaData::setSponsor(int fighterIdx, const char* sponsor)
{
    if (!players_[fighterIdx].sponsor.replaceWith(sponsor))
        dispatcher.dispatch(&MetaDataListener::onMetaDataSponsorChanged, fighterIdx, sponsor);
}

// ----------------------------------------------------------------------------
const String& GameMetaData::tournamentName() const
{
    return tournamentName_;
}

// ----------------------------------------------------------------------------
void GameMetaData::setTournamentName(const char* name)
{
    if (!tournamentName_.replaceWith(name))
        dispatcher.dispatch(&MetaDataListener::onMetaDataTournamentNameChanged, name);
}

// ----------------------------------------------------------------------------
const String& GameMetaData::eventName() const
{
    return eventName_;
}

// ----------------------------------------------------------------------------
void GameMetaData::setEventName(const char* name)
{
    if (!eventName_.replaceWith(name))
        dispatcher.dispatch(&MetaDataListener::onMetaDataEventNameChanged, name);
}

// ----------------------------------------------------------------------------
const String& GameMetaData::roundName() const
{
    return roundName_;
}

// ----------------------------------------------------------------------------
void GameMetaData::setRoundName(const char* name)
{
    if (!roundName_.replaceWith(name))
        dispatcher.dispatch(&MetaDataListener::onMetaDataRoundNameChanged, name);
}

// ----------------------------------------------------------------------------
const SmallVector<String, 2>& GameMetaData::commentators() const
{
    return commentators_;
}

// ----------------------------------------------------------------------------
void GameMetaData::setCommentators(SmallVector<String, 2>&& names)
{
    commentators_ = std::move(names);
    dispatcher.dispatch(&MetaDataListener::onMetaDataCommentatorsChanged, commentators_);
}

// ----------------------------------------------------------------------------
GameNumber GameMetaData::gameNumber() const
{
    NOPROFILE();

    return gameNumber_;
}

// ----------------------------------------------------------------------------
void GameMetaData::setGameNumber(GameNumber gameNumber)
{
    PROFILE(GameMetaData, setGameNumber);

    bool notify = (gameNumber_ != gameNumber);
    gameNumber_ = gameNumber;
    if (notify)
        dispatcher.dispatch(&MetaDataListener::onMetaDataGameNumberChanged, gameNumber);
}

// ----------------------------------------------------------------------------
void GameMetaData::resetGameNumber()
{
    PROFILE(GameMetaData, resetGameNumber);

    setGameNumber(GameNumber::fromValue(1));
}

// ----------------------------------------------------------------------------
SetNumber GameMetaData::setNumber() const
{
    NOPROFILE();

    return setNumber_;
}

void GameMetaData::setSetNumber(SetNumber setNumber)
{
    PROFILE(GameMetaData, setSetNumber);

    bool notify = (setNumber_ != setNumber);
    setNumber_ = setNumber;
    if (notify)
        dispatcher.dispatch(&MetaDataListener::onMetaDataSetNumberChanged, setNumber);
}

// ----------------------------------------------------------------------------
void GameMetaData::resetSetNumber()
{
    PROFILE(GameMetaData, resetSetNumber);

    setSetNumber(SetNumber::fromValue(1));
}

// ----------------------------------------------------------------------------
SetFormat GameMetaData::setFormat() const
{
    NOPROFILE();

    return setFormat_;
}

// ----------------------------------------------------------------------------
void GameMetaData::setSetFormat(SetFormat format)
{
    PROFILE(GameMetaData, setSetFormat);

    bool notify = (setFormat_ != format);
    setFormat_ = format;
    if (notify)
        dispatcher.dispatch(&MetaDataListener::onMetaDataSetFormatChanged, format);
}

// ----------------------------------------------------------------------------
int GameMetaData::winner() const
{
    NOPROFILE();

    return winner_;
}

// ----------------------------------------------------------------------------
void GameMetaData::setWinner(int fighterIdx)
{
    PROFILE(GameMetaData, setWinner);

    bool notify = (winner_ != fighterIdx);
    winner_ = fighterIdx;
    if (notify)
        dispatcher.dispatch(&MetaDataListener::onMetaDataWinnerChanged, fighterIdx);
}

}
