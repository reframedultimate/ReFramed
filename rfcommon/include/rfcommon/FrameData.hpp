#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/Vector.hpp"
#include <cstdio>

namespace rfcommon {

class FighterState;
template <int N> class Frame;
class FrameDataListener;

class RFCOMMON_PUBLIC_API FrameData : public RefCounted
{
public:
    FrameData(SmallVector<Vector<FighterState>, 2>&& fighterFrames);
    FrameData(int fighterCount);
    ~FrameData();

    static FrameData* load(const void* data, uint64_t len);
    uint32_t save(FILE* fp) const;

    int fighterCount() const;
    int frameCount() const;

    const FighterState& stateAt(int fighterIdx, int frameIdx) const;

    const FighterState& firstState(int fighterIdx) const;

    const FighterState& lastState(int fighterIdx) const;

    void addFrame(Frame<4>&& frame);

    ListenerDispatcher<FrameDataListener> dispatcher;

private:
    bool framesHaveSameData(int frameIdx1, int frameIdx2) const;

private:
    SmallVector<Vector<FighterState>, 2> fighters_;
};

}
