#include "application/models/HitStatusMapping.hpp"

namespace uh {

// ----------------------------------------------------------------------------
const QString* HitStatusMapping::map(uint8_t status) const
{
    auto it = map_.find(status);
    if (it == map_.end())
        return nullptr;
    return &(*it);
}

// ----------------------------------------------------------------------------
void HitStatusMapping::add(uint8_t status, const QString& name)
{
    map_.insert(status, name);
}

}
