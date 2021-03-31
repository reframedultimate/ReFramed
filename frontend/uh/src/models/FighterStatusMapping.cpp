#include "uh/models/FighterStatusMapping.hpp"

namespace uh {

// ----------------------------------------------------------------------------
const QString* FighterStatusMapping::statusToBaseEnumName(uint16_t status) const
{
    auto it = baseEnumNames_.find(status);
    if (it == baseEnumNames_.end())
        return nullptr;
    return &(*it);
}

// ----------------------------------------------------------------------------
const QString* FighterStatusMapping::statusToFighterSpecificEnumName(uint16_t status, uint8_t fighterID) const
{
    auto fighter = fighterSpecificEnumNames_.find(fighterID);
    if (fighter == fighterSpecificEnumNames_.end())
        return nullptr;

    auto it = fighter.value().find(status);
    if (it == fighter.value().end())
        return nullptr;

    return &(*it);
}

// ----------------------------------------------------------------------------
void FighterStatusMapping::addBaseEnumName(uint16_t status, const QString& name)
{
    baseEnumNames_.insert(status, name);
}

// ----------------------------------------------------------------------------
void FighterStatusMapping::addFighterSpecificEnumName(uint16_t status, uint8_t fighterID, const QString& name)
{
    auto fighter = fighterSpecificEnumNames_.find(fighterID);
    if (fighter == fighterSpecificEnumNames_.end())
        fighter = fighterSpecificEnumNames_.insert(fighterID, QHash<uint16_t, QString>());
    fighter.value().insert(status, name);
}

}
