#pragma once

#include "uh/config.hpp"
#include "uh/Types.hpp"
#include "uh/String.hpp"
#include <unordered_map>

namespace uh {

class UH_PUBLIC_API StageIDMapping
{
public:
    const String* map(StageID stageID) const;
    void add(StageID stageID, const String& name);

    const std::unordered_map<StageID, String>& get() const { return map_; }

private:
    std::unordered_map<StageID, String> map_;
};

}
