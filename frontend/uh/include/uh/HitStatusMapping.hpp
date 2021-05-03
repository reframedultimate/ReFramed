#pragma once

#include "uh/config.hpp"
#include "uh/Types.hpp"
#include "uh/String.hpp"
#include "uh/LinearMap.hpp"

namespace uh {

extern template class UH_TEMPLATE_API SmallVector<FighterHitStatus, 6>;
extern template class UH_TEMPLATE_API SmallVector<String, 6>;
extern template class UH_TEMPLATE_API SmallLinearMap<FighterHitStatus, String, 6>;

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
