#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/LinearMap.hpp"

namespace rfcommon {

extern template class RFCOMMON_TEMPLATE_API SmallVector<StageID, 10>;
extern template class RFCOMMON_TEMPLATE_API SmallVector<String, 10>;
extern template class RFCOMMON_TEMPLATE_API SmallLinearMap<StageID, String, 10>;

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
