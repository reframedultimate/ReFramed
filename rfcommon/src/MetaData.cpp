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

    auto sponsors = SmallVector<String, 2>::makeResized(fighterIDs.count());
    auto names = tags;  // names = tags initially

    return new GameMetaData(
            "",        // Tourney name
            "",        // Event name
            now, now,  // Start, end
            stageID,
            std::move(fighterIDs),
            std::move(tags),
            std::move(names),
            std::move(sponsors),
            SmallVector<String, 2>(),  // No commentators
            GameNumber::fromValue(1),
            SetNumber::fromValue(1),
            SetFormat::fromType(SetFormat::FRIENDLIES),
            "",  // Round name
            -1);
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
            now, now,
            stageID,
            std::move(fighterIDs),
            std::move(tags),
            GameNumber::fromValue(1));
}

// ----------------------------------------------------------------------------
MetaData* MetaData::newSavedGameSession(
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
{
    PROFILE(MetaData, newSavedGameSession);

    return new GameMetaData(
        tournamentName,
        eventName,
        timeStarted,
        timeEnded,
        stageID,
        std::move(fighterIDs),
        std::move(tags),
        std::move(names),
        std::move(sponsors),
        std::move(commentators),
        gameNumber,
        setNumber,
        setFormat,
        roundName,
        winner);
}

// ----------------------------------------------------------------------------
MetaData* MetaData::newSavedTrainingSession(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<String, 2>&& tags,
        GameNumber sessionNumber)
{
    PROFILE(MetaData, newSavedTrainingSession);

    return new TrainingMetaData(
        timeStarted,
        timeEnded,
        stageID,
        std::move(fighterIDs),
        std::move(tags),
        sessionNumber);
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
        const auto setNumber = jSet.is_number_integer() ?
            SetNumber::fromValue(jSet.get<SetNumber::Type>()) : SetNumber::fromValue(1);
        const auto format = jFormat.is_string() ?
            SetFormat::fromDescription(jFormat.get<std::string>().c_str()) : SetFormat::fromType(SetFormat::FRIENDLIES);
        int winner = jWinner.is_number_unsigned() ?
            jWinner.get<int>() : -1;
        if (winner > fighterCount)
            winner = -1;

        auto sponsors = SmallVector<String, 2>::makeResized(fighterIDs.count());
        auto commentators = SmallVector<String, 2>::makeResized(fighterIDs.count());

        return MetaData::newSavedGameSession(
            "", "",
            timeStarted, timeEnded, stageID, 
            std::move(fighterIDs), std::move(tags), std::move(names), std::move(sponsors), 
            std::move(commentators), gameNumber, setNumber, format, "", winner);
    }
    if (type == "training")
    {
        json jNumber = jGameInfo["number"];
        const auto sessionNumber = jNumber.is_number_integer() ?
            GameNumber::fromValue(jNumber.get<GameNumber::Type>()) : GameNumber::fromValue(1);

        return MetaData::newSavedTrainingSession(
            timeStarted, timeEnded, stageID, std::move(fighterIDs), std::move(tags), sessionNumber);
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
        const auto setNumber = jSet.is_number_integer() ?
            SetNumber::fromValue(jSet.get<SetNumber::Type>()) : SetNumber::fromValue(1);
        const auto format = jFormat.is_string() ?
            SetFormat::fromDescription(jFormat.get<std::string>().c_str()) : SetFormat::fromType(SetFormat::FRIENDLIES);
        const auto round = jRound.is_string() ?
            jRound.get<std::string>() : std::string("");
        int winner = jWinner.is_number_unsigned() ?
            jWinner.get<int>() : -1;
        if (winner > fighterCount)
            winner = -1;

        return MetaData::newSavedGameSession(
            tournamentName.c_str(), eventName.c_str(),
            timeStarted, timeEnded, stageID, std::move(fighterIDs), std::move(tags),
            std::move(names), std::move(sponsors), std::move(commentators),
            gameNumber, setNumber, format, round.c_str(), winner);
    }
    if (type == "training")
    {
        json jNumber = jGameInfo["number"];
        const auto sessionNumber = jNumber.is_number_integer() ?
            GameNumber::fromValue(jNumber.get<GameNumber::Type>()) : GameNumber::fromValue(1);

        return MetaData::newSavedTrainingSession(
            timeStarted, timeEnded, stageID, std::move(fighterIDs), std::move(tags), sessionNumber);
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
            {"tag", tag(i).cStr()},
            {"name", name(i).cStr()},
            {"sponsor", sponsor(i).cStr()},
            {"fighterid", fighterID(i).value()}
        };
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
            jGameInfo["format"] = mdata->setFormat().shortDescription();
            jGameInfo["number"] = mdata->gameNumber().value();
            jGameInfo["set"] = mdata->setNumber().value();
            jGameInfo["round"] = mdata->roundName().cStr();
            jGameInfo["winner"] = mdata->winner();
        } break;

        case TRAINING: {
            assert(type() == TRAINING);
            auto mdata = asTraining();
            jGameInfo["number"] = mdata->sessionNumber().value();
        } break;
    }

    json j = {
        {"version", "1.6"},
        {"type", type() == GAME ? "game" : "training"},
        {"gameinfo", jGameInfo},
        {"playerinfo", jPlayerInfo}
    };

    switch (type())
    {
        case GAME: {
            auto mdata = asGame();

            j["tournamentinfo"] = {
                {"name", mdata->tournamentName().cStr()}
            };

            j["eventinfo"] = {
                {"name", mdata->eventName().cStr()}
            };

            json jCommentators = json::array();
            for (const auto& name : mdata->commentators())
                jCommentators.push_back(name.cStr());
            j["commentators"] = jCommentators;
        } break;

        case TRAINING: {

        } break;
    }

    const std::string jsonAsString = j.dump();
    if (fwrite(jsonAsString.data(), 1, jsonAsString.length(), fp) != jsonAsString.length())
        return 0;

    return jsonAsString.length();
}

