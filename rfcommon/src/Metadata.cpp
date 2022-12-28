#include "rfcommon/Metadata.hpp"
#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/TrainingMetadata.hpp"
#include "rfcommon/MetadataListener.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/time.h"
#include "nlohmann/json.hpp"

namespace rfcommon {

using nlohmann::json;

static Metadata* load_1_5(json& j);
static Metadata* load_1_6(json& j);
static Metadata* load_1_7(json& j);

// ----------------------------------------------------------------------------
Metadata* Metadata::newActiveGameSession(
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<Costume, 2>&& costumes,
        SmallVector<String, 2>&& tags)
{
    PROFILE(Metadata, newActiveGameSession);

    const auto now = TimeStamp::fromMillisSinceEpoch(
        time_milli_seconds_since_epoch());

    return new GameMetadata(
            now, now,  // Start, end
            stageID,
            std::move(fighterIDs),
            std::move(costumes),
            std::move(tags),
            -1);  // winner
}

// ----------------------------------------------------------------------------
Metadata* Metadata::newActiveTrainingSession(
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<Costume, 2>&& costumes,
        SmallVector<String, 2>&& tags)
{
    PROFILE(Metadata, newActiveTrainingSession);

    const auto now = TimeStamp::fromMillisSinceEpoch(
        time_milli_seconds_since_epoch());

    return new TrainingMetadata(
            now, now,  // start, end
            stageID,
            std::move(fighterIDs),
            std::move(costumes),
            std::move(tags));
}

// ----------------------------------------------------------------------------
Metadata* Metadata::newSavedGameSession(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<Costume, 2>&& costumes,
        SmallVector<String, 2>&& tags,
        int winner)
{
    PROFILE(Metadata, newSavedGameSession);

    return new GameMetadata(
        timeStarted,
        timeEnded,
        stageID,
        std::move(fighterIDs),
        std::move(costumes),
        std::move(tags),
        winner);
}

// ----------------------------------------------------------------------------
Metadata* Metadata::newSavedTrainingSession(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<Costume, 2>&& costumes,
        SmallVector<String, 2>&& tags)
{
    PROFILE(Metadata, newSavedTrainingSession);

    return new TrainingMetadata(
        timeStarted,
        timeEnded,
        stageID,
        std::move(fighterIDs),
        std::move(costumes),
        std::move(tags));
}

// ----------------------------------------------------------------------------
Metadata::Metadata(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<Costume, 2>&& costumes,
        SmallVector<String, 2>&& tags)
    : timeStarted_(timeStarted)
    , timeEnded_(timeEnded)
    , fighterIDs_(std::move(fighterIDs))
    , costumes_(std::move(costumes))
    , tags_(std::move(tags))
    , stageID_(stageID)
{
}

// ----------------------------------------------------------------------------
Metadata::~Metadata()
{}

// ----------------------------------------------------------------------------
GameMetadata* Metadata::asGame()
{
    assert(type() == GAME);
    return static_cast<GameMetadata*>(this);
}

// ----------------------------------------------------------------------------
const GameMetadata* Metadata::asGame() const
{
    assert(type() == GAME);
    return static_cast<const GameMetadata*>(this);
}

// ----------------------------------------------------------------------------
TrainingMetadata* Metadata::asTraining()
{
    assert(type() == TRAINING);
    return static_cast<TrainingMetadata*>(this);
}

// ----------------------------------------------------------------------------
const TrainingMetadata* Metadata::asTraining() const
{
    assert(type() == TRAINING);
    return static_cast<const TrainingMetadata*>(this);
}

// ----------------------------------------------------------------------------
Metadata* Metadata::load(const void* data, uint32_t size)
{
    PROFILE(Metadata, load);

    // Parse
    const unsigned char* const begin = static_cast<const unsigned char*>(data);
    const unsigned char* const end = static_cast<const unsigned char*>(data) + size;
    json j = json::parse(begin, end, nullptr, false);
    if (j.is_discarded())
        return nullptr;

    if (j["version"] == "1.7")
        return load_1_7(j);
    if (j["version"] == "1.6")
        return load_1_6(j);
    if (j["version"] == "1.5")
        return load_1_5(j);

    // unsupported version
    return nullptr;
}
static Metadata* load_1_5(json& j)
{
    PROFILE(MetadataGlobal, load_1_5);

    json jPlayerInfo = j["playerinfo"];
    json jGameInfo = j["gameinfo"];

    json jTimeStarted = jGameInfo["timestampstart"];
    json jTimeEnded = jGameInfo["timestampend"];
    json jStageID = jGameInfo["stageid"];
    const auto timeStarted = jTimeStarted.is_number_integer() ?
        TimeStamp::fromMillisSinceEpoch(jTimeStarted.get<TimeStamp::Type>()) : TimeStamp::makeInvalid();
    const auto timeEnded = jTimeEnded.is_number_integer() ?
        TimeStamp::fromMillisSinceEpoch(jTimeEnded.get<TimeStamp::Type>()) : TimeStamp::makeInvalid();
    const auto stageID = jStageID.is_number_integer() ?
        StageID::fromValue(jStageID.get<StageID::Type>()) : StageID::makeInvalid();

    SmallVector<FighterID, 2> fighterIDs;
    SmallVector<Costume, 2> costumes;
    SmallVector<String, 2> tags;
    int fighterCount = 0;
    for (auto& info : jPlayerInfo)
    {
        json jTag = info["tag"];
        json jFighterID = info["fighterid"];

        fighterIDs.push(jFighterID.is_number_integer() ?
            FighterID::fromValue(jFighterID.get<FighterID::Type>()) : FighterID::makeInvalid());
        costumes.push(Costume::makeDefault());
        tags.emplace(jTag.is_string() ?
            jTag.get<std::string>().c_str() : (std::string("Player ") + std::to_string(fighterCount)).c_str());

        fighterCount++;
    }

    json jType = j["type"];
    const std::string type = jType.is_string() ? jType.get<std::string>() : "";
    if (type == "game")
    {
        json jWinner = jGameInfo["winner"];
        int winner = jWinner.is_number_unsigned() ?
            jWinner.get<int>() : -1;
        if (winner > fighterCount)
            winner = -1;

        GameMetadata* g = Metadata::newSavedGameSession(
            timeStarted, timeEnded,
            stageID,
            std::move(fighterIDs),
            std::move(costumes),
            std::move(tags),
            winner)->asGame();

        json jSet = jGameInfo["set"];
        if (jSet.is_number_integer())
        {
            const auto sessionNumber = SessionNumber::fromValue(jSet.get<SessionNumber::Type>());
            g->setRound(Round::fromSessionNumber(sessionNumber));
        }

        json jFormat = jGameInfo["format"];
        if (jFormat.is_string())
            g->setSetFormat(SetFormat::fromDescription(jFormat.get<std::string>().c_str()));

        json jNumber = jGameInfo["number"];
        if (jNumber.is_number_integer())
        {
            const auto gameNumber = GameNumber::fromValue(jNumber.get<GameNumber::Type>());
            g->setScore(ScoreCount::fromGameNumber(gameNumber));
        }

        for (int i = 0; i != jPlayerInfo.size(); ++i)
        {
            json jName = jPlayerInfo[i]["name"];
            if (jName.is_string())
                g->setPlayerName(i, jName.get<std::string>().c_str());
        }

        return g;
    }
    if (type == "training")
    {
        TrainingMetadata* t = Metadata::newSavedTrainingSession(
            timeStarted,
            timeEnded,
            stageID,
            std::move(fighterIDs),
            std::move(costumes),
            std::move(tags))->asTraining();

        json jNumber = jGameInfo["number"];
        if (jNumber.is_number_integer())
            t->setSessionNumber(SessionNumber::fromValue(jNumber.get<SessionNumber::Type>()));

        return t;
    }

    return nullptr;
}
static Metadata* load_1_6(json& j)
{
    PROFILE(MetadataGlobal, load_1_6);

    json jPlayerInfo = j["playerinfo"];
    json jGameInfo = j["gameinfo"];

    json jTimeStarted = jGameInfo["timestampstart"];
    json jTimeEnded = jGameInfo["timestampend"];
    json jStageID = jGameInfo["stageid"];
    const auto timeStarted = jTimeStarted.is_number_integer() ?
        TimeStamp::fromMillisSinceEpoch(jTimeStarted.get<TimeStamp::Type>()) : TimeStamp::makeInvalid();
    const auto timeEnded = jTimeEnded.is_number_integer() ?
        TimeStamp::fromMillisSinceEpoch(jTimeEnded.get<TimeStamp::Type>()) : TimeStamp::makeInvalid();
    const auto stageID = jStageID.is_number_integer() ?
        StageID::fromValue(jStageID.get<StageID::Type>()) : StageID::makeInvalid();

    SmallVector<FighterID, 2> fighterIDs;
    SmallVector<Costume, 2> costumes;
    SmallVector<String, 2> tags;
    int fighterCount = 0;
    for (auto& info : jPlayerInfo)
    {
        json jTag = info["tag"];
        json jFighterID = info["fighterid"];

        fighterIDs.push(jFighterID.is_number_integer() ?
            FighterID::fromValue(jFighterID.get<FighterID::Type>()) : FighterID::makeInvalid());
        costumes.push(Costume::makeDefault());
        tags.emplace(jTag.is_string() ?
            jTag.get<std::string>().c_str() : (std::string("Player ") + std::to_string(fighterCount)).c_str());

        fighterCount++;
    }

    json jType = j["type"];
    const std::string type = jType.is_string() ? jType.get<std::string>() : "";
    if (type == "game")
    {
        json jWinner = jGameInfo["winner"];
        int winner = jWinner.is_number_unsigned() ?
            jWinner.get<int>() : -1;
        if (winner > fighterCount)
            winner = -1;

        GameMetadata* g = Metadata::newSavedGameSession(
            timeStarted,
            timeEnded,
            stageID,
            std::move(fighterIDs),
            std::move(costumes),
            std::move(tags),
            winner)->asGame();

        json jTournamentInfo = j["tournamentinfo"];
        json jTournamentName = jTournamentInfo["name"];
        if (jTournamentName.is_string())
            g->setTournamentName(jTournamentName.get<std::string>().c_str());

        json jCommentators = j["commentators"];
        for (auto& commentator : jCommentators)
            if (commentator.is_string())
                g->addCommentator(commentator.get<std::string>().c_str(), "");

        // In this version of the file, the user was able to type in the bracket
        // type by hand. Try to fuzzy match "singles" or "1v1", since that's the
        // most used type
        json jEventInfo = j["eventinfo"];
        json jEventName = jEventInfo["name"];
        const auto eventName = jEventName.is_string() ? jEventName.get<std::string>() : "";
        BracketType bracketType = [&eventName]() {
            auto eventNameLower = eventName;
            std::transform(eventNameLower.begin(), eventNameLower.end(), eventNameLower.begin(), [](unsigned char c){ return std::tolower(c); });
            if (eventNameLower.find("singles") != std::string::npos || eventNameLower.find("1v1") != std::string::npos)
                return BracketType::fromType(BracketType::SINGLES);
            else if (eventNameLower.find("amateur") != std::string::npos)
                 return BracketType::fromType(BracketType::AMATEURS);
            else if (eventName.empty() == false)
                return BracketType::fromDescription(eventName.c_str());
            else
                return BracketType::fromType(BracketType::FRIENDLIES);   // Default to friendlies
        }();
        g->setBracketType(bracketType);

        // In this version of the file, the user was able to type in the round
        // type, e.g. "WR1". Try to convert this string to our enum
        json jRound = jGameInfo["round"];
        json jSet = jGameInfo["set"];
        const auto sessionNumber = jSet.is_number_integer() ?
            SessionNumber::fromValue(jSet.get<SessionNumber::Type>()) : SessionNumber::fromValue(1);
        const auto roundName = jRound.is_string() ?
            jRound.get<std::string>() : std::string("");
        Round round = [&roundName, &sessionNumber]() {
            Round round = Round::fromDescription(roundName.c_str());
            if (round.type() == Round::FREE)
                round = Round::fromSessionNumber(sessionNumber);
            return round;
        }();
        g->setRound(round);

        json jNumber = jGameInfo["number"];
        if (jNumber.is_number_integer())
        {
            auto gameNumber = GameNumber::fromValue(jNumber.get<GameNumber::Type>());
            g->setScore(ScoreCount::fromGameNumber(gameNumber));
        }

        json jFormat = jGameInfo["format"];
        if (jFormat.is_string())
            g->setSetFormat(SetFormat::fromDescription(jFormat.get<std::string>().c_str()));

        for (int i = 0; i != jPlayerInfo.size(); ++i)
        {
            json jName = jPlayerInfo[i]["name"];
            if (jName.is_string())
                g->setPlayerName(i, jName.get<std::string>().c_str());

            json jSponsor = jPlayerInfo[i]["sponsor"];
            if (jSponsor.is_string())
                g->setPlayerSponsor(i, jSponsor.get<std::string>().c_str());
        }

        return g;
    }
    if (type == "training")
    {
        TrainingMetadata* t = Metadata::newSavedTrainingSession(
            timeStarted,
            timeEnded,
            stageID,
            std::move(fighterIDs),
            std::move(costumes),
            std::move(tags))->asTraining();

        json jNumber = jGameInfo["number"];
        if (jNumber.is_number_integer())
            t->setSessionNumber(SessionNumber::fromValue(jNumber.get<SessionNumber::Type>()));

        return t;
    }

    return nullptr;
}
static Metadata* load_1_7(json& j)
{
    PROFILE(MetadataGlobal, load_1_7);

    json jType = j["type"];
    json jPlayerInfo = j["playerinfo"];
    json jGameInfo = j["gameinfo"];

    const std::string type = jType.is_string() ? jType.get<std::string>() : "";

    json jTimeStarted = jGameInfo["timestampstart"];
    json jTimeEnded = jGameInfo["timestampend"];
    json jStageID = jGameInfo["stageid"];
    const auto timeStarted = jTimeStarted.is_number_integer() ?
        TimeStamp::fromMillisSinceEpoch(jTimeStarted.get<TimeStamp::Type>()) : TimeStamp::makeInvalid();
    const auto timeEnded = jTimeEnded.is_number_integer() ?
        TimeStamp::fromMillisSinceEpoch(jTimeEnded.get<TimeStamp::Type>()) : TimeStamp::makeInvalid();
    const auto stageID = jStageID.is_number_integer() ?
        StageID::fromValue(jStageID.get<StageID::Type>()) : StageID::makeInvalid();

    SmallVector<FighterID, 2> fighterIDs;
    SmallVector<Costume, 2> costumes;
    SmallVector<String, 2> tags;
    int fighterCount = 0;
    for (auto& info : jPlayerInfo)
    {
        json jTag = info["tag"];
        json jFighterID = info["fighterid"];
        json jCostume = info["costume"];

        fighterIDs.push(jFighterID.is_number_integer() ?
            FighterID::fromValue(jFighterID.get<FighterID::Type>()) : FighterID::makeInvalid());
        costumes.push(jCostume.is_number_integer() ?
            Costume::fromValue(jCostume.get<Costume::Type>()) : Costume::makeDefault());
        tags.emplace(jTag.is_string() ?
            jTag.get<std::string>().c_str() : (std::string("Player ") + std::to_string(fighterCount)).c_str());

        fighterCount++;
    }

    if (type == "game")
    {
        json jWinner = jGameInfo["winner"];
        int winner = jWinner.is_number_unsigned() ?
                    jWinner.get<int>() : -1;
        if (winner > fighterCount)
            winner = -1;

        GameMetadata* g = Metadata::newSavedGameSession(
            timeStarted,
            timeEnded,
            stageID,
            std::move(fighterIDs),
            std::move(costumes),
            std::move(tags),
            winner)->asGame();

        json jTournament = j["tournament"];

        json jTournamentName = jTournament["name"];
        if (jTournamentName.is_string())
            g->setTournamentName(jTournamentName.get<std::string>().c_str());

        json jTournamentWebsite = jTournament["website"];
        if (jTournamentWebsite.is_string())
            g->setTournamentWebsite(jTournamentWebsite.get<std::string>().c_str());

        for (auto& jOrganizer : jTournament["organizers"])
        {
            json jName = jOrganizer["name"];
            json jSocial = jOrganizer["social"];
            json jPronouns = jOrganizer["pronouns"];

            const auto name = jName.is_string() ? jName.get<std::string>() : "";
            const auto social = jSocial.is_string() ? jSocial.get<std::string>() : "";
            const auto pronouns = jPronouns.is_string() ? jPronouns.get<std::string>() : "";

            g->addTournamentOrganizer(name.c_str(), social.c_str(), pronouns.c_str());
        }

        for (auto& jSponsor : jTournament["sponsors"])
        {
            json jName = jSponsor["name"];
            json jWebsite = jSponsor["website"];

            const auto name = jName.is_string() ? jName.get<std::string>() : "";
            const auto website = jWebsite.is_string() ? jWebsite.get<std::string>() : "";

            g->addSponsor(name.c_str(), website.c_str());
        }

        for (auto& jCommentator : j["commentators"])
        {
            json jName = jCommentator["name"];
            json jSocial = jCommentator["social"];
            json jPronouns = jCommentator["pronouns"];

            const auto name = jName.is_string() ? jName.get<std::string>() : "";
            const auto social = jSocial.is_string() ? jSocial.get<std::string>() : "";
            const auto pronouns = jPronouns.is_string() ? jPronouns.get<std::string>() : "";

            g->addCommentator(name.c_str(), social.c_str(), pronouns.c_str());
        }

        json jEvent = j["event"];

        json jBracketType = jEvent["type"];
        const auto bracketType = jBracketType.is_string() ?
            BracketType::fromDescription(jBracketType.get<std::string>().c_str()) :
            BracketType::fromType(BracketType::FRIENDLIES);
        g->setBracketType(bracketType);
        switch (bracketType.type())
        {
            case BracketType::SINGLES:
            case BracketType::DOUBLES:
            case BracketType::AMATEURS:
            case BracketType::SIDE: {
                json jBracketURL = jEvent["url"];
                if (jBracketURL.is_string())
                    g->setBracketURL(jBracketURL.get<std::string>().c_str());
            } break;

            case BracketType::PRACTICE:
            case BracketType::FRIENDLIES:
            case BracketType::MONEYMATCH:
            case BracketType::OTHER:
                break;
        }

        json jRound = jGameInfo["round"];
        const auto round = jRound.is_string() ?
            Round::fromDescription(jRound.get<std::string>().c_str()) :
            Round::makeFree();
        g->setRound(round);

        json jFormat = jGameInfo["format"];
        const auto format = jFormat.is_string() ?
            SetFormat::fromDescription(jFormat.get<std::string>().c_str()) :
            SetFormat::fromType(SetFormat::FREE);
        g->setSetFormat(format);

        json jScore1 = jGameInfo["score1"];
        json jScore2 = jGameInfo["score2"];
        const auto score = jScore1.is_number_unsigned() && jScore2.is_number_unsigned() ?
            ScoreCount::fromScore(jScore1.get<int>(), jScore2.get<int>()) :
            ScoreCount::fromGameNumber(GameNumber::fromValue(1));
        g->setScore(score);

        for (int i = 0; i != jPlayerInfo.size(); ++i)
        {
            json jName = jPlayerInfo[i]["name"];
            if (jName.is_string())
                g->setPlayerName(i, jName.get<std::string>().c_str());

            json jSponsor = jPlayerInfo[i]["sponsor"];
            if (jSponsor.is_string())
                g->setPlayerSponsor(i, jSponsor.get<std::string>().c_str());

            json jSocial = jPlayerInfo[i]["social"];
            if (jSocial.is_string())
                g->setPlayerSocial(i, jSocial.get<std::string>().c_str());

            json jPronouns = jPlayerInfo[i]["pronouns"];
            if (jPronouns.is_string())
                g->setPlayerPronouns(i, jPronouns.get<std::string>().c_str());

            json jLoserSide = jPlayerInfo[i]["loserside"];
            if (jLoserSide.is_boolean())
                g->setPlayerIsLoserSide(i, jLoserSide.get<bool>());
        }

        return g;
    }
    if (type == "training")
    {
        TrainingMetadata* t = Metadata::newSavedTrainingSession(
            timeStarted,
            timeEnded,
            stageID,
            std::move(fighterIDs),
            std::move(costumes),
            std::move(tags))->asTraining();

        json jNumber = jGameInfo["number"];
        if (jNumber.is_number_integer())
            t->setSessionNumber(SessionNumber::fromValue(jNumber.get<SessionNumber::Type>()));

        return t;
    }

    return nullptr;
}

// ----------------------------------------------------------------------------
uint32_t Metadata::save(FILE* fp) const
{
    PROFILE(Metadata, save);

    json jPlayerInfo = json::array();
    for (int i = 0; i != fighterCount(); ++i)
    {
        jPlayerInfo += {
            {"tag", playerTag(i).cStr()},
            {"fighterid", playerFighterID(i).value()},
            {"costume", playerCostume(i).slot()}
        };

        switch (type())
        {
            case GAME: {
                auto mdata = asGame();
                jPlayerInfo[i]["name"] = mdata->playerName(i).cStr();
                jPlayerInfo[i]["sponsor"] = mdata->playerSponsor(i).cStr();
                jPlayerInfo[i]["social"] = mdata->playerSocial(i).cStr();
                jPlayerInfo[i]["pronouns"] = mdata->playerPronouns(i).cStr();
                jPlayerInfo[i]["loserside"] = mdata->playerIsLoserSide(i);
            } break;

            case TRAINING: break;
        }
    }

    json jGameInfo = {
        {"stageid", stageID_.value()},
        {"timestampstart", timeStarted_.millisSinceEpoch()},
        {"timestampend", timeEnded_.millisSinceEpoch()},
    };

    switch (type())
    {
        case GAME: {
            auto mdata = asGame();
            jGameInfo["round"] = mdata->round().shortDescription().cStr();
            jGameInfo["format"] = mdata->setFormat().shortDescription();
            jGameInfo["score1"] = mdata->score().left();
            jGameInfo["score2"] = mdata->score().right();
            jGameInfo["winner"] = mdata->winner();
        } break;

        case TRAINING: {
            auto mdata = asTraining();
            jGameInfo["session"] = mdata->sessionNumber().value();
        } break;
    }

    auto sessionTypeToString = [](rfcommon::Metadata::Type type) -> const char* {
        switch (type)
        {
            case rfcommon::Metadata::GAME: return "game";
            case rfcommon::Metadata::TRAINING: return "training";
        }
        return "";
    };

    json j = {
        {"version", "1.7"},
        {"type", sessionTypeToString(type())},
        {"playerinfo", jPlayerInfo},
        {"gameinfo", jGameInfo}
    };

    switch (type())
    {
        case GAME: {
            auto g = asGame();

            auto jOrganizers = json::array();
            for (int i = 0; i != g->tournamentOrganizerCount(); ++i)
            {
                jOrganizers += {
                    {"name", g->tournamentOrganizerName(i).cStr()},
                    {"social", g->tournamentOrganizerSocial(i).cStr()},
                    {"pronouns", g->tournamentOrganizerPronouns(i).cStr()}
                };
            }

            auto jSponsors = json::array();
            for (int i = 0; i != g->sponsorCount(); ++i)
            {
                jSponsors += {
                    {"name", g->sponsorName(i).cStr()},
                    {"website", g->sponsorWebsite(i).cStr()}
                };
            }

            json jCommentators = json::array();
            for (int i = 0; i != g->commentatorCount(); ++i)
            {
                jCommentators += {
                    {"name", g->commentatorName(i).cStr()},
                    {"social", g->commentatorSocial(i).cStr()},
                    {"pronouns", g->commentatorPronouns(i).cStr()}
                };
            }

            json jEvent = {
                {"type", g->bracketType().description()}
            };
            switch (g->bracketType().type())
            {
                case BracketType::SINGLES:
                case BracketType::DOUBLES:
                case BracketType::AMATEURS:
                case BracketType::SIDE:
                    jEvent["url"] = g->bracketURL().cStr();
                    break;

                case BracketType::PRACTICE:
                case BracketType::FRIENDLIES:
                case BracketType::MONEYMATCH:
                case BracketType::OTHER:
                    break;
            };

            j["commentators"] = jCommentators;
            j["tournament"] = {
                {"name", g->tournamentName().cStr()},
                {"website", g->tournamentWebsite().cStr()},
                {"organizers", jOrganizers},
                {"sponsors", jSponsors}
            };
            j["event"] = jEvent;
        } break;

        case TRAINING: break;
    }

    const std::string jsonAsString = j.dump();
    if (fwrite(jsonAsString.data(), 1, jsonAsString.length(), fp) != jsonAsString.length())
        return 0;

    return jsonAsString.length();
}

// ----------------------------------------------------------------------------
void Metadata::setTimeStarted(TimeStamp timeStamp)
{
    PROFILE(Metadata, setTimeStarted);

    bool notify = (timeStarted_ == timeStamp);
    timeStarted_ = timeStamp;
    if (notify)
        dispatcher.dispatch(&MetadataListener::onMetadataTimeChanged, timeStamp, timeEnded_);
}

// ----------------------------------------------------------------------------
void Metadata::setTimeEnded(TimeStamp timeStamp)
{
    PROFILE(Metadata, setTimeEnded);

    bool notify = (timeEnded_ == timeStamp);
    timeEnded_ = timeStamp;
    if (notify)
        dispatcher.dispatch(&MetadataListener::onMetadataTimeChanged, timeStarted_, timeStamp);
}

// ----------------------------------------------------------------------------
DeltaTime Metadata::length() const
{
    NOPROFILE();

    return timeEnded_ - timeStarted_;
}

}
