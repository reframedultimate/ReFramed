#include "uh/models/StageIDMapping.hpp"

namespace uh {

// ----------------------------------------------------------------------------
const QString* StageIDMapping::map(uint16_t stageID) const
{
    auto it = map_.find(stageID);
    if (it == map_.end())
        return nullptr;
    return &(*it);
}

// ----------------------------------------------------------------------------
void StageIDMapping::add(uint16_t stageID, const QString& name)
{
    map_.insert(stageID, name);
}

}
