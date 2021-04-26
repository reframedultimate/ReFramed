#include "uh/HitStatusMapping.hpp"

namespace uh {

// ----------------------------------------------------------------------------
const String* HitStatusMapping::map(FighterHitStatus status) const
{
    auto it = map_.find(status);
    if (it == map_.end())
        return nullptr;
    return &it->value();
}

// ----------------------------------------------------------------------------
void HitStatusMapping::add(FighterHitStatus status, const String& name)
{
    map_.insert(status, name);
}

}
