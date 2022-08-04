#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/HashMap.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API MappingInfoFighter
{
public:
    MappingInfoFighter();
    ~MappingInfoFighter();

    const char* toName(FighterID fighterID) const;
    const char* toName(FighterID fighterID, const char* fallback) const;
    const FighterID toID(const char* name) const;

    void add(FighterID fighterID, const char* name);

    Vector<String> names() const;
    Vector<FighterID> IDs() const;

private:
    HashMap<FighterID, String, FighterID::Hasher> nameMap_;
    HashMap<String, FighterID> fighterMap_;
};

}
