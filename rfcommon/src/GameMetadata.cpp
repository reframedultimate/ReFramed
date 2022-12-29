#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/MetadataListener.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Log.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
GameMetadata::GameMetadata(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<Costume, 2>&& costumes,
        SmallVector<String, 2>&& tags,
        int winner)
    : Metadata(timeStarted, timeEnded, stageID, std::move(fighterIDs), std::move(costumes), std::move(tags))
    , players_(SmallVector<Player, 2>::makeResized(Metadata::fighterCount()))
    , bracketType_(BracketType::fromType(BracketType::FRIENDLIES))
    , round_(Round::makeFree())
    , setFormat_(SetFormat::fromType(SetFormat::FREE))
    , score_(ScoreCount::fromScore(0, 0))
    , winner_(winner)
{
}

// ----------------------------------------------------------------------------
GameMetadata::~GameMetadata()
{}

// ----------------------------------------------------------------------------
void GameMetadata::setTournamentName(const char* name)
{
    PROFILE(GameMetadata, setTournamentName);

    if (!tournamentName_.replaceWith(name))
        dispatcher.dispatch(&MetadataListener::onMetadataTournamentDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::setTournamentWebsite(const char* name)
{
    PROFILE(GameMetadata, setTournamentWebsite);

    if (!tournamentURL_.replaceWith(name))
        dispatcher.dispatch(&MetadataListener::onMetadataTournamentDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::addTournamentOrganizer(const char* name, const char* social, const char* pronouns)
{
    PROFILE(GameMetadata, addTournamentOrganizer);

    organizers_.push({ name, social, pronouns });
    dispatcher.dispatch(&MetadataListener::onMetadataTournamentDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::setTournamentOrganizer(int toIdx, const char* name, const char* social, const char* pronouns)
{
    PROFILE(GameMetadata, setTournamentOrganizer);

    Log::root()->debug("TO #%d set to %s, %s, %s", toIdx, name, social, pronouns);
    organizers_[toIdx].name = name;
    organizers_[toIdx].social = social;
    organizers_[toIdx].pronouns = pronouns;

    dispatcher.dispatch(&MetadataListener::onMetadataTournamentDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::removeTournamentOrganizer(int idx)
{
    PROFILE(GameMetadata, removeTournamentOrganizer);

    organizers_.erase(idx);
    dispatcher.dispatch(&MetadataListener::onMetadataTournamentDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::addSponsor(const char* name, const char* website)
{
    PROFILE(GameMetadata, addSponsor);

    sponsors_.push({ name, website });
    dispatcher.dispatch(&MetadataListener::onMetadataTournamentDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::setSponsor(int idx, const char* name, const char* website)
{
    PROFILE(GameMetadata, setSponsor);

    Log::root()->debug("Sponsor #%d set to %s, %s", idx, name, website);
    sponsors_[idx].name = name;
    sponsors_[idx].website = website;

    dispatcher.dispatch(&MetadataListener::onMetadataTournamentDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::removeSponsor(int idx)
{
    PROFILE(GameMetadata, removeSponsor);

    sponsors_.erase(idx);
    dispatcher.dispatch(&MetadataListener::onMetadataTournamentDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::addCommentator(const char* name, const char* social, const char* pronouns)
{
    PROFILE(GameMetadata, addCommentator);

    commentators_.push({ name, social, pronouns });
    dispatcher.dispatch(&MetadataListener::onMetadataCommentatorsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::setCommentator(int idx, const char* name, const char* social, const char* pronouns)
{
    PROFILE(GameMetadata, setCommentator);

    commentators_[idx].name = name;
    commentators_[idx].social = social;
    commentators_[idx].pronouns = pronouns;
    dispatcher.dispatch(&MetadataListener::onMetadataCommentatorsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::removeCommentator(int idx)
{
    PROFILE(GameMetadata, removeCommentator);

    commentators_.erase(idx);
    dispatcher.dispatch(&MetadataListener::onMetadataCommentatorsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::setBracketType(BracketType type)
{
    PROFILE(GameMetadata, setBracketType);

    bracketType_ = type;
    dispatcher.dispatch(&MetadataListener::onMetadataEventDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::setBracketURL(const char* url)
{
    PROFILE(GameMetadata, setBracketURL);

    if (!eventURL_.replaceWith(url))
        dispatcher.dispatch(&MetadataListener::onMetadataEventDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::setRound(Round round)
{
    PROFILE(GameMetadata, setRound);

    round_ = round;
    dispatcher.dispatch(&MetadataListener::onMetadataGameDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::setSetFormat(SetFormat format)
{
    PROFILE(GameMetadata, setSetFormat);

    setFormat_ = format;
    dispatcher.dispatch(&MetadataListener::onMetadataGameDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::setScore(ScoreCount score)
{
    PROFILE(GameMetadata, setScore);

    score_ = score;
    dispatcher.dispatch(&MetadataListener::onMetadataGameDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::setPlayerName(int fighterIdx, const char* name)
{
    PROFILE(GameMetadata, setPlayerName);

    assert(fighterIdx < fighterCount());
    if (!players_[fighterIdx].name.replaceWith(name))
        dispatcher.dispatch(&MetadataListener::onMetadataPlayerDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::setPlayerSponsor(int fighterIdx, const char* sponsor)
{
    PROFILE(GameMetadata, setPlayerSponsor);

    assert(fighterIdx < fighterCount());
    if (!players_[fighterIdx].sponsor.replaceWith(sponsor))
        dispatcher.dispatch(&MetadataListener::onMetadataPlayerDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::setPlayerSocial(int fighterIdx, const char* social)
{
    PROFILE(GameMetadata, setPlayerSocial);

    assert(fighterIdx < fighterCount());
    if (!players_[fighterIdx].social.replaceWith(social))
        dispatcher.dispatch(&MetadataListener::onMetadataPlayerDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::setPlayerPronouns(int fighterIdx, const char* pronouns)
{
    PROFILE(GameMetadata, setPlayerPronouns);

    assert(fighterIdx < fighterCount());
    if (!players_[fighterIdx].pronouns.replaceWith(pronouns))
        dispatcher.dispatch(&MetadataListener::onMetadataPlayerDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::setPlayerIsLoserSide(int fighterIdx, bool isLoser)
{
    PROFILE(GameMetadata, setPlayerIsLoserSide);

    assert(fighterIdx < fighterCount());
    players_[fighterIdx].isLoser = isLoser;
    dispatcher.dispatch(&MetadataListener::onMetadataPlayerDetailsChanged);
}

// ----------------------------------------------------------------------------
void GameMetadata::setWinner(int fighterIdx)
{
    PROFILE(GameMetadata, setWinner);

    bool notify = (winner_ != fighterIdx);
    winner_ = fighterIdx;
    if (notify)
        dispatcher.dispatch(&MetadataListener::onMetadataWinnerChanged, fighterIdx);
}

}
