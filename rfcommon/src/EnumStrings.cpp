#include "rfcommon/EnumStrings.hpp"
#include "nlohmann/json.hpp"

using nlohmann::json;

namespace rfcommon {

// ----------------------------------------------------------------------------
EnumStrings::EnumStrings(uint32_t checksum)
    : checksum_(checksum)
{
}

// ----------------------------------------------------------------------------
bool EnumStrings::saveJSON(const char* fileName) const
{
    json fighterBaseStatusMapping;
    const auto& baseEnumNames = fighterStatus.baseEnumNames();
    for (const auto& it : baseEnumNames)
    {
        /*const QString* shortName = mappingInfo_.fighterStatus.mapToShortName(it.key());
        const QString* customName = mappingInfo_.fighterStatus.mapToCustom(it.key());*/

        fighterBaseStatusMapping[it->key().toStdString()] = {it->value().cStr(), "", ""};
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
    const auto& fighterIDMap = fighterID.get();
    for (const auto& it : fighterIDMap)
        fighterIDMapping[it->key().toStdString()] = it->value().cStr();

    json stageIDMapping;
    const auto& stageIDMap = stageID.get();
    for (const auto& it : stageIDMap)
        stageIDMapping[it->key().toStdString()] = it->value().cStr();

    json hitStatusMapping;
    const auto& hitStatusMap = hitStatus.get();
    for (const auto& it : hitStatusMap)
        hitStatusMapping[it.key().toStdString()] = it.value().cStr();

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
bool EnumStrings::loadJSON(const char* fileName)
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

    const json jsonEnumStrings = json::parse(std::move(s), nullptr, false);
    if (jsonEnumStrings == json::value_t::discarded)
        return nullptr;

    if (jsonEnumStrings.contains("checksum") == false || jsonEnumStrings["checksum"].is_number() == false)
        return nullptr;
    if (jsonEnumStrings.contains("fighterstatus") == false || jsonEnumStrings["fighterstatus"].is_object() == false)
        return nullptr;
    if (jsonEnumStrings.contains("fighterid") == false || jsonEnumStrings["fighterid"].is_object() == false)
        return nullptr;
    if (jsonEnumStrings.contains("stageid") == false || jsonEnumStrings["stageid"].is_object() == false)
        return nullptr;
    if (jsonEnumStrings.contains("hitstatus") == false || jsonEnumStrings["hitstatus"].is_object() == false)
        return nullptr;

    std::unique_ptr<EnumStrings> mappingInfo(new EnumStrings(jsonEnumStrings["checksum"].get<uint32_t>()));

    const json jsonFighterStatusMapping = jsonEnumStrings["fighterstatus"];
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

    for (const auto& [key, value] : jsonEnumStrings["fighterid"].items())
    {
        std::size_t pos;
        FighterID fighterID = static_cast<FighterID>(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo->fighterID.add(fighterID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jsonEnumStrings["stageid"].items())
    {
        std::size_t pos;
        StageID stageID = static_cast<StageID>(std::stoul(key, &pos));
        if (pos != key.length())
            return nullptr;
        if (value.is_string() == false)
            return nullptr;

        mappingInfo->stageID.add(stageID, value.get<std::string>().c_str());
    }

    for (const auto& [key, value] : jsonEnumStrings["hitstatus"].items())
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

// ----------------------------------------------------------------------------
/*
const char* EnumStrings::statusString(FighterStatus status) const
{
    const auto it = statusMap_.find(status);
    if (it == statusMap_.end())
        return nullptr;
    return it->value().cStr();
}*/
const char* EnumStrings::statusString(FighterStatus status, FighterID fighterID) const
{
    const auto fighter = statusStringSpecificMap_.find(status);
    if (fighter == statusStringSpecificMap_.end())
    {
        const auto it = statusStringMap_.find(status);
        if (it == statusStringMap_.end())
            return nullptr;
        return it->value().cStr();
    }

    const auto it = fighter->value().find(status);
    if (it == fighter->value().end())
        return nullptr;

    return it->value().cStr();
}
const FighterStatus* EnumStrings::status(const char* name) const
{
    const auto it = statusMap_.find(name);
    if (it == statusMap_.end())
        return nullptr;
    return &it->value();
}
void EnumStrings::addBaseStatus(FighterStatus status, const char* name)
{
    statusMap_.insertNew(name, status);
    statusStringMap_.insertNew(status, name);
}
void EnumStrings::addSpecificStatus(FighterStatus status, FighterID fighterID, const char* name)
{
    statusMap_.insertNew(name, status);
    auto fighterIt = statusStringSpecificMap_.insertDefaultOrGet(fighterID);
    fighterIt->value().insertNew(status, name);
}

// ----------------------------------------------------------------------------
const char* EnumStrings::hitStatusString(FighterHitStatus status) const
{
    const auto it = hitStatusMap_.findKey(status);
    if (it == hitStatusMap_.end())
        return nullptr;
    return it->value().cStr();
}
const FighterHitStatus* EnumStrings::hitStatus(const char* name) const
{
    const auto it = hitStatusMap_.findValue(name);
    if (it == hitStatusMap_.end())
        return nullptr;
    return &it->key();
}
void EnumStrings::addHitStatus(FighterHitStatus status, const char* name)
{
    hitStatusMap_.insertNew(status, name);
}

// ----------------------------------------------------------------------------
const char* EnumStrings::fighterName(FighterID fighterID) const
{
    const auto it = fighterNameMap_.find(fighterID);
    if (it == fighterNameMap_.end())
        return nullptr;
    return it->value().cStr();
}
const FighterID* EnumStrings::fighterID(const char* name) const
{
    const auto it = fighterMap_.find(name);
    if (it == fighterMap_.end())
        return nullptr;
    return &it->value();
}
void EnumStrings::addFighter(FighterID fighterID, const char* name)
{
    fighterMap_.insert(name, fighterID);
    fighterNameMap_.insert(fighterID, name);
}

// ----------------------------------------------------------------------------
const char* EnumStrings::stageName(StageID stageID) const
{
    const auto it = stageMap_.findKey(stageID);
    if (it == stageMap_.end())
        return nullptr;
    return it->value().cStr();
}
const StageID* EnumStrings::stage(const char* name) const
{
    const auto it = stageMap_.findValue(name);
    if (it == stageMap_.end())
        return nullptr;
    return &it->key();
}
void EnumStrings::addStage(StageID stageID, const char* name)
{
    stageMap_.insertNew(stageID, name);
}

}
