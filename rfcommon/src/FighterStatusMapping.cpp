#include "rfcommon/FighterStatusMapping.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
const String* FighterStatusMapping::statusToBaseEnumName(FighterStatus status) const
{
    auto it = baseEnumNames_.find(status);
    if (it == baseEnumNames_.end())
        return nullptr;
    return &it->value();
}

// ----------------------------------------------------------------------------
const String* FighterStatusMapping::statusToFighterSpecificEnumName(FighterStatus status, FighterID fighterID) const
{
    auto fighter = fighterSpecificEnumNames_.find(fighterID);
    if (fighter == fighterSpecificEnumNames_.end())
        return nullptr;

    auto it = fighter->value().find(status);
    if (it == fighter->value().end())
        return nullptr;

    return &it->value();
}

// ----------------------------------------------------------------------------
void FighterStatusMapping::addBaseEnumName(FighterStatus status, const String& name)
{
    baseEnumNames_.insertOrGet(status, name);
}

// ----------------------------------------------------------------------------
void FighterStatusMapping::addFighterSpecificEnumName(FighterStatus status, FighterID fighterID, const String& name)
{
    auto result = fighterSpecificEnumNames_.insertOrGet(fighterID, HashMap<FighterStatus, String>());
    result->value().insertOrGet(status, name);
}

}
