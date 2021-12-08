#include "rfcommon/StageIDMapping.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
const String* StageIDMapping::map(StageID stageID) const
{
    auto it = map_.find(stageID);
    if (it == map_.end())
        return nullptr;
    return &it->value();
}

// ----------------------------------------------------------------------------
void StageIDMapping::add(StageID stageID, const String& name)
{
    map_.insertOrGet(stageID, name);
}

}
