#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/LinearMap.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API StageIDMapping
{
public:
    const String* map(StageID stageID) const;
    void add(StageID stageID, const String& name);

    const SmallLinearMap<StageID, String, 10>& get() const { return map_; }

private:
    SmallLinearMap<StageID, String, 10> map_;
};

}
