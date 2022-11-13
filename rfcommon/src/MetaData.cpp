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

// ----------------------------------------------------------------------------
MetaData* MetaData::newActiveGameSession(
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<String, 2>&& tags)
{
    PROFILE(MetaData, newActiveGameSession);

    const auto now = TimeStamp::fromMillisSinceEpoch(
        time_milli_seconds_since_epoch());

    auto names = SmallVector<String, 2>::makeResized(fighterIDs.count());
    auto sponsors = SmallVector<String, 2>::makeResized(fighterIDs.count());
    auto pronouns = SmallVector<String, 2>::makeResized(fighterIDs.count());

    return new GameMetaData(
            now, now,  // Start, end
            stageID,
            EventType::fromType(EventType::FRIENDLIES),
            Round::makeFree(),
            SetFormat::fromType(SetFormat::FREE),
            ScoreCount::fromScore(0, 0),
            std::move(fighterIDs),
            std::move(tags),
            std::move(names),
            std::move(sponsors),
            std::move(pronouns),
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
            SessionNumber::fromValue(1),
            std::move(fighterIDs),
            std::move(tags));
}

// ----------------------------------------------------------------------------
MetaData* MetaData::newSavedGameSession(
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
{
    PROFILE(MetaData, newSavedGameSession);

    return new GameMetaData(
        timeStarted,
        timeEnded,
        stageID,
        eventType,
        round,
        format,
        score,
        std::move(fighterIDs),
        std::move(tags),
        std::move(names),
        std::move(sponsors),
        std::move(pronouns),
        winner);
}

// ----------------------------------------------------------------------------
MetaData* MetaData::newSavedTrainingSession(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SessionNumber sessionNumber,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<String, 2>&& tags)
{
    PROFILE(MetaData, newSavedTrainingSession);

    return new TrainingMetaData(
        timeStarted,
        timeEnded,
        stageID,
        sessionNumber,
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
    SmallVector<String, 2> names;
    int fighterCount = 0;
    for (const auto& info : jPlayerInfo)
    {
        json jTag = info["tag"];
        json jName = info["name"];
        json jFighterID = info["fighterid"];

        fighterIDs.push(jFighterID.is_number_integer() ?
            FighterID::fromValue(jFighterID.get<FighterID::Type>()) : FighterID::makeInvalid());
        tags.emplace(jTag.is_string() ?
            jTag.get<std::string>().c_str() : (std::string("Player ") + std::to_string(fighterCount)).c_str());
        names.emplace(jName.is_string() ?
            jName.get<std::string>().c_str() : (std::string("Player ") + std::to_string(fighterCount)).c_str());

        fighterCount++;
    }

    json jType = j["type"];
    const std::string type = jType.is_string() ? jType.get<std::string>() : "";
    if (type == "game")
    {
        json jNumber = jGameInfo["number"];
        json jSet = jGameInfo["set"];
        json jFormat = jGameInfo["format"];
        json jWinner = jGameInfo["winner"];
        const auto gameNumber = jNumber.is_number_integer() ?
            GameNumber::fromValue(jNumber.get<GameNumber::Type>()) : GameNumber::fromValue(1);
        const auto sessionNumber = jSet.is_number_integer() ?
            SessionNumber::fromValue(jSet.get<SessionNumber::Type>()) : SessionNumber::fromValue(1);
        const auto format = jFormat.is_string() ?
            SetFormat::fromDescription(jFormat.get<std::string>().c_str()) : SetFormat::fromType(SetFormat::FREE);
        int winner = jWinner.is_number_unsigned() ?
            jWinner.get<int>() : -1;
        if (winner > fighterCount)
            winner = -1;

        // This data isn't available in 1.5
        auto sponsors = SmallVector<String, 2>::makeResized(fighterIDs.count());
        auto pronouns = SmallVector<String, 2>::makeResized(fighterIDs.count());

        return MetaData::newSavedGameSession(
            timeStarted, timeEnded,
            stageID,
            EventType::fromType(EventType::FRIENDLIES),
            Round::fromSessionNumber(sessionNumber),
            format,
            ScoreCount::fromGameNumber(gameNumber),
            std::move(fighterIDs),
            std::move(tags),
            std::move(names),
            std::move(sponsors),
            std::move(pronouns),
            winner);
    }
    if (type == "training")
    {
        json jNumber = jGameInfo["number"];
        const auto sessionNumber = jNumber.is_number_integer() ?
            SessionNumber::fromValue(jNumber.get<SessionNumber::Type>()) : SessionNumber::fromValue(1);

        return MetaData::newSavedTrainingSession(
            timeStarted,
            timeEnded,
            stageID,
            sessionNumber,
            std::move(fighterIDs),
            std::move(tags));
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
    SmallVector<String, 2> names;
    SmallVector<String, 2> sponsors;
    SmallVector<String, 2> pronouns;
    int fighterCount = 0;
    for (const auto& info : jPlayerInfo)
    {
        json jTag = info["tag"];
        json jName = info["name"];
        json jSponsor = info["sponsor"];
        json jFighterID = info["fighterid"];

        fighterIDs.push(jFighterID.is_number_integer() ?
            FighterID::fromValue(jFighterID.get<FighterID::Type>()) : FighterID::makeInvalid());
        tags.emplace(jTag.is_string() ?
            jTag.get<std::string>().c_str() : (std::string("Player ") + std::to_string(fighterCount)).c_str());
        sponsors.emplace(jSponsor.is_string() ?
            jSponsor.get<std::string>().c_str() : "");
        names.emplace(jName.is_string() ?
            jName.get<std::string>().c_str() : (std::string("Player ") + std::to_string(fighterCount)).c_str());
        pronouns.emplace("");

        fighterCount++;
    }

    json jType = j["type"];
    const std::string type = jType.is_string() ? jType.get<std::string>() : "";
    if (type == "game")
    {
        json jTournamentInfo = j["tournamentinfo"];
        json jEventInfo = j["eventinfo"];
        json jCommentators = j["commentators"];

        json jTournamentName = jTournamentInfo["name"];
        const auto tournamentName = jTournamentName.is_string() ? jTournamentName.get<std::string>() : "";

        json jEventName = jEventInfo["name"];
        const auto eventName = jEventName.is_string() ? jEventName.get<std::string>() : "";

        rfcommon::SmallVector<rfcommon::String, 2> commentators;
        for (const auto& commentator : jCommentators)
            if (commentator.is_string())
                commentators.push(commentator.get<std::string>().c_str());

        json jNumber = jGameInfo["number"];
        json jSet = jGameInfo["set"];
        json jFormat = jGameInfo["format"];
        json jRound = jGameInfo["round"];
        json jWinner = jGameInfo["winner"];

        const auto gameNumber = jNumber.is_number_integer() ?
            GameNumber::fromValue(jNumber.get<GameNumber::Type>()) : GameNumber::fromValue(1);
        const auto sessionNumber = jSet.is_number_integer() ?
            SessionNumber::fromValue(jSet.get<SessionNumber::Type>()) : SessionNumber::fromValue(1);
        const auto format = jFormat.is_string() ?
            SetFormat::fromDescription(jFormat.get<std::string>().c_str()) : SetFormat::fromType(SetFormat::FREE);
        const auto round = jRound.is_string() ?
            jRound.get<std::string>() : std::string("");
        int winner = jWinner.is_number_unsigned() ?
            jWinner.get<int>() : -1;
        if (winner > fighterCount)
            winner = -1;

        return MetaData::newSavedGameSession(
            timeStarted,
            timeEnded,
            stageID,
            EventType::fromType(EventType::FRIENDLIES),
            Round::fromSessionNumber(sessionNumber),
            format,
            ScoreCount::fromGameNumber(gameNumber),
            std::move(fighterIDs),
            std::move(tags),
            std::move(names),
            std::move(sponsors),
            std::move(pronouns),
            winner);
    }
    if (type == "training")
    {
        json jNumber = jGameInfo["number"];
        const auto sessionNumber = jNumber.is_number_integer() ?
            SessionNumber::fromValue(jNumber.get<SessionNumber::Type>()) : SessionNumber::fromValue(1);

        return MetaData::newSavedTrainingSession(
            timeStarted,
            timeEnded,
            stageID,
            sessionNumber,
            std::move(fighterIDs),
            std::move(tags));
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
            jGameInfo["number"] = mdata->score().gameNumber().value();
            jGameInfo["score1"] = mdata->score().left();
            jGameInfo["score2"] = mdata->score().right();
            jGameInfo["winner"] = mdata->winner();
        } break;

        case TRAINING: {
            auto mdata = asTraining();
            jGameInfo["number"] = mdata->sessionNumber().value();
        } break;
    }

    json j = {
        {"version", "1.7"},
        {"type", type() == GAME ? "game" : "training"},
        {"gameinfo", jGameInfo},
        {"playerinfo", jPlayerInfo}
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

            j["commentators"] = jCommentators;
            j["tournament"] = {
                {"name", mdata->tournamentName().cStr()},
                {"website", mdata->tournamentWebsite().cStr()},
                {"organizers", jOrganizers},
                {"sponsors", jSponsors}
            };
            j["event"] = {
                {"type", mdata->eventType().description()},
                {"url", mdata->eventURL().cStr()}
            };
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
