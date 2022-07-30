#pragma once

namespace rfcommon {

class Frame;

class FrameDataListener
{
public:
    virtual void onFrameDataNewUniqueFrame(int frameIdx, const Frame& frame) = 0;
    virtual void onFrameDataNewFrame(int frameIdx, const Frame& frame) = 0;
};

}
