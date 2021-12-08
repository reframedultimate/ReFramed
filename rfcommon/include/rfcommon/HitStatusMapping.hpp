#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/LinearMap.hpp"

namespace rfcommon {

extern template class RFCOMMON_TEMPLATE_API SmallVector<FighterHitStatus, 6>;
extern template class RFCOMMON_TEMPLATE_API SmallVector<String, 6>;
extern template class RFCOMMON_TEMPLATE_API SmallLinearMap<FighterHitStatus, String, 6>;

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
