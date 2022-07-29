#include "rfcommon/Endian.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/SessionMetaData.hpp"
#include "rfcommon/StreamBuffer.hpp"
#include "rfcommon/time.h"
#include "nlohmann/json.hpp"
#include "cpp-base64/base64.h"
#include "zlib.h"
#include <unordered_set>

namespace rfcommon {

using nlohmann::json;

static bool loadLegacy_1_0(
        const json& jptr,
        Reference<SessionMetaData>* metaData,
        Reference<MappingInfo>* mappingInfo,
        Reference<FrameData>* frameData);
static bool loadLegacy_1_1(
        const json& jptr,
        Reference<SessionMetaData>* metaData,
        Reference<MappingInfo>* mappingInfo,
        Reference<FrameData>* frameData);
static bool loadLegacy_1_2(
        const json& jptr,
        Reference<SessionMetaData>* metaData,
        Reference<MappingInfo>* mappingInfo,
        Reference<FrameData>* frameData);
static bool loadLegacy_1_3(
        const json& jptr,
        Reference<SessionMetaData>* metaData,
        Reference<MappingInfo>* mappingInfo,
        Reference<FrameData>* frameData);
static bool loadLegacy_1_4(
        const json& jptr,
        Reference<SessionMetaData>* metaData,
        Reference<MappingInfo>* mappingInfo,
        Reference<FrameData>* frameData);
static bool loadModern(
        FILE* fp,
        Reference<SessionMetaData>* metaData,
        Reference<MappingInfo>* mappingInfo,
        Reference<FrameData>* frameData);
static int findWinner(const SmallVector<FighterState, 4>& frame);

static const char* magic               = "RFR1";
static const char* blobTypeMeta        = "META";
static const char* blobTypeMappingInfo = "MAPI";
static const char* blobTypeFrameData   = "FDAT";

// ----------------------------------------------------------------------------
static std::string decompressGZFile(const char* fileName)
{
    unsigned char header;
    std::string out;
    gzFile f;
    FILE* fp;

    fp = fopen(fileName, "rb");
    if (fp == nullptr)
        goto fopen_failed;

    fread(&header, 1, 1, fp); if (header != 0x1f) goto header_error;
    fread(&header, 1, 1, fp); if (header != 0x8b) goto header_error;

    f = gzopen(fileName, "rb");
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
static std::string decompressQtZFile(const char* fileName)
{
#define CHUNK (256*1024)
    std::string out;
    FILE* fp = fopen(fileName, "rb");
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
static std::string readUncompressedFile(const char* fileName)
{
#define CHUNK (256*1024)
    std::string out;
    FILE* fp = fopen(fileName, "rb");
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
Session* Session::load(const char* fileName, MappingInfo* globalMappingInfo)
{
    Reference<SessionMetaData> metaData;
    Reference<MappingInfo> mappingInfo;
    Reference<FrameData> frameData;

    // Assume we're dealing with a modern format first
    FILE* fp = fopen(fileName, "rb");
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
        bool success = loadModern(fp, &metaData, &mappingInfo, &frameData);
        fclose(fp);

        if (success)
            return new Session(metaData, mappingInfo, globalMappingInfo, frameData);
        return nullptr;
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

    std::string version = j["version"];
    if (version == "1.4")
    {
        if (loadLegacy_1_4(j, &metaData, &mappingInfo, &frameData))
            return new Session(metaData, mappingInfo, globalMappingInfo, frameData);
    }
    else if (version == "1.3")
    {
        if (loadLegacy_1_3(j, &metaData, &mappingInfo, &frameData))
            return new Session(metaData, mappingInfo, globalMappingInfo, frameData);
    }
    else if (version == "1.2")
    {
        if (loadLegacy_1_2(j, &metaData, &mappingInfo, &frameData))
            return new Session(metaData, mappingInfo, globalMappingInfo, frameData);
    }
    else if (version == "1.1")
    {
        if (loadLegacy_1_1(j, &metaData, &mappingInfo, &frameData))
            return new Session(metaData, mappingInfo, globalMappingInfo, frameData);
    }
    else if (version == "1.0")
    {
        if (loadLegacy_1_0(j, &metaData, &mappingInfo, &frameData))
            return new Session(metaData, mappingInfo, globalMappingInfo, frameData);
    }

    return nullptr;
}

// ----------------------------------------------------------------------------
static bool loadLegacy_1_0(
        const json& j,
        Reference<SessionMetaData>* metaDataOut,
        Reference<MappingInfo>* mappingInfoOut,
        Reference<FrameData>* frameDataOut)
{
    const json jMappingInfo = j["mappinginfo"];
    const json jGameInfo = j["gameinfo"];
    const json jPlayerInfo = j["playerinfo"];
    const json jPlayerStates = j["playerstates"];

    const json jFighterStatuses = jMappingInfo["fighterstatus"];
    const json jFighterIDs = jMappingInfo["fighterid"];
    const json jStageIDs = jMappingInfo["stageid"];

    Reference<MappingInfo> mappingInfo(new MappingInfo(0));  // Since we're loading it, checksum is irrelevant
    for (const auto& [key, value] : jFighterIDs.items())
    {
        std::size_t pos;
        auto fighterID = FighterID::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return false;
        if (value.is_string() == false)
            return false;

        mappingInfo->fighter.add(fighterID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jStageIDs.items())
    {
        std::size_t pos;
        auto stageID = StageID::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return false;
        if (value.is_string() == false)
            return false;

        mappingInfo->stage.add(stageID, value.get<std::string>().c_str());
    }

    // skip loading status mappings, it was broken in 1.0

    SmallVector<FighterID, 2> playerFighterIDs;
    SmallVector<SmallString<15>, 2> playerTags;
    SmallVector<SmallString<15>, 2> playerNames;
    for (const auto& info : jPlayerInfo)
    {
        const json jFighterID = info["fighterid"];
        const json jTag = info["tag"];

        playerFighterIDs.push(FighterID::fromValue(jFighterID.get<FighterID::Type>()));
        playerTags.emplace(jTag.get<std::string>().c_str());
        playerNames.emplace(jTag.get<std::string>().c_str());  // "name" property didn't exist in 1.0
    }

    // There must be at least 2 fighters, otherwise the data is invalid
    if (playerFighterIDs.count() < 2)
        return false;
    if (playerFighterIDs.count() != playerTags.count() || playerFighterIDs.count() != playerNames.count())
        return false;

    const json jDate = jGameInfo["date"];
    const json jSetFormat = jGameInfo["format"];
    const json jGameNumber = jGameInfo["number"];
    const json jStageID = jGameInfo["stageid"];

    Reference<SessionMetaData> metaData(new GameSessionMetaData(
        StageID::fromValue(jStageID.get<StageID::Type>()),
        std::move(playerFighterIDs),
        std::move(playerTags),
        std::move(playerNames),
        GameNumber::fromValue(jGameNumber.get<GameNumber::Type>()),
        SetNumber::fromValue(1), // SetNumber did not exist in 1.0 yet
        SetFormat(jSetFormat.get<std::string>().c_str())));

    const auto firstFrameTimeStamp = TimeStamp::fromMillisSinceEpoch(
        time_qt_to_milli_seconds_since_epoch(jDate.get<std::string>().c_str()));

    std::string streamDecoded = base64_decode(jPlayerStates.get<std::string>());
    StreamBuffer stream(streamDecoded.data(), static_cast<int>(streamDecoded.length()));
    SmallVector<Vector<FighterState>, 2> frameData(metaData->fighterCount());
    for (int i = 0; i < metaData->fighterCount(); ++i)
    {
        int error = 0;
        const FramesLeft::Type stateCount = stream.readBU32(&error);
        if (error)
            return false;

        // zero states are invalid
        if (stateCount == 0)
            return false;

        FramesLeft::Type frameCounter = 0;
        for (FramesLeft::Type f = 0; f < stateCount; ++f)
        {
            const auto framesLeft = FramesLeft::fromValue(stream.readBU32(&error));
            const auto status = FighterStatus::fromValue(stream.readBU16(&error));
            const float damage = static_cast<float>(stream.readBF64(&error));
            const auto stocks = FighterStocks::fromValue(stream.readU8(&error));
            const auto flags = FighterFlags::fromFlags(false, false, false);

            if (error)
                return false;
            if (framesLeft.value() == 0)
                return false;

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
                const TimeStamp frameTimeStamp =  firstFrameTimeStamp +
                        DeltaTime::fromMillis(frameCounter * 1000.0 / 60.0);
                frameData[i].emplace(frameTimeStamp, FrameNumber::fromValue(frameCounter), framesLeft, 0.0f, 0.0f, damage, 0.0f, 50.0f, status, FighterMotion::makeInvalid(), FighterHitStatus::makeInvalid(), stocks, flags);
            }
        }
    }

    // Ensure that every fighter has the same number of frames
    const int highestFrameIdx = std::max_element(frameData.begin(), frameData.end(),
        [](const Vector<FighterState>& a, const Vector<FighterState>& b){
            return a.count() < b.count();
    })->count();
    for (auto& frames : frameData)
        while (frames.count() < highestFrameIdx)
            frames.emplace(frames.back());

    // Cache winner
    if (frameData.count() > 0)
    {
        SmallVector<FighterState, 4> frame;
        for (const auto& state : frameData.back())
            frame.push(state);
        const int winner = findWinner(frame);
        static_cast<GameSessionMetaData*>(metaData.get())->setWinner(winner);
    }

    *metaDataOut = metaData;
    *mappingInfoOut = mappingInfo;
    *frameDataOut = new FrameData(std::move(frameData));

    return true;
}

// ----------------------------------------------------------------------------
static bool loadLegacy_1_1(
        const json& j,
        Reference<SessionMetaData>* metaDataOut,
        Reference<MappingInfo>* mappingInfoOut,
        Reference<FrameData>* frameDataOut)
{
    const json jMappingInfo = j["mappinginfo"];
    const json jGameInfo = j["gameinfo"];
    const json jPlayerInfo = j["playerinfo"];
    const json jPlayerStates = j["playerstates"];

    const json jFighterStatuses = jMappingInfo["fighterstatus"];
    const json jFighterIDs = jMappingInfo["fighterid"];
    const json jStageIDs = jMappingInfo["stageid"];

    const json jFighterStatusMapping = jMappingInfo["fighterstatus"];
    const json jFighterBaseStatusMapping = jFighterStatusMapping["base"];
    const json jFighterSpecificStatusMapping = jFighterStatusMapping["specific"];

    Reference<MappingInfo> mappingInfo(new MappingInfo(0));  // Since we're loading it, checksum is irrelevant
    for (const auto& [key, value] : jFighterBaseStatusMapping.items())
    {
        std::size_t pos;
        auto status = FighterStatus::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return false;
        if (value.is_array() == false)
            return false;

        if (value.size() != 3)
            return false;
        if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
            return false;

        mappingInfo->status.addBaseName(status, value[0].get<std::string>().c_str());
    }
    for (const auto& [fighter, jSpecificMapping] : jFighterSpecificStatusMapping.items())
    {
        std::size_t pos;
        auto fighterID = FighterID::fromValue(std::stoul(fighter, &pos));
        if (pos != fighter.length())
            return false;
        if (jSpecificMapping.is_object() == false)
            return false;

        for (const auto& [key, value] : jSpecificMapping.items())
        {
            auto status = FighterStatus::fromValue(std::stoul(key, &pos));
            if (pos != key.length())
                return false;
            if (value.is_array() == false)
                return false;

            if (value.size() != 3)
                return false;
            if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
                return false;

            mappingInfo->status.addSpecificName(fighterID, status, value[0].get<std::string>().c_str());
        }
    }

    for (const auto& [key, value] : jFighterIDs.items())
    {
        std::size_t pos;
        auto fighterID = FighterID::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return false;
        if (value.is_string() == false)
            return false;

        mappingInfo->fighter.add(fighterID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jStageIDs.items())
    {
        std::size_t pos;
        auto stageID = StageID::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return false;
        if (value.is_string() == false)
            return false;

        mappingInfo->stage.add(stageID, value.get<std::string>().c_str());
    }

    SmallVector<FighterID, 2> playerFighterIDs;
    SmallVector<SmallString<15>, 2> playerTags;
    SmallVector<SmallString<15>, 2> playerNames;
    for (const auto& info : jPlayerInfo)
    {
        const json jFighterID = info["fighterid"];
        const json jTag = info["tag"];

        playerFighterIDs.push(FighterID::fromValue(jFighterID.get<FighterID::Type>()));
        playerTags.emplace(jTag.get<std::string>().c_str());
        playerNames.emplace(jTag.get<std::string>().c_str());  // "name" property didn't exist in 1.1
    }

    // There must be at least 2 fighters, otherwise the data is invalid
    if (playerFighterIDs.count() < 2)
        return false;
    if (playerFighterIDs.count() != playerTags.count() || playerFighterIDs.count() != playerNames.count())
        return false;

    const json jDate = jGameInfo["date"];
    const json jSetFormat = jGameInfo["format"];
    const json jGameNumber = jGameInfo["number"];
    const json jStageID = jGameInfo["stageid"];

    Reference<SessionMetaData> metaData(new GameSessionMetaData(
        StageID::fromValue(jStageID.get<StageID::Type>()),
        std::move(playerFighterIDs),
        std::move(playerTags),
        std::move(playerNames),
        GameNumber::fromValue(jGameNumber.get<GameNumber::Type>()),
        SetNumber::fromValue(1), // SetNumber did not exist in 1.0 yet
        SetFormat(jSetFormat.get<std::string>().c_str())));

    const auto firstFrameTimeStamp = TimeStamp::fromMillisSinceEpoch(
        time_qt_to_milli_seconds_since_epoch(jDate.get<std::string>().c_str()));

    const std::string streamDecoded = base64_decode(jPlayerStates.get<std::string>());
    StreamBuffer stream(streamDecoded.data(), static_cast<int>(streamDecoded.length()));
    SmallVector<Vector<FighterState>, 2> frameData(metaData->fighterCount());
    for (int i = 0; i < metaData->fighterCount(); ++i)
    {
        int error = 0;
        const FramesLeft::Type stateCount = stream.readBU32(&error);
        if (error)
            return false;

        // zero states are invalid
        if (stateCount == 0)
            return false;

        FramesLeft::Type frameCounter = 0;
        for (FramesLeft::Type f = 0; f < stateCount; ++f)
        {
            const auto framesLeft = FramesLeft::fromValue(stream.readBU32(&error));
            const auto status = FighterStatus::fromValue(stream.readBU16(&error));
            const float damage = static_cast<float>(stream.readBF64(&error));
            const auto stocks = FighterStocks::fromValue(stream.readU8(&error));
            const auto flags = FighterFlags::fromFlags(false, false, false);

            if (error)
                return false;
            if (framesLeft.value() == 0)
                return false;

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
                const TimeStamp frameTimeStamp =  firstFrameTimeStamp +
                        DeltaTime::fromMillis(frameCounter * 1000.0 / 60.0);
                frameData[i].emplace(frameTimeStamp, FrameNumber::fromValue(frameCounter), framesLeft, 0.0f, 0.0f, damage, 0.0f, 50.0f, status, FighterMotion::makeInvalid(), FighterHitStatus::makeInvalid(), stocks, flags);
            }
        }
    }

    // Ensure that every fighter has the same number of frames
    const int highestFrameIdx = std::max_element(frameData.begin(), frameData.end(),
        [](const Vector<FighterState>& a, const Vector<FighterState>& b){
            return a.count() < b.count();
    })->count();
    for (auto& frames : frameData)
        while (frames.count() < highestFrameIdx)
            frames.emplace(frames.back());

    // Cache winner
    if (frameData.count() > 0)
    {
        SmallVector<FighterState, 4> frame;
        for (const auto& state : frameData.back())
            frame.push(state);
        const int winner = findWinner(frame);
        static_cast<GameSessionMetaData*>(metaData.get())->setWinner(winner);
    }

    *metaDataOut = metaData;
    *mappingInfoOut = mappingInfo;
    *frameDataOut = new FrameData(std::move(frameData));

    return true;
}

// ----------------------------------------------------------------------------
static bool loadLegacy_1_2(
        const json& j,
        Reference<SessionMetaData>* metaDataOut,
        Reference<MappingInfo>* mappingInfoOut,
        Reference<FrameData>* frameDataOut)
{
    const json jMappingInfo = j["mappinginfo"];
    const json jGameInfo = j["gameinfo"];
    const json jPlayerInfo = j["playerinfo"];
    const json jPlayerStates = j["playerstates"];

    const json jFighterStatuses = jMappingInfo["fighterstatus"];
    const json jFighterIDs = jMappingInfo["fighterid"];
    const json jStageIDs = jMappingInfo["stageid"];

    const json jFighterStatusMapping = jMappingInfo["fighterstatus"];
    const json jFighterBaseStatusMapping = jFighterStatusMapping["base"];
    const json jFighterSpecificStatusMapping = jFighterStatusMapping["specific"];

    Reference<MappingInfo> mappingInfo(new MappingInfo(0));  // Since we're loading it, checksum is irrelevant
    for (const auto& [key, value] : jFighterBaseStatusMapping.items())
    {
        std::size_t pos;
        const auto status = FighterStatus::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return false;
        if (value.is_array() == false)
            return false;

        if (value.size() != 3)
            return false;
        if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
            return false;

        /*QString shortName  = arr[1].get<std::string>();
        QString customName = arr[2].get<std::string>();*/

        mappingInfo->status.addBaseName(status, value[0].get<std::string>().c_str());
    }
    for (const auto& [fighter, jSpecificMapping] : jFighterSpecificStatusMapping.items())
    {
        std::size_t pos;
        const auto fighterID = FighterID::fromValue(std::stoul(fighter, &pos));
        if (pos != fighter.length())
            return false;
        if (jSpecificMapping.is_object() == false)
            return false;

        for (const auto& [key, value] : jSpecificMapping.items())
        {
            const auto status = FighterStatus::fromValue(std::stoul(key, &pos));
            if (pos != key.length())
                return false;
            if (value.is_array() == false)
                return false;

            if (value.size() != 3)
                return false;
            if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
                return false;

            /*QString shortName  = arr[1].get<std::string>();
            QString customName = arr[2].get<std::string>();*/

            mappingInfo->status.addSpecificName(fighterID, status, value[0].get<std::string>().c_str());
        }
    }

    for (const auto& [key, value] : jFighterIDs.items())
    {
        std::size_t pos;
        const auto fighterID = FighterID::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return false;
        if (value.is_string() == false)
            return false;

        mappingInfo->fighter.add(fighterID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jStageIDs.items())
    {
        std::size_t pos;
        const auto stageID = StageID::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return false;
        if (value.is_string() == false)
            return false;

        mappingInfo->stage.add(stageID, value.get<std::string>().c_str());
    }

    SmallVector<FighterID, 2> playerFighterIDs;
    SmallVector<SmallString<15>, 2> playerTags;
    SmallVector<SmallString<15>, 2> playerNames;
    for (const auto& info : jPlayerInfo)
    {
        const json jFighterID = info["fighterid"];
        const json jTag = info["tag"];
        const json jName = info["name"];

        playerFighterIDs.push(FighterID::fromValue(jFighterID.get<FighterID::Type>()));
        playerTags.emplace(jTag.get<std::string>().c_str());
        playerNames.emplace(jName.get<std::string>().c_str());
    }

    // There must be at least 2 fighters, otherwise the data is invalid
    if (playerFighterIDs.count() < 2)
        return false;
    if (playerFighterIDs.count() != playerTags.count() || playerFighterIDs.count() != playerNames.count())
        return false;

    const json jStageID = jGameInfo["stageid"];
    const json jDate = jGameInfo["date"];
    const json jSetFormat = jGameInfo["format"];
    const json jGameNumber = jGameInfo["number"];
    const json jSetNumber = jGameInfo["set"];

    Reference<SessionMetaData> metaData(new GameSessionMetaData(
        StageID::fromValue(jStageID.get<StageID::Type>()),
        std::move(playerFighterIDs),
        std::move(playerTags),
        std::move(playerNames),
        GameNumber::fromValue(jGameNumber.get<GameNumber::Type>()),
        SetNumber::fromValue(jSetNumber.get<SetNumber::Type>()),
        SetFormat(jSetFormat.get<std::string>().c_str())));

    const TimeStamp firstFrameTimeStamp = TimeStamp::fromMillisSinceEpoch(
        time_qt_to_milli_seconds_since_epoch(jDate.get<std::string>().c_str()));

    const std::string streamDecoded = base64_decode(jPlayerStates.get<std::string>());
    StreamBuffer stream(streamDecoded.data(), static_cast<int>(streamDecoded.length()));
    SmallVector<Vector<FighterState>, 2> frameData(metaData->fighterCount());
    for (int i = 0; i < metaData->fighterCount(); ++i)
    {
        int error = 0;
        const FramesLeft::Type stateCount = stream.readBU32(&error);
        if (error)
            return false;

        // zero states are invalid
        if (stateCount == 0)
            return false;

        FramesLeft::Type frameCounter = 0;
        for (FramesLeft::Type f = 0; f < stateCount; ++f)
        {
            const auto framesLeft = FramesLeft::fromValue(stream.readBU32(&error));
            const float posx = static_cast<float>(stream.readBF64(&error));
            const float posy = static_cast<float>(stream.readBF64(&error));
            const float damage = static_cast<float>(stream.readBF64(&error));
            const float hitstun = static_cast<float>(stream.readBF64(&error));
            const float shield = static_cast<float>(stream.readBF64(&error));
            const auto status = FighterStatus::fromValue(stream.readBU16(&error));
            const auto motion = FighterMotion::fromValue(stream.readBU64(&error));
            const auto hitStatus = FighterHitStatus::fromValue(stream.readU8(&error));
            const auto stocks = FighterStocks::fromValue(stream.readU8(&error));
            const auto flags = FighterFlags::fromValue(stream.readU8(&error));

            if (error)
                return false;
            if (framesLeft.value() == 0)
                return false;

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
                const TimeStamp frameTimeStamp =  firstFrameTimeStamp +
                        DeltaTime::fromMillis(frameCounter * 1000.0 / 60.0);
                frameData[i].emplace(frameTimeStamp, FrameNumber::fromValue(frameCounter), framesLeft, posx, posy, damage, hitstun, shield, status, motion, hitStatus, stocks, flags);
            }
        }
    }

    // Ensure that every fighter has the same number of frames
    const int highestFrameIdx = std::max_element(frameData.begin(), frameData.end(),
        [](const Vector<FighterState>& a, const Vector<FighterState>& b){
            return a.count() < b.count();
    })->count();
    for (auto& frames : frameData)
        while (frames.count() < highestFrameIdx)
            frames.emplace(frames.back());

    // Cache winner
    if (frameData.count() > 0)
    {
        SmallVector<FighterState, 4> frame;
        for (const auto& state : frameData.back())
            frame.push(state);
        const int winner = findWinner(frame);
        static_cast<GameSessionMetaData*>(metaData.get())->setWinner(winner);
    }

    *metaDataOut = metaData;
    *mappingInfoOut = mappingInfo;
    *frameDataOut = new FrameData(std::move(frameData));

    return true;
}

// ----------------------------------------------------------------------------
static bool loadLegacy_1_3(
        const json& j,
        Reference<SessionMetaData>* metaDataOut,
        Reference<MappingInfo>* mappingInfoOut,
        Reference<FrameData>* frameDataOut)
{
    const json jMappingInfo = j["mappinginfo"];
    const json jGameInfo = j["gameinfo"];
    const json jPlayerInfo = j["playerinfo"];
    const json jPlayerStates = j["playerstates"];

    const json jFighterStatuses = jMappingInfo["fighterstatus"];
    const json jFighterIDs = jMappingInfo["fighterid"];
    const json jStageIDs = jMappingInfo["stageid"];
    const json jHitStatuses = jMappingInfo["hitstatus"];

    const json jFighterStatusMapping = jMappingInfo["fighterstatus"];
    const json jFighterBaseStatusMapping = jFighterStatusMapping["base"];
    const json jFighterSpecificStatusMapping = jFighterStatusMapping["specific"];

    Reference<MappingInfo> mappingInfo(new MappingInfo(0));  // Since we're loading it, checksum is irrelevant
    for (const auto& [key, value] : jFighterBaseStatusMapping.items())
    {
        std::size_t pos;
        const auto status = FighterStatus::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return false;
        if (value.is_array() == false)
            return false;

        if (value.size() != 3)
            return false;
        if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
            return false;

        /*QString shortName  = arr[1].get<std::string>();
        QString customName = arr[2].get<std::string>();*/

        mappingInfo->status.addBaseName(status, value[0].get<std::string>().c_str());
    }
    for (const auto& [fighter, jSpecificMapping] : jFighterSpecificStatusMapping.items())
    {
        std::size_t pos;
        const auto fighterID = FighterID::fromValue(std::stoul(fighter, &pos));
        if (pos != fighter.length())
            return false;
        if (jSpecificMapping.is_object() == false)
            return false;

        for (const auto& [key, value] : jSpecificMapping.items())
        {
            const auto status = FighterStatus::fromValue(std::stoul(key, &pos));
            if (pos != key.length())
                return false;
            if (value.is_array() == false)
                return false;

            if (value.size() != 3)
                return false;
            if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
                return false;

            /*QString shortName  = arr[1].get<std::string>();
            QString customName = arr[2].get<std::string>();*/

            mappingInfo->status.addSpecificName(fighterID, status, value[0].get<std::string>().c_str());
        }
    }

    for (const auto& [key, value] : jFighterIDs.items())
    {
        std::size_t pos;
        const auto fighterID = FighterID::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return false;
        if (value.is_string() == false)
            return false;

        mappingInfo->fighter.add(fighterID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jStageIDs.items())
    {
        std::size_t pos;
        const auto stageID = StageID::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return false;
        if (value.is_string() == false)
            return false;

        mappingInfo->stage.add(stageID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jHitStatuses.items())
    {
        std::size_t pos;
        const auto hitStatusID = FighterHitStatus::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return false;
        if (value.is_string() == false)
            return false;

        mappingInfo->hitStatus.add(hitStatusID, value.get<std::string>().c_str());
    }

    SmallVector<FighterID, 2> playerFighterIDs;
    SmallVector<SmallString<15>, 2> playerTags;
    SmallVector<SmallString<15>, 2> playerNames;
    for (const auto& info : jPlayerInfo)
    {
        const json jFighterID = info["fighterid"];
        const json jTag = info["tag"];
        const json jName = info["name"];

        playerFighterIDs.push(FighterID::fromValue(jFighterID.get<FighterID::Type>()));
        playerTags.emplace(jTag.get<std::string>().c_str());
        playerNames.emplace(jName.get<std::string>().c_str());
    }

    // There must be at least 2 fighters, otherwise the data is invalid
    if (playerFighterIDs.count() < 2)
        return false;
    if (playerFighterIDs.count() != playerTags.count() || playerFighterIDs.count() != playerNames.count())
        return false;

    const json jStageID = jGameInfo["stageid"];
    const json jDate = jGameInfo["date"];
    const json jSetFormat = jGameInfo["format"];
    const json jGameNumber = jGameInfo["number"];
    const json jSetNumber = jGameInfo["set"];
    const json jWinner = jGameInfo["winner"];

    Reference<SessionMetaData> metaData(new GameSessionMetaData(
        StageID::fromValue(jStageID.get<StageID::Type>()),
        std::move(playerFighterIDs),
        std::move(playerTags),
        std::move(playerNames),
        GameNumber::fromValue(jGameNumber.get<GameNumber::Type>()),
        SetNumber::fromValue(jSetNumber.get<SetNumber::Type>()),
        SetFormat(jSetFormat.get<std::string>().c_str())));

    static_cast<GameSessionMetaData*>(metaData.get())->setWinner(jWinner.get<int>());

    const auto firstFrameTimeStamp = TimeStamp::fromMillisSinceEpoch(
        time_qt_to_milli_seconds_since_epoch(jDate.get<std::string>().c_str()));

    const std::string streamDecoded = base64_decode(j["playerstates"].get<std::string>());
    StreamBuffer stream(streamDecoded.data(), static_cast<int>(streamDecoded.length()));
    SmallVector<Vector<FighterState>, 2> frameData(metaData->fighterCount());
    for (int i = 0; i < metaData->fighterCount(); ++i)
    {
        int error = 0;
        const FramesLeft::Type stateCount = stream.readLU32(&error);
        if (error)
            return false;

        // zero states are invalid
        if (stateCount == 0)
            return false;

        FramesLeft::Type frameCounter = 0;
        for (FramesLeft::Type f = 0; f < stateCount; ++f)
        {
            const auto framesLeft = FramesLeft::fromValue(stream.readLU32(&error));
            const float posx = stream.readLF32(&error);
            const float posy = stream.readLF32(&error);
            const float damage = stream.readLF32(&error);
            const float hitstun = stream.readLF32(&error);
            const float shield = stream.readLF32(&error);
            const auto status = FighterStatus::fromValue(stream.readLU16(&error));
            const uint32_t motion_l = stream.readLU32(&error);
            const uint8_t motion_h = stream.readU8(&error);
            const auto motion = FighterMotion::fromParts(motion_h, motion_l);
            const auto hitStatus = FighterHitStatus::fromValue(stream.readU8(&error));
            const auto stocks = FighterStocks::fromValue(stream.readU8(&error));
            const auto flags = FighterFlags::fromValue(stream.readU8(&error));

            if (error)
                return false;
            if (framesLeft.value() == 0)
                return false;

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
                const TimeStamp frameTimeStamp =  firstFrameTimeStamp +
                        DeltaTime::fromMillis(frameCounter * 1000.0 / 60.0);
                frameData[i].emplace(frameTimeStamp, FrameNumber::fromValue(frameCounter), framesLeft, posx, posy, damage, hitstun, shield, status, motion, hitStatus, stocks, flags);
            }
        }
    }

    // Ensure that every fighter has the same number of frames
    const int highestFrameIdx = std::max_element(frameData.begin(), frameData.end(),
        [](const Vector<FighterState>& a, const Vector<FighterState>& b){
            return a.count() < b.count();
    })->count();
    for (auto& frames : frameData)
        while (frames.count() < highestFrameIdx)
            frames.emplace(frames.back());

    // Winner sanity check
#ifndef NDEBUG
    if (frameData.count() > 0)
    {
        SmallVector<FighterState, 4> frame;
        for (const auto& state : frameData.back())
            frame.push(state);
        const int winner = findWinner(frame);
        assert(winner == static_cast<GameSessionMetaData*>(metaData.get())->winner());
    }
#endif

    *metaDataOut = metaData;
    *mappingInfoOut = mappingInfo;
    *frameDataOut = new FrameData(std::move(frameData));

    return true;
}

// ----------------------------------------------------------------------------
static bool loadLegacy_1_4(
        const json& j,
        Reference<SessionMetaData>* metaDataOut,
        Reference<MappingInfo>* mappingInfoOut,
        Reference<FrameData>* frameDataOut)
{
    const json jMappingInfo = j["mappinginfo"];
    const json jGameInfo = j["gameinfo"];
    const json jPlayerInfo = j["playerinfo"];
    const json jPlayerStates = j["playerstates"];

    const json jFighterStatuses = jMappingInfo["fighterstatus"];
    const json jFighterIDs = jMappingInfo["fighterid"];
    const json jStageIDs = jMappingInfo["stageid"];
    const json jHitStatuses = jMappingInfo["hitstatus"];

    const json jFighterStatusMapping = jMappingInfo["fighterstatus"];
    const json jFighterBaseStatusMapping = jFighterStatusMapping["base"];
    const json jFighterSpecificStatusMapping = jFighterStatusMapping["specific"];

    Reference<MappingInfo> mappingInfo(new MappingInfo(0));  // Since we're loading it, checksum is irrelevant
    for (const auto& [key, value] : jFighterBaseStatusMapping.items())
    {
        std::size_t pos;
        const auto status = FighterStatus::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return false;
        if (value.is_array() == false)
            return false;

        if (value.size() != 3)
            return false;
        if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
            return false;

        /*QString shortName  = arr[1].get<std::string>();
        QString customName = arr[2].get<std::string>();*/

        mappingInfo->status.addBaseName(status, value[0].get<std::string>().c_str());
    }
    for (const auto& [fighter, jsonSpecificMapping] : jFighterSpecificStatusMapping.items())
    {
        std::size_t pos;
        const auto fighterID = FighterID::fromValue(std::stoul(fighter, &pos));
        if (pos != fighter.length())
            return false;
        if (jsonSpecificMapping.is_object() == false)
            return false;

        for (const auto& [key, value] : jsonSpecificMapping.items())
        {
            const auto status = FighterStatus::fromValue(std::stoul(key, &pos));
            if (pos != key.length())
                return false;
            if (value.is_array() == false)
                return false;

            if (value.size() != 3)
                return false;
            if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
                return false;

            /*QString shortName  = arr[1].get<std::string>();
            QString customName = arr[2].get<std::string>();*/

            mappingInfo->status.addSpecificName(fighterID, status, value[0].get<std::string>().c_str());
        }
    }

    for (const auto& [key, value] : jFighterIDs.items())
    {
        std::size_t pos;
        const auto fighterID = FighterID::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return false;
        if (value.is_string() == false)
            return false;

        mappingInfo->fighter.add(fighterID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jStageIDs.items())
    {
        std::size_t pos;
        const auto stageID = StageID::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return false;
        if (value.is_string() == false)
            return false;

        mappingInfo->stage.add(stageID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jHitStatuses.items())
    {
        std::size_t pos;
        const auto hitStatusID = FighterHitStatus::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return false;
        if (value.is_string() == false)
            return false;

        mappingInfo->hitStatus.add(hitStatusID, value.get<std::string>().c_str());
    }

    SmallVector<FighterID, 2> playerFighterIDs;
    SmallVector<SmallString<15>, 2> playerTags;
    SmallVector<SmallString<15>, 2> playerNames;
    for (const auto& info : jPlayerInfo)
    {
        const json jFighterID = info["fighterid"];
        const json jTag = info["tag"];
        const json jName = info["name"];

        playerFighterIDs.push(FighterID::fromValue(jFighterID.get<FighterID::Type>()));
        playerTags.emplace(jTag.get<std::string>().c_str());
        playerNames.emplace(jName.get<std::string>().c_str());
    }

    // There must be at least 2 fighters, otherwise the data is invalid
    if (playerFighterIDs.count() < 2)
        return false;
    if (playerFighterIDs.count() != playerTags.count() || playerFighterIDs.count() != playerNames.count())
        return false;

    const json jStageID = jGameInfo["stageid"];
    const json jTimeStampSart = jGameInfo["timestampstart"];
    const json jTimeStampEnd = jGameInfo["timestampend"];
    const json jSetFormat = jGameInfo["format"];
    const json jGameNumber = jGameInfo["number"];
    const json jSetNumber = jGameInfo["set"];
    const json jWinner = jGameInfo["winner"];

    Reference<SessionMetaData> metaData(new GameSessionMetaData(
        StageID::fromValue(jStageID.get<StageID::Type>()),
        std::move(playerFighterIDs),
        std::move(playerTags),
        std::move(playerNames),
        GameNumber::fromValue(jGameNumber.get<GameNumber::Type>()),
        SetNumber::fromValue(jSetNumber.get<SetNumber::Type>()),
        SetFormat(jSetFormat.get<std::string>().c_str())));

    static_cast<GameSessionMetaData*>(metaData.get())->setWinner(jWinner.get<int>());

    const std::string streamDecoded = base64_decode(j["playerstates"].get<std::string>());
    StreamBuffer stream(streamDecoded.data(), static_cast<int>(streamDecoded.length()));
    SmallVector<Vector<FighterState>, 2> frameData(metaData->fighterCount());
    for (int i = 0; i < metaData->fighterCount(); ++i)
    {
        int error = 0;
        const FramesLeft::Type stateCount = stream.readLU32(&error);
        if (error)
            return false;

        // zero states are invalid
        if (stateCount == 0)
            return false;

        FramesLeft::Type frameCounter = 0;
        for (FramesLeft::Type f = 0; f < stateCount; ++f)
        {
            const auto frameTimeStamp = TimeStamp::fromMillisSinceEpoch(stream.readLU64(&error));
            const auto framesLeft = FramesLeft::fromValue(stream.readLU32(&error));
            const float posx = stream.readLF32(&error);
            const float posy = stream.readLF32(&error);
            const float damage = stream.readLF32(&error);
            const float hitstun = stream.readLF32(&error);
            const float shield = stream.readLF32(&error);
            const auto status = FighterStatus::fromValue(stream.readLU16(&error));
            const uint32_t motion_l = stream.readLU32(&error);
            const uint8_t motion_h = stream.readU8(&error);
            const auto motion = FighterMotion::fromParts(motion_h, motion_l);
            const auto hitStatus = FighterHitStatus::fromValue(stream.readU8(&error));
            const auto stocks = FighterStocks::fromValue(stream.readU8(&error));
            const auto flags = FighterFlags::fromValue(stream.readU8(&error));

            if (error)
                return false;
            if (framesLeft.value() == 0)
                return false;

            // Usually only unique states are saved, which means there will be
            // gaps in between frames. Duplicate the current frame as many times
            // as necessary and make sure to update the time stamp and frame
            // counters
            for (; frameCounter < framesLeft.value(); ++frameCounter)
            {
                const TimeStamp actualTimeStamp = frameTimeStamp +
                        DeltaTime::fromMillis((framesLeft.value() - frameCounter - 1) * 1000.0 / 60.0);
                frameData[i].emplace(actualTimeStamp, FrameNumber::fromValue(frameCounter), framesLeft, posx, posy, damage, hitstun, shield, status, motion, hitStatus, stocks, flags);
            }
        }
    }

    // Ensure that every fighter has the same number of frames
    const int highestFrameIdx = std::max_element(frameData.begin(), frameData.end(),
        [](const Vector<FighterState>& a, const Vector<FighterState>& b){
            return a.count() < b.count();
    })->count();
    for (auto& frames : frameData)
        while (frames.count() < highestFrameIdx)
            frames.emplace(frames.back());

    // Winner sanity check
#ifndef NDEBUG
    if (frameData.count() > 0)
    {
        SmallVector<FighterState, 4> frame;
        for (const auto& state : frameData.back())
            frame.push(state);
        const int winner = findWinner(frame);
        assert(winner == static_cast<GameSessionMetaData*>(metaData.get())->winner());
    }
#endif

    *metaDataOut = metaData;
    *mappingInfoOut = mappingInfo;
    *frameDataOut = new FrameData(std::move(frameData));

    return true;
}

// ----------------------------------------------------------------------------
static bool loadModern(
        FILE* fp,
        Reference<SessionMetaData>* metaData,
        Reference<MappingInfo>* mappingInfo,
        Reference<FrameData>* frameData)
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
        return false;
    for (int i = 0; i != numEntries; ++i)
    {
        Entry entry;
        if (fread(entry.type, 1, 4, fp) != 4)
            return false;
        if (fread(&entry.offset, 1, 4, fp) != 4)
            return false;
        if (fread(&entry.size, 1, 4, fp) != 4)
            return false;

        entry.offset = fromLittleEndian32(entry.offset);
        entry.size = fromLittleEndian32(entry.size);
        entryTable.push(entry);
    }

    for (const auto& entry : entryTable)
    {
        // Load session mapping info, also known as "local" mapping info
        if (memcmp(entry.type, blobTypeMappingInfo, 4) == 0)
        {
            if (fseek(fp, entry.offset, SEEK_SET) != 0)
                return false;

            *mappingInfo = MappingInfo::load(fp, entry.size);
            if (mappingInfo->isNull())
                return false;
        }
        // Load session metadata
        else if (memcmp(entry.type, blobTypeMeta, 4) == 0)
        {
            if (fseek(fp, entry.offset, SEEK_SET) != 0)
                return false;

            *metaData = SessionMetaData::load(fp, entry.size);
            if (metaData->isNull())
                return false;
        }
        // Load frame data
        else if (memcmp(entry.type, blobTypeFrameData, 4) == 0)
        {
            if (fseek(fp, entry.offset, SEEK_SET) != 0)
                return false;

            *frameData = FrameData::load(fp, entry.size);
            if (frameData->isNull())
                return false;
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
Session::Session(SessionMetaData* metaData, MappingInfo* mappingInfo, MappingInfo* globalMappingInfo, FrameData* frameData)
    : mappingInfo_(mappingInfo)
    , globalMappingInfo_(globalMappingInfo)
    , metaData_(metaData)
    , frameData_(frameData)
{
}

// ----------------------------------------------------------------------------
Session::~Session()
{
}

// ----------------------------------------------------------------------------
bool Session::save(const char* fileName)
{
    if (frameData_->frameCount() == 0)
        return false;

    const uint8_t numEntries = 3;
    struct Entry
    {
        char type[4];
        uint32_t offset;
        uint32_t size;
    } entries[numEntries];

    const uint32_t magicSize = 4;
    const uint32_t entryCountSize = 1;
    const uint32_t entrySize = 12;  // sizeof(Entry)
    const uint32_t headerSize = magicSize + entryCountSize + entrySize * numEntries;

    FILE* file;
    file = fopen(fileName, "wb");
    if (file == nullptr)
        goto fopen_fail;

    // We skip header writing the header until after data is written, because
    // there is not enough information yet
    if (fseek(file, headerSize, SEEK_SET) != 0)
        goto write_fail;

    // Write meta data
    memcpy(entries[0].type, blobTypeMeta, 4);
    entries[0].offset = headerSize;
    entries[0].size = metaData_->save(file);
    if (entries[0].size == 0)
        goto write_fail;

    // Write frame data
    memcpy(entries[1].type, blobTypeFrameData, 4);
    entries[1].offset = entries[0].offset + entries[0].size;
    entries[1].size = frameData_->save(file);
    if (entries[1].size == 0)
        goto write_fail;

    // Write local mapping info
    memcpy(entries[2].type, blobTypeMappingInfo, 4);
    entries[2].offset = entries[1].offset + entries[1].size;
    entries[2].size = mappingInfo_->saveFiltered(file, metaData_, frameData_);
    if (entries[2].size == 0)
        goto write_fail;

    // Rewind and write header
    if (fseek(file, 0, SEEK_SET) != 0)
        goto write_fail;

    // Write magic
    if (fwrite(magic, 1, 4, file) != 4)
        goto write_fail;

    // Write contents table
    if (fwrite(&numEntries, 1, 1, file) != 1)
        goto write_fail;
    for (uint8_t i = 0; i != numEntries; ++i)
    {
        uint32_t offsetLE = toLittleEndian32(entries[i].offset);
        uint32_t sizeLE = toLittleEndian32(entries[i].size);
        if (fwrite(entries[i].type, 1, 4, file) != 4)
            goto write_fail;
        if (fwrite(&offsetLE, 1, 4, file) != 4)
            goto write_fail;
        if (fwrite(&sizeLE, 1, 4, file) != 4)
            goto write_fail;
    }

    if (fclose(file) != 0)
        goto write_fail;

    return true;

    write_fail:
        fclose(file);
        remove(fileName);
    fopen_fail:;
        /*
        if (QMessageBox::warning(nullptr, "Failed to save recording", QString("Failed to open file for writing: ") + f.fileName() + "\n\nWould you like to save the file manually?", QMessageBox::Save | QMessageBox::Discard) == QMessageBox::Save)
        {
            QFileDialog::getSaveFileName(nullptr, "Save Recording", f.fileName());
        }*/
    return false;
}

// ----------------------------------------------------------------------------
void Session::onFrameDataNewUniqueFrame(int frameIdx, const Frame& frame)
{
    (void)frameIdx;

    // Winner might have changed
    if (metaData_->type() == SessionMetaData::GAME)
        static_cast<GameSessionMetaData*>(metaData_.get())->setWinner(findWinner(frame));
}

// ----------------------------------------------------------------------------
void Session::onFrameDataNewFrame(int frameIdx, const Frame& frame)
{
    (void)frameIdx; (void)frame;
}

// ----------------------------------------------------------------------------
static int findWinner(const SmallVector<FighterState, 4>& frame)
{
    // The winner is the player with most stocks and least damage
    int winneridx = 0;
    for (int i = 0; i != frame.count(); ++i)
    {
        const auto& current = frame[i];
        const auto& winner = frame[winneridx];

        if (current.stocks() > winner.stocks())
            winneridx = i;
        else if (current.stocks() == winner.stocks())
            if (current.damage() < winner.damage())
                winneridx = i;
    }

    return winneridx;
}

}
