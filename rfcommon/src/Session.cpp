#include "rfcommon/Session.hpp"
#include "rfcommon/GameSession.hpp"
#include "rfcommon/TrainingSession.hpp"
#include "rfcommon/SessionListener.hpp"
#include "rfcommon/PlayerState.hpp"
#include "rfcommon/StreamBuffer.hpp"
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
    , frames_(fighterIDs.count())
{
    assert(fighterIDs_.count() == tags_.count());
    assert(fighterIDs_.count() == frames_.count());
}

// ----------------------------------------------------------------------------
Session::~Session()
{
}

// ----------------------------------------------------------------------------
bool Session::save(const String& fileName)
{
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
    for (const auto& player : frames_)
        for (const auto& state : player)
        {
            usedStatuses.insert(state.status());
            usedHitStatuses.insert(state.hitStatus());
        }
    std::unordered_set<FighterID, FighterIDHasherStd> usedFighterIDs;
    for (const auto& fighterID : fighterIDs_)
        usedFighterIDs.insert(fighterID);

    json gameInfo = {
        {"stageid", stageID_.value()},
        {"timestampstart", timeStampStartedMs().value()},
        {"timestampend", timeStampEndedMs().value()}
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

    int stateBufferSize = 0;
    for (const auto& states : frames_)
    {
        stateBufferSize += 4; // state count
        for (const auto& state : states)
            stateBufferSize +=
                    sizeof(state.timeStampMs()) +
                    sizeof(state.frame()) +
                    sizeof(state.posx()) +
                    sizeof(state.posy()) +
                    sizeof(state.damage()) +
                    sizeof(state.hitstun()) +
                    sizeof(state.shield()) +
                    sizeof(state.status()) +
                    5 + // motion is a hash40 (40 bits)
                    sizeof(state.hitStatus()) +
                    sizeof(state.stocks()) +
                    1;  // flags
    }

    StreamBuffer stream(stateBufferSize);
    for (const auto& states : frames_)
    {
        stream.writeLU32(states.count());
        for (const auto& state : states)
        {
            stream
                .writeLU64(state.timeStampMs().value())
                .writeLU32(state.frame().value())
                .writeLF32(state.posx())
                .writeLF32(state.posy())
                .writeLF32(state.damage())
                .writeLF32(state.hitstun())
                .writeLF32(state.shield())
                .writeLU16(state.status().value())
                .writeLU32(state.motion().lower())
                .writeU8(state.motion().upper())
                .writeU8(state.hitStatus().value())
                .writeU8(state.stocks().value())
                .writeU8(state.flags().value());
        }
    }

    json j = {
        {"version", "1.4"},
        {"mappinginfo", mappingInfo},
        {"gameinfo", gameInfo},
        {"videoinfo", videoInfo},
        {"playerinfo", playerInfo},
        {"playerstates", base64_encode(static_cast<unsigned char*>(stream.get()), stream.size(), false)}
    };

    std::string s = j.dump();
    gzFile f = gzopen(fileName.cStr(), "wb");
    if (f == nullptr)
        goto gz_open_fail;
    if (gzsetparams(f, 9, Z_DEFAULT_STRATEGY) != Z_OK)
        goto gz_fail;
    if (gzwrite(f, s.data(), static_cast<unsigned int>(s.length())) == 0)
        goto gz_fail;
    gzclose(f);

    return true;

    gz_fail      : gzclose(f);
    gz_open_fail :;
        /*
        if (QMessageBox::warning(nullptr, "Failed to save recording", QString("Failed to open file for writing: ") + f.fileName() + "\n\nWould you like to save the file manually?", QMessageBox::Save | QMessageBox::Discard) == QMessageBox::Save)
        {
            QFileDialog::getSaveFileName(nullptr, "Save Recording", f.fileName());
        }*/
    return false;
}

// ----------------------------------------------------------------------------
TimeStampMS Session::timeStampStartedMs() const
{
    return frames_[0][0].timeStampMs();
}

// ----------------------------------------------------------------------------
TimeStampMS Session::timeStampEndedMs() const
{
    return frames_.back().back().timeStampMs();
}

// ----------------------------------------------------------------------------
int Session::findWinner() const
{
    assert(frameCount() > 0);

    // The winner is the player with most stocks and least damage
    int winneridx = 0;
    for (int i = 0; i != fighterCount(); ++i)
    {
        const auto& current = frames_[i].back();
        const auto& winner = frames_[winneridx].back();

        if (current.stocks() > winner.stocks())
            winneridx = i;
        else if (current.stocks() == winner.stocks())
            if (current.damage() < winner.damage())
                winneridx = i;
    }

    return winneridx;
}

}
