#include "rfcommon/MetaData.hpp"
#include "rfcommon/GameMetaData.hpp"
#include "rfcommon/TrainingMetaData.hpp"
#include "rfcommon/MetaDataListener.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/time.h"
#include "nlohmann/json.hpp"

namespace rfcommon {

using nlohmann::json;

static MetaData* load_1_5(json& j);
static MetaData* load_1_6(json& j);
static MetaData* load_1_7(json& j);

// ----------------------------------------------------------------------------
MetaData* MetaData::newActiveGameSession(
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<String, 2>&& tags)
{
    PROFILE(MetaData, newActiveGameSession);

    const auto now = TimeStamp::fromMillisSinceEpoch(
        time_milli_seconds_since_epoch());

    return new GameMetaData(
            now, now,  // Start, end
            stageID,
            std::move(fighterIDs),
            std::move(tags),
            -1);  // winner
}

// ----------------------------------------------------------------------------
MetaData* MetaData::newActiveTrainingSession(
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<String, 2>&& tags)
{
    PROFILE(MetaData, newActiveTrainingSession);

    const auto now = TimeStamp::fromMillisSinceEpoch(
        time_milli_seconds_since_epoch());

    return new TrainingMetaData(
            now, now,  // start, end
            stageID,
            std::move(fighterIDs),
            std::move(tags));
}

// ----------------------------------------------------------------------------
MetaData* MetaData::newSavedGameSession(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<String, 2>&& tags,
        int winner)
{
    PROFILE(MetaData, newSavedGameSession);

    return new GameMetaData(
        timeStarted,
        timeEnded,
        stageID,
        std::move(fighterIDs),
        std::move(tags),
        winner);
}

// ----------------------------------------------------------------------------
MetaData* MetaData::newSavedTrainingSession(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<String, 2>&& tags)
{
    PROFILE(MetaData, newSavedTrainingSession);

    return new TrainingMetaData(
        timeStarted,
        timeEnded,
        stageID,
        std::move(fighterIDs),
        std::move(tags));
}

// ----------------------------------------------------------------------------
MetaData::MetaData(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<String, 2>&& tags)
    : timeStarted_(timeStarted)
    , timeEnded_(timeEnded)
    , fighterIDs_(std::move(fighterIDs))
    , tags_(std::move(tags))
    , stageID_(stageID)
{
}

// ----------------------------------------------------------------------------
MetaData::~MetaData()
{}

// ----------------------------------------------------------------------------
GameMetaData* MetaData::asGame()
{
    assert(type() == GAME);
    return static_cast<GameMetaData*>(this);
}

// ----------------------------------------------------------------------------
const GameMetaData* MetaData::asGame() const
{
    assert(type() == GAME);
    return static_cast<const GameMetaData*>(this);
}

// ----------------------------------------------------------------------------
TrainingMetaData* MetaData::asTraining()
{
    assert(type() == TRAINING);
    return static_cast<TrainingMetaData*>(this);
}

// ----------------------------------------------------------------------------
const TrainingMetaData* MetaData::asTraining() const
{
    assert(type() == TRAINING);
    return static_cast<const TrainingMetaData*>(this);
}

// ----------------------------------------------------------------------------
MetaData* MetaData::load(const void* data, uint32_t size)
{
    PROFILE(MetaData, load);

    // Parse
    const unsigned char* const begin = static_cast<const unsigned char*>(data);
    const unsigned char* const end = static_cast<const unsigned char*>(data) + size;
    json j = json::parse(begin, end, nullptr, false);
    if (j.is_discarded())
        return nullptr;

    if (j["version"] == "1.5")
        return load_1_5(j);
    if (j["version"] == "1.6")
        return load_1_6(j);
    if (j["version"] == "1.7")
        return load_1_7(j);

    // unsupported version
    return nullptr;
}
static MetaData* load_1_5(json& j)
{
    PROFILE(MetaDataGlobal, load_1_5);

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
    SmallVector<String, 2> tags;
    int fighterCount = 0;
    for (const auto& info : jPlayerInfo)
    {
        json jTag = info["tag"];
        json jFighterID = info["fighterid"];

        fighterIDs.push(jFighterID.is_number_integer() ?
            FighterID::fromValue(jFighterID.get<FighterID::Type>()) : FighterID::makeInvalid());
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

        GameMetaData* g = MetaData::newSavedGameSession(
            timeStarted, timeEnded,
            stageID,
            std::move(fighterIDs),
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
        TrainingMetaData* t = MetaData::newSavedTrainingSession(
            timeStarted,
            timeEnded,
            stageID,
            std::move(fighterIDs),
            std::move(tags))->asTraining();

        json jNumber = jGameInfo["number"];
        if (jNumber.is_number_integer())
            t->setSessionNumber(SessionNumber::fromValue(jNumber.get<SessionNumber::Type>()));

        return t;
    }

    return nullptr;
}
static MetaData* load_1_6(json& j)
{
    PROFILE(MetaDataGlobal, load_1_6);

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
    SmallVector<String, 2> tags;
    int fighterCount = 0;
    for (const auto& info : jPlayerInfo)
    {
        json jTag = info["tag"];
        json jFighterID = info["fighterid"];

        fighterIDs.push(jFighterID.is_number_integer() ?
            FighterID::fromValue(jFighterID.get<FighterID::Type>()) : FighterID::makeInvalid());
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

        GameMetaData* g = MetaData::newSavedGameSession(
            timeStarted,
            timeEnded,
            stageID,
            std::move(fighterIDs),
            std::move(tags),
            winner)->asGame();

        json jTournamentInfo = j["tournamentinfo"];
        json jTournamentName = jTournamentInfo["name"];
        if (jTournamentName.is_string())
            g->setTournamentName(jTournamentName.get<std::string>().c_str());

        json jCommentators = j["commentators"];
        for (const auto& commentator : jCommentators)
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
        TrainingMetaData* t = MetaData::newSavedTrainingSession(
            timeStarted,
            timeEnded,
            stageID,
            std::move(fighterIDs),
            std::move(tags))->asTraining();

        json jNumber = jGameInfo["number"];
        if (jNumber.is_number_integer())
            t->setSessionNumber(SessionNumber::fromValue(jNumber.get<SessionNumber::Type>()));

        return t;
    }

    return nullptr;
}
static MetaData* load_1_7(json& j)
{
    PROFILE(MetaDataGlobal, load_1_7);

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
    SmallVector<String, 2> tags;
    int fighterCount = 0;
    for (const auto& info : jPlayerInfo)
    {
        json jTag = info["tag"];
        json jFighterID = info["fighterid"];

        fighterIDs.push(jFighterID.is_number_integer() ?
            FighterID::fromValue(jFighterID.get<FighterID::Type>()) : FighterID::makeInvalid());
        tags.emplace(jTag.is_string() ?
            jTag.get<std::string>().c_str() : (std::string("Player ") + std::to_string(fighterCount)).c_str());

        fighterCount++;
    }

    if (type == "game")
    {
        json jEvent = j["event"];
        json jBracketType = jEvent["type"];
        const auto bracketType = jBracketType.is_string() ?
                    BracketType::fromDescription(jBracketType.get<std::string>().c_str()) :
                    BracketType::fromType(BracketType::FRIENDLIES);

        json jRound = jGameInfo["round"];
        json jFormat = jGameInfo["format"];
        json jScore1 = jGameInfo["score1"];
        json jScore2 = jGameInfo["score2"];
        json jWinner = jGameInfo["winner"];

        const auto round = jRound.is_string() ?
                    Round::fromDescription(jRound.get<std::string>().c_str()) :
                    Round::makeFree();
        const auto format = jFormat.is_string() ?
                    SetFormat::fromDescription(jFormat.get<std::string>().c_str()) :
                    SetFormat::fromType(SetFormat::FREE);
        const auto score = jScore1.is_number_unsigned() && jScore2.is_number_unsigned() ?
                    ScoreCount::fromScore(jScore1.get<int>(), jScore2.get<int>()) :
                    ScoreCount::fromGameNumber(GameNumber::fromValue(1));
        int winner = jWinner.is_number_unsigned() ?
                    jWinner.get<int>() : -1;
        if (winner > fighterCount)
            winner = -1;

        SmallVector<String, 2> names;
        SmallVector<String, 2> sponsors;
        SmallVector<String, 2> socials;
        SmallVector<String, 2> pronouns;
        for (const auto& info : jPlayerInfo)
        {
            json jName = info["name"];
            json jSponsor = info["sponsor"];

            sponsors.emplace(jSponsor.is_string() ?
                jSponsor.get<std::string>().c_str() : "");
            names.emplace(jName.is_string() ?
                jName.get<std::string>().c_str() : (std::string("Player ") + std::to_string(fighterCount)).c_str());
            pronouns.emplace("");
        }

        GameMetaData* g = MetaData::newSavedGameSession(
            timeStarted,
            timeEnded,
            stageID,
            std::move(fighterIDs),
            std::move(tags),
            winner)->asGame();

        json jTournament = j["tournament"];
        json jTournamentName = jTournament["name"];
        json jTournamentWebsite = jTournament["website"];
        if (jTournamentName.is_string())
            g->setTournamentName(jTournamentName.get<std::string>().c_str());
        if (jTournamentWebsite.is_string())
            g->setTournamentWebsite(jTournamentWebsite.get<std::string>().c_str());
        for (const auto& jOrganizer : jTournament)
        {
            json jName = jOrganizer["name"];
            json jSocial = jOrganizer["social"];
            json jPronouns = jOrganizer["pronouns"];

            const auto name = jName.is_string() ? jName.get<std::string>() : "";
            const auto social = jSocial.is_string() ? jSocial.get<std::string>() : "";
            const auto pronouns = jPronouns.is_string() ? jPronouns.get<std::string>() : "";

            g->addTournamentOrganizer(name.c_str(), social.c_str(), pronouns.c_str());
        }
        for (const auto& jSponsor : jTournament["sponsors"])
        {
            json jName = jSponsor["name"];
            json jWebsite = jSponsor["website"];

            const auto name = jName.is_string() ? jName.get<std::string>() : "";
            const auto website = jWebsite.is_string() ? jWebsite.get<std::string>() : "";

            g->addSponsor(name.c_str(), website.c_str());
        }

        rfcommon::SmallVector<rfcommon::String, 2> commentators;
        for (const auto& commentator : j["commentators"])
            if (commentator.is_string())
                commentators.push(commentator.get<std::string>().c_str());

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

        return g;
    }
    if (type == "training")
    {
        TrainingMetaData* t = MetaData::newSavedTrainingSession(
            timeStarted,
            timeEnded,
            stageID,
            std::move(fighterIDs),
            std::move(tags))->asTraining();

        json jNumber = jGameInfo["number"];
        if (jNumber.is_number_integer())
            t->setSessionNumber(SessionNumber::fromValue(jNumber.get<SessionNumber::Type>()));

        return t;
    }

    return nullptr;
}

// ----------------------------------------------------------------------------
uint32_t MetaData::save(FILE* fp) const
{
    PROFILE(MetaData, save);

    json jPlayerInfo = json::array();
    for (int i = 0; i != fighterCount(); ++i)
    {
        jPlayerInfo += {
            {"tag", playerTag(i).cStr()},
            {"fighterid", playerFighterID(i).value()}
        };

        switch (type())
        {
            case GAME: {
                auto mdata = asGame();
                jPlayerInfo[i]["name"] = mdata->playerName(i).cStr();
                jPlayerInfo[i]["sponsor"] = mdata->playerSponsor(i).cStr();
                jPlayerInfo[i]["social"] = mdata->playerSocial(i).cStr();
                jPlayerInfo[i]["pronouns"] = mdata->playerPronouns(i).cStr();
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

    json j = {
        {"version", "1.7"},
        {"type", type() == GAME ? "game" : "training"},
        {"playerinfo", jPlayerInfo},
        {"gameinfo", jGameInfo}
    };

    switch (type())
    {
        case GAME: {
            auto mdata = asGame();

            auto jOrganizers = json::array();
            for (int i = 0; i != mdata->tournamentOrganizerCount(); ++i)
            {
                jOrganizers += {
                    {"name", mdata->tournamentOrganizerName(i).cStr()},
                    {"social", mdata->tournamentOrganizerSocial(i).cStr()},
                    {"pronouns", mdata->tournamentOrganizerPronouns(i).cStr()}
                };
            }

            auto jSponsors = json::array();
            for (int i = 0; i != mdata->sponsorCount(); ++i)
            {
                jSponsors += {
                    {"name", mdata->sponsorName(i).cStr()},
                    {"website", mdata->sponsorWebsite(i).cStr()}
                };
            }

            json jCommentators = json::array();
            for (int i = 0; i != mdata->commentatorCount(); ++i)
            {
                jCommentators += {
                    {"name", mdata->commentatorName(i).cStr()},
                    {"social", mdata->commentatorSocial(i).cStr()},
                    {"pronouns", mdata->commentatorPronouns(i).cStr()}
                };
            }

            json jEvent = {
                {"type", mdata->bracketType().description()}
            };
            switch (mdata->bracketType().type())
            {
                case BracketType::SINGLES:
                case BracketType::DOUBLES:
                case BracketType::AMATEURS:
                case BracketType::SIDE:
                    jEvent["url"] = mdata->bracketURL().cStr();
                    break;

                case BracketType::PRACTICE:
                case BracketType::FRIENDLIES:
                case BracketType::MONEYMATCH:
                case BracketType::OTHER:
                    break;
            };

            j["commentators"] = jCommentators;
            j["tournament"] = {
                {"name", mdata->tournamentName().cStr()},
                {"website", mdata->tournamentWebsite().cStr()},
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
void MetaData::setTimeStarted(TimeStamp timeStamp)
{
    PROFILE(MetaData, setTimeStarted);

    bool notify = (timeStarted_ == timeStamp);
    timeStarted_ = timeStamp;
    if (notify)
        dispatcher.dispatch(&MetaDataListener::onMetaDataTimeChanged, timeStamp, timeEnded_);
}

// ----------------------------------------------------------------------------
void MetaData::setTimeEnded(TimeStamp timeStamp)
{
    PROFILE(MetaData, setTimeEnded);

    bool notify = (timeEnded_ == timeStamp);
    timeEnded_ = timeStamp;
    if (notify)
        dispatcher.dispatch(&MetaDataListener::onMetaDataTimeChanged, timeStarted_, timeStamp);
}

// ----------------------------------------------------------------------------
DeltaTime MetaData::length() const
{
    NOPROFILE();

    return timeEnded_ - timeStarted_;
}

}
