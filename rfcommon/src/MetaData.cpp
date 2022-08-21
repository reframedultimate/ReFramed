#include "rfcommon/MetaData.hpp"
#include "rfcommon/MetaDataListener.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/time.h"
#include "nlohmann/json.hpp"

namespace rfcommon {

using nlohmann::json;

static MetaData* load_1_5(json& j);

// ----------------------------------------------------------------------------
MetaData* MetaData::newActiveGameSession(
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<String, 2>&& tags,
        SmallVector<String, 2>&& names)
{
    PROFILE(MetaData, newActiveGameSession);

    const auto now = TimeStamp::fromMillisSinceEpoch(
        time_milli_seconds_since_epoch());

    return new GameMetaData(
            now, now,
            stageID,
            std::move(fighterIDs),
            std::move(tags),
            std::move(names),
            GameNumber::fromValue(1),
            SetNumber::fromValue(1),
            SetFormat::fromType(SetFormat::FRIENDLIES),
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
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<String, 2>&& tags,
        SmallVector<String, 2>&& names,
        GameNumber gameNumber,
        SetNumber setNumber,
        SetFormat setFormat,
        int winner)
{
    PROFILE(MetaData, newSavedGameSession);

    return new GameMetaData(
        timeStarted,
        timeEnded,
        stageID,
        std::move(fighterIDs),
        std::move(tags),
        std::move(names),
        gameNumber,
        setNumber,
        setFormat,
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
MetaData* MetaData::load(const void* data, uint32_t size)
{
    PROFILE(MetaData, load);

    // Parse
    const unsigned char* const begin = static_cast<const unsigned char*>(data);
    const unsigned char* const end = static_cast<const unsigned char*>(data) + size;
    json j = json::parse(begin, end, nullptr, false);
    if (j == json::value_t::discarded)
        return nullptr;

    if (j["version"] == "1.5")
        return load_1_5(j);

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

        return MetaData::newSavedGameSession(
            timeStarted, timeEnded, stageID, std::move(fighterIDs), std::move(tags),
            std::move(names), gameNumber, setNumber, format, winner);
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
            {"fighterid", fighterID(i).value()}
        };
    }

    json jGameInfo = {
        {"stageid", stageID_.value()},
        {"timestampstart", timeStarted_.millisSinceEpoch()},
        {"timestampend", timeEnded_.millisSinceEpoch()},
    };
    if (type() == GAME)
    {
        const GameMetaData* meta = static_cast<const GameMetaData*>(this);
        jGameInfo["format"] = meta->setFormat().shortDescription();
        jGameInfo["number"] = meta->gameNumber().value();
        jGameInfo["set"] = meta->setNumber().value();
        jGameInfo["winner"] = meta->winner();
    }
    else
    {
        assert(type() == TRAINING);
        const TrainingMetaData* meta = static_cast<const TrainingMetaData*>(this);
        jGameInfo["number"] = meta->sessionNumber().value();
    }

    json videoInfo = {
        {"filename", ""},
        {"filepath", ""},
        {"offsetms", ""}
    };

    json j = {
        {"version", "1.5"},
        {"type", type() == GAME ? "game" : "training"},
        {"gameinfo", jGameInfo},
        {"playerinfo", jPlayerInfo},
        {"videoinfo", videoInfo},
    };

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

// ----------------------------------------------------------------------------
GameMetaData::GameMetaData(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<String, 2>&& tags,
        SmallVector<String, 2>&& names,
        GameNumber gameNumber,
        SetNumber setNumber,
        SetFormat setFormat,
        int winner)
    : MetaData(timeStarted, timeEnded, stageID, std::move(fighterIDs), std::move(tags))
    , names_(std::move(names))
    , gameNumber_(gameNumber)
    , setNumber_(setNumber)
    , setFormat_(setFormat)
    , winner_(winner)
{}

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

    return names_[playerIdx];
}

// ----------------------------------------------------------------------------
void GameMetaData::setName(int fighterIdx, const String& name)
{
    PROFILE(GameMetaData, setName);

    bool notify = (names_[fighterIdx] != name);
    names_[fighterIdx] = name;
    if (notify)
        dispatcher.dispatch(&MetaDataListener::onMetaDataPlayerNameChanged, fighterIdx, name);
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

// ----------------------------------------------------------------------------
TrainingMetaData::TrainingMetaData(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<String, 2>&& tags,
        GameNumber sessionNumber)
    : MetaData(timeStarted, timeEnded, stageID, std::move(fighterIDs), std::move(tags))
    , sessionNumber_(sessionNumber)
{}

// ----------------------------------------------------------------------------
MetaData::Type TrainingMetaData::type() const
{
    NOPROFILE();

    return TRAINING;
}

// ----------------------------------------------------------------------------
const String& TrainingMetaData::name(int playerIdx) const
{
    NOPROFILE();

    return tag(playerIdx);
}

// ----------------------------------------------------------------------------
FighterID TrainingMetaData::playerFighterID() const
{
    NOPROFILE();

    return fighterID(0);
}

// ----------------------------------------------------------------------------
FighterID TrainingMetaData::cpuFighterID() const
{
    NOPROFILE();

    return fighterID(1);
}

// ----------------------------------------------------------------------------
GameNumber TrainingMetaData::sessionNumber() const
{
    NOPROFILE();

    return sessionNumber_;
}

// ----------------------------------------------------------------------------
void TrainingMetaData::setSessionNumber(GameNumber sessionNumber)
{
    PROFILE(TrainingMetaData, setSessionNumber);

    bool notify = (sessionNumber_ != sessionNumber);
    sessionNumber_ = sessionNumber;
    if (notify)
        dispatcher.dispatch(&MetaDataListener::onMetaDataTrainingSessionNumberChanged, sessionNumber);
}

// ----------------------------------------------------------------------------
void TrainingMetaData::resetSessionNumber()
{
    PROFILE(TrainingMetaData, resetSessionNumber);

    setSessionNumber(GameNumber::fromValue(1));
}

}
