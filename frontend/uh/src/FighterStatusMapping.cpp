#include "uh/FighterStatusMapping.hpp"

namespace uh {

// ----------------------------------------------------------------------------
const String* FighterStatusMapping::statusToBaseEnumName(FighterStatus status) const
{
    auto it = baseEnumNames_.find(status);
    if (it == baseEnumNames_.end())
        return nullptr;
    return &it->second;
}

// ----------------------------------------------------------------------------
const String* FighterStatusMapping::statusToFighterSpecificEnumName(FighterStatus status, FighterID fighterID) const
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
void FighterStatusMapping::addBaseEnumName(FighterStatus status, const String& name)
{
    baseEnumNames_.emplace(status, name);
}

// ----------------------------------------------------------------------------
void FighterStatusMapping::addFighterSpecificEnumName(FighterStatus status, FighterID fighterID, const String& name)
{
    auto result = fighterSpecificEnumNames_.emplace(fighterID, std::unordered_map<FighterStatus, String>());
    result.first->second.emplace(status, name);
}

}
