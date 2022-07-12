#include "rfcommon/Session.hpp"
#include "rfcommon/GameSession.hpp"
#include "rfcommon/TrainingSession.hpp"
#include "rfcommon/SessionListener.hpp"
#include "rfcommon/StreamBuffer.hpp"
#include "rfcommon/Endian.hpp"
#include "nlohmann/json.hpp"
#include "cpp-base64/base64.h"
#include "zlib.h"
#include <unordered_set>

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
    // Assume we're dealing with a modern format first
    FILE* fp = fopen(fileName.cStr(), "rb");
    if (fp == nullptr)
        return nullptr;

    // If this is a modern file, it will start with "RFR1"
    char magic[4];
    if (fread(magic, 1, 4, fp) != 4)
    {
        fclose(fp);
        return nullptr;
    }
    if (memcmp(magic, "RFR1", 4) == 0)
    {
        SavedSession* session = loadModern(fp);
        fclose(fp);
        return session;
    }

    // Legacy format was compressed json
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
        if ((session = loadLegacy_1_4(static_cast<const void*>(&j))) != nullptr)
            return session;
    }
    else if (version == "1.3")
    {
        if ((session = loadLegacy_1_3(static_cast<const void*>(&j))) != nullptr)
            return session;
    }
    else if (version == "1.2")
    {
        if ((session = loadLegacy_1_2(static_cast<const void*>(&j))) != nullptr)
            return session;
    }
    else if (version == "1.1")
    {
        if ((session = loadLegacy_1_1(static_cast<const void*>(&j))) != nullptr)
            return session;
    }
    else if (version == "1.0")
    {
        if ((session = loadLegacy_1_0(static_cast<const void*>(&j))) != nullptr)
            return session;
    }

    return nullptr;
}

// ----------------------------------------------------------------------------
SavedSession* SavedSession::loadLegacy_1_0(const void* jptr)
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

    /*std::unique_ptr<SavedGameSession> session(new SavedGameSession(
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
        const FramesLeft::Type stateCount = stream.readBU32(&error);
        if (error)
            return nullptr;

        // zero states are invalid
        if (stateCount == 0)
            return nullptr;

        FramesLeft::Type frameCounter = 0;
        for (FramesLeft::Type f = 0; f < stateCount; ++f)
        {
            const FramesLeft framesLeft = stream.readBU32(&error);
            const FighterStatus status = stream.readBU16(&error);
            const float damage = static_cast<float>(stream.readBF64(&error));
            const FighterStocks stocks = stream.readU8(&error);
            const FighterFlags flags(false, false);

            if (error)
                return nullptr;
            if (framesLeft.value() == 0)
                return nullptr;

            // Usually only unique states are saved, which means there will be
            // gaps in between frames. Duplicate the current frame as many times
            // as necessary and make sure to update the time stamp and frame
            // counter
            for (; frameCounter < framesLeft.value(); ++frameCounter)
            {
                // Version 1.0 did not timestamp each frame so we have to make a guesstimate
                // based on the timestamp of when the recording started and how many
                // frames passed since. This will not account for game pauses or
                // lag, but it should be good enough.
                const TimeStampMS frameTimeStamp =  firstFrameTimeStampMs +
                        DeltaTimeMS(frameCounter * 1000.0 / 60.0);
                session->frames_[i].emplace(frameTimeStamp, frameCounter, framesLeft, 0.0f, 0.0f, damage, 0.0f, 50.0f, status, 0, 0, stocks, flags);
            }
        }
    }

    // Ensure that every fighter has the same number of frames
    const int highestFrameIdx = std::max_element(session->frames_.begin(), session->frames_.end(),
        [](const Vector<FighterState>& a, const Vector<FighterState>& b){
            return a.count() < b.count();
    })->count();
    for (auto& frames : session->frames_)
        while (frames.count() < highestFrameIdx)
            frames.emplace(frames.back());

    // Cache winner
    session->winner_ = session->findWinner();

    return session.release();*/
}

