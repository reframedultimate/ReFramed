#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/LinearMap.hpp"
#include "rfcommon/StageID.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API MappingInfoStage
{
public:
    MappingInfoStage();
    ~MappingInfoStage();

    const char* toName(StageID stageID) const;
    const StageID toID(const char* name) const;

    void add(StageID stageID, const char* name);

    int count() const;
    SmallVector<String, 10> names() const;
    SmallVector<StageID, 10> IDs() const;

private:
    SmallLinearMap<StageID, String, 10> map_;
};

}
