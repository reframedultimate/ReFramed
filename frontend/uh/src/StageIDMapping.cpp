#include "uh/StageIDMapping.hpp"

namespace uh {

// ----------------------------------------------------------------------------
const std::string* StageIDMapping::map(StageID stageID) const
{
    auto it = map_.find(stageID);
    if (it == map_.end())
        return nullptr;
    return &it->second;
}

// ----------------------------------------------------------------------------
void StageIDMapping::add(StageID stageID, const std::string& name)
{
    map_.emplace(stageID, name);
}

}
