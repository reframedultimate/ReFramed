#pragma once

#include "rfcommon/FighterState.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

class FrameDataListener
{
public:
    virtual void onFrameDataNewUniqueFrame(int frameIdx, const SmallVector<FighterState, 4>& frame) = 0;
    virtual void onFrameDataNewFrame(int frameIdx, const SmallVector<FighterState, 4>& frame) = 0;
};

}
