#pragma once

#include "uh/config.hpp"
#include "uh/Types.hpp"
#include "uh/String.hpp"
#include <unordered_map>

namespace uh {

class UH_PUBLIC_API FighterIDMapping
{
public:
    const String* map(FighterID fighterID) const;
    void add(FighterID fighterId, const String& name);

    const std::unordered_map<FighterID, String>& get() const { return map_; }

private:
    std::unordered_map<FighterID, String> map_;
};

}
