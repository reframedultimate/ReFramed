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
static std::string decompressGZFile(const std::string& fileName)
{
    int header;
    std::string out;
    gzFile f = gzopen(fileName.c_str(), "rb");
    if (f == nullptr)
        goto open_failed;

    header = gzgetc(f); if (header != 0x1f) goto read_error;
    header = gzgetc(f); if (header != 0x8b) goto read_error;
    gzrewind(f);

    if (gzbuffer(f, 256 * 1024) != 0)
        goto read_error;

    while (true)
    {
        size_t prevSize = out.size();
        out.resize(out.size() + 0x1000, '\0');
        void* dst = static_cast<void*>(out.data() + prevSize);
        int len = gzread(f, dst, 0x1000);
        if (len < 0)
            goto read_error;
        if (len < 0x1000)
        {
            if (gzeof(f))
            {
                out.resize(prevSize + len);
                break;
            }
            else
            {
                goto read_error;
            }
        }
    }

    if (gzclose(f) != Z_OK)
        return "";
    return out;

    read_error      : gzclose(f);
    open_failed     : return "";
}

// ----------------------------------------------------------------------------
static std::string decompressQtZFile(const std::string& fileName)
{
#define CHUNK (256*1024)
    std::string out;
    FILE* fp = fopen(fileName.c_str(), "rb");
    if (fp == nullptr)
        return "";

    unsigned char* buf = new unsigned char[CHUNK];

    // Qt prepends 4 bytes describing the uncompressed length. This is not
    // normally part of a Z compressed file so remove it
    uint32_t qtLengthHeader;
    fread(&qtLengthHeader, 4, 1, fp);

    z_stream s;
    s.zalloc = Z_NULL;
    s.zfree = Z_NULL;
    s.opaque = Z_NULL;
    s.avail_in = 0;
    s.next_in = Z_NULL;
    if (inflateInit(&s) != Z_OK)
        goto init_stream_failed;

    int ret;
    do {
        s.avail_in = fread(buf, 1, CHUNK, fp);
        if (ferror(fp))
            goto read_error;
        if (s.avail_in == 0)
            break;
        s.next_in = buf;

        do {
            size_t prevSize = out.size();
            out.resize(prevSize + CHUNK);
            s.avail_out = CHUNK;
            s.next_out = reinterpret_cast<unsigned char*>(out.data() + prevSize);

            ret = inflate(&s, Z_NO_FLUSH);
            if (ret == Z_STREAM_END)
                break;
            if (ret != Z_OK)
                goto read_error;

            int have = CHUNK - s.avail_out;
            if (have < CHUNK)
                out.resize(prevSize + have);
        } while (s.avail_out == 0);
    } while (ret != Z_STREAM_END);
    assert(ret == Z_STREAM_END);

    deflateEnd(&s);
    delete[] buf;
    fclose(fp);

    return out;

read_error :
    deflateEnd(&s);
init_stream_failed :
    delete[] buf;
    fclose(fp);
    return "";
#undef CHUNK
}

