#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/FighterHitStatus.hpp"
#include "rfcommon/LinearMap.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

class RFCOMMON_PUBLIC_API MappingInfoHitStatus
{
public:
    MappingInfoHitStatus();
    ~MappingInfoHitStatus();
    
    const char* toName(FighterHitStatus status, const char* fallback=nullptr) const;
    FighterHitStatus toID(const char* name) const;

    void add(FighterHitStatus status, const char* name);

    SmallVector<String, 6> names() const;
    SmallVector<FighterHitStatus, 6> statuses() const;

private:
    SmallLinearMap<FighterHitStatus, String, 6> map_;
};

}