// ----------------------------------------------------------------------------
SavedSession* SavedSession::loadLegacy_1_1(const void* jptr)
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

    /*std::unique_ptr<SavedGameSession> session(new SavedGameSession(
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
        const FramesLeft::Type stateCount = stream.readBU32(&error);
        if (error)
            return nullptr;

        // zero states are invalid
        if (stateCount == 0)
            return nullptr;

        FramesLeft::Type frameCounter = 0;
        for (FramesLeft::Type f = 0; f < stateCount; ++f)
        {
            const FramesLeft framesLeft = stream.readBU32(&error);
            const FighterStatus status = stream.readBU16(&error);
            const float damage = static_cast<float>(stream.readBF64(&error));
            const FighterStocks stocks = stream.readU8(&error);
            const FighterFlags flags(false, false);

            if (error)
                return nullptr;
            if (framesLeft.value() == 0)
                return nullptr;

            // Usually only unique states are saved, which means there will be
            // gaps in between frames. Duplicate the current frame as many times
            // as necessary and make sure to update the time stamp and frame
            // counter
            for (; frameCounter < framesLeft.value(); ++frameCounter)
            {
                // Version 1.1 did not timestamp each frame so we have to make a guesstimate
                // based on the timestamp of when the recording started and how many
                // frames passed since. This will not account for game pauses or
                // lag, but it should be good enough.
                const TimeStampMS frameTimeStamp =  firstFrameTimeStampMs +
                        DeltaTimeMS(frameCounter * 1000.0 / 60.0);
                session->frames_[i].emplace(frameTimeStamp, frameCounter, framesLeft, 0.0f, 0.0f, damage, 0.0f, 50.0f, status, 0, 0, stocks, flags);
            }
        }
    }

    // Ensure that every fighter has the same number of frames
    const int highestFrameIdx = std::max_element(session->frames_.begin(), session->frames_.end(),
        [](const Vector<FighterState>& a, const Vector<FighterState>& b){
            return a.count() < b.count();
    })->count();
    for (auto& frames : session->frames_)
        while (frames.count() < highestFrameIdx)
            frames.emplace(frames.back());

    // Cache winner
    session->winner_ = session->findWinner();

    return session.release();*/
}

// ----------------------------------------------------------------------------
SavedSession* SavedSession::loadLegacy_1_2(const void* jptr)
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

    /*std::unique_ptr<SavedGameSession> session(new SavedGameSession(
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
        const FramesLeft::Type stateCount = stream.readBU32(&error);
        if (error)
            return nullptr;

        // zero states are invalid
        if (stateCount == 0)
            return nullptr;

        FramesLeft::Type frameCounter = 0;
        for (FramesLeft::Type f = 0; f < stateCount; ++f)
        {
            const FramesLeft framesLeft = stream.readBU32(&error);
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
            if (framesLeft.value() == 0)
                return nullptr;

            // Usually only unique states are saved, which means there will be
            // gaps in between frames. Duplicate the current frame as many times
            // as necessary and make sure to update the time stamp and frame
            // counter
            for (; frameCounter < framesLeft.value(); ++frameCounter)
            {
                // Version 1.2 did not timestamp each frame so we have to make a guesstimate
                // based on the timestamp of when the recording started and how many
                // frames passed since. This will not account for game pauses or
                // lag, but it should be good enough.
                const TimeStampMS frameTimeStamp =  firstFrameTimeStampMs +
                        DeltaTimeMS(frameCounter * 1000.0 / 60.0);
                session->frames_[i].emplace(frameTimeStamp, frameCounter, framesLeft, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, flags);
            }
        }
    }

    // Ensure that every fighter has the same number of frames
    const int highestFrameIdx = std::max_element(session->frames_.begin(), session->frames_.end(),
        [](const Vector<FighterState>& a, const Vector<FighterState>& b){
            return a.count() < b.count();
    })->count();
    for (auto& frames : session->frames_)
        while (frames.count() < highestFrameIdx)
            frames.emplace(frames.back());

    // Cache winner
    session->winner_ = session->findWinner();

    return session.release();*/
}

