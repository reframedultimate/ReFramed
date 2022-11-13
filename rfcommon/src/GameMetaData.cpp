#include "rfcommon/GameMetaData.hpp"
#include "rfcommon/MetaDataListener.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
GameMetaData::GameMetaData(
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
        int winner)
    : MetaData(timeStarted, timeEnded, stageID, std::move(fighterIDs), std::move(tags))
    , eventType_(eventType)
    , round_(round)
    , setFormat_(format)
    , score_(score)
    , winner_(winner)
{
    assert(names.count() == sponsors.count());
    for (int i = 0; i != names.count(); ++i)
        players_.push({ std::move(names[i]), std::move(sponsors[i]), std::move(pronouns[i]) });
}

// ----------------------------------------------------------------------------
GameMetaData::~GameMetaData()
{}

// ----------------------------------------------------------------------------
void GameMetaData::setTournamentName(const char* name)
{
    if (!tournamentName_.replaceWith(name))
        dispatcher.dispatch(&MetaDataListener::onMetaDataTournamentDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::setTournamentWebsite(const char* name)
{
    if (!tournamentURL_.replaceWith(name))
        dispatcher.dispatch(&MetaDataListener::onMetaDataTournamentDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::addTournamentOrganizer(const char* name, const char* social, const char* pronouns)
{
    organizers_.push({ name, social, pronouns });
    dispatcher.dispatch(&MetaDataListener::onMetaDataTournamentDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::removeTournamentOrganizer(int idx)
{
    organizers_.erase(idx);
    dispatcher.dispatch(&MetaDataListener::onMetaDataTournamentDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::addSponsor(const char* name, const char* website)
{
    sponsors_.push({ name, website });
    dispatcher.dispatch(&MetaDataListener::onMetaDataTournamentDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::removeSponsor(int idx)
{
    sponsors_.erase(idx);
    dispatcher.dispatch(&MetaDataListener::onMetaDataTournamentDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::addCommentator(const char* name, const char* social, const char* pronouns)
{
    commentators_.push({ name, social, pronouns });
    dispatcher.dispatch(&MetaDataListener::onMetaDataCommentatorsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::removeCommentator(int idx)
{
    sponsors_.erase(idx);
    dispatcher.dispatch(&MetaDataListener::onMetaDataCommentatorsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::setEventType(EventType type)
{
    eventType_ = type;
    dispatcher.dispatch(&MetaDataListener::onMetaDataEventDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::setEventURL(const char* url)
{
    if (!eventURL_.replaceWith(url))
        dispatcher.dispatch(&MetaDataListener::onMetaDataEventDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::setRound(Round round)
{
    round_ = round;
    dispatcher.dispatch(&MetaDataListener::onMetaDataGameDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::setSetFormat(SetFormat format)
{
    setFormat_ = format;
    dispatcher.dispatch(&MetaDataListener::onMetaDataGameDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::setScore(ScoreCount score)
{
    score_ = score;
    dispatcher.dispatch(&MetaDataListener::onMetaDataGameDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::setPlayerName(int fighterIdx, const char* name)
{
    assert(fighterIdx < fighterCount());
    if (!players_[fighterIdx].name.replaceWith(name))
        dispatcher.dispatch(&MetaDataListener::onMetaDataPlayerDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::setPlayerSponsor(int fighterIdx, const char* sponsor)
{
    assert(fighterIdx < fighterCount());
    if (!players_[fighterIdx].sponsor.replaceWith(sponsor))
        dispatcher.dispatch(&MetaDataListener::onMetaDataPlayerDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::setPlayerSocial(int fighterIdx, const char* social)
{
    assert(fighterIdx < fighterCount());
    if (!players_[fighterIdx].social.replaceWith(social))
        dispatcher.dispatch(&MetaDataListener::onMetaDataPlayerDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::setPlayerPronouns(int fighterIdx, const char* pronouns)
{
    assert(fighterIdx < fighterCount());
    if (!players_[fighterIdx].pronouns.replaceWith(pronouns))
        dispatcher.dispatch(&MetaDataListener::onMetaDataPlayerDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::setPlayerIsLoserSide(int fighterIdx, bool isLoser)
{
    assert(fighterIdx < fighterCount());
    players_[fighterIdx].isLoser = isLoser;
    dispatcher.dispatch(&MetaDataListener::onMetaDataPlayerDetailsChanged);
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
