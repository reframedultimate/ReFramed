#include "rfcommon/MappingInfoStage.hpp"
#include "rfcommon/Profiler.hpp"

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
    PROFILE(MappingInfoStage, toName);

    const auto it = map_.findKey(stageID);
    if (it != map_.end())
        return it->value().cStr();
    return "(Unknown Stage)";
}

// ----------------------------------------------------------------------------
const StageID MappingInfoStage::toID(const char* name) const
{
    PROFILE(MappingInfoStage, toID);

    const auto it = map_.findValue(name);
    if (it != map_.end())
        return it->key();
    return StageID::makeInvalid();
}

// ----------------------------------------------------------------------------
void MappingInfoStage::add(StageID stageID, const char* name)
{
    PROFILE(MappingInfoStage, add);

    if (stageID.isValid() == false)
        return;
    map_.insertIfNew(stageID, name);
}

// ----------------------------------------------------------------------------
int MappingInfoStage::count() const
{
    NOPROFILE();

    return map_.count();
}

// ----------------------------------------------------------------------------
SmallVector<String, 10> MappingInfoStage::names() const
{
    PROFILE(MappingInfoStage, names);

    SmallVector<String, 10> result;
    for (const auto it : map_)
        result.push(it->value());
    return result;
}

// ----------------------------------------------------------------------------
SmallVector<StageID, 10> MappingInfoStage::IDs() const
{
    PROFILE(MappingInfoStage, IDs);

    SmallVector<StageID, 10> result;
    for (const auto it : map_)
        result.push(it->key());
    return result;
}

}