// ----------------------------------------------------------------------------
SavedSession* SavedSession::loadLegacy_1_3(const void* jptr)
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

    /*std::unique_ptr<SavedGameSession> session(new SavedGameSession(
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
        const FramesLeft::Type stateCount = stream.readLU32(&error);
        if (error)
            return nullptr;

        // zero states are invalid
        if (stateCount == 0)
            return nullptr;

        FramesLeft::Type frameCounter = 0;
        for (FramesLeft::Type f = 0; f < stateCount; ++f)
        {
            const FramesLeft framesLeft = stream.readLU32(&error);
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
            if (framesLeft.value() == 0)
                return nullptr;

            const FighterMotion motion = static_cast<uint64_t>(motion_l)
                                       | (static_cast<uint64_t>(motion_h) << 32);

            // Usually only unique states are saved, which means there will be
            // gaps in between frames. Duplicate the current frame as many times
            // as necessary and make sure to update the time stamp and frame
            // counter
            for (; frameCounter < framesLeft.value(); ++frameCounter)
            {
                // Version 1.3 did not timestamp each frame so we have to make a guesstimate
                // based on the timestamp of when the recording started and how many
                // frames passed since. This will not account for game pauses or
                // lag, but it should be good enough.
                const TimeStampMS frameTimeStamp =  firstFrameTimeStampMs +
                        DeltaTimeMS(frameCounter * 1000.0 / 60.0);
                session->frames_[i].emplace(frameTimeStamp, frameCounter, framesLeft, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, flags);
            }
        }
    }

    // Ensure that every fighter has the same number of frames
    const int highestFrameIdx = std::max_element(session->frames_.begin(), session->frames_.end(),
        [](const Vector<FighterState>& a, const Vector<FighterState>& b){
            return a.count() < b.count();
    })->count();
    for (auto& frames : session->frames_)
        while (frames.count() < highestFrameIdx)
            frames.emplace(frames.back());

    // Cache winner
    session->winner_ = session->findWinner();

    return session.release();*/
}

// ----------------------------------------------------------------------------
SavedSession* SavedSession::loadLegacy_1_4(const void* jptr)
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

    /*std::unique_ptr<SavedGameSession> session(new SavedGameSession(
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
        const FramesLeft::Type stateCount = stream.readLU32(&error);
        if (error)
            return nullptr;

        // zero states are invalid
        if (stateCount == 0)
            return nullptr;

        FramesLeft::Type frameCounter = 0;
        for (FramesLeft::Type f = 0; f < stateCount; ++f)
        {
            const TimeStampMS frameTimeStamp = stream.readLU64(&error);
            const FramesLeft framesLeft = stream.readLU32(&error);
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
            if (framesLeft.value() == 0)
                return nullptr;

            const FighterMotion motion = static_cast<uint64_t>(motion_l)
                                       | (static_cast<uint64_t>(motion_h) << 32);

            // Usually only unique states are saved, which means there will be
            // gaps in between frames. Duplicate the current frame as many times
            // as necessary and make sure to update the time stamp and frame
            // counters
            for (; frameCounter < framesLeft.value(); ++frameCounter)
            {
                const TimeStampMS actualTimeStamp = frameTimeStamp +
                        DeltaTimeMS((framesLeft.value() - frameCounter - 1) * 1000.0 / 60.0);
                session->frames_[i].emplace(actualTimeStamp, frameCounter, framesLeft, posx, posy, damage, hitstun, shield, status, motion, hit_status, stocks, flags);
            }
        }
    }

    // Ensure that every fighter has the same number of frames
    const int highestFrameIdx = std::max_element(session->frames_.begin(), session->frames_.end(),
        [](const Vector<FighterState>& a, const Vector<FighterState>& b){
            return a.count() < b.count();
    })->count();
    for (auto& frames : session->frames_)
        while (frames.count() < highestFrameIdx)
            frames.emplace(frames.back());

    // Cache winner
    session->winner_ = session->findWinner();

    return session.release();*/
}

