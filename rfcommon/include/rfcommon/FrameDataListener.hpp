#pragma once

namespace rfcommon {

template <int N> class Frame;

class FrameDataListener
{
public:
    virtual void onFrameDataNewUniqueFrame(int frameIdx, const Frame<4>& frame) = 0;
    virtual void onFrameDataNewFrame(int frameIdx, const Frame<4>& frame) = 0;
};

}