// ----------------------------------------------------------------------------
int MetaData::fighterCount() const
{
    NOPROFILE();

    return fighterIDs_.count();
}

// ----------------------------------------------------------------------------
const String& MetaData::tag(int fighterIdx) const
{
    NOPROFILE();

    return tags_[fighterIdx];
}

// ----------------------------------------------------------------------------
FighterID MetaData::fighterID(int fighterIdx) const
{
    NOPROFILE();

    return fighterIDs_[fighterIdx];
}

// ----------------------------------------------------------------------------
StageID MetaData::stageID() const
{
    NOPROFILE();

    return stageID_;
}

// ----------------------------------------------------------------------------
TimeStamp MetaData::timeStarted() const
{
    NOPROFILE();

    return timeStarted_;
}

// ----------------------------------------------------------------------------
void MetaData::setTimeStarted(TimeStamp timeStamp)
{
    PROFILE(MetaData, setTimeStarted);

    bool notify = (timeStarted_ == timeStamp);
    timeStarted_ = timeStamp;
    if (notify)
        dispatcher.dispatch(&MetaDataListener::onMetaDataTimeStartedChanged, timeStamp);
}

// ----------------------------------------------------------------------------
TimeStamp MetaData::timeEnded() const
{
    NOPROFILE();

    return timeEnded_;
}

// ----------------------------------------------------------------------------
void MetaData::setTimeEnded(TimeStamp timeStamp)
{
    PROFILE(MetaData, setTimeEnded);

    bool notify = (timeEnded_ == timeStamp);
    timeEnded_ = timeStamp;
    if (notify)
        dispatcher.dispatch(&MetaDataListener::onMetaDataTimeEndedChanged, timeStamp);
}

// ----------------------------------------------------------------------------
DeltaTime MetaData::length() const
{
    NOPROFILE();

    return timeEnded_ - timeStarted_;
}

}
