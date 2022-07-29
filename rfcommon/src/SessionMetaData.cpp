#include "rfcommon/SessionMetaData.hpp"
#include "nlohmann/json.hpp"

namespace rfcommon {

using nlohmann::json;

static SessionMetaData* load_1_5(const json& j);

// ----------------------------------------------------------------------------
SessionMetaData* SessionMetaData::load(FILE* fp, uint32_t size)
{
    // Load json into memory
    Vector<char> jsonBlob(size);
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

    SmallVector<FighterID, 2> fighterIDs;
    SmallVector<SmallString<15>, 2> tags;
    SmallVector<SmallString<15>, 2> names;
    for (const auto& info : jPlayerInfo)
    {
        const json jTag = info["tag"];
        const json jName = info["name"];
        const json jFighterID = info["fighterid"];

        fighterIDs.emplace(jFighterID.get<FighterID::Type>());
        tags.emplace(jTag.get<std::string>().c_str());
        names.emplace(jName.get<std::string>().c_str());
    }

    const std::string type = j["type"].get<std::string>();
    if (type == "game")
    {
        return new GameSessionMetaData(
                jGameInfo["stageid"].get<StageID::Type>(),
                std::move(fighterIDs),
                std::move(tags),
                std::move(names),
                jGameInfo["number"].get<GameNumber::Type>(),
                jGameInfo["set"].get<SetNumber::Type>(),
                SetFormat(jGameInfo["format"].get<std::string>().c_str()));
    }
    if (type == "training")
    {
        return new TrainingSessionMetaData(
                jGameInfo["stageid"].get<StageID::Type>(),
                std::move(fighterIDs),
                std::move(tags));
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

}
