#include "uh/models/FighterStatusMapping.hpp"

namespace uh {

// ----------------------------------------------------------------------------
const QString* FighterStatusMapping::map(uint16_t status) const
{
    auto it = map_.find(status);
    if (it == map_.end())
        return nullptr;
    return &(*it);
}

// ----------------------------------------------------------------------------
void FighterStatusMapping::add(uint16_t status, const QString& name)
{
    map_.insert(status, name);
}

}
