#include "uh/SavedRecording.hpp"
#include "uh/MappingInfo.hpp"
#include "uh/PlayerState.hpp"
#include "uh/StreamBuffer.hpp"
#include "uh/Types.hpp"
#include "uh/time.h"
#include "nlohmann/json.hpp"
#include "cpp-base64/base64.h"
#include "zlib.h"
#include <iostream>

namespace uh {

using nlohmann::json;

// ----------------------------------------------------------------------------
SavedRecording::SavedRecording(MappingInfo&& mapping,
                               std::vector<uint8_t>&& playerFighterIDs,
                               std::vector<std::string>&& playerTags,
                               uint16_t stageID)
    : Recording(std::move(mapping),
                std::move(playerFighterIDs),
                std::move(playerTags),
                stageID)
{
}

// ----------------------------------------------------------------------------
SavedRecording* SavedRecording::load(const std::string& fileName)
{
    auto decompressFile = [](const std::string& fileName) -> std::string {
        gzFile f = gzopen(fileName.c_str(), "rb");
        if (f == nullptr)
            return "";
        if (gzbuffer(f, 128 * 1024) != 0)
        {
            gzclose(f);
            return "";
        }

        std::string buf;
        while (true)
        {
            size_t prevSize = buf.size();
            buf.resize(buf.size() + 0x1000, '\0');
            void* dst = static_cast<void*>(buf.data() + prevSize);
            int len = gzread(f, dst, 0x1000);
            if (len < 0)
                return "";
            if (len < 0x1000)
            {
                if (gzeof(f))
                {
                    buf.resize(prevSize + len);
                    return buf;
                }
                return "";
            }
        }
    };

    std::string s = decompressFile(fileName);
    if (s.length() == 0)
        return nullptr;

    json j;
    try {
        j = json::parse(std::move(s));
    } catch (const json::parse_error& e) {
        return nullptr;
    }

    if (j.contains("version") == false || j["version"].is_string() == false)
        return nullptr;

    SavedRecording* recording;
    std::string version = j["version"];
    if (version == "1.4")
    {
        if ((recording = loadVersion_1_4(static_cast<const void*>(&j))) != nullptr)
            return recording;
    }
    else if (version == "1.3")
    {
        if ((recording = loadVersion_1_3(static_cast<const void*>(&j))) != nullptr)
            return recording;
    }
    else if (version == "1.2")
    {
        if ((recording = loadVersion_1_2(static_cast<const void*>(&j))) != nullptr)
            return recording;
    }
    else if (version == "1.1")
    {
        if ((recording = loadVersion_1_1(static_cast<const void*>(&j))) != nullptr)
            return recording;
    }
    else if (version == "1.0")
    {
        if ((recording = loadVersion_1_0(static_cast<const void*>(&j))) != nullptr)
            return recording;
    }

    return nullptr;
}

// ----------------------------------------------------------------------------
SavedRecording* SavedRecording::loadVersion_1_0(const void* jptr)
{
    const json& j = *static_cast<const json*>(jptr);
    if (j.contains("mappinginfo") == false || j["mappinginfo"].is_object() == false)
        return nullptr;
    if (j.contains("gameinfo") == false || j["gameinfo"].is_object() == false)
        return nullptr;
    if (j.contains("playerinfo") == false || j["playerinfo"].is_array() == false)
        return nullptr;
    if (j.contains("playerstates") == false || j["playerstates"].is_string() == false)
        return nullptr;

    const json jsonMappingInfo = j["mappinginfo"];
    const json jsonGameInfo = j["gameinfo"];
    const json jsonPlayerInfo = j["playerinfo"];

    if (jsonMappingInfo.contains("fighterstatus") == false || jsonMappingInfo["fighterstatus"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("fighterid") == false || jsonMappingInfo["fighterid"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("stageid") == false || jsonMappingInfo["stageid"].is_object() == false)
        return nullptr;

    MappingInfo mappingInfo;

    for (const auto& [key, value] : jsonMappingInfo["fighterid"].items())
    {
        std::size_t pos;
        FighterID fighterID = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.fighterID.add(fighterID, value.get<std::string>());
    }

    for (const auto& [key, value] : jsonMappingInfo["stageid"].items())
    {
        std::size_t pos;
        StageID stageID = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.stageID.add(stageID, value.get<std::string>());
    }

    // skip loading status mappings, it was broken in 1.0
    /*const json jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"];*/

    std::vector<FighterID> playerFighterIDs;
    std::vector<std::string> playerTags;
    for (const auto& info : jsonPlayerInfo)
    {
        if (info.contains("fighterid") == false || info["fighterid"].is_number() == false)
            return nullptr;
        if (info.contains("tag") == false || info["tag"].is_string() == false)
            return nullptr;

        playerFighterIDs.push_back(info["fighterid"].get<FighterID>());
        playerTags.push_back(info["tag"].get<std::string>());
    }

    if (jsonGameInfo.contains("date") == false || jsonGameInfo["date"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("format") == false || jsonGameInfo["format"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("number") == false || jsonGameInfo["number"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("stageid") == false || jsonGameInfo["stageid"].is_number() == false)
        return nullptr;

    std::unique_ptr<SavedRecording> recording(new SavedRecording(
        std::move(mappingInfo),
        std::move(playerFighterIDs),
        std::move(playerTags),
        jsonGameInfo["stageid"].get<StageID>()
    ));

    recording->timeStarted_ = time_qt_to_milli_seconds_since_epoch(jsonGameInfo["date"].get<std::string>().c_str());
    recording->format_ = SetFormat(jsonGameInfo["format"].get<std::string>());
    recording->gameNumber_ = jsonGameInfo["number"].get<GameNumber>();

    StreamBuffer stream(base64_decode(j["playerstates"].get<std::string>()));
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        if (stream.readBytesLeft() < 4)
            return nullptr;
        Frame frameCount = stream.readU32();
        if (stream.readBytesLeft() < static_cast<int>((4+2+8+1) * frameCount))
            return nullptr;

        for (Frame f = 0; f < frameCount; ++f)
        {
            Frame frame = stream.readU32();
            FighterStatus status = stream.readU16();
            float damage = stream.readF64();
            FighterStocks stocks = stream.readU8();
            recording->playerStates_[i].push_back(PlayerState(frame, 0.0, 0.0, damage, 0.0, 50.0, status, 0, 0, stocks, false, false));
        }
    }

    recording->winner_ = recording->findWinner();

    return recording.release();
}

// ----------------------------------------------------------------------------
SavedRecording* SavedRecording::loadVersion_1_1(const void* jptr)
{
    const json& j = *static_cast<const json*>(jptr);
    if (j.contains("mappinginfo") == false || j["mappinginfo"].is_object() == false)
        return nullptr;
    if (j.contains("gameinfo") == false || j["gameinfo"].is_object() == false)
        return nullptr;
    if (j.contains("playerinfo") == false || j["playerinfo"].is_array() == false)
        return nullptr;
    if (j.contains("playerstates") == false || j["playerstates"].is_string() == false)
        return nullptr;

    const json jsonMappingInfo = j["mappinginfo"];
    const json jsonGameInfo = j["gameinfo"];
    const json jsonPlayerInfo = j["playerinfo"];

    if (jsonMappingInfo.contains("fighterstatus") == false || jsonMappingInfo["fighterstatus"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("fighterid") == false || jsonMappingInfo["fighterid"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("stageid") == false || jsonMappingInfo["stageid"].is_object() == false)
        return nullptr;

    MappingInfo mappingInfo;

    const json jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"];
    if (jsonFighterStatusMapping.contains("base") == false || jsonFighterStatusMapping["base"].is_object() == false)
        return nullptr;
    if (jsonFighterStatusMapping.contains("specific") == false || jsonFighterStatusMapping["specific"].is_object() == false)
        return nullptr;

    for (const auto& [key, value] : jsonFighterStatusMapping["base"].items())
    {
        std::size_t pos;
        FighterStatus status = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_array() == false)
            return nullptr;

        if (value.size() != 3)
            return nullptr;
        if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
            return nullptr;

        mappingInfo.fighterStatus.addBaseEnumName(status, value[0].get<std::string>());
    }
    for (const auto& [fighter, jsonSpecificMapping] : jsonFighterStatusMapping["specific"].items())
    {
        std::size_t pos;
        FighterID fighterID = std::stoul(fighter, &pos);
        if (pos != fighter.length())
            return nullptr;
        if (jsonSpecificMapping.is_object() == false)
            return nullptr;

        for (const auto& [key, value] : jsonSpecificMapping.items())
        {
            FighterStatus status = std::stoul(key, &pos);
            if (pos != key.length())
                return nullptr;
            if (value.is_array() == false)
                return nullptr;

            if (value.size() != 3)
                return nullptr;
            if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
                return nullptr;

            mappingInfo.fighterStatus.addFighterSpecificEnumName(status, fighterID, value[0].get<std::string>());
        }
    }

    for (const auto& [key, value] : jsonMappingInfo["fighterid"].items())
    {
        std::size_t pos;
        FighterID fighterID = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.fighterID.add(fighterID, value.get<std::string>());
    }

    for (const auto& [key, value] : jsonMappingInfo["stageid"].items())
    {
        std::size_t pos;
        StageID stageID = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.stageID.add(stageID, value.get<std::string>());
    }

    std::vector<FighterID> playerFighterIDs;
    std::vector<std::string> playerTags;
    for (const auto& info : jsonPlayerInfo)
    {
        if (info.contains("fighterid") == false || info["fighterid"].is_number() == false)
            return nullptr;
        if (info.contains("tag") == false || info["tag"].is_string() == false)
            return nullptr;

        playerFighterIDs.push_back(info["fighterid"].get<FighterID>());
        playerTags.push_back(info["tag"].get<std::string>());
    }

    if (jsonGameInfo.contains("date") == false || jsonGameInfo["date"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("format") == false || jsonGameInfo["format"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("number") == false || jsonGameInfo["number"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("stageid") == false || jsonGameInfo["stageid"].is_number() == false)
        return nullptr;

    std::unique_ptr<SavedRecording> recording(new SavedRecording(
        std::move(mappingInfo),
        std::move(playerFighterIDs),
        std::move(playerTags),
        jsonGameInfo["stageid"].get<StageID>()
    ));

    recording->timeStarted_ = time_qt_to_milli_seconds_since_epoch(jsonGameInfo["date"].get<std::string>().c_str());
    recording->format_ = SetFormat(jsonGameInfo["format"].get<std::string>());
    recording->gameNumber_ = jsonGameInfo["number"].get<int>();

    StreamBuffer stream(base64_decode(j["playerstates"].get<std::string>()));
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        if (stream.readBytesLeft() < 4)
            return nullptr;
        Frame frameCount = stream.readU32();
        if (stream.readBytesLeft() < static_cast<int>((4+2+8+1) * frameCount))
            return nullptr;

        for (Frame f = 0; f < frameCount; ++f)
        {
            Frame frame = stream.readU32();
            FighterStatus status = stream.readU16();
            float damage = stream.readF64();
            FighterStocks stocks = stream.readU8();
            recording->playerStates_[i].push_back(PlayerState(frame, 0.0, 0.0, damage, 0.0, 50.0, status, 0, 0, stocks, false, false));
        }
    }

    recording->winner_ = recording->findWinner();

    return recording.release();
}

// ----------------------------------------------------------------------------
SavedRecording* SavedRecording::loadVersion_1_2(const void* jptr)
{
    const json& j = *static_cast<const json*>(jptr);
    if (j.contains("mappinginfo") == false || j["mappinginfo"].is_object() == false)
        return nullptr;
    if (j.contains("gameinfo") == false || j["gameinfo"].is_object() == false)
        return nullptr;
    if (j.contains("playerinfo") == false || j["playerinfo"].is_array() == false)
        return nullptr;
    if (j.contains("playerstates") == false || j["playerstates"].is_string() == false)
        return nullptr;

    const json jsonMappingInfo = j["mappinginfo"];
    const json jsonGameInfo = j["gameinfo"];
    const json jsonPlayerInfo = j["playerinfo"];

    if (jsonMappingInfo.contains("fighterstatus") == false || jsonMappingInfo["fighterstatus"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("fighterid") == false || jsonMappingInfo["fighterid"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("stageid") == false || jsonMappingInfo["stageid"].is_object() == false)
        return nullptr;

    MappingInfo mappingInfo;

    const json jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"];
    if (jsonFighterStatusMapping.contains("base") == false || jsonFighterStatusMapping["base"].is_object() == false)
        return nullptr;
    if (jsonFighterStatusMapping.contains("specific") == false || jsonFighterStatusMapping["specific"].is_object() == false)
        return nullptr;

    for (const auto& [key, value] : jsonFighterStatusMapping["base"].items())
    {
        std::size_t pos;
        FighterStatus status = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_array() == false)
            return nullptr;

        if (value.size() != 3)
            return nullptr;
        if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
            return nullptr;

        /*QString shortName  = arr[1].get<std::string>();
        QString customName = arr[2].get<std::string>();*/

        mappingInfo.fighterStatus.addBaseEnumName(status, value[0].get<std::string>());
    }
    for (const auto& [fighter, jsonSpecificMapping] : jsonFighterStatusMapping["specific"].items())
    {
        std::size_t pos;
        FighterID fighterID = std::stoul(fighter, &pos);
        if (pos != fighter.length())
            return nullptr;
        if (jsonSpecificMapping.is_object() == false)
            return nullptr;

        for (const auto& [key, value] : jsonSpecificMapping.items())
        {
            FighterStatus status = std::stoul(key, &pos);
            if (pos != key.length())
                return nullptr;
            if (value.is_array() == false)
                return nullptr;

            if (value.size() != 3)
                return nullptr;
            if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
                return nullptr;

            /*QString shortName  = arr[1].get<std::string>();
            QString customName = arr[2].get<std::string>();*/

            mappingInfo.fighterStatus.addFighterSpecificEnumName(status, fighterID, value[0].get<std::string>());
        }
    }

    for (const auto& [key, value] : jsonMappingInfo["fighterid"].items())
    {
        std::size_t pos;
        FighterID fighterID = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.fighterID.add(fighterID, value.get<std::string>());
    }

    for (const auto& [key, value] : jsonMappingInfo["stageid"].items())
    {
        std::size_t pos;
        StageID stageID = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.stageID.add(stageID, value.get<std::string>());
    }

    std::vector<FighterID> playerFighterIDs;
    std::vector<std::string> playerTags;
    std::vector<std::string> playerNames;
    for (const auto& info : jsonPlayerInfo)
    {
        if (info.contains("fighterid") == false || info["fighterid"].is_number() == false)
            return nullptr;
        if (info.contains("tag") == false || info["tag"].is_string() == false)
            return nullptr;
        if (info.contains("name") == false || info["name"].is_string() == false)
            return nullptr;

        playerFighterIDs.push_back(info["fighterid"].get<FighterID>());
        playerTags.push_back(info["tag"].get<std::string>());
        playerNames.push_back(info["name"].get<std::string>());
    }

    if (jsonGameInfo.contains("stageid") == false || jsonGameInfo["stageid"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("date") == false || jsonGameInfo["date"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("format") == false || jsonGameInfo["format"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("number") == false || jsonGameInfo["number"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("set") == false || jsonGameInfo["set"].is_number() == false)
        return nullptr;

    std::unique_ptr<SavedRecording> recording(new SavedRecording(
        std::move(mappingInfo),
        std::move(playerFighterIDs),
        std::move(playerTags),
        jsonGameInfo["stageid"].get<StageID>()
    ));

    recording->timeStarted_ = time_qt_to_milli_seconds_since_epoch(jsonGameInfo["date"].get<std::string>().c_str());
    recording->format_ = SetFormat(jsonGameInfo["format"].get<std::string>());
    recording->gameNumber_ = jsonGameInfo["number"].get<GameNumber>();
    recording->setNumber_ = jsonGameInfo["set"].get<SetNumber>();
    recording->playerNames_ = std::move(playerNames);

    StreamBuffer stream(base64_decode(j["playerstates"].get<std::string>()));
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        if (stream.readBytesLeft() < 4)
            return nullptr;
        Frame frameCount = stream.readU32();
        if (stream.readBytesLeft() < static_cast<int>((4+8+8+8+8+8+2+8+1+1+1) * frameCount))
            return nullptr;

        for (Frame f = 0; f < frameCount; ++f)
        {
            Frame frame = stream.readU32();
            float posx = stream.readF64();
            float posy = stream.readF64();
            float damage = stream.readF64();
            float hitstun = stream.readF64();
            float shield = stream.readF64();
            FighterStatus status = stream.readU16();
            FighterMotion motion = stream.readU64();
            FighterHitStatus hit_status = stream.readU8();
            FighterStocks stocks = stream.readU8();
            uint8_t flags = stream.readU8();

            bool attack_connected = !!(flags & 0x01);
            bool facing_direction = !!(flags & 0x02);

            recording->playerStates_[i].push_back(PlayerState(frame, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, attack_connected, facing_direction));
        }
    }

    recording->winner_ = recording->findWinner();

    return recording.release();
}

// ----------------------------------------------------------------------------
SavedRecording* SavedRecording::loadVersion_1_3(const void* jptr)
{
    const json& j = *static_cast<const json*>(jptr);
    if (j.contains("mappinginfo") == false || j["mappinginfo"].is_object() == false)
        return nullptr;
    if (j.contains("gameinfo") == false || j["gameinfo"].is_object() == false)
        return nullptr;
    if (j.contains("playerinfo") == false || j["playerinfo"].is_array() == false)
        return nullptr;
    if (j.contains("playerstates") == false || j["playerstates"].is_string() == false)
        return nullptr;

    const json jsonMappingInfo = j["mappinginfo"];
    const json jsonGameInfo = j["gameinfo"];
    const json jsonPlayerInfo = j["playerinfo"];

    if (jsonMappingInfo.contains("fighterstatus") == false || jsonMappingInfo["fighterstatus"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("fighterid") == false || jsonMappingInfo["fighterid"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("stageid") == false || jsonMappingInfo["stageid"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("hitstatus") == false || jsonMappingInfo["hitstatus"].is_object() == false)
        return nullptr;

    MappingInfo mappingInfo;

    const json jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"];
    if (jsonFighterStatusMapping.contains("base") == false || jsonFighterStatusMapping["base"].is_object() == false)
        return nullptr;
    if (jsonFighterStatusMapping.contains("specific") == false || jsonFighterStatusMapping["specific"].is_object() == false)
        return nullptr;

    for (const auto& [key, value] : jsonFighterStatusMapping["base"].items())
    {
        std::size_t pos;
        FighterStatus status = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_array() == false)
            return nullptr;

        if (value.size() != 3)
            return nullptr;
        if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
            return nullptr;

        /*QString shortName  = arr[1].get<std::string>();
        QString customName = arr[2].get<std::string>();*/

        mappingInfo.fighterStatus.addBaseEnumName(status, value[0].get<std::string>());
    }
    for (const auto& [fighter, jsonSpecificMapping] : jsonFighterStatusMapping["specific"].items())
    {
        std::size_t pos;
        FighterID fighterID = std::stoul(fighter, &pos);
        if (pos != fighter.length())
            return nullptr;
        if (jsonSpecificMapping.is_object() == false)
            return nullptr;

        for (const auto& [key, value] : jsonSpecificMapping.items())
        {
            FighterStatus status = std::stoul(key, &pos);
            if (pos != key.length())
                return nullptr;
            if (value.is_array() == false)
                return nullptr;

            if (value.size() != 3)
                return nullptr;
            if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
                return nullptr;

            /*QString shortName  = arr[1].get<std::string>();
            QString customName = arr[2].get<std::string>();*/

            mappingInfo.fighterStatus.addFighterSpecificEnumName(status, fighterID, value[0].get<std::string>());
        }
    }

    for (const auto& [key, value] : jsonMappingInfo["fighterid"].items())
    {
        std::size_t pos;
        FighterID fighterID = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.fighterID.add(fighterID, value.get<std::string>());
    }

    for (const auto& [key, value] : jsonMappingInfo["stageid"].items())
    {
        std::size_t pos;
        StageID stageID = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.stageID.add(stageID, value.get<std::string>());
    }

    for (const auto& [key, value] : jsonMappingInfo["hitstatus"].items())
    {
        std::size_t pos;
        FighterHitStatus hitStatusID = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.hitStatus.add(hitStatusID, value.get<std::string>());
    }

    std::vector<FighterID> playerFighterIDs;
    std::vector<std::string> playerTags;
    std::vector<std::string> playerNames;
    for (const auto& info : jsonPlayerInfo)
    {
        if (info.contains("fighterid") == false || info["fighterid"].is_number() == false)
            return nullptr;
        if (info.contains("tag") == false || info["tag"].is_string() == false)
            return nullptr;
        if (info.contains("name") == false || info["name"].is_string() == false)
            return nullptr;

        playerFighterIDs.push_back(info["fighterid"].get<FighterID>());
        playerTags.push_back(info["tag"].get<std::string>());
        playerNames.push_back(info["name"].get<std::string>());
    }

    if (jsonGameInfo.contains("stageid") == false || jsonGameInfo["stageid"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("date") == false || jsonGameInfo["date"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("format") == false || jsonGameInfo["format"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("number") == false || jsonGameInfo["number"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("set") == false || jsonGameInfo["set"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("winner") == false || jsonGameInfo["winner"].is_number() == false)
        return nullptr;

    std::unique_ptr<SavedRecording> recording(new SavedRecording(
        std::move(mappingInfo),
        std::move(playerFighterIDs),
        std::move(playerTags),
        jsonGameInfo["stageid"].get<StageID>()
    ));

    recording->timeStarted_ = time_qt_to_milli_seconds_since_epoch(jsonGameInfo["date"].get<std::string>().c_str());
    recording->format_ = SetFormat(jsonGameInfo["format"].get<std::string>());
    recording->gameNumber_ = jsonGameInfo["number"].get<GameNumber>();
    recording->setNumber_ = jsonGameInfo["set"].get<SetNumber>();
    recording->playerNames_ = std::move(playerNames);

    StreamBuffer stream(base64_decode(j["playerstates"].get<std::string>()));
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        if (stream.readBytesLeft() < 4)
            return nullptr;
        Frame frameCount = stream.readU32();
        if (stream.readBytesLeft() < static_cast<int>((4+4+4+4+4+4+2+5+1+1+1) * frameCount))
            return nullptr;

        for (Frame f = 0; f < frameCount; ++f)
        {
            Frame frame = stream.readU32();
            float posx = stream.readF64();
            float posy = stream.readF64();
            float damage = stream.readF64();
            float hitstun = stream.readF64();
            float shield = stream.readF64();
            FighterStatus status = stream.readU16();
            uint32_t motion_l = stream.readU32();
            uint8_t motion_h = stream.readU8();
            FighterHitStatus hit_status = stream.readU8();
            FighterStocks stocks = stream.readU8();
            uint8_t flags = stream.readU8();

            FighterMotion motion = static_cast<uint64_t>(motion_l)
                                 | (static_cast<uint64_t>(motion_h) << 32);

            bool attack_connected = !!(flags & 0x01);
            bool facing_direction = !!(flags & 0x02);

            recording->playerStates_[i].push_back(PlayerState(frame, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, attack_connected, facing_direction));
        }
    }

    recording->winner_ = recording->findWinner();

    return recording.release();
}

// ----------------------------------------------------------------------------
SavedRecording* SavedRecording::loadVersion_1_4(const void* jptr)
{
    const json& j = *static_cast<const json*>(jptr);
    if (j.contains("mappinginfo") == false || j["mappinginfo"].is_object() == false)
        return nullptr;
    if (j.contains("gameinfo") == false || j["gameinfo"].is_object() == false)
        return nullptr;
    if (j.contains("playerinfo") == false || j["playerinfo"].is_array() == false)
        return nullptr;
    if (j.contains("playerstates") == false || j["playerstates"].is_string() == false)
        return nullptr;

    const json jsonMappingInfo = j["mappinginfo"];
    const json jsonGameInfo = j["gameinfo"];
    const json jsonPlayerInfo = j["playerinfo"];

    if (jsonMappingInfo.contains("fighterstatus") == false || jsonMappingInfo["fighterstatus"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("fighterid") == false || jsonMappingInfo["fighterid"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("stageid") == false || jsonMappingInfo["stageid"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("hitstatus") == false || jsonMappingInfo["hitstatus"].is_object() == false)
        return nullptr;

    MappingInfo mappingInfo;

    const json jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"];
    if (jsonFighterStatusMapping.contains("base") == false || jsonFighterStatusMapping["base"].is_object() == false)
        return nullptr;
    if (jsonFighterStatusMapping.contains("specific") == false || jsonFighterStatusMapping["specific"].is_object() == false)
        return nullptr;

    for (const auto& [key, value] : jsonFighterStatusMapping["base"].items())
    {
        std::size_t pos;
        FighterStatus status = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_array() == false)
            return nullptr;

        if (value.size() != 3)
            return nullptr;
        if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
            return nullptr;

        /*QString shortName  = arr[1].get<std::string>();
        QString customName = arr[2].get<std::string>();*/

        mappingInfo.fighterStatus.addBaseEnumName(status, value[0].get<std::string>());
    }
    for (const auto& [fighter, jsonSpecificMapping] : jsonFighterStatusMapping["specific"].items())
    {
        std::size_t pos;
        FighterID fighterID = std::stoul(fighter, &pos);
        if (pos != fighter.length())
            return nullptr;
        if (jsonSpecificMapping.is_object() == false)
            return nullptr;

        for (const auto& [key, value] : jsonSpecificMapping.items())
        {
            FighterStatus status = std::stoul(key, &pos);
            if (pos != key.length())
                return nullptr;
            if (value.is_array() == false)
                return nullptr;

            if (value.size() != 3)
                return nullptr;
            if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
                return nullptr;

            /*QString shortName  = arr[1].get<std::string>();
            QString customName = arr[2].get<std::string>();*/

            mappingInfo.fighterStatus.addFighterSpecificEnumName(status, fighterID, value[0].get<std::string>());
        }
    }

    for (const auto& [key, value] : jsonMappingInfo["fighterid"].items())
    {
        std::size_t pos;
        FighterID fighterID = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.fighterID.add(fighterID, value.get<std::string>());
    }

    for (const auto& [key, value] : jsonMappingInfo["stageid"].items())
    {
        std::size_t pos;
        StageID stageID = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.stageID.add(stageID, value.get<std::string>());
    }

    for (const auto& [key, value] : jsonMappingInfo["hitstatus"].items())
    {
        std::size_t pos;
        FighterHitStatus hitStatusID = std::stoul(key, &pos);
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.hitStatus.add(hitStatusID, value.get<std::string>());
    }

    std::vector<FighterID> playerFighterIDs;
    std::vector<std::string> playerTags;
    std::vector<std::string> playerNames;
    for (const auto& info : jsonPlayerInfo)
    {
        if (info.contains("fighterid") == false || info["fighterid"].is_number() == false)
            return nullptr;
        if (info.contains("tag") == false || info["tag"].is_string() == false)
            return nullptr;
        if (info.contains("name") == false || info["name"].is_string() == false)
            return nullptr;

        playerFighterIDs.push_back(info["fighterid"].get<FighterID>());
        playerTags.push_back(info["tag"].get<std::string>());
        playerNames.push_back(info["name"].get<std::string>());
    }

    if (jsonGameInfo.contains("stageid") == false || jsonGameInfo["stageid"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("date") == false || jsonGameInfo["date"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("format") == false || jsonGameInfo["format"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("number") == false || jsonGameInfo["number"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("set") == false || jsonGameInfo["set"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("winner") == false || jsonGameInfo["winner"].is_number() == false)
        return nullptr;

    std::unique_ptr<SavedRecording> recording(new SavedRecording(
        std::move(mappingInfo),
        std::move(playerFighterIDs),
        std::move(playerTags),
        jsonGameInfo["stageid"].get<StageID>()
    ));

    recording->timeStarted_ = jsonGameInfo["date"].get<uint64_t>();
    recording->format_ = SetFormat(jsonGameInfo["format"].get<std::string>());
    recording->gameNumber_ = jsonGameInfo["number"].get<GameNumber>();
    recording->setNumber_ = jsonGameInfo["set"].get<SetNumber>();
    recording->playerNames_ = std::move(playerNames);

    StreamBuffer stream(base64_decode(j["playerstates"].get<std::string>()));
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        if (stream.readBytesLeft() < 4)
            return nullptr;
        Frame frameCount = stream.readU32();
        if (stream.readBytesLeft() < static_cast<int>((4+4+4+4+4+4+2+5+1+1+1) * frameCount))
            return nullptr;

        for (Frame f = 0; f < frameCount; ++f)
        {
            Frame frame = stream.readU32();
            float posx = stream.readF32();
            float posy = stream.readF32();
            float damage = stream.readF32();
            float hitstun = stream.readF32();
            float shield = stream.readF32();
            FighterStatus status = stream.readU16();
            uint32_t motion_l = stream.readU32();
            uint8_t motion_h = stream.readU8();
            FighterHitStatus hit_status = stream.readU8();
            FighterStocks stocks = stream.readU8();
            uint8_t flags = stream.readU8();

            FighterMotion motion = static_cast<uint64_t>(motion_l)
                                 | (static_cast<uint64_t>(motion_h) << 32);

            bool attack_connected = !!(flags & 0x01);
            bool facing_direction = !!(flags & 0x02);

            recording->playerStates_[i].push_back(PlayerState(frame, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, attack_connected, facing_direction));
        }
    }

    recording->winner_ = recording->findWinner();

    return recording.release();
}

}
