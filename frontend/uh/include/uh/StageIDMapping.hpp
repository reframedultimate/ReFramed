#pragma once

#include "uh/config.hpp"
#include "uh/Types.hpp"
#include "uh/String.hpp"
#include "uh/LinearMap.hpp"

namespace uh {

class UH_PUBLIC_API StageIDMapping
{
public:
    const String* map(StageID stageID) const;
    void add(StageID stageID, const String& name);

    const SmallLinearMap<StageID, String, 10>& get() const { return map_; }

private:
    SmallLinearMap<StageID, String, 10> map_;
};

}
