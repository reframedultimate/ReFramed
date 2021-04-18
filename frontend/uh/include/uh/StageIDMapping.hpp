#pragma once

#include <unordered_map>
#include <string>

namespace uh {

class StageIDMapping
{
public:
    const std::string* map(uint16_t stageID) const;
    void add(uint16_t stageID, const std::string& name);

    const std::unordered_map<uint16_t, std::string>& get() const { return map_; }

private:
    std::unordered_map<uint16_t, std::string> map_;
};

}
