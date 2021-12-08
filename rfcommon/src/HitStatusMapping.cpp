#include "rfcommon/HitStatusMapping.hpp"

namespace rfcommon {

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
    map_.insertOrGet(status, name);
}

}
