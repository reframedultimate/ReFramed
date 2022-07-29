#pragma once

#include "rfcommon/FighterState.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

class FrameDataListener
{
public:
    using Frame = SmallVector<FighterState, 4>;
    virtual void onFrameDataNewUniqueFrame(int frameIdx, const Frame& frame) = 0;
    virtual void onFrameDataNewFrame(int frameIdx, const Frame& frame) = 0;
};

}
