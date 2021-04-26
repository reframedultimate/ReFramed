#pragma once

#include "uh/config.hpp"
#include "uh/Types.hpp"
#include "uh/String.hpp"
#include <unordered_map>

namespace uh {

class UH_PUBLIC_API HitStatusMapping
{
public:
    const String* map(FighterHitStatus status) const;
    void add(FighterHitStatus status, const String& name);

    const std::unordered_map<FighterHitStatus, String>& get() const { return map_; }

private:
    std::unordered_map<FighterHitStatus, String> map_;
};

}
