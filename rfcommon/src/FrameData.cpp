#include "rfcommon/FrameData.hpp"
#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/FighterState.hpp"
#include <cassert>

namespace rfcommon {

// ----------------------------------------------------------------------------
FrameData::FrameData(int fighterCount)
    : fighters_(fighterCount)
{}

// ----------------------------------------------------------------------------
FrameData::~FrameData()
{}

// ----------------------------------------------------------------------------
int FrameData::fighterCount() const
{
    return fighters_.count();
}

// ----------------------------------------------------------------------------
int FrameData::frameCount() const
{
    return fighters_.count() ? fighters_[0].count() : 0;
}

// ----------------------------------------------------------------------------
const FighterState& FrameData::stateAt(int frameIdx, int fighterIdx) const
{
    assert(fighterIdx < fighterCount());
    assert(frameIdx < frameCount());
    return fighters_[fighterIdx][frameIdx];
}

// ----------------------------------------------------------------------------
const FighterState& FrameData::firstState(int fighterIdx) const
{
    assert(frameCount() > 0);
    return fighters_[fighterIdx].front();
}

// ----------------------------------------------------------------------------
const FighterState& FrameData::lastState(int fighterIdx) const
{
    assert(frameCount() > 0);
    return fighters_[fighterIdx].back();
}

// ----------------------------------------------------------------------------
void FrameData::addFrame(SmallVector<FighterState, 4>&& frame)
{
    // Sanity checks
    assert(frame.count() == fighterCount());
#ifndef NDEBUG
    for (int i = 1; i < fighterCount(); ++i)
    {
        assert(frame[0].framesLeft() == frame[i].framesLeft());
        assert(frame[0].frameNumber() == frame[i].frameNumber());
    }
#endif

    for (int i = 0; i < fighterCount(); ++i)
        fighters_[i].push(frame[i]);

    // If any fighter state is different from the previous one, notify
    if (frameCount() < 2 || framesHaveSameData(frameCount() - 1, frameCount() - 2))
        dispatcher.dispatch(&FrameDataListener::onFrameDataNewUniqueFrame, frameCount() - 1, frame);

    // The UI cares about every frame
    dispatcher.dispatch(&FrameDataListener::onFrameDataNewFrame, frameCount() - 1, frame);
}

// ----------------------------------------------------------------------------
bool FrameData::framesHaveSameData(int frameIdx1, int frameIdx2) const
{
    // Compare each fighter state between each frame. If they are all equal,
    // then it means both frames compare equal
    for (int i = 0; i != fighterCount(); ++i)
        if (fighters_[i][frameIdx1].hasSameDataAs(fighters_[i][frameIdx2]) == false)
            return false;
    return true;
}

}
