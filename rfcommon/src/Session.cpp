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

using nlohmann::json;

namespace rfcommon {

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

    MemoryBuffer frameData(5 + frameSize * frameCount());
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

    MemoryBuffer compressedFrameData(compressBound(frameData.bytesWritten()) + 6);
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
    const char blobTypeJSON[] = {'J', 'S', 'O', 'N'};
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

    if (fwrite(&blobTypeJSON, 1, 4, file) != 4)
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
