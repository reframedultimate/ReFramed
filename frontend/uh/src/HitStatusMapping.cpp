#include "uh/HitStatusMapping.hpp"

namespace uh {

// ----------------------------------------------------------------------------
const std::string* HitStatusMapping::map(uint8_t status) const
{
    auto it = map_.find(status);
    if (it == map_.end())
        return nullptr;
    return &it->second;
}

// ----------------------------------------------------------------------------
void HitStatusMapping::add(uint8_t status, const std::string& name)
{
    map_.emplace(status, name);
}

}