// ----------------------------------------------------------------------------
SavedSession* SavedSession::loadModern(FILE* fp)
{
    uint8_t numEntries;
    struct Entry
    {
        char type[4];
        uint32_t offset;
        uint32_t size;
    };

    // Read entry table
    SmallVector<Entry, 4> entryTable;
    if (fread(&numEntries, 1, 1, fp) != 1)
        return nullptr;
    for (int i = 0; i != numEntries; ++i)
    {
        Entry entry;
        if (fread(entry.type, 1, 4, fp) != 4)
            return nullptr;
        if (fread(&entry.offset, 1, 4, fp) != 4)
            return nullptr;
        if (fread(&entry.size, 1, 4, fp) != 4)
            return nullptr;

        entry.offset = fromLittleEndian32(entry.offset);
        entry.size = fromLittleEndian32(entry.size);
        entryTable.push(entry);
    }

    std::unique_ptr<SavedSession> session;
    Vector<Frame> frameData;
    for (const auto& entry : entryTable)
    {
        // Load session metadata
        if (memcmp(entry.type, "META", 4) == 0)
        {
            if (fseek(fp, entry.offset, SEEK_SET) != 0)
                return nullptr;

            Vector<char> jsonBlob(entry.size);
            if (fread(jsonBlob.data(), 1, entry.size, fp) != (size_t)entry.size)
                return nullptr;
            json j = json::parse(jsonBlob.begin(), jsonBlob.end(), nullptr, false);

            if (j["version"] == "1.5")
            {
                session.reset(loadJSON_1_5(static_cast<const void*>(&j)));
                if (session.get() == nullptr)
                    return nullptr;
            }
            else
            {
                // unsupported version
                return nullptr;
            }
        }
        // Load frame data
        else if (memcmp(entry.type, "FDAT", 4) == 0)
        {
            if (fseek(fp, entry.offset, SEEK_SET) != 0)
                return nullptr;

            StreamBuffer compressed(entry.size);
            size_t bytesRead = fread(compressed.get(), 1, entry.size, fp);
            if (bytesRead != (size_t)entry.size)
                return nullptr;
            compressed.seekW(entry.size);

            int error = 0;
            const uint8_t major = compressed.readU8(&error);  if (error) return nullptr;
            const uint8_t minor = compressed.readU8(&error);  if (error) return nullptr;

            if (major == 1 && minor == 5)
            {
                uLongf decompressedSize = compressed.readLU32(&error);
                if (error)
                    return nullptr;

                StreamBuffer decompressed(decompressedSize);
                int result = uncompress(
                        static_cast<uint8_t*>(decompressed.get()), &decompressedSize,
                        static_cast<const uint8_t*>(compressed.get()) + 6, compressed.capacity() - 6);
                if (result != Z_OK)
                    return nullptr;
                if (decompressedSize != (uLongf)decompressed.capacity())
                    return nullptr;
                decompressed.seekW(decompressedSize);
                frameData = loadFrameData_1_5(&decompressed);
            }
            else
            {
                // unsupported version
                return nullptr;
            }
        }
        // Load session mapping info, also known as "local" mapping info
        else if (memcmp(entry.type, "MINF", 4) == 0)
        {

        }
        else
        {
            // Unsupported binary blob, ignore
        }
    }

    if (session.get())
    {
        session->frames_ = std::move(frameData);

        // Cache winner
        session->winner_ = session->findWinner();
    }

    return session.release();
}

