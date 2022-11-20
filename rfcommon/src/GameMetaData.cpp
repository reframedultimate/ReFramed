#include "rfcommon/GameMetaData.hpp"
#include "rfcommon/MetaDataListener.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Log.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
GameMetaData::GameMetaData(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<String, 2>&& tags,
        int winner)
    : MetaData(timeStarted, timeEnded, stageID, std::move(fighterIDs), std::move(tags))
    , players_(SmallVector<Player, 2>::makeResized(MetaData::fighterCount()))
    , bracketType_(BracketType::fromType(BracketType::FRIENDLIES))
    , round_(Round::makeFree())
    , setFormat_(SetFormat::fromType(SetFormat::FREE))
    , score_(ScoreCount::fromScore(0, 0))
    , winner_(winner)
{
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
void GameMetaData::setTournamentOrganizer(int toIdx, const char* name, const char* social, const char* pronouns)
{
    Log::root()->debug("TO #%d set to %s, %s, %s", toIdx, name, social, pronouns);
    organizers_[toIdx].name = name;
    organizers_[toIdx].social = social;
    organizers_[toIdx].pronouns = pronouns;

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
void GameMetaData::setSponsor(int idx, const char* name, const char* website)
{
    Log::root()->debug("Sponsor #%d set to %s, %s", idx, name, website);
    sponsors_[idx].name = name;
    sponsors_[idx].website = website;

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
void GameMetaData::setCommentator(int idx, const char* name, const char* social, const char* pronouns)
{
    commentators_[idx].name = name;
    commentators_[idx].social = social;
    commentators_[idx].pronouns = pronouns;
    dispatcher.dispatch(&MetaDataListener::onMetaDataCommentatorsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::removeCommentator(int idx)
{
    sponsors_.erase(idx);
    dispatcher.dispatch(&MetaDataListener::onMetaDataCommentatorsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::setBracketType(BracketType type)
{
    bracketType_ = type;
    dispatcher.dispatch(&MetaDataListener::onMetaDataEventDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetaData::setBracketURL(const char* url)
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
