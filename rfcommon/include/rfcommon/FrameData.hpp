#pragma once

#include "rfcommon/config.hpp"
#include "rfcommon/RefCounted.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {

class FrameDataListener;
class FighterState;

class RFCOMMON_PUBLIC_API FrameData : public RefCounted
{
public:
    FrameData(int fighterCount);
    ~FrameData();

    static FrameData* load(FILE* fp, uint32_t size);
    uint32_t save(FILE* fp) const;

    int fighterCount() const;
    int frameCount() const;

    const FighterState& stateAt(int frameIdx, int fighterIdx) const;

    const FighterState& firstState(int fighterIdx) const;

    const FighterState& lastState(int fighterIdx) const;

    void addFrame(SmallVector<FighterState, 4>&& frame);

    ListenerDispatcher<FrameDataListener> dispatcher;

private:
    bool framesHaveSameData(int frameIdx1, int frameIdx2) const;

private:
    SmallVector<Vector<FighterState>, 2> fighters_;
};

}
