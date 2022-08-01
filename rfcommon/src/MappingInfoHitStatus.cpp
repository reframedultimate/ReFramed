#include "rfcommon/MappingInfoHitStatus.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
MappingInfoHitStatus::MappingInfoHitStatus()
{}

// ----------------------------------------------------------------------------
MappingInfoHitStatus::~MappingInfoHitStatus()
{}

// ----------------------------------------------------------------------------
const char* MappingInfoHitStatus::toName(FighterHitStatus status) const
{
    const auto it = map_.findKey(status);
    if (it != map_.end())
        return it->value().cStr();
    return "(Unknown Hit Status)";
}

// ----------------------------------------------------------------------------
FighterHitStatus MappingInfoHitStatus::toID(const char* name) const
{
    const auto it = map_.findValue(name);
    if (it != map_.end())
        return it->key();
    return FighterHitStatus::makeInvalid();
}

// ----------------------------------------------------------------------------
void MappingInfoHitStatus::add(FighterHitStatus status, const char* name)
{
    map_.insertNew(status, name);
}

// ----------------------------------------------------------------------------
SmallVector<String, 6> MappingInfoHitStatus::names() const
{
    SmallVector<String, 6> result;
    for (const auto it : map_)
        result.push(it->value());
    return result;
}

// ----------------------------------------------------------------------------
SmallVector<FighterHitStatus, 6> MappingInfoHitStatus::statuses() const
{
    SmallVector<FighterHitStatus, 6> result;
    for (const auto it : map_)
        result.push(it->key());
    return result;
}

}
