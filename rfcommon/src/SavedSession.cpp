#include "rfcommon/SavedGameSession.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/FighterFrame.hpp"
#include "rfcommon/StreamBuffer.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/time.h"
#include "nlohmann/json.hpp"
#include "cpp-base64/base64.h"
#include "zlib.h"
#include <iostream>

namespace rfcommon {

using nlohmann::json;

// ----------------------------------------------------------------------------
static std::string decompressGZFile(const String& fileName)
{
    unsigned char header;
    std::string out;
    gzFile f;
    FILE* fp;

    fp = fopen(fileName.cStr(), "rb");
    if (fp == nullptr)
        goto fopen_failed;

    fread(&header, 1, 1, fp); if (header != 0x1f) goto header_error;
    fread(&header, 1, 1, fp); if (header != 0x8b) goto header_error;

    f = gzopen(fileName.cStr(), "rb");
    if (f == nullptr)
        goto gzopen_failed;

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

    fclose(fp);
    if (gzclose(f) != Z_OK)
        return "";

    return out;

    read_error      : gzclose(f);
    gzopen_failed   :
    header_error    : fclose(fp);
    fopen_failed    : return "";
}

// ----------------------------------------------------------------------------
static std::string decompressQtZFile(const String& fileName)
{
#define CHUNK (256*1024)
    std::string out;
    FILE* fp = fopen(fileName.cStr(), "rb");
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
        s.avail_in = static_cast<uInt>(fread(buf, 1, CHUNK, fp));
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

    inflateEnd(&s);
    delete[] buf;
    fclose(fp);

    return out;

read_error :
    inflateEnd(&s);
init_stream_failed :
    delete[] buf;
    fclose(fp);
    return "";
#undef CHUNK
}

// ----------------------------------------------------------------------------
static std::string readUncompressedFile(const String& fileName)
{
#define CHUNK (256*1024)
    std::string out;
    FILE* fp = fopen(fileName.cStr(), "rb");
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
SavedSession::SavedSession()
{
}

// ----------------------------------------------------------------------------
SavedSession* SavedSession::load(const String& fileName)
{
    json j;
    for (auto readFile : {decompressGZFile, decompressQtZFile, readUncompressedFile})
    {
        std::string s = readFile(fileName);
        if (s.length() == 0)
            continue;

        j = json::parse(std::move(s), nullptr, false);
        if (j == json::value_t::discarded)
            return nullptr;

        break;
    }

    if (j.contains("version") == false || j["version"].is_string() == false)
        return nullptr;

    SavedSession* session;
    std::string version = j["version"];
    if (version == "1.4")
    {
        if ((session = loadVersion_1_4(static_cast<const void*>(&j))) != nullptr)
            return session;
    }
    else if (version == "1.3")
    {
        if ((session = loadVersion_1_3(static_cast<const void*>(&j))) != nullptr)
            return session;
    }
    else if (version == "1.2")
    {
        if ((session = loadVersion_1_2(static_cast<const void*>(&j))) != nullptr)
            return session;
    }
    else if (version == "1.1")
    {
        if ((session = loadVersion_1_1(static_cast<const void*>(&j))) != nullptr)
            return session;
    }
    else if (version == "1.0")
    {
        if ((session = loadVersion_1_0(static_cast<const void*>(&j))) != nullptr)
            return session;
    }

    return nullptr;
}

// ----------------------------------------------------------------------------
SavedSession* SavedSession::loadVersion_1_0(const void* jptr)
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

    MappingInfo mappingInfo(0);

    for (const auto& [key, value] : jsonMappingInfo["fighterid"].items())
    {
        std::size_t pos;
        FighterID fighterID(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.fighterID.add(fighterID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jsonMappingInfo["stageid"].items())
    {
        std::size_t pos;
        StageID stageID(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.stageID.add(stageID, value.get<std::string>().c_str());
    }

    // skip loading status mappings, it was broken in 1.0
    /*const json jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"];*/

    SmallVector<FighterID, 2> playerFighterIDs;
    SmallVector<SmallString<15>, 2> playerTags;
    SmallVector<SmallString<15>, 2> playerNames;
    for (const auto& info : jsonPlayerInfo)
    {
        if (info.contains("fighterid") == false || info["fighterid"].is_number() == false)
            return nullptr;
        if (info.contains("tag") == false || info["tag"].is_string() == false)
            return nullptr;

        playerFighterIDs.emplace(info["fighterid"].get<FighterID::Type>());
        playerTags.emplace(info["tag"].get<std::string>().c_str());
        playerNames.emplace(info["tag"].get<std::string>().c_str());  // "name" property didn't exist in 1.0
    }

    if (jsonGameInfo.contains("date") == false || jsonGameInfo["date"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("format") == false || jsonGameInfo["format"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("number") == false || jsonGameInfo["number"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("stageid") == false || jsonGameInfo["stageid"].is_number() == false)
        return nullptr;

    std::unique_ptr<SavedGameSession> session(new SavedGameSession(
        std::move(mappingInfo),
        jsonGameInfo["stageid"].get<StageID::Type>(),
        std::move(playerFighterIDs),
        std::move(playerTags),
        std::move(playerNames),
        jsonGameInfo["number"].get<GameNumber::Type>(),
        1, // SetFormat did not exist in 1.0 yet
        SetFormat(jsonGameInfo["format"].get<std::string>().c_str())
    ));

    const TimeStampMS firstFrameTimeStampMs = time_qt_to_milli_seconds_since_epoch(jsonGameInfo["date"].get<std::string>().c_str());

    std::string streamDecoded = base64_decode(j["playerstates"].get<std::string>());
    StreamBuffer stream(streamDecoded.data(), static_cast<int>(streamDecoded.length()));
    for (int i = 0; i < session->fighterCount(); ++i)
    {
        int error = 0;
        const Frame::Type stateCount = stream.readBU32(&error);
        if (error)
            return nullptr;

        // zero states are invalid
        if (stateCount == 0)
            return nullptr;

        Frame::Type frameCounter = 0;
        for (Frame::Type f = 0; f < stateCount; ++f)
        {
            const Frame frame = stream.readBU32(&error);
            const FighterStatus status = stream.readBU16(&error);
            const float damage = static_cast<float>(stream.readBF64(&error));
            const FighterStocks stocks = stream.readU8(&error);
            const FighterFlags flags(false, false);

            if (error)
                return nullptr;
            if (frame.value() == 0)
                return nullptr;

            // Usually only unique states are saved, which means there will be
            // gaps in between frames. Duplicate the current frame as many times
            // as necessary and make sure to update the time stamp and frame
            // counter
            for (; frameCounter < frame.value(); ++frameCounter)
            {
                // Version 1.0 did not timestamp each frame so we have to make a guesstimate
                // based on the timestamp of when the recording started and how many
                // frames passed since. This will not account for game pauses or
                // lag, but it should be good enough.
                const TimeStampMS frameTimeStamp =  firstFrameTimeStampMs +
                        DeltaTimeMS(frameCounter * 1000.0 / 60.0);
                session->frames_[i].emplace(frameTimeStamp, frame, 0.0f, 0.0f, damage, 0.0f, 50.0f, status, 0, 0, stocks, flags);
            }
        }
    }

    // Ensure that every fighter has the same number of frames
    const int highestFrameIdx = std::max_element(session->frames_.begin(), session->frames_.end(),
        [](const Vector<FighterFrame>& a, const Vector<FighterFrame>& b){
            return a.count() < b.count();
    })->count();
    for (auto& frames : session->frames_)
        while (frames.count() < highestFrameIdx)
            frames.emplace(frames.back());

    // Cache winner
    session->winner_ = session->findWinner();

    return session.release();
}

// ----------------------------------------------------------------------------
SavedSession* SavedSession::loadVersion_1_1(const void* jptr)
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

    MappingInfo mappingInfo(0);

    const json jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"];
    if (jsonFighterStatusMapping.contains("base") == false || jsonFighterStatusMapping["base"].is_object() == false)
        return nullptr;
    if (jsonFighterStatusMapping.contains("specific") == false || jsonFighterStatusMapping["specific"].is_object() == false)
        return nullptr;

    for (const auto& [key, value] : jsonFighterStatusMapping["base"].items())
    {
        std::size_t pos;
        FighterStatus status(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_array() == false)
            return nullptr;

        if (value.size() != 3)
            return nullptr;
        if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
            return nullptr;

        mappingInfo.fighterStatus.addBaseEnumName(status, value[0].get<std::string>().c_str());
    }
    for (const auto& [fighter, jsonSpecificMapping] : jsonFighterStatusMapping["specific"].items())
    {
        std::size_t pos;
        FighterID fighterID(std::stoul(fighter, &pos));
        if (pos != fighter.length())
            return nullptr;
        if (jsonSpecificMapping.is_object() == false)
            return nullptr;

        for (const auto& [key, value] : jsonSpecificMapping.items())
        {
            FighterStatus status(std::stoul(key, &pos));
            if (pos != key.length())
                return nullptr;
            if (value.is_array() == false)
                return nullptr;

            if (value.size() != 3)
                return nullptr;
            if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
                return nullptr;

            mappingInfo.fighterStatus.addFighterSpecificEnumName(status, fighterID, value[0].get<std::string>().c_str());
        }
    }

    for (const auto& [key, value] : jsonMappingInfo["fighterid"].items())
    {
        std::size_t pos;
        FighterID fighterID(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.fighterID.add(fighterID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jsonMappingInfo["stageid"].items())
    {
        std::size_t pos;
        StageID stageID = static_cast<StageID>(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.stageID.add(stageID, value.get<std::string>().c_str());
    }

    SmallVector<FighterID, 2> playerFighterIDs;
    SmallVector<SmallString<15>, 2> playerTags;
    SmallVector<SmallString<15>, 2> playerNames;
    for (const auto& info : jsonPlayerInfo)
    {
        if (info.contains("fighterid") == false || info["fighterid"].is_number() == false)
            return nullptr;
        if (info.contains("tag") == false || info["tag"].is_string() == false)
            return nullptr;

        playerFighterIDs.emplace(info["fighterid"].get<FighterID::Type>());
        playerTags.emplace(info["tag"].get<std::string>().c_str());
        playerNames.emplace(info["tag"].get<std::string>().c_str());  // "name" property didn't exist in 1.1
    }

    if (jsonGameInfo.contains("date") == false || jsonGameInfo["date"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("format") == false || jsonGameInfo["format"].is_string() == false)
        return nullptr;
    if (jsonGameInfo.contains("number") == false || jsonGameInfo["number"].is_number() == false)
        return nullptr;
    if (jsonGameInfo.contains("stageid") == false || jsonGameInfo["stageid"].is_number() == false)
        return nullptr;

    std::unique_ptr<SavedGameSession> session(new SavedGameSession(
        std::move(mappingInfo),
        jsonGameInfo["stageid"].get<StageID::Type>(),
        std::move(playerFighterIDs),
        std::move(playerTags),
        std::move(playerNames),
        jsonGameInfo["number"].get<int>(),
        1,  // SetNumber did not exist yet in 1.1
        SetFormat(jsonGameInfo["format"].get<std::string>().c_str())
    ));

    const TimeStampMS firstFrameTimeStampMs = time_qt_to_milli_seconds_since_epoch(jsonGameInfo["date"].get<std::string>().c_str());

    const std::string streamDecoded = base64_decode(j["playerstates"].get<std::string>());
    StreamBuffer stream(streamDecoded.data(), static_cast<int>(streamDecoded.length()));
    for (int i = 0; i < session->fighterCount(); ++i)
    {
        int error = 0;
        const Frame::Type stateCount = stream.readBU32(&error);
        if (error)
            return nullptr;

        // zero states are invalid
        if (stateCount == 0)
            return nullptr;

        Frame::Type frameCounter = 0;
        for (Frame::Type f = 0; f < stateCount; ++f)
        {
            const Frame frame = stream.readBU32(&error);
            const FighterStatus status = stream.readBU16(&error);
            const float damage = static_cast<float>(stream.readBF64(&error));
            const FighterStocks stocks = stream.readU8(&error);
            const FighterFlags flags(false, false);

            if (error)
                return nullptr;
            if (frame.value() == 0)
                return nullptr;

            // Usually only unique states are saved, which means there will be
            // gaps in between frames. Duplicate the current frame as many times
            // as necessary and make sure to update the time stamp and frame
            // counter
            for (; frameCounter < frame.value(); ++frameCounter)
            {
                // Version 1.1 did not timestamp each frame so we have to make a guesstimate
                // based on the timestamp of when the recording started and how many
                // frames passed since. This will not account for game pauses or
                // lag, but it should be good enough.
                const TimeStampMS frameTimeStamp =  firstFrameTimeStampMs +
                        DeltaTimeMS(frameCounter * 1000.0 / 60.0);
                session->frames_[i].emplace(frameTimeStamp, frame, 0.0f, 0.0f, damage, 0.0f, 50.0f, status, 0, 0, stocks, flags);
            }
        }
    }

    // Ensure that every fighter has the same number of frames
    const int highestFrameIdx = std::max_element(session->frames_.begin(), session->frames_.end(),
        [](const Vector<FighterFrame>& a, const Vector<FighterFrame>& b){
            return a.count() < b.count();
    })->count();
    for (auto& frames : session->frames_)
        while (frames.count() < highestFrameIdx)
            frames.emplace(frames.back());

    // Cache winner
    session->winner_ = session->findWinner();

    return session.release();
}

// ----------------------------------------------------------------------------
SavedSession* SavedSession::loadVersion_1_2(const void* jptr)
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

    MappingInfo mappingInfo(0);

    const json jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"];
    if (jsonFighterStatusMapping.contains("base") == false || jsonFighterStatusMapping["base"].is_object() == false)
        return nullptr;
    if (jsonFighterStatusMapping.contains("specific") == false || jsonFighterStatusMapping["specific"].is_object() == false)
        return nullptr;

    for (const auto& [key, value] : jsonFighterStatusMapping["base"].items())
    {
        std::size_t pos;
        FighterStatus status(std::stoul(key, &pos));
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

        mappingInfo.fighterStatus.addBaseEnumName(status, value[0].get<std::string>().c_str());
    }
    for (const auto& [fighter, jsonSpecificMapping] : jsonFighterStatusMapping["specific"].items())
    {
        std::size_t pos;
        FighterID fighterID(std::stoul(fighter, &pos));
        if (pos != fighter.length())
            return nullptr;
        if (jsonSpecificMapping.is_object() == false)
            return nullptr;

        for (const auto& [key, value] : jsonSpecificMapping.items())
        {
            FighterStatus status(std::stoul(key, &pos));
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

            mappingInfo.fighterStatus.addFighterSpecificEnumName(status, fighterID, value[0].get<std::string>().c_str());
        }
    }

    for (const auto& [key, value] : jsonMappingInfo["fighterid"].items())
    {
        std::size_t pos;
        FighterID fighterID(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.fighterID.add(fighterID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jsonMappingInfo["stageid"].items())
    {
        std::size_t pos;
        StageID stageID(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.stageID.add(stageID, value.get<std::string>().c_str());
    }

    SmallVector<FighterID, 2> playerFighterIDs;
    SmallVector<SmallString<15>, 2> playerTags;
    SmallVector<SmallString<15>, 2> playerNames;
    for (const auto& info : jsonPlayerInfo)
    {
        if (info.contains("fighterid") == false || info["fighterid"].is_number() == false)
            return nullptr;
        if (info.contains("tag") == false || info["tag"].is_string() == false)
            return nullptr;
        if (info.contains("name") == false || info["name"].is_string() == false)
            return nullptr;

        playerFighterIDs.emplace(info["fighterid"].get<FighterID::Type>());
        playerTags.emplace(info["tag"].get<std::string>().c_str());
        playerNames.emplace(info["name"].get<std::string>().c_str());
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

    std::unique_ptr<SavedGameSession> session(new SavedGameSession(
        std::move(mappingInfo),
        jsonGameInfo["stageid"].get<StageID::Type>(),
        std::move(playerFighterIDs),
        std::move(playerTags),
        std::move(playerNames),
        jsonGameInfo["number"].get<GameNumber::Type>(),
        jsonGameInfo["set"].get<SetNumber::Type>(),
        SetFormat(jsonGameInfo["format"].get<std::string>().c_str())
    ));

    const TimeStampMS firstFrameTimeStampMs = time_qt_to_milli_seconds_since_epoch(jsonGameInfo["date"].get<std::string>().c_str());

    const std::string streamDecoded = base64_decode(j["playerstates"].get<std::string>());
    StreamBuffer stream(streamDecoded.data(), static_cast<int>(streamDecoded.length()));
    for (int i = 0; i < session->fighterCount(); ++i)
    {
        int error = 0;
        const Frame::Type stateCount = stream.readBU32(&error);
        if (error)
            return nullptr;

        // zero states are invalid
        if (stateCount == 0)
            return nullptr;

        Frame::Type frameCounter = 0;
        for (Frame::Type f = 0; f < stateCount; ++f)
        {
            const Frame frame = stream.readBU32(&error);
            const float posx = static_cast<float>(stream.readBF64(&error));
            const float posy = static_cast<float>(stream.readBF64(&error));
            const float damage = static_cast<float>(stream.readBF64(&error));
            const float hitstun = static_cast<float>(stream.readBF64(&error));
            const float shield = static_cast<float>(stream.readBF64(&error));
            const FighterStatus status = stream.readBU16(&error);
            const FighterMotion motion = stream.readBU64(&error);
            const FighterHitStatus hit_status = stream.readU8(&error);
            const FighterStocks stocks = stream.readU8(&error);
            const FighterFlags flags = stream.readU8(&error);

            if (error)
                return nullptr;
            if (frame.value() == 0)
                return nullptr;

            // Usually only unique states are saved, which means there will be
            // gaps in between frames. Duplicate the current frame as many times
            // as necessary and make sure to update the time stamp and frame
            // counter
            for (; frameCounter < frame.value(); ++frameCounter)
            {
                // Version 1.2 did not timestamp each frame so we have to make a guesstimate
                // based on the timestamp of when the recording started and how many
                // frames passed since. This will not account for game pauses or
                // lag, but it should be good enough.
                const TimeStampMS frameTimeStamp =  firstFrameTimeStampMs +
                        DeltaTimeMS(frameCounter * 1000.0 / 60.0);
                session->frames_[i].emplace(frameTimeStamp, frame, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, flags);
            }
        }
    }

    // Ensure that every fighter has the same number of frames
    const int highestFrameIdx = std::max_element(session->frames_.begin(), session->frames_.end(),
        [](const Vector<FighterFrame>& a, const Vector<FighterFrame>& b){
            return a.count() < b.count();
    })->count();
    for (auto& frames : session->frames_)
        while (frames.count() < highestFrameIdx)
            frames.emplace(frames.back());

    // Cache winner
    session->winner_ = session->findWinner();

    return session.release();
}

// ----------------------------------------------------------------------------
SavedSession* SavedSession::loadVersion_1_3(const void* jptr)
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

    MappingInfo mappingInfo(0);

    const json jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"];
    if (jsonFighterStatusMapping.contains("base") == false || jsonFighterStatusMapping["base"].is_object() == false)
        return nullptr;
    if (jsonFighterStatusMapping.contains("specific") == false || jsonFighterStatusMapping["specific"].is_object() == false)
        return nullptr;

    for (const auto& [key, value] : jsonFighterStatusMapping["base"].items())
    {
        std::size_t pos;
        FighterStatus status(std::stoul(key, &pos));
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

        mappingInfo.fighterStatus.addBaseEnumName(status, value[0].get<std::string>().c_str());
    }
    for (const auto& [fighter, jsonSpecificMapping] : jsonFighterStatusMapping["specific"].items())
    {
        std::size_t pos;
        FighterID fighterID(std::stoul(fighter, &pos));
        if (pos != fighter.length())
            return nullptr;
        if (jsonSpecificMapping.is_object() == false)
            return nullptr;

        for (const auto& [key, value] : jsonSpecificMapping.items())
        {
            FighterStatus status(std::stoul(key, &pos));
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

            mappingInfo.fighterStatus.addFighterSpecificEnumName(status, fighterID, value[0].get<std::string>().c_str());
        }
    }

    for (const auto& [key, value] : jsonMappingInfo["fighterid"].items())
    {
        std::size_t pos;
        FighterID fighterID(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.fighterID.add(fighterID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jsonMappingInfo["stageid"].items())
    {
        std::size_t pos;
        StageID stageID(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.stageID.add(stageID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jsonMappingInfo["hitstatus"].items())
    {
        std::size_t pos;
        FighterHitStatus hitStatusID(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.hitStatus.add(hitStatusID, value.get<std::string>().c_str());
    }

    SmallVector<FighterID, 2> playerFighterIDs;
    SmallVector<SmallString<15>, 2> playerTags;
    SmallVector<SmallString<15>, 2> playerNames;
    for (const auto& info : jsonPlayerInfo)
    {
        if (info.contains("fighterid") == false || info["fighterid"].is_number() == false)
            return nullptr;
        if (info.contains("tag") == false || info["tag"].is_string() == false)
            return nullptr;
        if (info.contains("name") == false || info["name"].is_string() == false)
            return nullptr;

        playerFighterIDs.emplace(info["fighterid"].get<FighterID::Type>());
        playerTags.emplace(info["tag"].get<std::string>().c_str());
        playerNames.emplace(info["name"].get<std::string>().c_str());
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

    std::unique_ptr<SavedGameSession> session(new SavedGameSession(
        std::move(mappingInfo),
        jsonGameInfo["stageid"].get<StageID::Type>(),
        std::move(playerFighterIDs),
        std::move(playerTags),
        std::move(playerNames),
        jsonGameInfo["number"].get<GameNumber::Type>(),
        jsonGameInfo["set"].get<SetNumber::Type>(),
        SetFormat(jsonGameInfo["format"].get<std::string>().c_str())
    ));

    const TimeStampMS firstFrameTimeStampMs = time_qt_to_milli_seconds_since_epoch(jsonGameInfo["date"].get<std::string>().c_str());

    const std::string streamDecoded = base64_decode(j["playerstates"].get<std::string>());
    StreamBuffer stream(streamDecoded.data(), static_cast<int>(streamDecoded.length()));
    for (int i = 0; i < session->fighterCount(); ++i)
    {
        int error = 0;
        const Frame::Type stateCount = stream.readLU32(&error);
        if (error)
            return nullptr;

        // zero states are invalid
        if (stateCount == 0)
            return nullptr;

        Frame::Type frameCounter = 0;
        for (Frame::Type f = 0; f < stateCount; ++f)
        {
            const Frame frame = stream.readLU32(&error);
            const float posx = stream.readLF32(&error);
            const float posy = stream.readLF32(&error);
            const float damage = stream.readLF32(&error);
            const float hitstun = stream.readLF32(&error);
            const float shield = stream.readLF32(&error);
            const FighterStatus status = stream.readLU16(&error);
            const uint32_t motion_l = stream.readLU32(&error);
            const uint8_t motion_h = stream.readU8(&error);
            const FighterHitStatus hit_status = stream.readU8(&error);
            const FighterStocks stocks = stream.readU8(&error);
            const uint8_t flags = stream.readU8(&error);

            if (error)
                return nullptr;
            if (frame.value() == 0)
                return nullptr;

            const FighterMotion motion = static_cast<uint64_t>(motion_l)
                                       | (static_cast<uint64_t>(motion_h) << 32);

            // Usually only unique states are saved, which means there will be
            // gaps in between frames. Duplicate the current frame as many times
            // as necessary and make sure to update the time stamp and frame
            // counter
            for (; frameCounter < frame.value(); ++frameCounter)
            {
                // Version 1.3 did not timestamp each frame so we have to make a guesstimate
                // based on the timestamp of when the recording started and how many
                // frames passed since. This will not account for game pauses or
                // lag, but it should be good enough.
                const TimeStampMS frameTimeStamp =  firstFrameTimeStampMs +
                        DeltaTimeMS(frameCounter * 1000.0 / 60.0);
                session->frames_[i].emplace(frameTimeStamp, frameCounter + 1, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, flags);
            }
        }
    }

    // Ensure that every fighter has the same number of frames
    const int highestFrameIdx = std::max_element(session->frames_.begin(), session->frames_.end(),
        [](const Vector<FighterFrame>& a, const Vector<FighterFrame>& b){
            return a.count() < b.count();
    })->count();
    for (auto& frames : session->frames_)
        while (frames.count() < highestFrameIdx)
            frames.emplace(frames.back());

    // Cache winner
    session->winner_ = session->findWinner();

    return session.release();
}

// ----------------------------------------------------------------------------
SavedSession* SavedSession::loadVersion_1_4(const void* jptr)
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

    MappingInfo mappingInfo(0);

    const json jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"];
    if (jsonFighterStatusMapping.contains("base") == false || jsonFighterStatusMapping["base"].is_object() == false)
        return nullptr;
    if (jsonFighterStatusMapping.contains("specific") == false)
        return nullptr;

    for (const auto& [key, value] : jsonFighterStatusMapping["base"].items())
    {
        std::size_t pos;
        FighterStatus status(std::stoul(key, &pos));
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

        mappingInfo.fighterStatus.addBaseEnumName(status, value[0].get<std::string>().c_str());
    }

    if (!jsonFighterStatusMapping["specific"].is_null())
    {
        for (const auto& [fighter, jsonSpecificMapping] : jsonFighterStatusMapping["specific"].items())
        {
            std::size_t pos;
            FighterID fighterID(std::stoul(fighter, &pos));
            if (pos != fighter.length())
                return nullptr;
            if (jsonSpecificMapping.is_object() == false)
                return nullptr;

            for (const auto& [key, value] : jsonSpecificMapping.items())
            {
                FighterStatus status(std::stoul(key, &pos));
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

                mappingInfo.fighterStatus.addFighterSpecificEnumName(status, fighterID, value[0].get<std::string>().c_str());
            }
        }
    }

    for (const auto& [key, value] : jsonMappingInfo["fighterid"].items())
    {
        std::size_t pos;
        FighterID fighterID(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.fighterID.add(fighterID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jsonMappingInfo["stageid"].items())
    {
        std::size_t pos;
        StageID stageID(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.stageID.add(stageID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jsonMappingInfo["hitstatus"].items())
    {
        std::size_t pos;
        FighterHitStatus hitStatusID(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo.hitStatus.add(hitStatusID, value.get<std::string>().c_str());
    }

    SmallVector<FighterID, 2> playerFighterIDs;
    SmallVector<SmallString<15>, 2> playerTags;
    SmallVector<SmallString<15>, 2> playerNames;
    for (const auto& info : jsonPlayerInfo)
    {
        if (info.contains("fighterid") == false || info["fighterid"].is_number() == false)
            return nullptr;
        if (info.contains("tag") == false || info["tag"].is_string() == false)
            return nullptr;
        if (info.contains("name") == false || info["name"].is_string() == false)
            return nullptr;

        playerFighterIDs.emplace(info["fighterid"].get<FighterID::Type>());
        playerTags.emplace(info["tag"].get<std::string>().c_str());
        playerNames.emplace(info["name"].get<std::string>().c_str());
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

    std::unique_ptr<SavedGameSession> session(new SavedGameSession(
        std::move(mappingInfo),
        jsonGameInfo["stageid"].get<StageID::Type>(),
        std::move(playerFighterIDs),
        std::move(playerTags),
        std::move(playerNames),
        jsonGameInfo["number"].get<GameNumber::Type>(),
        jsonGameInfo["set"].get<SetNumber::Type>(),
        SetFormat(jsonGameInfo["format"].get<std::string>().c_str())
    ));

    const std::string streamDecoded = base64_decode(j["playerstates"].get<std::string>());
    StreamBuffer stream(streamDecoded.data(), static_cast<int>(streamDecoded.length()));
    for (int i = 0; i < session->fighterCount(); ++i)
    {
        int error = 0;
        const Frame::Type stateCount = stream.readLU32(&error);
        if (error)
            return nullptr;

        // zero states are invalid
        if (stateCount == 0)
            return nullptr;

        Frame::Type frameCounter = 0;
        for (Frame::Type f = 0; f < stateCount; ++f)
        {
            const TimeStampMS frameTimeStamp = stream.readLU64(&error);
            const Frame frame = stream.readLU32(&error);
            const float posx = stream.readLF32(&error);
            const float posy = stream.readLF32(&error);
            const float damage = stream.readLF32(&error);
            const float hitstun = stream.readLF32(&error);
            const float shield = stream.readLF32(&error);
            const FighterStatus status = stream.readLU16(&error);
            const uint32_t motion_l = stream.readLU32(&error);
            const uint8_t motion_h = stream.readU8(&error);
            const FighterHitStatus hit_status = stream.readU8(&error);
            const FighterStocks stocks = stream.readU8(&error);
            const uint8_t flags = stream.readU8(&error);

            if (error)
                return nullptr;
            if (frame.value() == 0)
                return nullptr;

            const FighterMotion motion = static_cast<uint64_t>(motion_l)
                                       | (static_cast<uint64_t>(motion_h) << 32);

            // Usually only unique states are saved, which means there will be
            // gaps in between frames. Duplicate the current frame as many times
            // as necessary and make sure to update the time stamp and frame
            // counter
            for (; frameCounter < frame.value(); ++frameCounter)
            {
                const TimeStampMS actualTimeStamp = frameTimeStamp +
                        DeltaTimeMS((frame.value() - frameCounter - 1) * 1000.0 / 60.0);
                session->frames_[i].emplace(actualTimeStamp, frameCounter + 1, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, flags);
            }
        }
    }

    // Ensure that every fighter has the same number of frames
    const int highestFrameIdx = std::max_element(session->frames_.begin(), session->frames_.end(),
        [](const Vector<FighterFrame>& a, const Vector<FighterFrame>& b){
            return a.count() < b.count();
    })->count();
    for (auto& frames : session->frames_)
        while (frames.count() < highestFrameIdx)
            frames.emplace(frames.back());

    // Cache winner
    session->winner_ = session->findWinner();

    return session.release();
}

}
