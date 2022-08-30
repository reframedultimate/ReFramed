#include "rfcommon/Deserializer.hpp"
#include "rfcommon/Endian.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/FrameIndex.hpp"
#include "rfcommon/FramesLeft.hpp"
#include "rfcommon/FighterFlags.hpp"
#include "rfcommon/FighterStocks.hpp"
#include "rfcommon/Log.hpp"
#include "rfcommon/MappedFile.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/Profiler.hpp"
#include "rfcommon/Session.hpp"
#include "rfcommon/VideoMeta.hpp"
#include "rfcommon/VideoEmbed.hpp"
#include "rfcommon/time.h"
#include "nlohmann/json.hpp"
#include "cpp-base64/base64.h"
#include "zlib.h"
#include <unordered_set>

namespace rfcommon {

using nlohmann::json;

static bool loadLegacy_1_3(
        json& jptr,
        Reference<MetaData>* metaData,
        Reference<MappingInfo>* mappingInfo,
        Reference<FrameData>* frameData);
static bool loadLegacy_1_4(
        json& jptr,
        Reference<MetaData>* metaData,
        Reference<MappingInfo>* mappingInfo,
        Reference<FrameData>* frameData);
static int findWinner(const Frame<4>& frame);

static const char* magic               = "RFR1";
static const char* blobTypeMetaData    = "META";
static const char* blobTypeMappingInfo = "MAPI";
static const char* blobTypeFrameData   = "FDAT";
static const char* blobTypeVideoMeta   = "VIDM";
static const char* blobTypeVideoEmbed  = "VIDE";

// ----------------------------------------------------------------------------
static std::string decompressGZFile(const char* fileName)
{
    PROFILE(SessionGlobal, decompressGZFile);

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
    PROFILE(SessionGlobal, decompressQtZFile);

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
    PROFILE(SessionGlobal, readUncompressedFile);

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
static bool loadLegacy_1_3(
        json& j,
        Reference<MetaData>* metaDataOut,
        Reference<MappingInfo>* mappingInfoOut,
        Reference<FrameData>* frameDataOut)
{
    PROFILE(SessionGlobal, loadLegacy_1_3);

    json jMappingInfo = j["mappinginfo"];
    json jGameInfo = j["gameinfo"];
    json jPlayerInfo = j["playerinfo"];
    json jPlayerStates = j["playerstates"];

    json jFighterStatuses = jMappingInfo["fighterstatus"];
    json jFighterIDs = jMappingInfo["fighterid"];
    json jStageIDs = jMappingInfo["stageid"];
    json jHitStatuses = jMappingInfo["hitstatus"];

    json jFighterStatusMapping = jMappingInfo["fighterstatus"];
    json jFighterBaseStatusMapping = jFighterStatusMapping["base"];
    json jFighterSpecificStatusMapping = jFighterStatusMapping["specific"];

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
    SmallVector<String, 2> playerTags;
    SmallVector<String, 2> playerNames;
    for (const auto& info : jPlayerInfo)
    {
        json jFighterID = info["fighterid"];
        json jTag = info["tag"];
        json jName = info["name"];

        playerFighterIDs.push(FighterID::fromValue(jFighterID.get<FighterID::Type>()));
        playerTags.emplace(jTag.get<std::string>().c_str());
        playerNames.emplace(jName.get<std::string>().c_str());
    }

    // There must be at least 2 fighters, otherwise the data is invalid
    if (playerFighterIDs.count() < 2)
        return false;
    if (playerFighterIDs.count() != playerTags.count() || playerFighterIDs.count() != playerNames.count())
        return false;

    json jStageID = jGameInfo["stageid"];
    json jDate = jGameInfo["date"];
    json jSetFormat = jGameInfo["format"];
    json jGameNumber = jGameInfo["number"];
    json jSetNumber = jGameInfo["set"];
    json jWinner = jGameInfo["winner"];

    Reference<MetaData> metaData = MetaData::newSavedGameSession(
        TimeStamp::fromMillisSinceEpoch(0),
        TimeStamp::fromMillisSinceEpoch(0),
        StageID::fromValue(jStageID.get<StageID::Type>()),
        std::move(playerFighterIDs),
        std::move(playerTags),
        std::move(playerNames),
        GameNumber::fromValue(jGameNumber.get<GameNumber::Type>()),
        SetNumber::fromValue(jSetNumber.get<SetNumber::Type>()),
        SetFormat::fromDescription(jSetFormat.get<std::string>().c_str()),
        jWinner.get<int>());

    const auto firstFrameTimeStamp = TimeStamp::fromMillisSinceEpoch(
        time_qt_to_milli_seconds_since_epoch(jDate.get<std::string>().c_str()));

    const std::string streamDecoded = base64_decode(j["playerstates"].get<std::string>());
    Deserializer stream(streamDecoded.data(), streamDecoded.length());
    auto frameData = SmallVector<Vector<FighterState>, 2>::makeResized(metaData->fighterCount());
    for (int i = 0; i < metaData->fighterCount(); ++i)
    {
        const FramesLeft::Type stateCount = stream.readLU32();

        // zero states are invalid
        if (stateCount == 0)
            return false;

        FrameIndex::Type frameCounter = 0;
        FrameIndex::Type highestFramesLeft = 0;
        for (FrameIndex::Type f = 0; f < stateCount; ++f)
        {
            const auto framesLeft = FramesLeft::fromValue(stream.readLU32());
            const float posx = stream.readLF32();
            const float posy = stream.readLF32();
            const auto pos = Vec2::fromValues(posx, posy);
            const float damage = stream.readLF32();
            const float hitstun = stream.readLF32();
            const float shield = stream.readLF32();
            const auto status = FighterStatus::fromValue(stream.readLU16());
            const uint32_t motion_l = stream.readLU32();
            const uint8_t motion_h = stream.readU8();
            const auto motion = FighterMotion::fromParts(motion_h, motion_l);
            const auto hitStatus = FighterHitStatus::fromValue(stream.readU8());
            const auto stocks = FighterStocks::fromValue(stream.readU8());
            const auto flags = FighterFlags::fromValue(stream.readU8());

            if (framesLeft.count() == 0)
                return false;

            // Usually only unique states are saved, which means there will be
            // gaps in between frames. Duplicate the current frame as many times
            // as necessary and make sure to update the time stamp and frame
            // counters
            if (highestFramesLeft < framesLeft.count())
                highestFramesLeft = framesLeft.count();
            while (frameCounter <= highestFramesLeft - framesLeft.count())
            {
                // Version 1.3 did not timestamp each frame so we have to make a guesstimate
                // based on the timestamp of when the recording started and how many
                // frames passed since. This will not account for game pauses or
                // lag, but it should be good enough.
                const TimeStamp frameTimeStamp =  firstFrameTimeStamp +
                        DeltaTime::fromMillis(frameCounter * 1000.0 / 60.0);

                const auto framesDiff = highestFramesLeft - framesLeft.count() - frameCounter;
                const auto actualFramesLeft = FramesLeft::fromValue(framesLeft.count() + framesDiff);
                const auto actualTimeStamp = frameTimeStamp -
                        DeltaTime::fromMillis(framesDiff * 1000.0 / 60.0);
                frameData[i].emplace(actualTimeStamp, FrameIndex::fromValue(frameCounter), actualFramesLeft, pos, damage, hitstun, shield, status, motion, hitStatus, stocks, flags);
                frameCounter++;
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

    // Calculate start and end times
    const TimeStamp lastFrameTimeStamp = frameData[0].count() ?
            frameData[0].back().timeStamp() : firstFrameTimeStamp;
    metaData->setTimeStarted(firstFrameTimeStamp);
    metaData->setTimeEnded(lastFrameTimeStamp);

    // Winner sanity check
#ifndef NDEBUG
    if (frameData.count() > 0)
    {
        Frame<4> frame;
        for (const auto& state : frameData.back())
            frame.push(state);
        const int winner = findWinner(frame);
        assert(winner == static_cast<GameMetaData*>(metaData.get())->winner());
    }
#endif

    *metaDataOut = metaData;
    *mappingInfoOut = mappingInfo;
    *frameDataOut = new FrameData(std::move(frameData));

    return true;
}

// ----------------------------------------------------------------------------
static bool loadLegacy_1_4(
        json& j,
        Reference<MetaData>* metaDataOut,
        Reference<MappingInfo>* mappingInfoOut,
        Reference<FrameData>* frameDataOut)
{
    PROFILE(SessionGlobal, loadLegacy_1_4);

    json jMappingInfo = j["mappinginfo"];
    json jGameInfo = j["gameinfo"];
    json jPlayerInfo = j["playerinfo"];
    json jPlayerStates = j["playerstates"];

    json jFighterStatuses = jMappingInfo["fighterstatus"];
    json jFighterIDs = jMappingInfo["fighterid"];
    json jStageIDs = jMappingInfo["stageid"];
    json jHitStatuses = jMappingInfo["hitstatus"];

    json jFighterStatusMapping = jMappingInfo["fighterstatus"];
    json jFighterBaseStatusMapping = jFighterStatusMapping["base"];
    json jFighterSpecificStatusMapping = jFighterStatusMapping["specific"];

    Reference<MappingInfo> mappingInfo(new MappingInfo(0));  // Since we're loading it, checksum is irrelevant
    for (const auto& [key, value] : jFighterBaseStatusMapping.items())
    {
        std::size_t pos;
        const auto status = FighterStatus::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            continue;
        if (value.is_array() == false)
            continue;

        if (value.size() != 3)
            continue;
        if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
            continue;

        /*QString shortName  = arr[1].get<std::string>();
        QString customName = arr[2].get<std::string>();*/

        mappingInfo->status.addBaseName(status, value[0].get<std::string>().c_str());
    }
    for (const auto& [fighter, jsonSpecificMapping] : jFighterSpecificStatusMapping.items())
    {
        std::size_t pos;
        const auto fighterID = FighterID::fromValue(std::stoul(fighter, &pos));
        if (pos != fighter.length())
            continue;
        if (jsonSpecificMapping.is_object() == false)
            continue;

        for (const auto& [key, value] : jsonSpecificMapping.items())
        {
            const auto status = FighterStatus::fromValue(std::stoul(key, &pos));
            if (pos != key.length())
                continue;
            if (value.is_array() == false)
                continue;

            if (value.size() != 3)
                continue;
            if (value[0].is_string() == false || value[1].is_string() == false || value[2].is_string() == false)
                continue;

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
            continue;
        if (value.is_string() == false)
            continue;

        mappingInfo->fighter.add(fighterID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jStageIDs.items())
    {
        std::size_t pos;
        const auto stageID = StageID::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            continue;
        if (value.is_string() == false)
            continue;

        mappingInfo->stage.add(stageID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jHitStatuses.items())
    {
        std::size_t pos;
        const auto hitStatusID = FighterHitStatus::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            continue;
        if (value.is_string() == false)
            continue;

        mappingInfo->hitStatus.add(hitStatusID, value.get<std::string>().c_str());
    }

    SmallVector<FighterID, 2> playerFighterIDs;
    SmallVector<String, 2> playerTags;
    SmallVector<String, 2> playerNames;
    int fighterCount = 0;
    for (const auto& info : jPlayerInfo)
    {
        json jTag = info["tag"];
        json jName = info["name"];
        json jFighterID = info["fighterid"];

        playerFighterIDs.push(jFighterID.is_number_integer() ?
            FighterID::fromValue(jFighterID.get<FighterID::Type>()) : FighterID::makeInvalid());
        playerTags.emplace(jTag.is_string() ?
            jTag.get<std::string>().c_str() : (std::string("Player ") + std::to_string(fighterCount)).c_str());
        playerNames.emplace(jName.is_string() ?
            jName.get<std::string>().c_str() : (std::string("Player ") + std::to_string(fighterCount)).c_str());

        fighterCount++;
    }

    // There must be at least 2 fighters, otherwise the data is invalid
    if (playerFighterIDs.count() < 2)
        return false;
    if (playerFighterIDs.count() != playerTags.count() || playerFighterIDs.count() != playerNames.count())
        return false;

    json jStageID = jGameInfo["stageid"];
    json jTimeStampSart = jGameInfo["timestampstart"];
    json jTimeStampEnd = jGameInfo["timestampend"];
    json jSetFormat = jGameInfo["format"];
    json jGameNumber = jGameInfo["number"];
    json jSetNumber = jGameInfo["set"];
    json jWinner = jGameInfo["winner"];

    const auto timeStarted = jTimeStampSart.is_string() ?
        TimeStamp::fromMillisSinceEpoch(
            time_qt_to_milli_seconds_since_epoch(jTimeStampSart.get<std::string>().c_str())) :
        TimeStamp::makeInvalid();
    const auto timeEnded = jTimeStampEnd.is_string() ?
        TimeStamp::fromMillisSinceEpoch(
            time_qt_to_milli_seconds_since_epoch(jTimeStampEnd.get<std::string>().c_str())) :
        TimeStamp::makeInvalid();
    const auto stageID = jStageID.is_number_integer() ?
        StageID::fromValue(jStageID.get<StageID::Type>()) : StageID::makeInvalid();
    const auto gameNumber = jGameNumber.is_number_integer() ?
        GameNumber::fromValue(jGameNumber.get<GameNumber::Type>()) : GameNumber::fromValue(1);
    const auto setNumber = jSetNumber.is_number_integer() ?
        SetNumber::fromValue(jSetNumber.get<SetNumber::Type>()) : SetNumber::fromValue(1);
    const auto format = jSetFormat.is_string() ?
        SetFormat::fromDescription(jSetFormat.get<std::string>().c_str()) : SetFormat::fromType(SetFormat::FRIENDLIES);
    int winner = jWinner.is_number_unsigned() ?
        jWinner.get<int>() : -1;
    if (winner > fighterCount)
        winner = -1;

    Reference<MetaData> metaData = MetaData::newSavedGameSession(
        timeStarted, timeEnded, stageID, std::move(playerFighterIDs),
            std::move(playerTags), std::move(playerNames), gameNumber,
            setNumber, format, winner);

    const std::string streamDecoded = jPlayerStates.is_string() ?
        base64_decode(jPlayerStates.get<std::string>()) : "";
    Deserializer stream(streamDecoded.data(), streamDecoded.length());
    auto uniqueFrameData = SmallVector<Vector<FighterState>, 2>::makeResized(fighterCount);
    for (int i = 0; i < fighterCount; ++i)
    {
        const FrameIndex::Type stateCount = stream.readLU32();

        // zero states are invalid
        if (stateCount == 0)
            return false;

        for (FrameIndex::Type f = 0; f < stateCount; ++f)
        {
            const auto frameTimeStamp = TimeStamp::fromMillisSinceEpoch(stream.readLU64());
            const auto framesLeft = FramesLeft::fromValue(stream.readLU32());
            const float posx = stream.readLF32();
            const float posy = stream.readLF32();
            const auto pos = Vec2::fromValues(posx, posy);
            const float damage = stream.readLF32();
            const float hitstun = stream.readLF32();
            const float shield = stream.readLF32();
            const auto status = FighterStatus::fromValue(stream.readLU16());
            const uint32_t motion_l = stream.readLU32();
            const uint8_t motion_h = stream.readU8();
            const auto motion = FighterMotion::fromParts(motion_h, motion_l);
            const auto hitStatus = FighterHitStatus::fromValue(stream.readU8());
            const auto stocks = FighterStocks::fromValue(stream.readU8());
            const auto flags = FighterFlags::fromValue(stream.readU8());

            uniqueFrameData[i].emplace(frameTimeStamp, FrameIndex::fromValue(0),  // We update the frame number later
                framesLeft, pos, damage, hitstun, shield, status, motion, hitStatus, stocks, flags);
        }
    }

    // In this version of the file, one of the players has an extra frame in
    // the beginning. We want the frame numbers to line up properly so determine
    // which player this is and remove that frame
    auto checkFirstFramesLeftMatches = [](const SmallVector<Vector<FighterState>, 2>& frameData) -> bool {
        const auto framesLeft = frameData[0].front().framesLeft();
        for (int i = 1; i != frameData.count(); ++i)
            if (frameData[i].front().framesLeft() != framesLeft)
                return false;
        return true;
    };
    while (checkFirstFramesLeftMatches(uniqueFrameData) == false)
    {
        auto it = std::max_element(uniqueFrameData.begin(), uniqueFrameData.end(),
            [](const Vector<FighterState>& a, const Vector<FighterState>& b) {
                return a.front().framesLeft().count() < b.front().framesLeft().count();
        });
        it->erase(0);
    }
    // From the code above, all players should have the same frames left value now
    const auto highestFramesLeft = uniqueFrameData[0].front().framesLeft();

    // Usually only unique states are saved, which means there will be
    // gaps in between frames. Go through all states and duplicate any frames
    // to fill in these gaps, while making sure to update the frame number,
    // frames left and timestamp correctly
    auto frameData = SmallVector<Vector<FighterState>, 2>::makeResized(fighterCount);
    for (int fighter = 0; fighter != fighterCount; ++fighter)
    {
        const auto& uniqueStates = uniqueFrameData[fighter];
        auto& states = frameData[fighter];
        states.reserve(uniqueFrameData[fighter].count());
        states.push(uniqueStates[0]);
        FramesLeft::Type framesLeftCounter = highestFramesLeft.count();
        FrameIndex::Type frameCounter = 0;
        for (int i = 1; i < uniqueStates.count(); ++i)
        {
            const auto& uniqueState = uniqueStates[i];

            while (framesLeftCounter > uniqueState.framesLeft().count())
            {
                framesLeftCounter--; frameCounter++;
                const auto diff = framesLeftCounter - uniqueState.framesLeft().count();
                const auto newTimeStamp = uniqueState.timeStamp() - DeltaTime::fromMillis(diff * 1000.0 / 60.0);
                states.push(uniqueState.withNewFrameCounters(
                        newTimeStamp, FrameIndex::fromValue(frameCounter), FramesLeft::fromValue(framesLeftCounter)));
            }
        }
    }

    // Ensure that every fighter has the same number of frames
    const auto highestFrameIdx = std::max_element(frameData.begin(), frameData.end(),
        [](const Vector<FighterState>& a, const Vector<FighterState>& b) {
            return a.back().frameIndex().index() < b.back().frameIndex().index();
    })->back().frameIndex();
    for (auto& states : frameData)
    {
        const auto finalState = states.back();
        const auto finalFrameIdx = states.back().frameIndex().index();
        while (states.back().frameIndex().index() < highestFrameIdx.index())
        {
            const auto framesDiff = states.back().frameIndex().index() - finalFrameIdx + 1;
            const TimeStamp actualTimeStamp = finalState.timeStamp() + DeltaTime::fromMillis(framesDiff * 1000.0 / 60.0);
            const auto actualFrameIdx = FrameIndex::fromValue(finalFrameIdx + framesDiff);
            const auto actualFramesLeft = FramesLeft::fromValue(finalState.framesLeft().count() - framesDiff);
            states.emplace(finalState.withNewFrameCounters(actualTimeStamp, actualFrameIdx, actualFramesLeft));
        }
    }

    *metaDataOut = metaData;
    *mappingInfoOut = mappingInfo;
    *frameDataOut = new FrameData(std::move(frameData));

    return true;
}

// ----------------------------------------------------------------------------
Session::Session(MappedFile* file, MappingInfo* mappingInfo, MetaData* metaData, FrameData* frameData)
    : file_(file)
    , mappingInfo_(mappingInfo)
    , metaData_(metaData)
    , frameData_(frameData)
{
    if (frameData_.notNull())
        frameData_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
Session::~Session()
{
    if (frameData_.notNull())
        frameData_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
Session* Session::newModernSavedSession(MappedFile* file)
{
    PROFILE(Session, newModernSavedSession);

    return new Session(
        file,
        nullptr,
        nullptr,
        nullptr);
}

// ----------------------------------------------------------------------------
Session* Session::newLegacySavedSession(MappingInfo* mappingInfo, MetaData* metaData, FrameData* frameData)
{
    PROFILE(Session, newLegacySavedSession);

    return new Session(
        nullptr,
        mappingInfo,
        metaData,
        frameData);
}

// ----------------------------------------------------------------------------
Session* Session::newActiveSession(MappingInfo* globalMappingInfo, MetaData* metaData)
{
    PROFILE(Session, newActiveSession);

    return new Session(
        nullptr,
        globalMappingInfo,
        metaData,
        new FrameData(metaData->fighterCount()));
}

// ----------------------------------------------------------------------------
Session* Session::load(const char* fileName, uint8_t loadFlags)
{
    PROFILE(Session, load);

    // Assume we're dealing with a modern format first
    Reference<MappedFile> file(new MappedFile);
    if (file->open(fileName) == false)
        return nullptr;

    Deserializer deserializer(file->address(), file->size());

    // If this is a modern file, it will start with "RFR1"
    const void* magic = deserializer.readFromPtr(4);
    if (memcmp(magic, "RFR1", 4) == 0)
    {
        Reference<Session> session = newModernSavedSession(file);

        // Read content table
        uint8_t numEntries = deserializer.readU8();
        for (int i = 0; i != numEntries; ++i)
        {
            Session::ContentTableEntry& entry = session->contentTable_.emplace();
            memcpy(entry.type, deserializer.readFromPtr(4), 4);
            entry.offset = deserializer.readLU32();
            entry.size = deserializer.readLU32();
        }

        // Load sections immediately if necessary
#define X(name, value)                                 \
        if (loadFlags & Flags::name)                   \
            if (session->tryGet##name() == nullptr)    \
                return nullptr;
        RFCOMMON_SESSION_SECTIONS_LIST
#undef X

        return session.detach();
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

    Reference<MetaData> metaData;
    Reference<MappingInfo> mappingInfo;
    Reference<FrameData> frameData;
    std::string version = j["version"];
    if (version == "1.4")
    {
        if (loadLegacy_1_4(j, &metaData, &mappingInfo, &frameData))
            return new Session(nullptr, mappingInfo, metaData, frameData);
    }
    else if (version == "1.3")
    {
        if (loadLegacy_1_3(j, &metaData, &mappingInfo, &frameData))
            return newLegacySavedSession(mappingInfo, metaData, frameData);
    }

    return nullptr;
}

// ----------------------------------------------------------------------------
Session::ContentTableEntry::ContentTableEntry()
{}

// ----------------------------------------------------------------------------
Session::ContentTableEntry::ContentTableEntry(const char* typeStr)
    : offset(0)
    , size(0)
{
    assert(strlen(typeStr) == 4);
    memcpy(type, typeStr, 4);
}

// ----------------------------------------------------------------------------
bool Session::save(const char* fileName, uint8_t saveFlags)
{
    PROFILE(Session, save);

    Log* log = Log::root();
    log->beginDropdown("Saving session %s", fileName);

    // This could either be a session that was recorded live, or a session
    // loaded from disk. If loaded from disk, then we have to make sure to
    // load all of the sections that can be edited and weren't lazy loaded
    // yet, otherwise we'll skip saving that data into the new file and lose
    // data.
    //
    // The only sections currently editable are meta data and video meta. the
    // other sections can simply be copied over to the new file byte-by-byte
    // without having to go through the deserialize/serialize process.
    SmallVector<ContentTableEntry, 5> table;
    if (file_.notNull())
    {
        // Load necessary sections (meta data and video meta)
        for (const auto& entry : contentTable_)
        {
#define LOAD_SECTION(name)                                    \
            if (memcmp(entry.type, blobType##name, 4) == 0)   \
            {                                                 \
                if (!(saveFlags & Flags::name))               \
                    continue;                                 \
                if (tryGet##name() == nullptr)                \
                {                                             \
                    log->error("Content table contains " #name ", but unable to load the data. Can't create new replay file!"); \
                    return false;                             \
                }                                             \
            }
            LOAD_SECTION(MetaData)
            LOAD_SECTION(VideoMeta)
#undef LOAD_SECTION
        }

        // Copy existing content table
        table = contentTable_;
    }
    else  // This was a live session with no data to lazy load
    {
        // Any property of Session is allowed to be null, so we first calculate
        // how many entries we need. We don't know the offsets or sizes yet.
        if (mappingInfo_.notNull() && (saveFlags & Flags::MappingInfo))
            table.emplace(blobTypeMappingInfo);
        if (metaData_.notNull() && (saveFlags & Flags::MetaData))
            table.emplace(blobTypeMetaData);
        if (frameData_.notNull() && (saveFlags & Flags::FrameData))
            table.emplace(blobTypeFrameData);
        if (videoMeta_.notNull() && (saveFlags & Flags::VideoMeta))
            table.emplace(blobTypeVideoMeta);
        if (videoEmbed_.notNull() && (saveFlags & Flags::VideoEmbed))
            table.emplace(blobTypeVideoEmbed);
    }

    // Calculate header size and data offset
    const int headerSize = 5 + 12 * table.count();
    uint8_t numEntries = static_cast<uint8_t>(table.count());
    int offset = headerSize;

    log->info("Content Table has %d entries. Header size %d", numEntries, headerSize);

    FILE* file;
    file = fopen(fileName, "wb");
    if (file == nullptr)
    {
        log->error("Failed to open file %s: %s", fileName, strerror(errno));
        goto fopen_fail;
    }

    // We skip writing the header until after data is written, because
    // there is not enough information yet
    if (fseek(file, headerSize, SEEK_SET) != 0)
    {
        log->error("Failed to seek to first offset 0x%x", headerSize);
        goto write_fail;
    }

    // Write content and update offsets/sizes in content table
    for (auto& entry : table)
    {
#define COPY_EXISTING_BLOB() do {                                     \
            /* Locate entry in current content table */               \
            for (const auto& existingEntry : contentTable_)           \
                if (memcmp(existingEntry.type, entry.type, 4) == 0)   \
                {                                                     \
                    entry.offset = offset;                            \
                    entry.size = fwrite(file_->addressAtOffset(existingEntry.offset), 1, existingEntry.size, file); \
                    if (entry.size != existingEntry.size)             \
                    {                                                 \
                        log->error("Failed to write 0x%x bytes to file. Only wrote 0x%x bytes.", existingEntry.size, entry.size); \
                        goto write_fail;                              \
                    }                                                 \
                }                                                     \
            } while(0)

        if (memcmp(entry.type, blobTypeMappingInfo, 4) == 0)
        {
            if (file_.notNull())  // Can write blob directly without deserializing/serializing it
                COPY_EXISTING_BLOB();
            else
            {
                entry.offset = offset;
                entry.size = metaData_ && frameData_ ?
                    mappingInfo_->saveNecessary(file, metaData_, frameData_) :
                    mappingInfo_->save(file);
                if (entry.size == 0)
                    goto write_fail;
            }
            offset += entry.size;
            log->info("Saved mapping info at offset 0x%x, size 0x%x", entry.offset, entry.size);
        }
        else if (memcmp(entry.type, blobTypeMetaData, 4) == 0)
        {
            entry.offset = offset;
            entry.size = metaData_->save(file);
            if (entry.size == 0)
                goto write_fail;
            offset += entry.size;
            log->info("Saved meta data at offset 0x%x, size 0x%x", entry.offset, entry.size);
        }
        else if (memcmp(entry.type, blobTypeFrameData, 4) == 0)
        {
            if (file_.notNull())  // Can write blob directly without deserializing/serializing it
                COPY_EXISTING_BLOB();
            else
            {
                entry.offset = offset;
                entry.size = frameData_->save(file);
                if (entry.size == 0)
                    goto write_fail;
            }
            offset += entry.size;
            log->info("Saved frame data at offset 0x%x, size 0x%x", entry.offset, entry.size);
        }
        else if (memcmp(entry.type, blobTypeVideoMeta, 4) == 0)
        {
            entry.offset = offset;
            entry.size = videoMeta_->save(file);
            if (entry.size == 0)
                goto write_fail;
            offset += entry.size;
            log->info("Saved video meta at offset 0x%x, size 0x%x", entry.offset, entry.size);
        }
        else if (memcmp(entry.type, blobTypeVideoEmbed, 4) == 0)
        {
            if (file_.notNull())  // Can write blob directly without deserializing/serializing it
                COPY_EXISTING_BLOB();
            else
            {
                entry.offset = offset;
                entry.size = fwrite(videoEmbed_->address(), 1, videoEmbed_->size(), file);
                if (entry.size == 0)
                    goto write_fail;
            }
            offset += entry.size;
            log->info("Saved video embed at offset 0x%x, size 0x%x", entry.offset, entry.size);
        }
        else
        {
            log->warning("Unknown entry type in content table: %c%c%c%c. Ignoring.", entry.type[0], entry.type[1], entry.type[2], entry.type[3]);
        }
#undef COPY_EXISTING_BLOB
    }

    // Rewind and write header
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        log->error("Failed to seek to beginning");
        goto write_fail;
    }

    // Write magic
    if (fwrite(magic, 1, 4, file) != 4)
    {
        log->error("Failed to write magic");
        goto write_fail;
    }

    // Write contents table
    if (fwrite(&numEntries, 1, 1, file) != 1)
    {
        log->error("Failed to write content table size");
        goto write_fail;
    }
    for (const auto& entry : table)
    {
        uint32_t offsetLE = toLittleEndian32(entry.offset);
        uint32_t sizeLE = toLittleEndian32(entry.size);
        if (fwrite(entry.type, 1, 4, file) != 4) { log->error("Failed to write content table entry"); goto write_fail; }
        if (fwrite(&offsetLE, 1, 4, file) != 4) { log->error("Failed to write content table entry"); goto write_fail; }
        if (fwrite(&sizeLE, 1, 4, file) != 4) { log->error("Failed to write content table entry"); goto write_fail; }
    }

    if (fclose(file) != 0)
    {
        log->error("Failed to close file");
        goto write_fail;
    }

    log->endDropdown();
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
        log->endDropdown();
    return false;
}

// ----------------------------------------------------------------------------
bool Session::existsInContentTable(Flags::Flag flag) const
{
    PROFILE(Session, existsInContentTable);

    const char* blobType = [=]() -> const char* {
        switch(flag) {
#define X(name, value) case Flags::name : return blobType##name;
            RFCOMMON_SESSION_SECTIONS_LIST
#undef X
            default: return "    ";
        }
    }();

    for (const auto& entry : contentTable_)
        if (memcmp(entry.type, blobType, 4) == 0)
            return true;

    return false;
}

// ----------------------------------------------------------------------------
MappingInfo* Session::tryGetMappingInfo()
{
    PROFILE(Session, tryGetMappingInfo);

    if (mappingInfo_.isNull())
    {
        assert(file_.notNull());
        for (const auto& entry : contentTable_)
            if (memcmp(entry.type, blobTypeMappingInfo, 4) == 0)
            {
                mappingInfo_ = MappingInfo::load(file_->addressAtOffset(entry.offset), entry.size);
                break;
            }
    }

    return mappingInfo_;
}

// ----------------------------------------------------------------------------
MetaData* Session::tryGetMetaData()
{
    PROFILE(Session, tryGetMetaData);

    if (metaData_.isNull())
    {
        assert(file_.notNull());
        for (const auto& entry : contentTable_)
            if (memcmp(entry.type, blobTypeMetaData, 4) == 0)
            {
                metaData_ = MetaData::load(file_->addressAtOffset(entry.offset), entry.size);
                break;
            }

        // Sanity check
#ifndef NDEBUG
        if (metaData_.notNull() && frameData_.notNull())
            assert(metaData_->fighterCount() == frameData_->fighterCount());
#endif
    }

    return metaData_;
}

// ----------------------------------------------------------------------------
FrameData* Session::tryGetFrameData()
{
    PROFILE(Session, tryGetFrameData);

    if (frameData_.isNull())
    {
        assert(file_.notNull());
        for (const auto& entry : contentTable_)
            if (memcmp(entry.type, blobTypeFrameData, 4) == 0)
            {
                frameData_ = FrameData::load(file_->addressAtOffset(entry.offset), entry.size);
                break;
            }

        // Sanity check
#ifndef NDEBUG
        if (metaData_.notNull() && frameData_.notNull())
            assert(metaData_->fighterCount() == frameData_->fighterCount());
#endif
    }

    return frameData_;
}

// ----------------------------------------------------------------------------
VideoMeta* Session::tryGetVideoMeta()
{
    PROFILE(Session, tryGetVideoMeta);

    return nullptr;
}

// ----------------------------------------------------------------------------
VideoEmbed* Session::tryGetVideoEmbed()
{
    PROFILE(Session, tryGetVideoEmbed);

    return nullptr;
}

// ----------------------------------------------------------------------------
void Session::onFrameDataNewUniqueFrame(int frameIdx, const Frame<4>& frame)
{
    PROFILE(Session, onFrameDataNewUniqueFrame);

    (void)frameIdx;

    // Winner might have changed
    if (metaData_ && metaData_->type() == MetaData::GAME)
        static_cast<GameMetaData*>(metaData_.get())->setWinner(findWinner(frame));
}

// ----------------------------------------------------------------------------
void Session::onFrameDataNewFrame(int frameIdx, const Frame<4>& frame)
{
    PROFILE(Session, onFrameDataNewFrame);

    (void)frameIdx;

    // Update ended timestamp
    auto stamp = frame.back().timeStamp();
    metaData_->setTimeEnded(stamp);
}

// ----------------------------------------------------------------------------
static int findWinner(const Frame<4>& frame)
{
    PROFILE(SessionGlobal, findWinner);

    // The winner is the player with most stocks and least damage
    int winneridx = 0;
    for (int i = 1; i != frame.count(); ++i)
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
