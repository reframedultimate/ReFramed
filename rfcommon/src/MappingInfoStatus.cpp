#include "rfcommon/MappingInfoStatus.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
MappingInfoStatus::MappingInfoStatus()
{}

// ----------------------------------------------------------------------------
MappingInfoStatus::~MappingInfoStatus()
{}

// ----------------------------------------------------------------------------
const char* MappingInfoStatus::toName(FighterID fighterID, FighterStatus status) const
{
    const auto baseIt = statusMap_.find(status);
    if (baseIt != statusMap_.end())
        return baseIt->value().cStr();

    const auto fighterIt = specificStatusMap_.find(fighterID);
    if (fighterIt == specificStatusMap_.end())
        return "(Unknown Status)";

    const auto specificIt = fighterIt->value().find(status);
    if (specificIt != fighterIt->value().end())
        return specificIt->value().cStr();

    return "(Unknown Status)";
}

// ----------------------------------------------------------------------------
FighterStatus MappingInfoStatus::toStatus(const char* enumName) const
{
    const auto it = enumNameMap_.find(enumName);
    if (it != enumNameMap_.end())
        return it->value();
    return FighterStatus::makeInvalid();
}

// ----------------------------------------------------------------------------
void MappingInfoStatus::addBaseName(FighterStatus status, const char* name)
{
    if (statusMap_.insertIfNew(status, name) == statusMap_.end())
        return;
    enumNameMap_.insertAlways(name, status);
}

// ----------------------------------------------------------------------------
void MappingInfoStatus::addSpecificName(FighterID fighterID, FighterStatus status, const char* name)
{
    auto fighterIt = specificStatusMap_.insertOrGet(fighterID, HashMap<FighterStatus, SmallString<31>, FighterStatus::Hasher>());
    fighterIt->value().insertIfNew(status, name);
    enumNameMap_.insertIfNew(name, status);
}

// ----------------------------------------------------------------------------
Vector<FighterID> MappingInfoStatus::fighterIDs() const
{
    auto result = Vector<FighterID>::makeReserved(specificStatusMap_.count());
    for (const auto it : specificStatusMap_)
        result.push(it->key());
    return result;
}

// ----------------------------------------------------------------------------
Vector<SmallString<31>> MappingInfoStatus::baseNames() const
{
    auto result = Vector<SmallString<31>>::makeReserved(statusMap_.count());
    for (const auto it : statusMap_)
        result.push(it->value());
    return result;
}

// ----------------------------------------------------------------------------
Vector<FighterStatus> MappingInfoStatus::baseStatuses() const
{
    auto result = Vector<FighterStatus>::makeReserved(statusMap_.count());
    for (const auto it : statusMap_)
        result.push(it->key());
    return result;
}

// ----------------------------------------------------------------------------
Vector<SmallString<31>> MappingInfoStatus::specificNames(FighterID fighterID) const
{
    Vector<SmallString<31>> result;
    for (const auto fighterIt : specificStatusMap_)
        for (const auto it : fighterIt->value())
            result.push(it->value());
    return result;
}

// ----------------------------------------------------------------------------
Vector<FighterStatus> MappingInfoStatus::specificStatuses(FighterID fighterID) const
{
    Vector<FighterStatus> result;
    for (const auto fighterIt : specificStatusMap_)
        for (const auto it : fighterIt->value())
            result.push(it->key());
    return result;
}

}
