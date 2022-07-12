#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/LinearMap.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API HitStatusMapping
{
public:
    const String* map(FighterHitStatus status) const;
    void add(FighterHitStatus status, const String& name);

    const SmallLinearMap<FighterHitStatus, String, 6>& get() const { return map_; }

private:
    SmallLinearMap<FighterHitStatus, String, 6> map_;
};

}
