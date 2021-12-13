#include "rfcommon/MappingInfo.hpp"
#include "nlohmann/json.hpp"

using nlohmann::json;

namespace rfcommon {

// ----------------------------------------------------------------------------
MappingInfo::MappingInfo(uint32_t checksum)
    : checksum_(checksum)
{
}

// ----------------------------------------------------------------------------
bool MappingInfo::save(const String& fileName) const
{
    json fighterBaseStatusMapping;
    const auto& baseEnumNames = fighterStatus.baseEnumNames();
    for (const auto& it : baseEnumNames)
    {
        /*const QString* shortName = mappingInfo_.fighterStatus.mapToShortName(it.key());
        const QString* customName = mappingInfo_.fighterStatus.mapToCustom(it.key());*/

        fighterBaseStatusMapping[std::to_string(it->key())] = {it->value().cStr(), "", ""};
    }

    json fighterSpecificStatusMapping;
    const auto& specificEnumNames = fighterStatus.fighterSpecificEnumNames();
    for (const auto& fighter : specificEnumNames)
    {
        json specificMapping = json::object();
        for (const auto& it : fighter->value())
        {
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
    const auto& fighterIDMap = fighterID.get();
    for (const auto& it : fighterIDMap)
        fighterIDMapping[std::to_string(it->key())] = it->value().cStr();

    json stageIDMapping;
    const auto& stageIDMap = stageID.get();
    for (const auto& it : stageIDMap)
        stageIDMapping[std::to_string(it->key())] = it->value().cStr();

    json hitStatusMapping;
    const auto& hitStatusMap = hitStatus.get();
    for (const auto& it : hitStatusMap)
        hitStatusMapping[std::to_string(it.key())] = it.value().cStr();

    json mappingInfo = {
        {"checksum", checksum_},
        {"fighterstatus", fighterStatusMapping},
        {"fighterid", fighterIDMapping},
        {"stageid", stageIDMapping},
        {"hitstatus", hitStatusMapping}
    };

    FILE* fp = fopen(fileName.cStr(), "w");
    if (fp == nullptr)
        return false;

    std::string s = mappingInfo.dump();
    fwrite(s.data(), 1, s.length(), fp);
    fclose(fp);
    return true;
}

// ----------------------------------------------------------------------------
MappingInfo* MappingInfo::load(const String& fileName)
{
    std::string s;
    FILE* fp = fopen(fileName.cStr(), "r");
    if (fp == nullptr)
        return nullptr;

    while (!feof(fp))
    {
        size_t prevSize = s.size();
        s.resize(s.size() + 0x1000, '\0');
        void* dst = static_cast<void*>(s.data() + prevSize);
        int len = fread(dst, 1, 0x1000, fp);
        if (len < 0x1000)
        {
            if (feof(fp))
            {
                s.resize(prevSize + len);
                break;
            }
            else
            {
                fclose(fp);
                return nullptr;
            }
        }
    }
    fclose(fp);

    const json jsonMappingInfo = json::parse(std::move(s), nullptr, false);
    if (jsonMappingInfo == json::value_t::discarded)
        return nullptr;

    if (jsonMappingInfo.contains("checksum") == false || jsonMappingInfo["checksum"].is_number() == false)
        return nullptr;
    if (jsonMappingInfo.contains("fighterstatus") == false || jsonMappingInfo["fighterstatus"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("fighterid") == false || jsonMappingInfo["fighterid"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("stageid") == false || jsonMappingInfo["stageid"].is_object() == false)
        return nullptr;
    if (jsonMappingInfo.contains("hitstatus") == false || jsonMappingInfo["hitstatus"].is_object() == false)
        return nullptr;

    std::unique_ptr<MappingInfo> mappingInfo(new MappingInfo(jsonMappingInfo["checksum"].get<uint32_t>()));

    const json jsonFighterStatusMapping = jsonMappingInfo["fighterstatus"];
    if (jsonFighterStatusMapping.contains("base") == false || jsonFighterStatusMapping["base"].is_object() == false)
        return nullptr;
    if (jsonFighterStatusMapping.contains("specific") == false)
        return nullptr;

    for (const auto& [key, value] : jsonFighterStatusMapping["base"].items())
    {
        std::size_t pos;
        FighterStatus status = static_cast<FighterStatus>(std::stoul(key, &pos));
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

        mappingInfo->fighterStatus.addBaseEnumName(status, value[0].get<std::string>().c_str());
    }

    if (!jsonFighterStatusMapping["specific"].is_null())
    {
        for (const auto& [fighter, jsonSpecificMapping] : jsonFighterStatusMapping["specific"].items())
        {
            std::size_t pos;
            FighterID fighterID = static_cast<FighterID>(std::stoul(fighter, &pos));
            if (pos != fighter.length())
                return nullptr;
            if (jsonSpecificMapping.is_object() == false)
                return nullptr;

            for (const auto& [key, value] : jsonSpecificMapping.items())
            {
                FighterStatus status = static_cast<FighterStatus>(std::stoul(key, &pos));
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

                mappingInfo->fighterStatus.addFighterSpecificEnumName(status, fighterID, value[0].get<std::string>().c_str());
            }
        }
    }

    for (const auto& [key, value] : jsonMappingInfo["fighterid"].items())
    {
        std::size_t pos;
        FighterID fighterID = static_cast<FighterID>(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo->fighterID.add(fighterID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jsonMappingInfo["stageid"].items())
    {
        std::size_t pos;
        StageID stageID = static_cast<StageID>(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo->stageID.add(stageID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jsonMappingInfo["hitstatus"].items())
    {
        std::size_t pos;
        FighterHitStatus hitStatusID = static_cast<FighterHitStatus>(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo->hitStatus.add(hitStatusID, value.get<std::string>().c_str());
    }

    return mappingInfo.release();
}

}
