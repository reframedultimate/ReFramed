#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/HashMap.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API FighterIDMapping
{
public:
    const String* map(FighterID fighterID) const;
    void add(FighterID fighterID, const String& name);

    const HashMap<FighterID, String, FighterIDHasher>& get() const { return map_; }

private:
    HashMap<FighterID, String, FighterIDHasher> map_;
};

}
