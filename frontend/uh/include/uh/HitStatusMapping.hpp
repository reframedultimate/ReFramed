#pragma once

#include "uh/config.hpp"
#include "uh/Types.hpp"
#include "uh/String.hpp"
#include "uh/LinearMap.hpp"

namespace uh {

template class UH_PUBLIC_API SmallLinearMap<FighterHitStatus, String, 6>::KeyContainer;
template class UH_PUBLIC_API SmallLinearMap<FighterHitStatus, String, 6>::ValueContainer;
template class UH_PUBLIC_API SmallLinearMap<FighterHitStatus, String, 6>;

class UH_PUBLIC_API HitStatusMapping
{
public:
    const String* map(FighterHitStatus status) const;
    void add(FighterHitStatus status, const String& name);

    const SmallLinearMap<FighterHitStatus, String, 6>& get() const { return map_; }

private:
    SmallLinearMap<FighterHitStatus, String, 6> map_;
};

}
