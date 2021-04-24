#pragma once

#include "uh/config.hpp"
#include <string>
#include <unordered_map>

namespace uh {

class UH_PUBLIC_API FighterIDMapping
{
public:
    const std::string* map(uint8_t fighterID) const;
    void add(uint8_t fighterId, const std::string& name);

    const std::unordered_map<uint8_t, std::string>& get() const { return map_; }

private:
    std::unordered_map<uint8_t, std::string> map_;
};

}
