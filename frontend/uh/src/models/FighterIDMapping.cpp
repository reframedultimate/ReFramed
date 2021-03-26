#include "uh/models/FighterIDMapping.hpp"

namespace uh {

// ----------------------------------------------------------------------------
const QString* FighterIDMapping::map(uint8_t fighterID) const
{
    auto it = map_.find(fighterID);
    if (it == map_.end())
        return nullptr;
    return &(*it);
}

// ----------------------------------------------------------------------------
void FighterIDMapping::add(uint8_t fighterID, const QString& name)
{
    map_.insert(fighterID, name);
}

}
