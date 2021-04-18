#include "uh/FighterStatusMapping.hpp"

namespace uh {

// ----------------------------------------------------------------------------
const std::string* FighterStatusMapping::statusToBaseEnumName(uint16_t status) const
{
    auto it = baseEnumNames_.find(status);
    if (it == baseEnumNames_.end())
        return nullptr;
    return &it->second;
}

// ----------------------------------------------------------------------------
const std::string* FighterStatusMapping::statusToFighterSpecificEnumName(uint16_t status, uint8_t fighterID) const
{
    auto fighter = fighterSpecificEnumNames_.find(fighterID);
    if (fighter == fighterSpecificEnumNames_.end())
        return nullptr;

    auto it = fighter->second.find(status);
    if (it == fighter->second.end())
        return nullptr;

    return &it->second;
}

// ----------------------------------------------------------------------------
void FighterStatusMapping::addBaseEnumName(uint16_t status, const std::string& name)
{
    baseEnumNames_.emplace(status, name);
}

// ----------------------------------------------------------------------------
void FighterStatusMapping::addFighterSpecificEnumName(uint16_t status, uint8_t fighterID, const std::string& name)
{
    auto result = fighterSpecificEnumNames_.emplace(fighterID, std::unordered_map<uint16_t, std::string>());
    result.first->second.emplace(status, name);
}

}
