#include "rfcommon/PlayerState.hpp"
#include "rfcommon/RunningGameSession.hpp"
#include "rfcommon/SessionListener.hpp"
#include "rfcommon/StreamBuffer.hpp"
#include "rfcommon/time.h"
#include "nlohmann/json.hpp"
#include "cpp-base64/base64.h"
#include "zlib.h"
#include <cassert>
#include <unordered_set>

using nlohmann::json;

namespace rfcommon {

// ----------------------------------------------------------------------------
RunningGameSession::RunningGameSession(
        MappingInfo&& mapping,
        uint16_t stageID,
        SmallVector<FighterID, 8>&& playerFighterIDs,
        SmallVector<SmallString<15>, 8>&& playerTags,
        SmallVector<SmallString<15>, 8>&& playerNames)
    : Session(std::move(mapping), stageID, std::move(playerFighterIDs), std::move(playerTags))
    , RunningSession()
    , GameSession(std::move(playerNames))
    , timeStampStarted_(time_milli_seconds_since_epoch())
{
}

// ----------------------------------------------------------------------------
bool RunningGameSession::save(const String& fileName)
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
        {"timestampend", playerStates_[0][playerStates_[0].count() - 1].timeStampMs()},
        {"format", format_.description().cStr()},
        {"number", gameNumber_},
        {"set", setNumber_},
        {"winner", currentWinner_}
    };

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

        fighterBaseStatusMapping[std::to_string(it->key())] = {it->value().cStr(), "", ""};
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

            specificMapping[std::to_string(it->key())] = {it->value().cStr(), "", ""};
        }

        if (specificMapping.size() > 0)
            fighterSpecificStatusMapping[std::to_string(fighter->key())] = specificMapping;
    }

    json fighterStatusMapping = {
        {"base", fighterBaseStatusMapping},
        {"specific", fighterSpecificStatusMapping}
    };

    json fighterIDMapping;
    const auto& fighterIDMap = mappingInfo_.fighterID.get();
    for (const auto& it : fighterIDMap)
        if (usedFighterIDs.find(it->key()) != usedFighterIDs.end())
            fighterIDMapping[std::to_string(it->key())] = it->value().cStr();

    json stageIDMapping;
    const auto& stageIDMap = mappingInfo_.stageID.get();
    for (const auto& it : stageIDMap)
        if (it->key() == stageID_)  // Only care about saving the stage that was played on
            stageIDMapping[std::to_string(it->key())] = it->value().cStr();

    json hitStatusMapping;
    const auto& hitStatusMap = mappingInfo_.hitStatus.get();
    for (const auto& it : hitStatusMap)
        if (usedHitStatuses.find(it.key()) != usedHitStatuses.end())
            hitStatusMapping[std::to_string(it.key())] = it.value().cStr();

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
            {"tag", playerTags_[i].cStr()},
            {"name", playerNames_[i].cStr()},
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
        stream.writeLU32(states.count());
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
void RunningGameSession::setPlayerName(int index, const SmallString<15>& name)
{
    assert(name.length() > 0);
    playerNames_[index] = name;
    dispatcher.dispatch(&SessionListener::onRunningGameSessionPlayerNameChanged, index, name);
}

// ----------------------------------------------------------------------------
void RunningGameSession::setGameNumber(GameNumber number)
{
    gameNumber_ = number;
    dispatcher.dispatch(&SessionListener::onRunningGameSessionGameNumberChanged, number);
}

// ----------------------------------------------------------------------------
void RunningGameSession::setSetNumber(SetNumber number)
{
    setNumber_ = number;
    dispatcher.dispatch(&SessionListener::onRunningGameSessionSetNumberChanged, number);
}

// ----------------------------------------------------------------------------
void RunningGameSession::setFormat(const SetFormat& format)
{
    format_ = format;
    dispatcher.dispatch(&SessionListener::onRunningGameSessionFormatChanged, format);
}

// ----------------------------------------------------------------------------
void RunningGameSession::addPlayerState(int playerIdx, PlayerState&& state)
{
    // If this is the first state we receive just store it. Need at least 1
    // past state to determine if the game started yet
    if (playerStates_[playerIdx].count() == 0)
    {
        playerStates_[playerIdx].push(std::move(state));
        dispatcher.dispatch(&SessionListener::onRunningSessionNewUniquePlayerState, playerIdx, playerStates_[playerIdx][0]);
        dispatcher.dispatch(&SessionListener::onRunningSessionNewPlayerState, playerIdx, playerStates_[playerIdx][0]);
        return;
    }

    // Only add a new state if the previous one was different
    if (playerStates_[playerIdx].back() != state)
    {
        playerStates_[playerIdx].push(state);
        dispatcher.dispatch(&SessionListener::onRunningSessionNewUniquePlayerState, playerIdx, state);

        // Winner might have changed
        int winner = findWinner();
        if (currentWinner_ != winner)
        {
            currentWinner_ = winner;
            dispatcher.dispatch(&SessionListener::onRunningGameSessionWinnerChanged, currentWinner_);
        }
    }

    // The UI still cares about every frame
    dispatcher.dispatch(&SessionListener::onRunningSessionNewPlayerState, playerIdx, state);
}

}
