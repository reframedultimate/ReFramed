#include "rfcommon/MappingInfoStatus.hpp"
#include "rfcommon/Profiler.hpp"

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
    NOPROFILE();

    if (const char* name = toName(fighterID, status, nullptr))
        return name;
    return "(Unknown Status)";
}

// ----------------------------------------------------------------------------
const char* MappingInfoStatus::toName(FighterID fighterID, FighterStatus status, const char* fallback) const
{
    PROFILE(MappingInfoStatus, toName);

    const auto baseIt = statusMap_.find(status);
    if (baseIt != statusMap_.end())
        return baseIt->value().cStr();

    const auto fighterIt = specificStatusMap_.find(fighterID);
    if (fighterIt == specificStatusMap_.end())
        return fallback;

    const auto specificIt = fighterIt->value().find(status);
    if (specificIt != fighterIt->value().end())
        return specificIt->value().cStr();

    return fallback;
}

// ----------------------------------------------------------------------------
FighterStatus MappingInfoStatus::toStatus(const char* enumName) const
{
    PROFILE(MappingInfoStatus, toStatus);

    const auto it = enumNameMap_.find(enumName);
    if (it != enumNameMap_.end())
        return it->value();
    return FighterStatus::makeInvalid();
}

// ----------------------------------------------------------------------------
void MappingInfoStatus::addBaseName(FighterStatus status, const char* name)
{
    PROFILE(MappingInfoStatus, addBaseName);

    if (status.isValid() == false)
        return;
    if (statusMap_.insertIfNew(status, name) == statusMap_.end())
        return;
    enumNameMap_.insertAlways(name, status);
}

// ----------------------------------------------------------------------------
void MappingInfoStatus::addSpecificName(FighterID fighterID, FighterStatus status, const char* name)
{
    PROFILE(MappingInfoStatus, addSpecificName);

    if (status.isValid() == false)
        return;
    auto fighterIt = specificStatusMap_.insertOrGet(fighterID, HashMap<FighterStatus, SmallString<31>, FighterStatus::Hasher>());
    fighterIt->value().insertIfNew(status, name);
    enumNameMap_.insertIfNew(name, status);
}

// ----------------------------------------------------------------------------
Vector<FighterID> MappingInfoStatus::fighterIDs() const
{
    PROFILE(MappingInfoStatus, fighterIDs);

    auto result = Vector<FighterID>::makeReserved(specificStatusMap_.count());
    for (const auto it : specificStatusMap_)
        result.push(it->key());
    return result;
}

// ----------------------------------------------------------------------------
Vector<SmallString<31>> MappingInfoStatus::baseNames() const
{
    PROFILE(MappingInfoStatus, baseNames);

    auto result = Vector<SmallString<31>>::makeReserved(statusMap_.count());
    for (const auto it : statusMap_)
        result.push(it->value());
    return result;
}

// ----------------------------------------------------------------------------
Vector<FighterStatus> MappingInfoStatus::baseStatuses() const
{
    PROFILE(MappingInfoStatus, baseStatuses);

    auto result = Vector<FighterStatus>::makeReserved(statusMap_.count());
    for (const auto it : statusMap_)
        result.push(it->key());
    return result;
}

// ----------------------------------------------------------------------------
Vector<SmallString<31>> MappingInfoStatus::specificNames(FighterID fighterID) const
{
    PROFILE(MappingInfoStatus, specificNames);

    Vector<SmallString<31>> result;
    const auto fighterIt = specificStatusMap_.find(fighterID);
    if (fighterIt != specificStatusMap_.end())
        for (const auto it : fighterIt->value())
            result.push(it->value());
    return result;
}

// ----------------------------------------------------------------------------
Vector<FighterStatus> MappingInfoStatus::specificStatuses(FighterID fighterID) const
{
    PROFILE(MappingInfoStatus, specificStatuses);

    Vector<FighterStatus> result;
    const auto fighterIt = specificStatusMap_.find(fighterID);
    if (fighterIt != specificStatusMap_.end())
        for (const auto it : fighterIt->value())
            result.push(it->key());
    return result;
}

}
