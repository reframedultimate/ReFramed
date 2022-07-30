#include "rfcommon/SessionMetaData.hpp"
#include "rfcommon/SessionMetaDataListener.hpp"
#include "rfcommon/time.h"
#include "nlohmann/json.hpp"

namespace rfcommon {

using nlohmann::json;

static SessionMetaData* load_1_5(const json& j);

// ----------------------------------------------------------------------------
SessionMetaData* SessionMetaData::newActiveGameSession(
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<SmallString<15>, 2>&& tags,
        SmallVector<SmallString<15>, 2>&& names)
{
    const auto now = TimeStamp::fromMillisSinceEpoch(
        time_milli_seconds_since_epoch());

    return new GameSessionMetaData(
            now, now,
            stageID,
            std::move(fighterIDs),
            std::move(tags),
            std::move(names),
            GameNumber::fromValue(1),
            SetNumber::fromValue(1),
            SetFormat::FRIENDLIES,
            -1);
}

// ----------------------------------------------------------------------------
SessionMetaData* SessionMetaData::newActiveTrainingSession(
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<SmallString<15>, 2>&& tags)
{
    const auto now = TimeStamp::fromMillisSinceEpoch(
        time_milli_seconds_since_epoch());

    return new TrainingSessionMetaData(
            now, now,
            stageID,
            std::move(fighterIDs),
            std::move(tags),
            GameNumber::fromValue(1));
}

// ----------------------------------------------------------------------------
SessionMetaData* SessionMetaData::newSavedGameSession(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<SmallString<15>, 2>&& tags,
        SmallVector<SmallString<15>, 2>&& names,
        GameNumber gameNumber,
        SetNumber setNumber,
        SetFormat setFormat,
        int winner)
{
    return new GameSessionMetaData(
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
SessionMetaData* SessionMetaData::newSavedTrainingSession(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<SmallString<15>, 2>&& tags,
        GameNumber sessionNumber)
{
    return new TrainingSessionMetaData(
        timeStarted,
        timeEnded,
        stageID,
        std::move(fighterIDs),
        std::move(tags),
        sessionNumber);
}

// ----------------------------------------------------------------------------
SessionMetaData::~SessionMetaData()
{}

// ----------------------------------------------------------------------------
SessionMetaData* SessionMetaData::load(FILE* fp, uint32_t size)
{
    // Load json into memory
    auto jsonBlob = Vector<char>::makeResized(size);
    if (fread(jsonBlob.data(), 1, size, fp) != (size_t)size)
        return nullptr;

    // Parse
    json j = json::parse(jsonBlob.begin(), jsonBlob.end(), nullptr, false);
    if (j == json::value_t::discarded)
        return nullptr;

    if (j["version"] == "1.5")
        return load_1_5(j);

    // unsupported version
    return nullptr;
}
static SessionMetaData* load_1_5(const json& j)
{
    const json jPlayerInfo = j["playerinfo"];
    const json jGameInfo = j["gameinfo"];

    const auto stageID = StageID::fromValue(
        jGameInfo["stageid"].get<StageID::Type>());
    const auto timeStarted = TimeStamp::fromMillisSinceEpoch(
        jGameInfo["timestampstart"].get<TimeStamp::Type>());
    const auto timeEnded = TimeStamp::fromMillisSinceEpoch(
        jGameInfo["timestampend"].get<TimeStamp::Type>());

    SmallVector<FighterID, 2> fighterIDs;
    SmallVector<SmallString<15>, 2> tags;
    SmallVector<SmallString<15>, 2> names;
    for (const auto& info : jPlayerInfo)
    {
        const json jTag = info["tag"];
        const json jName = info["name"];
        const json jFighterID = info["fighterid"];

        fighterIDs.push(FighterID::fromValue(jFighterID.get<FighterID::Type>()));
        tags.emplace(jTag.get<std::string>().c_str());
        names.emplace(jName.get<std::string>().c_str());
    }

    const std::string type = j["type"].get<std::string>();
    if (type == "game")
    {
        return SessionMetaData::newSavedGameSession(
                timeStarted,
                timeStarted,
                stageID,
                std::move(fighterIDs),
                std::move(tags),
                std::move(names),
                GameNumber::fromValue(jGameInfo["number"].get<GameNumber::Type>()),
                SetNumber::fromValue(jGameInfo["set"].get<SetNumber::Type>()),
                SetFormat(jGameInfo["format"].get<std::string>().c_str()),
                jGameInfo["winner"].get<int>());
    }
    if (type == "training")
    {
        return SessionMetaData::newSavedTrainingSession(
                timeStarted,
                timeEnded,
                stageID,
                std::move(fighterIDs),
                std::move(tags),
                GameNumber::fromValue(jGameInfo["number"].get<GameNumber::Type>()));
    }

    return nullptr;
}

// ----------------------------------------------------------------------------
uint32_t SessionMetaData::save(FILE* fp) const
{
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
        const GameSessionMetaData* meta = static_cast<const GameSessionMetaData*>(this);
        jGameInfo["format"] = meta->setFormat().description().cStr();
        jGameInfo["number"] = meta->gameNumber().value();
        jGameInfo["set"] = meta->setNumber().value();
        jGameInfo["winner"] = meta->winner();
    }
    else
    {
        assert(type() == TRAINING);
        const TrainingSessionMetaData* meta = static_cast<const TrainingSessionMetaData*>(this);
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
int SessionMetaData::fighterCount() const
{
    return fighterIDs_.count();
}

// ----------------------------------------------------------------------------
const SmallString<15>& SessionMetaData::tag(int fighterIdx) const
{
    return tags_[fighterIdx];
}

// ----------------------------------------------------------------------------
FighterID SessionMetaData::fighterID(int fighterIdx) const
{
    return fighterIDs_[fighterIdx];
}

// ----------------------------------------------------------------------------
StageID SessionMetaData::stageID() const
{
    return stageID_;
}

// ----------------------------------------------------------------------------
TimeStamp SessionMetaData::timeStampStarted() const
{
    return timeStarted_;
}

// ----------------------------------------------------------------------------
TimeStamp SessionMetaData::timeStampEnded() const
{
    return timeEnded_;
}

// ----------------------------------------------------------------------------
DeltaTime SessionMetaData::length() const
{
    return timeStarted_ - timeEnded_;
}

// ----------------------------------------------------------------------------
GameSessionMetaData::GameSessionMetaData(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<SmallString<15>, 2>&& tags,
        SmallVector<SmallString<15>, 2>&& names,
        GameNumber gameNumber,
        SetNumber setNumber,
        SetFormat setFormat,
        int winner)
    : SessionMetaData(timeStarted, timeEnded, stageID, std::move(fighterIDs), std::move(tags))
    , names_(std::move(names))
    , gameNumber_(gameNumber)
    , setNumber_(setNumber)
    , setFormat_(setFormat)
    , winner_(winner)
{}

// ----------------------------------------------------------------------------
SessionMetaData::Type GameSessionMetaData::type() const
{
    return GAME;
}

// ----------------------------------------------------------------------------
const SmallString<15>& GameSessionMetaData::name(int playerIdx) const
{
    return names_[playerIdx];
}

// ----------------------------------------------------------------------------
GameNumber GameSessionMetaData::gameNumber() const
{
    return gameNumber_;
}

// ----------------------------------------------------------------------------
void GameSessionMetaData::setGameNumber(GameNumber gameNumber)
{
    bool notify = (gameNumber_ != gameNumber);
    gameNumber_ = gameNumber;
    if (notify)
        dispatcher.dispatch(&SessionMetaDataListener::onSessionMetaDataGameNumberChanged, gameNumber);
}

// ----------------------------------------------------------------------------
void GameSessionMetaData::resetGameNumber()
{
    setGameNumber(GameNumber::fromValue(1));
}

// ----------------------------------------------------------------------------
SetNumber GameSessionMetaData::setNumber() const
{
    return setNumber_;
}

void GameSessionMetaData::setSetNumber(SetNumber setNumber)
{
    bool notify = (setNumber_ != setNumber);
    setNumber_ = setNumber;
    if (notify)
        dispatcher.dispatch(&SessionMetaDataListener::onSessionMetaDataSetNumberChanged, setNumber);
}

// ----------------------------------------------------------------------------
void GameSessionMetaData::resetSetNumber()
{
    setSetNumber(SetNumber::fromValue(1));
}

// ----------------------------------------------------------------------------
SetFormat GameSessionMetaData::setFormat() const
{
    return setFormat_;
}

// ----------------------------------------------------------------------------
void GameSessionMetaData::setSetFormat(SetFormat format)
{
    bool notify = (setFormat_ != format);
    setFormat_ = format;
    if (notify)
        dispatcher.dispatch(&SessionMetaDataListener::onSessionMetaDataSetFormatChanged, format);
}

// ----------------------------------------------------------------------------
int GameSessionMetaData::winner() const
{
    return winner_;
}

// ----------------------------------------------------------------------------
void GameSessionMetaData::setWinner(int fighterIdx)
{
    winner_ = fighterIdx;
}

// ----------------------------------------------------------------------------
TrainingSessionMetaData::TrainingSessionMetaData(
        TimeStamp timeStarted,
        TimeStamp timeEnded,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<SmallString<15>, 2>&& tags,
        GameNumber sessionNumber)
    : SessionMetaData(timeStarted, timeEnded, stageID, std::move(fighterIDs), std::move(tags))
    , sessionNumber_(sessionNumber)
{}

// ----------------------------------------------------------------------------
SessionMetaData::Type TrainingSessionMetaData::type() const
{
    return TRAINING;
}

// ----------------------------------------------------------------------------
const SmallString<15>& TrainingSessionMetaData::name(int playerIdx) const
{
    return tag(playerIdx);
}

// ----------------------------------------------------------------------------
FighterID TrainingSessionMetaData::playerFighterID() const
{
    return fighterID(0);
}

// ----------------------------------------------------------------------------
FighterID TrainingSessionMetaData::cpuFighterID() const
{
    return fighterID(1);
}

// ----------------------------------------------------------------------------
GameNumber TrainingSessionMetaData::sessionNumber() const
{
    return sessionNumber_;
}

// ----------------------------------------------------------------------------
void TrainingSessionMetaData::setSessionNumber(GameNumber sessionNumber)
{
    bool notify = (sessionNumber_ != sessionNumber);
    sessionNumber_ = sessionNumber;
    if (notify)
        dispatcher.dispatch(&SessionMetaDataListener::onSessionMetaDataSessionNumberChanged, sessionNumber);
}

// ----------------------------------------------------------------------------
void TrainingSessionMetaData::resetSessionNumber()
{
    setSessionNumber(GameNumber::fromValue(1));
}

}
