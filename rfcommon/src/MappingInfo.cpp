#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/FighterState.hpp"
#include "rfcommon/SessionMetaData.hpp"
#include "nlohmann/json.hpp"
#include <memory>
#include <unordered_set>

namespace rfcommon {

using nlohmann::json;

static MappingInfo* load_1_5(const json& j);

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
    Vector<char> jsonBlob(size);
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
static MappingInfo* load_1_5(const json& j)
{
    const json jFighterStatuses = j["fighterstatus"];
    const json jFighterIDs = j["fighterid"];
    const json jStageIDs = j["stageid"];
    const json jHitStatuses = j["hitstatus"];

    // The checksum is optional. Replay files don't contain a checksum, but
    // the mappingInfo.json file which stores the info we obtained from the
    // Nintendo Switch does. If it doesn't exist we just use 0.
    const json jChecksum = j["checksum"];
    const uint32_t checksum = jChecksum.is_number() ? jChecksum.get<uint32_t>() : 0u;
    std::unique_ptr<MappingInfo> mappingInfo(new MappingInfo(checksum));

    const json jFighterBaseStatusMapping = jFighterStatuses["base"];
    for (const auto& [key, value] : jFighterBaseStatusMapping.items())
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

        mappingInfo->status.addBaseEnumName(status, value[0].get<std::string>().c_str());
    }

    const json jFighterSpecificStatusMapping = jFighterStatuses["specific"];
    for (const auto& [fighter, jsonSpecificMapping] : jFighterSpecificStatusMapping.items())
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

            mappingInfo->status.addFighterSpecificEnumName(status, fighterID, value[0].get<std::string>().c_str());
        }
    }

    for (const auto& [key, value] : jFighterIDs.items())
    {
        std::size_t pos;
        FighterID fighterID(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo->fighterID.add(fighterID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jStageIDs.items())
    {
        std::size_t pos;
        StageID stageID(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo->stageID.add(stageID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jHitStatuses.items())
    {
        std::size_t pos;
        FighterHitStatus hitStatusID(std::stoul(key, &pos));
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
    const auto& baseEnumNames = status.baseMap();
    for (const auto& it : baseEnumNames)
    {
        const FighterStatus& status = it->key();
        const SmallString<31>& enumName = it->value();
        fighterBaseStatusMapping[status.valueToStdString()] = enumName.cStr();
    }

    json fighterSpecificStatusMapping;
    const auto& specificEnumNames = status.specificMap();
    for (const auto& fighter : specificEnumNames)
    {
        json specificMapping = json::object();
        for (const auto& it : fighter->value())
        {
            const FighterStatus& status = it->key();
            const SmallString<31>& enumName = it->value();
            specificMapping[status.valueToStdString()] = enumName.cStr();
        }

        if (specificMapping.size() > 0)
            fighterSpecificStatusMapping[fighter->key().valueToStdString()] = specificMapping;
    }

    json fighterStatusMapping = {
        {"base", fighterBaseStatusMapping},
        {"specific", fighterSpecificStatusMapping}
    };

    json fighterIDMapping;
    const auto& fighterIDMap = fighterID.map();
    for (const auto& it : fighterIDMap)
        fighterIDMapping[it->key().valueToStdString()] = it->value().cStr();

    json stageIDMapping;
    const auto& stageIDMap = stageID.map();
    for (const auto& it : stageIDMap)
        stageIDMapping[it->key().valueToStdString()] = it->value().cStr();

    json hitStatusMapping;
    const auto& hitStatusMap = hitStatus.map();
    for (const auto& it : hitStatusMap)
        hitStatusMapping[it.key().toStdString()] = it.value().cStr();

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
uint32_t MappingInfo::saveFiltered(FILE* fp, const SessionMetaData* metaData, const FrameData* frameData) const
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
    std::unordered_set<FighterStatus, FighterStatusHasherStd> usedStatuses;
    std::unordered_set<FighterHitStatus, FighterHitStatusHasherStd> usedHitStatuses;
    for (int fighter = 0; fighter != frameData->fighterCount(); ++fighter)
        for (int frame = 0; frame != frameData->frameCount(); ++frame)
        {
            const FighterState& state = frameData->stateAt(frame, fighter);
            usedStatuses.insert(state.status());
            usedHitStatuses.insert(state.hitStatus());
        }
    std::unordered_set<FighterID, FighterIDHasherStd> usedFighterIDs;
    for (int fighter = 0; fighter != metaData->fighterCount(); ++fighter)
        usedFighterIDs.insert(metaData->fighterID(fighter));

    json fighterBaseStatusMapping;
    const auto& baseEnumNames = status.baseMap();
    for (const auto& it : baseEnumNames)
    {
        // Skip saving enums that aren't actually used in the set of player states
        if (usedStatuses.find(it->key()) == usedStatuses.end())
            continue;

        /*const QString* shortName = mappingInfo_.fighterStatus.mapToShortName(it.key());
        const QString* customName = mappingInfo_.fighterStatus.mapToCustom(it.key());*/

        const FighterStatus& status = it->key();
        const SmallString<31>& enumName = it->value();
        fighterBaseStatusMapping[status.valueToStdString()] = enumName.cStr();
    }

    json fighterSpecificStatusMapping;
    const auto& specificEnumNames = status.specificMap();
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

            const FighterStatus& status = it->key();
            const SmallString<31>& enumName = it->value();
            specificMapping[status.valueToStdString()] = enumName.cStr();
        }

        if (specificMapping.size() > 0)
            fighterSpecificStatusMapping[fighter->key().valueToStdString()] = specificMapping;
    }

    json fighterStatusMapping = {
        {"base", fighterBaseStatusMapping},
        {"specific", fighterSpecificStatusMapping}
    };

    json fighterIDMapping;
    const auto& fighterIDMap = fighterID.map();
    for (const auto& it : fighterIDMap)
        if (usedFighterIDs.find(it->key()) != usedFighterIDs.end())
            fighterIDMapping[it->key().valueToStdString()] = it->value().cStr();

    json stageIDMapping;
    const auto& stageIDMap = stageID.map();
    for (const auto& it : stageIDMap)
        if (it->key() == metaData->stageID())  // Only care about saving the stage that was played on
            stageIDMapping[it->key().valueToStdString()] = it->value().cStr();

    json hitStatusMapping;
    const auto& hitStatusMap = hitStatus.map();
    for (const auto& it : hitStatusMap)
        if (usedHitStatuses.find(it.key()) != usedHitStatuses.end())
            hitStatusMapping[it.key().toStdString()] = it.value().cStr();

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
