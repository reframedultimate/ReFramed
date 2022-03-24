#pragma once

#include "rfcommon/FighterState.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

class Frame
{
public:
    template <int N>
    Frame(SmallVector<FighterState, N>&& frame)
        : fighters_(std::move(frame))
    {}

    int fighterCount() const
        { return fighters_.count(); }

    const FighterState& fighter(int fighterIdx) const
        { return fighters_[fighterIdx]; }

    const FighterState* begin() const { return fighters_.begin(); }
    const FighterState* end() const { return fighters_.end(); }

    bool hasSameDataAs(const Frame& other) const
    {
        assert(fighterCount() == other.fighterCount());

        // Compare each fighter state with the other fighter state. If they are
        // all equal, then the frame should also compare equal
        for (int i = 0; i != fighterCount(); ++i)
            if (fighter(i).hasSameDataAs(other.fighter(i)) == false)
                return false;
        return true;
    }

private:
    SmallVector<FighterState, 2> fighters_;
};

}
