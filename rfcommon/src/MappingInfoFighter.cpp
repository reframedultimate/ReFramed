#include "rfcommon/MappingInfoFighter.hpp"
#include "rfcommon/Profiler.hpp"

namespace rfcommon {

// ----------------------------------------------------------------------------
MappingInfoFighter::MappingInfoFighter()
{}

MappingInfoFighter::~MappingInfoFighter()
{}

// ----------------------------------------------------------------------------
const char* MappingInfoFighter::toName(FighterID fighterID) const
{
    NOPROFILE();

    return toName(fighterID, "(Unknown Fighter)");
}

// ----------------------------------------------------------------------------
const char* MappingInfoFighter::toName(FighterID fighterID, const char* fallback) const
{
    PROFILE(MappingInfoFighter, toName);

    const auto it = nameMap_.find(fighterID);
    if (it != nameMap_.end())
        return it->value().cStr();
    return fallback;
}

// ----------------------------------------------------------------------------
const FighterID MappingInfoFighter::toID(const char* name) const
{
    PROFILE(MappingInfoFighter, toID);

    const auto it = fighterMap_.find(name);
    if (it != fighterMap_.end())
        return it->value();
    return FighterID::makeInvalid();
}

// ----------------------------------------------------------------------------
void MappingInfoFighter::add(FighterID fighterID, const char* name)
{
    PROFILE(MappingInfoFighter, add);

    if (fighterID.isValid() == false || strlen(name) == 0)
        return;
    if (nameMap_.insertIfNew(fighterID, name) == nameMap_.end())
        return;
    fighterMap_.insertAlways(name, fighterID);
}

// ----------------------------------------------------------------------------
Vector<String> MappingInfoFighter::names() const
{
    PROFILE(MappingInfoFighter, names);

    auto result = Vector<String>::makeReserved(nameMap_.count());
    for (const auto it : nameMap_)
        result.push(it->value());
    return result;
}

// ----------------------------------------------------------------------------
Vector<FighterID> MappingInfoFighter::IDs() const
{
    PROFILE(MappingInfoFighter, IDs);

    auto result = Vector<FighterID>::makeReserved(nameMap_.count());
    for (const auto it : nameMap_)
        result.push(it->key());
    return result;
}

}
