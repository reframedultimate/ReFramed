#pragma once

#include <string>
#include <unordered_map>

namespace uh {

class HitStatusMapping
{
public:
    const std::string* map(uint8_t status) const;
    void add(uint8_t stageID, const std::string& name);

    const std::unordered_map<uint8_t, std::string>& get() const { return map_; }

private:
    std::unordered_map<uint8_t, std::string> map_;
};

}
