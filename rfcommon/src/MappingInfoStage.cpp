#include "rfcommon/MappingInfoStage.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
MappingInfoStage::MappingInfoStage()
{}

// ----------------------------------------------------------------------------
MappingInfoStage::~MappingInfoStage()
{}

// ----------------------------------------------------------------------------
const char* MappingInfoStage::toName(StageID stageID) const
{
    const auto it = map_.findKey(stageID);
    if (it != map_.end())
        return it->value().cStr();
    return "(Unknown Stage)";
}

// ----------------------------------------------------------------------------
const StageID MappingInfoStage::toID(const char* name) const
{
    const auto it = map_.findValue(name);
    if (it != map_.end())
        return it->key();
    return StageID::makeInvalid();
}

// ----------------------------------------------------------------------------
void MappingInfoStage::add(StageID stageID, const char* name)
{
    map_.insertNew(stageID, name);
}

// ----------------------------------------------------------------------------
SmallVector<String, 10> MappingInfoStage::names() const
{
    SmallVector<String, 10> result;
    for (const auto it : map_)
        result.push(it->value());
    return result;
}

// ----------------------------------------------------------------------------
SmallVector<StageID, 10> MappingInfoStage::IDs() const
{
    SmallVector<StageID, 10> result;
    for (const auto it : map_)
        result.push(it->key());
    return result;
}

}
