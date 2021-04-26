#include "uh/StageIDMapping.hpp"

namespace uh {

// ----------------------------------------------------------------------------
const String* StageIDMapping::map(StageID stageID) const
{
    auto it = map_.find(stageID);
    if (it == map_.end())
        return nullptr;
    return &it->second;
}

// ----------------------------------------------------------------------------
void StageIDMapping::add(StageID stageID, const String& name)
{
    map_.emplace(stageID, name);
}

}
