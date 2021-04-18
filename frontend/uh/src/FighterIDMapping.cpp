#include "uh/FighterIDMapping.hpp"

namespace uh {

// ----------------------------------------------------------------------------
const std::string* FighterIDMapping::map(uint8_t fighterID) const
{
    auto it = map_.find(fighterID);
    if (it == map_.end())
        return nullptr;
    return &it->second;
}

// ----------------------------------------------------------------------------
void FighterIDMapping::add(uint8_t fighterID, const std::string& name)
{
    map_.emplace(fighterID, name);
}

}
