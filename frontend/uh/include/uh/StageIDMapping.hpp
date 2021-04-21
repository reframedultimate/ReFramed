#pragma once

#include "uh/config.hpp"
#include "uh/Types.hpp"
#include <unordered_map>
#include <string>

namespace uh {

class UH_PUBLIC_API StageIDMapping
{
public:
    const std::string* map(StageID stageID) const;
    void add(StageID stageID, const std::string& name);

    const std::unordered_map<StageID, std::string>& get() const { return map_; }

private:
    std::unordered_map<StageID, std::string> map_;
};

}
