#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/HashMap.hpp"

namespace rfcommon {

extern template class RFCOMMON_TEMPLATE_API Vector<uint32_t>;
extern template class RFCOMMON_TEMPLATE_API HashMap<FighterID, String>;

class RFCOMMON_PUBLIC_API FighterIDMapping
{
public:
    const String* map(FighterID fighterID) const;
    void add(FighterID fighterId, const String& name);

    const HashMap<FighterID, String>& get() const { return map_; }

private:
    HashMap<FighterID, String> map_;
};

}
