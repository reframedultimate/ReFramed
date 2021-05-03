#pragma once

#include "uh/config.hpp"
#include "uh/Types.hpp"
#include "uh/String.hpp"
#include "uh/HashMap.hpp"

namespace uh {

template class UH_PUBLIC_API HashMap<FighterID, String>::TableContainer;
template class UH_PUBLIC_API HashMap<FighterID, String>;

class UH_PUBLIC_API FighterIDMapping
{
public:
    const String* map(FighterID fighterID) const;
    void add(FighterID fighterId, const String& name);

    const HashMap<FighterID, String>& get() const { return map_; }

private:
    HashMap<FighterID, String> map_;
};

}
