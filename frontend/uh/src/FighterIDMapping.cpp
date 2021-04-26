#include "uh/FighterIDMapping.hpp"

namespace uh {

// ----------------------------------------------------------------------------
const String* FighterIDMapping::map(FighterID fighterID) const
{
    auto it = map_.find(fighterID);
    if (it == map_.end())
        return nullptr;
    return &it->value();
}

// ----------------------------------------------------------------------------
void FighterIDMapping::add(FighterID fighterID, const String& name)
{
    map_.insertOrGet(fighterID, name);
}

}