// ----------------------------------------------------------------------------
SavedSession* SavedSession::loadJSON_1_5(const void* jptr)
{
    const json& j = *static_cast<const json*>(jptr);
    if (j.contains("mappinginfo") == false || j["mappinginfo"].is_object() == false)
        return nullptr;
    if (j.contains("gameinfo") == false || j["gameinfo"].is_object() == false)
        return nullptr;
    if (j.contains("playerinfo") == false || j["playerinfo"].is_array() == false)
        return nullptr;

    const json jsonMappingInfo = j["mappinginfo"];
    const json jsonGameInfo = j["gameinfo"];
    const json jsonPlayerInfo = j["playerinfo"];

    if (jsonGameInfo["type"] != "match")
        return nullptr;

    if (jsonMappingInfo.contains("fighterstatus") == false || jsonMappingInfo["fighterstatus"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("fighterid") == false || jsonMappingInfo["fighterid"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("stageid") == false)
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

    return new SavedGameSession(
        std::move(mappingInfo),
        jsonGameInfo["stageid"].get<StageID::Type>(),
        std::move(playerFighterIDs),
        std::move(playerTags),
        std::move(playerNames),
        jsonGameInfo["number"].get<GameNumber::Type>(),
        jsonGameInfo["set"].get<SetNumber::Type>(),
        SetFormat(jsonGameInfo["format"].get<std::string>().c_str())
    );
}

// ----------------------------------------------------------------------------
Vector<Frame> SavedSession::loadFrameData_1_5(StreamBuffer* data)
{
    Vector<Frame> frames;

    int error = 0;
    FramesLeft::Type frameCount = data->readLU32(&error);
    int fighterCount = data->readU8(&error);
    if (error)
        return Vector<Frame>();

    for (FramesLeft::Type f = 0; f < frameCount; ++f)
    {
        SmallVector<FighterState, 2> frame;
        for (int fighter = 0; fighter != fighterCount; ++fighter)
        {
            const TimeStamp timeStamp = TimeStamp::fromMillis(data->readLU64(&error));
            const FramesLeft framesLeft = data->readLU32(&error);
            const float posx = data->readLF32(&error);
            const float posy = data->readLF32(&error);
            const float damage = data->readLF32(&error);
            const float hitstun = data->readLF32(&error);
            const float shield = data->readLF32(&error);
            const FighterStatus status = data->readLU16(&error);
            const uint32_t motion_l = data->readLU32(&error);
            const uint8_t motion_h = data->readU8(&error);
            const FighterHitStatus hitStatus = data->readU8(&error);
            const FighterStocks stocks = data->readU8(&error);
            const uint8_t flags = data->readU8(&error);

            if (error)
                return Vector<Frame>();

            const FighterMotion motion = static_cast<uint64_t>(motion_l)
                                       | (static_cast<uint64_t>(motion_h) << 32);

            frame.emplace(
                timeStamp,
                FrameNumber(f),
                framesLeft,
                posx,
                posy,
                damage,
                hitstun,
                shield,
                status,
                motion,
                hitStatus,
                stocks,
                flags);
        }
        frames.push(std::move(frame));
    }

    return frames;
}


// ----------------------------------------------------------------------------
Session::Session(
        MappingInfo&& mapping,
        StageID stageID,
        SmallVector<FighterID, 2>&& fighterIDs,
        SmallVector<SmallString<15>, 2>&& tags)
    : mappingInfo_(std::move(mapping))
    , stageID_(stageID)
    , fighterIDs_(std::move(fighterIDs))
    , tags_(std::move(tags))
{
    assert(fighterIDs_.count() == tags_.count());
}

// ----------------------------------------------------------------------------
Session::~Session()
{
}

// ----------------------------------------------------------------------------
bool Session::save(const String& fileName)
{
    if (frameCount() == 0)
        return false;

    struct FighterStatusHasherStd {
        std::size_t operator()(const FighterStatus& status) const {
            return std::hash<FighterStatus::Type>()(status.value());
        }
    };
    struct FighterHitStatusHasherStd {
        std::size_t operator()(const FighterHitStatus& hitStatus) const {
            return std::hash<FighterHitStatus::Type>()(hitStatus.value());
        }
    };
    struct FighterIDHasherStd {
        std::size_t operator()(const FighterID& fighterID) const {
            return std::hash<FighterID::Type>()(fighterID.value());
        }
    };

    // Create sets of the IDs that were used in game so we don't end up saving
    // every ID
    std::unordered_set<FighterStatus, FighterStatusHasherStd> usedStatuses;
    std::unordered_set<FighterHitStatus, FighterHitStatusHasherStd> usedHitStatuses;
    for (const auto& frame : frames_)
        for (const auto& fighter : frame)
        {
            usedStatuses.insert(fighter.status());
            usedHitStatuses.insert(fighter.hitStatus());
        }
    std::unordered_set<FighterID, FighterIDHasherStd> usedFighterIDs;
    for (const auto& fighterID : fighterIDs_)
        usedFighterIDs.insert(fighterID);

    json gameInfo = {
        {"stageid", stageID_.value()},
        {"timestampstart", firstFrame().fighter(0).timeStamp().millis()},
        {"timestampend", lastFrame().fighter(0).timeStamp().millis()}
    };

    if (GameSession* gs = dynamic_cast<GameSession*>(this))
    {
        gameInfo["type"] = "match";
        gameInfo["format"] = gs->format().description().cStr();
        gameInfo["number"] = gs->gameNumber().value();
        gameInfo["set"] = gs->setNumber().value();
        gameInfo["winner"] = gs->winner();
    }
    else if (TrainingSession* ts = dynamic_cast<TrainingSession*>(this))
    {
        (void)ts;
        gameInfo["type"] = "training";
    }
    else
    {
        return false;
    }

    json videoInfo = {
        {"filename", ""},
        {"filepath", ""},
        {"offsetms", ""}
    };

    json fighterBaseStatusMapping;
    const auto& baseEnumNames = mappingInfo_.fighterStatus.baseEnumNames();
    for (const auto& it : baseEnumNames)
    {
        // Skip saving enums that aren't actually used in the set of player states
        if (usedStatuses.find(it->key()) == usedStatuses.end())
            continue;

        /*const QString* shortName = mappingInfo_.fighterStatus.mapToShortName(it.key());
        const QString* customName = mappingInfo_.fighterStatus.mapToCustom(it.key());*/

        fighterBaseStatusMapping[it->key().toStdString()] = {it->value().cStr(), "", ""};
    }

    json fighterSpecificStatusMapping;
    const auto& specificEnumNames = mappingInfo_.fighterStatus.fighterSpecificEnumNames();
    for (const auto& fighter : specificEnumNames)
    {
        // Skip saving enums for fighters that aren't being used
        if (usedFighterIDs.find(fighter->key()) == usedFighterIDs.end())
            continue;

        json specificMapping = json::object();
        for (const auto& it : fighter->value())
        {
            // Skip saving enums that aren't actually used in the set of player states
            if (usedStatuses.find(it->key()) == usedStatuses.end())
                continue;

            /*const QString* shortName = mappingInfo_.fighterStatus.mapToShortName(it.key());
            const QString* customName = mappingInfo_.fighterStatus.mapToCustom(it.key());*/

            specificMapping[it->key().toStdString()] = {it->value().cStr(), "", ""};
        }

        if (specificMapping.size() > 0)
            fighterSpecificStatusMapping[fighter->key().toStdString()] = specificMapping;
    }

    json fighterStatusMapping = {
        {"base", fighterBaseStatusMapping},
        {"specific", fighterSpecificStatusMapping}
    };

    json fighterIDMapping;
    const auto& fighterIDMap = mappingInfo_.fighterID.get();
    for (const auto& it : fighterIDMap)
        if (usedFighterIDs.find(it->key()) != usedFighterIDs.end())
            fighterIDMapping[it->key().toStdString()] = it->value().cStr();

    json stageIDMapping;
    const auto& stageIDMap = mappingInfo_.stageID.get();
    for (const auto& it : stageIDMap)
        if (it->key() == stageID_)  // Only care about saving the stage that was played on
            stageIDMapping[it->key().toStdString()] = it->value().cStr();

    json hitStatusMapping;
    const auto& hitStatusMap = mappingInfo_.hitStatus.get();
    for (const auto& it : hitStatusMap)
        if (usedHitStatuses.find(it.key()) != usedHitStatuses.end())
            hitStatusMapping[it.key().toStdString()] = it.value().cStr();

    json mappingInfo = {
        {"fighterstatus", fighterStatusMapping},
        {"fighterid", fighterIDMapping},
        {"stageid", stageIDMapping},
        {"hitstatus", hitStatusMapping}
    };

    json playerInfo = json::array();
    for (int i = 0; i < fighterCount(); ++i)
    {
        playerInfo += {
            {"tag", tag(i).cStr()},
            {"name", name(i).cStr()},
            {"fighterid", fighterID(i).value()}
        };
    }

    json jsonRoot = {
        {"version", "1.5"},
        {"mappinginfo", mappingInfo},
        {"gameinfo", gameInfo},
        {"videoinfo", videoInfo},
        {"playerinfo", playerInfo},
    };

    const int fighterStateSize =
            sizeof(TimeStamp::Type) +
            sizeof(FramesLeft::Type) +
            sizeof(frames_[0].fighter(0).posx()) +
            sizeof(frames_[0].fighter(0).posy()) +
            sizeof(frames_[0].fighter(0).damage()) +
            sizeof(frames_[0].fighter(0).hitstun()) +
            sizeof(frames_[0].fighter(0).shield()) +
            sizeof(FighterStatus::Type) +
            5 + // motion is a hash40 (40 bits)
            sizeof(FighterHitStatus::Type) +
            sizeof(FighterStocks::Type) +
            sizeof(uint8_t);  // flags
    const int frameSize = fighterStateSize * fighterCount();

    StreamBuffer frameData(5 + frameSize * frameCount());
    frameData.writeLU32(frames_.count());
    frameData.writeU8(fighterCount());
    for (const auto& frame : frames_)
    {
        for (const auto& fighter : frame)
        {
            frameData
                .writeLU64(fighter.timeStamp().millis())
                .writeLU32(fighter.framesLeft().value())
                .writeLF32(fighter.posx())
                .writeLF32(fighter.posy())
                .writeLF32(fighter.damage())
                .writeLF32(fighter.hitstun())
                .writeLF32(fighter.shield())
                .writeLU16(fighter.status().value())
                .writeLU32(fighter.motion().lower())
                .writeU8(fighter.motion().upper())
                .writeU8(fighter.hitStatus().value())
                .writeU8(fighter.stocks().value())
                .writeU8(fighter.flags().value());
        }
    }
    assert(frameData.bytesWritten() == 5 + frameSize * frameCount());

    StreamBuffer compressedFrameData(compressBound(frameData.bytesWritten()) + 6);
    compressedFrameData.writeU8(1);  // Major version
    compressedFrameData.writeU8(5);  // Minor version
    compressedFrameData.writeLU32(frameData.bytesWritten());  // Decompressed size
    uLongf compressedSize = compressedFrameData.capacity() - 6;
    if (compress2(
            static_cast<uint8_t*>(compressedFrameData.get()) + 6, &compressedSize,
            static_cast<const uint8_t*>(frameData.get()), frameData.bytesWritten(), 9) != Z_OK)
    {
        return false;
    }
    compressedFrameData.seekW(compressedSize + 6);

    FILE* file;
    const char magic[] = {'R', 'F', 'R', '1'};
    const char blobTypeMETA[] = {'M', 'E', 'T', 'A'};
    const char blobTypeFrameData[] = {'F', 'D', 'A', 'T'};
    const std::string jsonAsString = jsonRoot.dump();
    const uint8_t entriesLE = 2;  // Two binary blobs exist in this file
    const uint32_t jsonBlobOffsetLE = toLittleEndian32(4 + 1 + 12 + 12);
    const uint32_t jsonSizeLE = toLittleEndian32(jsonAsString.length());
    const uint32_t frameDataOffsetLE = toLittleEndian32(4 + 1 + 12 + 12 + jsonAsString.length());
    const uint32_t frameDataSizeLE = toLittleEndian32(compressedFrameData.bytesWritten());

    file = fopen(fileName.cStr(), "wb");
    if (file == nullptr)
        goto fopen_fail;

    // Write magic bytes
    if (fwrite(magic, 1, 4, file) != 4)
        goto write_fail;

    // Write contents table
    if (fwrite(&entriesLE, 1, 1, file) != 1)
        goto write_fail;

    if (fwrite(&blobTypeMETA, 1, 4, file) != 4)
        goto write_fail;
    if (fwrite(&jsonBlobOffsetLE, 1, 4, file) != 4)
        goto write_fail;
    if (fwrite(&jsonSizeLE, 1, 4, file) != 4)
        goto write_fail;

    if (fwrite(&blobTypeFrameData, 1, 4, file) != 4)
        goto write_fail;
    if (fwrite(&frameDataOffsetLE, 1, 4, file) != 4)
        goto write_fail;
    if (fwrite(&frameDataSizeLE, 1, 4, file) != 4)
        goto write_fail;

    // Write json blob
    if (fwrite(jsonAsString.data(), 1, jsonAsString.length(), file) != static_cast<int>(jsonAsString.length()))
        goto write_fail;

    // Write frame data blob
    if (fwrite(compressedFrameData.get(), 1, compressedFrameData.bytesWritten(), file) != static_cast<int>(compressedFrameData.bytesWritten()))
        goto write_fail;

    if (fclose(file) != 0)
        goto write_fail;

    return true;

    write_fail:
        fclose(file);
        remove(fileName.cStr());
    fopen_fail:;
        /*
        if (QMessageBox::warning(nullptr, "Failed to save recording", QString("Failed to open file for writing: ") + f.fileName() + "\n\nWould you like to save the file manually?", QMessageBox::Save | QMessageBox::Discard) == QMessageBox::Save)
        {
            QFileDialog::getSaveFileName(nullptr, "Save Recording", f.fileName());
        }*/
    return false;
}

// ----------------------------------------------------------------------------
void Session::addFrame(Frame&& frame)
{
    // Sanity checks
#ifndef NDEBUG
    for (int i = 1; i < frame.fighterCount(); ++i)
    {
        assert(frame.fighter(0).framesLeft() == frame.fighter(i).framesLeft());
        assert(frame.fighter(0).frameNumber() == frame.fighter(i).frameNumber());
    }
#endif

    frames_.push(std::move(frame));

    // If any fighter state is different from the previous one, notify
    if (frames_.count() < 2 || frames_.back(1).hasSameDataAs(frames_.back(2)))
        dispatcher.dispatch(&SessionListener::onRunningSessionNewUniqueFrame, frames_.count() - 1, frames_.back());

    // The UI cares about every frame
    dispatcher.dispatch(&SessionListener::onRunningSessionNewFrame, frames_.count() - 1, frames_.back());

    // Winner might have changed
    if (metaData_.type() == SessionMetaData::GAME)
    {
        metaData_.setWinner(findWinner());
    }
}

// ----------------------------------------------------------------------------
int Session::findWinner() const
{
    assert(frameCount() > 0);

    // The winner is the player with most stocks and least damage
    int winneridx = 0;
    for (int i = 0; i != fighterCount(); ++i)
    {
        const auto& current = frames_.back().fighter(i);
        const auto& winner = frames_.back().fighter(winneridx);

        if (current.stocks() > winner.stocks())
            winneridx = i;
        else if (current.stocks() == winner.stocks())
            if (current.damage() < winner.damage())
                winneridx = i;
    }

    return winneridx;
}

}
