#include "uh/Recording.hpp"
#include "uh/PlayerState.hpp"
#include "uh/RecordingListener.hpp"
#include "uh/time.h"
#include "uh/StreamBuffer.hpp"
#include "nlohmann/json.hpp"
#include "cpp-base64/base64.h"
#include "zlib.h"
#include <cassert>
#include <unordered_set>

namespace uh {

using nlohmann::json;

// ----------------------------------------------------------------------------
Recording::Recording(MappingInfo&& mapping,
                     std::vector<FighterID>&& playerFighterIDs,
                     std::vector<std::string>&& playerTags,
                     StageID stageID)
    : mappingInfo_(std::move(mapping))
    , playerTags_(playerTags)
    , playerNames_(playerTags)
    , playerFighterIDs_(std::move(playerFighterIDs))
    , playerStates_(playerTags.size())
    , format_(SetFormat::FRIENDLIES)
    , stageID_(stageID)
{
    assert(playerTags_.size() == playerNames_.size());
    assert(playerTags_.size() == playerFighterIDs_.size());
    assert(playerStates_.size() == playerFighterIDs_.size());
}

// ----------------------------------------------------------------------------
bool Recording::saveAs(const std::string& fileName)
{
    // Create sets of the IDs that were used in game so we don't end up saving
    // every ID
    std::unordered_set<uint16_t> usedStatuses;
    std::unordered_set<uint8_t> usedHitStatuses;
    for (const auto& player : playerStates_)
        for (const auto& state : player)
        {
            usedStatuses.insert(state.status());
            usedHitStatuses.insert(state.hitStatus());
        }
    std::unordered_set<uint8_t> usedFighterIDs;
    for (const auto& fighterID : playerFighterIDs_)
        usedFighterIDs.insert(fighterID);

    json gameInfo = {
        {"stageid", stageID_},
        {"timestampstart", timeStampStartedMs()},
        {"timestampend", timeStampEndedMs()},
        {"format", format_.description()},
        {"number", gameNumber_},
        {"set", setNumber_},
        {"winner", winner_}
    };

    json videoInfo = {
        {"filename", ""},
        {"filepath", ""},
        {"offsetms", ""}
    };

    json fighterBaseStatusMapping;
    const auto& baseEnumNames = mappingInfo_.fighterStatus.baseEnumNames();
    for (auto it = baseEnumNames.begin(); it != baseEnumNames.end(); ++it)
    {
        // Skip saving enums that aren't actually used in the set of player states
        if (usedStatuses.find(it->first) == usedStatuses.end())
            continue;

        /*const QString* shortName = mappingInfo_.fighterStatus.mapToShortName(it.key());
        const QString* customName = mappingInfo_.fighterStatus.mapToCustom(it.key());*/

        fighterBaseStatusMapping[std::to_string(it->first)] = {it->second, "", ""};
    }

    json fighterSpecificStatusMapping;
    const auto& specificEnumNames = mappingInfo_.fighterStatus.fighterSpecificEnumNames();
    for (auto fighter = specificEnumNames.begin(); fighter != specificEnumNames.end(); ++fighter)
    {
        // Skip saving enums for fighters that aren't being used
        if (usedFighterIDs.find(fighter->first) == usedFighterIDs.end())
            continue;

        json specificMapping = json::object();
        for (auto it = fighter->second.begin(); it != fighter->second.end(); ++it)
        {
            // Skip saving enums that aren't actually used in the set of player states
            if (usedStatuses.find(it->first) == usedStatuses.end())
                continue;

            /*const QString* shortName = mappingInfo_.fighterStatus.mapToShortName(it.key());
            const QString* customName = mappingInfo_.fighterStatus.mapToCustom(it.key());*/

            specificMapping[std::to_string(it->first)] = {it->second, "", ""};
        }

        if (specificMapping.size() > 0)
            fighterSpecificStatusMapping[std::to_string(fighter->first)] = specificMapping;
    }

    json fighterStatusMapping = {
        {"base", fighterBaseStatusMapping},
        {"specific", fighterSpecificStatusMapping}
    };

    json fighterIDMapping;
    const auto& fighterIDMap = mappingInfo_.fighterID.get();
    for (auto it = fighterIDMap.begin(); it != fighterIDMap.end(); ++it)
        if (usedFighterIDs.find(it->first) != usedFighterIDs.end())
            fighterIDMapping[std::to_string(it->first)] = it->second;

    json stageIDMapping;
    const auto& stageIDMap = mappingInfo_.stageID.get();
    for (auto it = stageIDMap.begin(); it != stageIDMap.end(); ++it)
        if (it->first == stageID_)  // Only care about saving the stage that was played on
            stageIDMapping[std::to_string(it->first)] = it->second;

    json hitStatusMapping;
    const auto& hitStatusMap = mappingInfo_.hitStatus.get();
    for (auto it = hitStatusMap.begin(); it != hitStatusMap.end(); ++it)
        if (usedHitStatuses.find(it->first) != usedHitStatuses.end())
            hitStatusMapping[std::to_string(it->first)] = it->second;

    json mappingInfo = {
        {"fighterstatus", fighterStatusMapping},
        {"fighterid", fighterIDMapping},
        {"stageid", stageIDMapping},
        {"hitstatus", hitStatusMapping}
    };

    json playerInfo = json::array();
    for (int i = 0; i < playerCount(); ++i)
    {
        playerInfo += {
            {"tag", playerTags_[i]},
            {"name", playerNames_[i]},
            {"fighterid", playerFighterIDs_[i]}
        };
    }

    int stateBufferSize = 0;
    for (const auto& states : playerStates_)
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
    for (const auto& states : playerStates_)
    {
        stream.writeLU32(states.size());
        for (const auto& state : states)
        {
            uint8_t flags = (state.attackConnected() << 0)
                          | (state.facingDirection() << 1);

            stream
                .writeLU64(state.timeStampMs())
                .writeLU32(state.frame())
                .writeLF32(state.posx())
                .writeLF32(state.posy())
                .writeLF32(state.damage())
                .writeLF32(state.hitstun())
                .writeLF32(state.shield())
                .writeLU16(state.status())
                .writeLU32(static_cast<uint32_t>(state.motion() & 0xFFFFFFFF))
                .writeU8(static_cast<uint8_t>((state.motion() >> 32) & 0xFF))
                .writeU8(state.hitStatus())
                .writeU8(state.stocks())
                .writeU8(flags);
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
    gzFile f = gzopen(fileName.c_str(), "wb");
    if (f == nullptr)
        goto gz_open_fail;
    if (gzsetparams(f, 9, Z_DEFAULT_STRATEGY) != Z_OK)
        goto gz_fail;
    if (gzwrite(f, s.data(), s.length()) == 0)
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
uint64_t Recording::timeStampStartedMs() const
{
    return playerStates_[0][0].timeStampMs();
}

// ----------------------------------------------------------------------------
uint64_t Recording::timeStampEndedMs() const
{
    return playerStates_.back().back().timeStampMs();
}

// ----------------------------------------------------------------------------
uint64_t Recording::gameLengthMs() const
{
    return timeStampEndedMs() - timeStampStartedMs();
}

// ----------------------------------------------------------------------------
int Recording::findWinner() const
{
    // The winner is the player with most stocks and least damage
    int winneridx = 0;
    for (int i = 0; i != playerCount(); ++i)
    {
        if (playerStateCount(i) == 0 || playerStateCount(winneridx) == 0)
            continue;

        const auto& current = playerState(i, playerStateCount(i) - 1);
        const auto& winner = playerState(winneridx, playerStateCount(winneridx) - 1);

        if (current.stocks() > winner.stocks())
            winneridx = i;
        else if (current.stocks() == winner.stocks())
            if (current.damage() < winner.damage())
                winneridx = i;
    }

    return winneridx;
}

}