// ----------------------------------------------------------------------------
static std::string readUncompressedFile(const std::string& fileName)
{
#define CHUNK (256*1024)
    std::string out;
    FILE* fp = fopen(fileName.c_str(), "rb");
    if (fp == nullptr)
        goto open_failed;

    do {
        size_t prevSize = out.size();
        out.resize(prevSize + CHUNK);
        size_t bytes = fread(out.data() + prevSize, 1, CHUNK, fp);
        if (ferror(fp))
            goto read_error;
        if (bytes < CHUNK)
            out.resize(prevSize + bytes);
    } while (!feof(fp));

    fclose(fp);
    return out;

    read_error  : fclose(fp);
    open_failed : return "";
#undef CHUNK
}

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
    json j;
    for (auto readFile : {decompressGZFile, decompressQtZFile, readUncompressedFile})
    {
        std::string s = readFile(fileName);
        if (s.length())
        {
            try {
                j = json::parse(std::move(s));
                break;
            } catch (const json::parse_error& e) {
                return nullptr;
            }
        }
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

    for (int i = 0; i != recording->playerCount(); ++i)
        assert(recording->playerStateCount(i) > 0);

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

    uint64_t firstFrameTimeStampMs = time_qt_to_milli_seconds_since_epoch(jsonGameInfo["date"].get<std::string>().c_str());
    recording->format_ = SetFormat(jsonGameInfo["format"].get<std::string>());
    recording->gameNumber_ = jsonGameInfo["number"].get<GameNumber>();

    StreamBuffer stream(base64_decode(j["playerstates"].get<std::string>()));
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        int error = 0;
        Frame stateCount = stream.readBU32(&error);
        if (error)
            return nullptr;

        Frame framesPassed = 0;
        Frame prevFrame = std::numeric_limits<Frame>::max();
        for (Frame f = 0; f < stateCount; ++f)
        {
            Frame frame = stream.readBU32(&error);
            FighterStatus status = stream.readBU16(&error);
            float damage = stream.readBF64(&error);
            FighterStocks stocks = stream.readU8(&error);

            if (error)
                return nullptr;

            // Version 1.0 did not timestamp each frame so we have to make a guesstimate
            // based on the timestamp of when the recording started and how many
            // frames passed since. This will not account for game pauses or
            // lag, but it should be good enough.
            uint64_t frameTimeStamp = firstFrameTimeStampMs + static_cast<uint64_t>(framesPassed * 1000.0 / 60.0);
            if (prevFrame != std::numeric_limits<Frame>::max())
                framesPassed += prevFrame - frame;
            prevFrame = frame;

            recording->playerStates_[i].push_back(PlayerState(frameTimeStamp, frame, 0.0, 0.0, damage, 0.0, 50.0, status, 0, 0, stocks, false, false));

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

    uint64_t firstFrameTimeStampMs = time_qt_to_milli_seconds_since_epoch(jsonGameInfo["date"].get<std::string>().c_str());
    recording->format_ = SetFormat(jsonGameInfo["format"].get<std::string>());
    recording->gameNumber_ = jsonGameInfo["number"].get<int>();

    StreamBuffer stream(base64_decode(j["playerstates"].get<std::string>()));
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        int error = 0;
        Frame stateCount = stream.readBU32(&error);
        if (error)
            return nullptr;

        Frame framesPassed = 0;
        Frame prevFrame = std::numeric_limits<Frame>::max();
        for (Frame f = 0; f < stateCount; ++f)
        {
            Frame frame = stream.readBU32(&error);
            FighterStatus status = stream.readBU16(&error);
            float damage = stream.readBF64(&error);
            FighterStocks stocks = stream.readU8(&error);

            if (error)
                return nullptr;

            // Version 1.1 did not timestamp each frame so we have to make a guesstimate
            // based on the timestamp of when the recording started and how many
            // frames passed since. This will not account for game pauses or
            // lag, but it should be good enough.
            uint64_t frameTimeStamp = firstFrameTimeStampMs + static_cast<uint64_t>(framesPassed * 1000.0 / 60.0);
            if (prevFrame != std::numeric_limits<Frame>::max())
                framesPassed += prevFrame - frame;
            prevFrame = frame;

            recording->playerStates_[i].push_back(PlayerState(frameTimeStamp, frame, 0.0, 0.0, damage, 0.0, 50.0, status, 0, 0, stocks, false, false));
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

    uint64_t firstFrameTimeStampMs = time_qt_to_milli_seconds_since_epoch(jsonGameInfo["date"].get<std::string>().c_str());
    recording->format_ = SetFormat(jsonGameInfo["format"].get<std::string>());
    recording->gameNumber_ = jsonGameInfo["number"].get<GameNumber>();
    recording->setNumber_ = jsonGameInfo["set"].get<SetNumber>();
    recording->playerNames_ = std::move(playerNames);

    StreamBuffer stream(base64_decode(j["playerstates"].get<std::string>()));
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        int error = 0;
        Frame stateCount = stream.readBU32(&error);
        if (error)
            return nullptr;

        Frame framesPassed = 0;
        Frame prevFrame = std::numeric_limits<Frame>::max();
        for (Frame f = 0; f < stateCount; ++f)
        {
            Frame frame = stream.readBU32(&error);
            float posx = stream.readBF64(&error);
            float posy = stream.readBF64(&error);
            float damage = stream.readBF64(&error);
            float hitstun = stream.readBF64(&error);
            float shield = stream.readBF64(&error);
            FighterStatus status = stream.readBU16(&error);
            FighterMotion motion = stream.readBU64(&error);
            FighterHitStatus hit_status = stream.readU8(&error);
            FighterStocks stocks = stream.readU8(&error);
            uint8_t flags = stream.readU8(&error);

            if (error)
                return nullptr;

            // Version 1.2 did not timestamp each frame so we have to make a guesstimate
            // based on the timestamp of when the recording started and how many
            // frames passed since. This will not account for game pauses or
            // lag, but it should be good enough.
            uint64_t frameTimeStamp = firstFrameTimeStampMs + static_cast<uint64_t>(framesPassed * 1000.0 / 60.0);
            if (prevFrame != std::numeric_limits<Frame>::max())
                framesPassed += prevFrame - frame;
            prevFrame = frame;

            bool attack_connected = !!(flags & 0x01);
            bool facing_direction = !!(flags & 0x02);

            recording->playerStates_[i].push_back(PlayerState(frameTimeStamp, frame, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, attack_connected, facing_direction));
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

    uint64_t firstFrameTimeStampMs = time_qt_to_milli_seconds_since_epoch(jsonGameInfo["date"].get<std::string>().c_str());
    recording->format_ = SetFormat(jsonGameInfo["format"].get<std::string>());
    recording->gameNumber_ = jsonGameInfo["number"].get<GameNumber>();
    recording->setNumber_ = jsonGameInfo["set"].get<SetNumber>();
    recording->playerNames_ = std::move(playerNames);

    StreamBuffer stream(base64_decode(j["playerstates"].get<std::string>()));
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        int error = 0;
        Frame stateCount = stream.readLU32(&error);
        if (error)
            return nullptr;

        Frame framesPassed = 0;
        Frame prevFrame = std::numeric_limits<Frame>::max();
        for (Frame f = 0; f < stateCount; ++f)
        {
            Frame frame = stream.readLU32(&error);
            float posx = stream.readLF32(&error);
            float posy = stream.readLF32(&error);
            float damage = stream.readLF32(&error);
            float hitstun = stream.readLF32(&error);
            float shield = stream.readLF32(&error);
            FighterStatus status = stream.readLU16(&error);
            uint32_t motion_l = stream.readLU32(&error);
            uint8_t motion_h = stream.readU8(&error);
            FighterHitStatus hit_status = stream.readU8(&error);
            FighterStocks stocks = stream.readU8(&error);
            uint8_t flags = stream.readU8(&error);

            if (error)
                return nullptr;

            FighterMotion motion = static_cast<uint64_t>(motion_l)
                                 | (static_cast<uint64_t>(motion_h) << 32);

            bool attack_connected = !!(flags & 0x01);
            bool facing_direction = !!(flags & 0x02);

            // Version 1.3 did not timestamp each frame so we have to make a guesstimate
            // based on the timestamp of when the recording started and how many
            // frames passed since. This will not account for game pauses or
            // lag, but it should be good enough.
            uint64_t frameTimeStamp = firstFrameTimeStampMs + static_cast<uint64_t>(framesPassed * 1000.0 / 60.0);
            if (prevFrame != std::numeric_limits<Frame>::max())
                framesPassed += prevFrame - frame;
            prevFrame = frame;

            recording->playerStates_[i].push_back(PlayerState(frameTimeStamp, frame, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, attack_connected, facing_direction));
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
    if (jsonGameInfo.contains("timestampstart") == false || jsonGameInfo["timestampstart"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("timestampend") == false || jsonGameInfo["timestampend"].is_number() == false)
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

    recording->format_ = SetFormat(jsonGameInfo["format"].get<std::string>());
    recording->gameNumber_ = jsonGameInfo["number"].get<GameNumber>();
    recording->setNumber_ = jsonGameInfo["set"].get<SetNumber>();
    recording->playerNames_ = std::move(playerNames);

    StreamBuffer stream(base64_decode(j["playerstates"].get<std::string>()));
    for (int i = 0; i < recording->playerCount(); ++i)
    {
        int error = 0;
        Frame stateCount = stream.readLU32(&error);
        if (error)
            return nullptr;

        for (Frame f = 0; f < stateCount; ++f)
        {
            uint64_t frameTimeStamp = stream.readLU64(&error);
            Frame frame = stream.readLU32(&error);
            float posx = stream.readLF32(&error);
            float posy = stream.readLF32(&error);
            float damage = stream.readLF32(&error);
            float hitstun = stream.readLF32(&error);
            float shield = stream.readLF32(&error);
            FighterStatus status = stream.readLU16(&error);
            uint32_t motion_l = stream.readLU32(&error);
            uint8_t motion_h = stream.readU8(&error);
            FighterHitStatus hit_status = stream.readU8(&error);
            FighterStocks stocks = stream.readU8(&error);
            uint8_t flags = stream.readU8(&error);

            if (error)
                return nullptr;

            FighterMotion motion = static_cast<uint64_t>(motion_l)
                                 | (static_cast<uint64_t>(motion_h) << 32);

            bool attack_connected = !!(flags & 0x01);
            bool facing_direction = !!(flags & 0x02);

            recording->playerStates_[i].push_back(PlayerState(frameTimeStamp, frame, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, attack_connected, facing_direction));
        }
    }

    recording->winner_ = recording->findWinner();

    return recording.release();
}

}
