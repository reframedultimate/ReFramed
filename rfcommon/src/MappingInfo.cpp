#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/FighterState.hpp"
#include "rfcommon/SessionMetaData.hpp"
#include "nlohmann/json.hpp"
#include <memory>
#include <unordered_set>
#include <vector>

namespace rfcommon {

using nlohmann::json;

static MappingInfo* load_1_5(json& j);

// ----------------------------------------------------------------------------
MappingInfo::MappingInfo(uint32_t checksum)
    : checksum_(checksum)
{}

// ----------------------------------------------------------------------------
MappingInfo::~MappingInfo()
{}

// ----------------------------------------------------------------------------
MappingInfo* MappingInfo::load(FILE* fp, uint32_t size)
{
    // Load json into memory
    auto jsonBlob = Vector<char>::makeResized(size);
    if (fread(jsonBlob.data(), 1, size, fp) != (size_t)size)
        return nullptr;

    // Parse
    json j = json::parse(jsonBlob.begin(), jsonBlob.end(), nullptr, false);
    if (j == json::value_t::discarded)
        return nullptr;

    if (j["version"] == "1.5")
        return load_1_5(j);

    // unsupported version
    return nullptr;
}

// ----------------------------------------------------------------------------
static MappingInfo* load_1_5(json& j)
{
    json jFighterStatuses = j["fighterstatus"];
    json jFighterIDs = j["fighterid"];
    json jStageIDs = j["stageid"];
    json jHitStatuses = j["hitstatus"];

    // The checksum is optional. Replay files don't contain a checksum, but
    // the mappingInfo.json file which stores the info we obtained from the
    // Nintendo Switch does. If it doesn't exist we just use 0.
    json jChecksum = j["checksum"];
    const uint32_t checksum = jChecksum.is_number() ? jChecksum.get<uint32_t>() : 0u;
    std::unique_ptr<MappingInfo> mappingInfo(new MappingInfo(checksum));

    json jFighterBaseStatusMapping = jFighterStatuses["base"];
    for (const auto& [key, value] : jFighterBaseStatusMapping.items())
    {
        std::size_t pos;
        const auto status = FighterStatus::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo->status.addBaseName(status, value.get<std::string>().c_str());
    }

    json jFighterSpecificStatusMapping = jFighterStatuses["specific"];
    for (const auto& [fighter, jsonSpecificMapping] : jFighterSpecificStatusMapping.items())
    {
        std::size_t pos;
        const auto fighterID = FighterID::fromValue(std::stoul(fighter, &pos));
        if (pos != fighter.length())
            return nullptr;
        if (jsonSpecificMapping.is_object() == false)
            return nullptr;

        for (const auto& [key, value] : jsonSpecificMapping.items())
        {
            const auto status = FighterStatus::fromValue(std::stoul(key, &pos));
            if (pos != key.length())
                return nullptr;
            if (value.is_string() == false)
                return nullptr;

            mappingInfo->status.addSpecificName(fighterID, status, value.get<std::string>().c_str());
        }
    }

    for (const auto& [key, value] : jFighterIDs.items())
    {
        std::size_t pos;
        const auto fighterID = FighterID::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo->fighter.add(fighterID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jStageIDs.items())
    {
        std::size_t pos;
        const auto stageID = StageID::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo->stage.add(stageID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jHitStatuses.items())
    {
        std::size_t pos;
        const auto hitStatusID = FighterHitStatus::fromValue(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo->hitStatus.add(hitStatusID, value.get<std::string>().c_str());
    }

    return mappingInfo.release();
}

// ----------------------------------------------------------------------------
uint32_t MappingInfo::save(FILE* fp) const
{
    json fighterBaseStatusMapping;
    const auto baseNames = status.baseNames();
    const auto baseStatuses = status.baseStatuses();
    for (int i = 0; i != baseNames.count(); ++i)
        fighterBaseStatusMapping[std::to_string(baseStatuses[i].value())] = baseNames[i].cStr();

    json fighterSpecificStatusMapping;
    for (const auto& fighterID : status.fighterIDs())
    {
        json specificMapping = json::object();
        const auto specificNames = status.specificNames(fighterID);
        const auto specificStatuses = status.specificStatuses(fighterID);
        for (int i = 0; i != specificNames.count(); ++i)
            specificMapping[std::to_string(specificStatuses[i].value())] = specificNames[i].cStr();

        if (specificMapping.size() > 0)
            fighterSpecificStatusMapping[std::to_string(fighterID.value())] = specificMapping;
    }

    json fighterStatusMapping = {
        {"base", fighterBaseStatusMapping},
        {"specific", fighterSpecificStatusMapping}
    };

    json fighterIDMapping;
    const auto fighterNames = fighter.names();
    const auto fighterIDs = fighter.IDs();
    for (int i = 0; i != fighterIDs.count(); ++i)
        fighterIDMapping[std::to_string(fighterIDs[i].value())] = fighterNames[i].cStr();

    json stageIDMapping;
    const auto stageNames = stage.names();
    const auto stageIDs = stage.IDs();
    for (int i = 0; i != stageIDs.count(); ++i)
        stageIDMapping[std::to_string(stageIDs[i].value())] = stageNames[i].cStr();

    json hitStatusMapping;
    const auto hitStatusNames = hitStatus.names();
    const auto hitStatuses = hitStatus.statuses();
    for (int i = 0; i != hitStatuses.count(); ++i)
        hitStatusMapping[std::to_string(hitStatuses[i].value())] = hitStatusNames[i].cStr();

    json j = {
        {"version", "1.5"},
        {"checksum", checksum_},
        {"fighterstatus", fighterStatusMapping},
        {"fighterid", fighterIDMapping},
        {"stageid", stageIDMapping},
        {"hitstatus", hitStatusMapping}
    };

    const std::string jsonAsString = j.dump();
    if (fwrite(jsonAsString.data(), 1, jsonAsString.length(), fp) != jsonAsString.length())
        return 0;

    return jsonAsString.length();
}

// ----------------------------------------------------------------------------
uint32_t MappingInfo::saveNecessary(FILE* fp, const SessionMetaData* metaData, const FrameData* frameData) const
{
    assert(metaData->fighterCount() == frameData->fighterCount());

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
    std::vector<std::unordered_set<FighterStatus, FighterStatusHasherStd>> usedStatuses;
    std::unordered_set<FighterHitStatus, FighterHitStatusHasherStd> usedHitStatuses;
    for (int fighter = 0; fighter != frameData->fighterCount(); ++fighter)
    {
        std::unordered_set<FighterStatus, FighterStatusHasherStd> usedFighterStatuses;
        for (int frame = 0; frame != frameData->frameCount(); ++frame)
        {
            const FighterState& state = frameData->stateAt(frame, fighter);
            usedFighterStatuses.insert(state.status());
            usedHitStatuses.insert(state.hitStatus());
        }
        usedStatuses.push_back(std::move(usedFighterStatuses));
    }
    std::unordered_set<FighterID, FighterIDHasherStd> usedFighterIDs;
    for (int fighter = 0; fighter != metaData->fighterCount(); ++fighter)
        usedFighterIDs.insert(metaData->fighterID(fighter));

    auto statusUsedByAnyone = [&usedStatuses](FighterStatus status) -> bool {
        for (const auto& usedFighterStatuses : usedStatuses)
            if (usedFighterStatuses.find(status) != usedFighterStatuses.end())
                return true;
        return false;
    };

    json fighterBaseStatusMapping;
    const auto baseNames = status.baseNames();
    const auto baseStatuses = status.baseStatuses();
    for (int i = 0; i != baseStatuses.count(); ++i)
    {
        // Skip saving enums that aren't actually used in the set of player states
        if (statusUsedByAnyone(baseStatuses[i]) == false)
            continue;

        fighterBaseStatusMapping[std::to_string(baseStatuses[i].value())] = baseNames[i].cStr();
    }

    json fighterSpecificStatusMapping;
    for (int fighter = 0; fighter != metaData->fighterCount(); ++fighter)
    {
        json specificMapping = json::object();
        const auto specificNames = status.specificNames(metaData->fighterID(fighter));
        const auto specificStatuses = status.specificStatuses(metaData->fighterID(fighter));
        for (int i = 0; i != specificStatuses.count(); ++i)
        {
            // Skip saving enums that aren't actually used in the set of player states
            if (usedStatuses[fighter].find(specificStatuses[i]) == usedStatuses[fighter].end())
                continue;

            specificMapping[std::to_string(specificStatuses[i].value())] = specificNames[i].cStr();
        }

        if (specificMapping.size() > 0)
            fighterSpecificStatusMapping[std::to_string(metaData->fighterID(fighter).value())] = specificMapping;
    }

    json fighterStatusMapping = {
        {"base", fighterBaseStatusMapping},
        {"specific", fighterSpecificStatusMapping}
    };

    json fighterIDMapping;
    const auto fighterNames = fighter.names();
    const auto fighterIDs = fighter.IDs();
    for (int i = 0; i != fighterIDs.count(); ++i)
        if (usedFighterIDs.find(fighterIDs[i]) != usedFighterIDs.end())
            fighterIDMapping[std::to_string(fighterIDs[i].value())] = fighterNames[i].cStr();

    // Only care about saving the stage that was played on
    json stageIDMapping = {
        {std::to_string(metaData->stageID().value()), stage.toName(metaData->stageID())}
    };

    json hitStatusMapping;
    const auto hitStatusNames = hitStatus.names();
    const auto hitStatuses = hitStatus.statuses();
    for (int i = 0; i != hitStatuses.count(); ++i)
        if (usedHitStatuses.find(hitStatuses[i]) != usedHitStatuses.end())
            hitStatusMapping[std::to_string(hitStatuses[i].value())] = hitStatusNames[i].cStr();

    json j = {
        {"version", "1.5"},
        {"fighterstatus", fighterStatusMapping},
        {"fighterid", fighterIDMapping},
        {"stageid", stageIDMapping},
        {"hitstatus", hitStatusMapping}
    };

    const std::string jsonAsString = j.dump();
    if (fwrite(jsonAsString.data(), 1, jsonAsString.length(), fp) != jsonAsString.length())
        return 0;

    return jsonAsString.length();
}

// ----------------------------------------------------------------------------
uint32_t MappingInfo::checksum() const
{
    return checksum_;
}

}
